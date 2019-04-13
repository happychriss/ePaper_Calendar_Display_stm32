
// STM32F103ZE ARM CortexM3 SoC
// mem 0x68000000 0x680FFFFF rw

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include <stm32f10x/stm32f10x_dma.h>
#include <stm32f10x/stm32f10x_sdio.h>
#include <ff.h>

#include "global.h"
#include "gde043a2.h"
#include "sram.h"
#include "uart.h"
#include "imgdec.h"
#include "time.h"

#include "usart.h"
#include "support_functions.h"
#include "init_stm32.h"

//#include "fonts/font-robo_8-8.h"
//#include "fonts/font-robo_bold_8-8.h"
#include "fonts/font-robo_12-12.h"
//#include "fonts/font-robo_bold_12-12.h"
#include "fonts/font-robo_20-20.h"
#include "fonts/font-DejaVuSerif-16-rle.h"
#include "fonts/font-robo_40-40.h"

#include "painter.h"
#include "usart_dma.h"
#include "sdcard.h"
#include "build_calendar.h"
#include "ini.h"
#include "read_config.h"
#include "read_config.c"



// Gloal Status
#define GLOBAL_INIT 1
#define GLOBAL_WAIT_FOR_ESP_COMMANDS 2
#define GLOBAL_ERROR 3
#define GLOBAL_SHUTDOWN 4

#define CALENDAR_READY  1
#define CALANDAR_MORE  2
#define CALENDAR_DONE  3


// Commands from the ESP
#define ESP_CMD_READ_CALENDAR 1
#define ESP_CMD_SHUTDOWN 2

#define CAL_LINE_FONT 1
#define CAL_LINE_FSIZE 20
#define CAL_LINE_HEIGHT 40
#define CAL_HEADER_OFFSET 70

#define CAL_DISPLAY_ENTRIES 12
#define CAL_DISPLAY_DAYS 14

//************** Global VAriables
const struct font *font_table[4] =
        {
                &font_robo_12_12,
                &font_robo_20_20,
                &font_robo_40_40,
                &font_DejaVuSerif_16_rle
        };

const char *const WEEKDAY[] = {"Mo.", "Di.", "Mi.", "Do.", "Fr.", "Sa.", "So."};
const char *const MONTH[] = {"Jan.", "Feb.", "März", "Apr.", "Mai", "Jun.", "Jul.", "Aug.", "Sept.", "Okt.", "Nov.",
                             "Dez."};

FATFS FatFs;   /* Work area (file system object) for logical drive */
uint8_t global_status;
char  global_error[100];
t_grafic_buffer_line *grafic_buffer_lines;
uint8_t *ptr_grafic_buffer;
struct tm global_time;

void UpdateDisplay() {
    //** Write to eInk Display from sram ******************************
    einkd_init(1);
    einkd_PowerOn();     // Power ON, draw and OFF again!
    einkd_paint_grafic_buffer(grafic_buffer_lines); //    ClearDisplay();
    einkd_PowerOff();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
    einkd_deinit();
}


int main() {

    // ******************************** INIT *****************************************************************

    global_status = GLOBAL_INIT;

    // Init System and Hardware
    initHW();
    FSMC_SRAM_Init();
    DMA_Configuration_USART();
    Serial_USART_Initialize();

    // Allocate Memory from SRAM (calloc over SRAM, linker.ld
    //display is 800x600
    //800 pixel = 100 bytes * 2 for grey colors = 200
    char *json_buffer = (char *) calloc((size_t) 51200, sizeof(char));  //json_buffer to read from esp

    char *tmp_text = (char *) calloc((size_t) 100, sizeof(char));  //json_buffer to read from esp
    char *tmp_text2 = (char *) calloc((size_t) 100, sizeof(char));  //json_buffer to read from esp

    char *calendar_request = (char *) calloc((size_t) 500, sizeof(char));  //json_buffer to read from esp

    struct cal_entry_type *cal_entries = (struct cal_entry_type *) calloc(100, (sizeof(struct cal_entry_type)));
    grafic_buffer_lines = calloc(600, sizeof(t_grafic_buffer_line));
    ptr_grafic_buffer = (uint8_t *) grafic_buffer_lines;
    configuration config = {0};


    // Mount SD Card and Read configuration
    if (f_mount(&FatFs, "", 1)) {
        strcpy(global_error,"SD Card Mount Error\0");
        global_status = GLOBAL_ERROR;
    }

    if (ini_parse("cal.ini", handler, &config) < 0) {
        strcpy(global_error,"Ini File Read Error\0");
        global_status = GLOBAL_ERROR;
    }

    //** Init eInk Display and update with start information*
    ClearDisplay();

    PaintText(font_table[1], 1, 1, "*** Init ***\0");
    sprintf(tmp_text, "Wifi SSID: %s", config.wifi_ssid);
    PaintText(font_table[1], 1, 30, tmp_text);
    sprintf(tmp_text, "Calendars available: %i", config.cal_number);
    PaintText(font_table[1], 1, 60, tmp_text);

    UpdateDisplay();

    global_status = GLOBAL_WAIT_FOR_ESP_COMMANDS;


    // ******************************** LOOP  *****************************************************************

    while (1) {

        switch (global_status) {

            case GLOBAL_WAIT_FOR_ESP_COMMANDS:

                // Return Status to ESP ************************************************
                ;

                USART_WriteStatus(CALENDAR_READY);

                // Update with Wifi Information
                USART_Write(config.wifi_ssid);
                delay_ms(500);
                USART_Write(config.wifi_pwd);

                // Wait for Instruction from ESP
                uint8_t cmd = 0; //USART_ReadByteSync(USART1, 0);

                // Wait for the magic word
                while (1) {
                    while (cmd != 0x2d) {
                        cmd = USART_ReadByteSync(USART1, 0);
                    }
                    cmd = USART_ReadByteSync(USART1, 0);
                    if (cmd == 0x5a) break;
                }
                // Then read command
                cmd = USART_ReadByteSync(USART1, 0);

                char str_esp_time[22]={0};

                switch (cmd) {

                    case ESP_CMD_READ_CALENDAR:

                        // read the current time from ESP
                        USART_ReadString(str_esp_time);
                        strptime(str_esp_time,"%Y %m %d %H:%M:%S",&global_time);

                        // Prepare time-min and time-max for calendar
                        struct tm my_time;
                        memcpy(&my_time,&global_time, sizeof(struct tm));
                        char time_min[11];
                        char time_max[11];
                        strftime(time_min,sizeof(time_min),"%Y-%m-%d",&my_time);
                        my_time.tm_mday=my_time.tm_mday+CAL_DISPLAY_DAYS;
                        mktime(&my_time);
                        strftime(time_max,sizeof(time_min),"%Y-%m-%d",&my_time);

                        int cal_entries_cnt = 0;

                        // ask ESP to read calendars from Google and build calendar
                        for (int cal=0;cal<config.cal_number;cal++){

                            //Ask ESP to return calendar information
                            BuildCalenarRequest(calendar_request, config.calendars[cal].link, time_min, time_max);

                            USART_WriteStatus(CALENDAR_READY);
                            USART_Write(calendar_request);

                            uint32_t buffer_counter;
                            buffer_counter=USART_ReadString(json_buffer);
                            BuildCalendar(json_buffer, buffer_counter, cal_entries, &cal_entries_cnt, &global_time, (char *) config.calendars[cal].text);
                            
                            
                            // Tell ESP we want to send more requests
                            if (cal<(config.cal_number-1) && (cal_entries_cnt<=CAL_DISPLAY_ENTRIES)) {
                                USART_WriteStatus(CALANDAR_MORE);
                            } else {
                                USART_WriteStatus(CALENDAR_DONE);
                            }

                            if (cal_entries_cnt>CAL_DISPLAY_ENTRIES) { break;}
                            
                        }


                        ClearDisplay();

                        strftime(tmp_text,15,"%d.%m.%Y",&global_time);
                        sprintf(tmp_text2,"Kalendar %s",tmp_text);

                        PaintText(font_table[2], 1, 1, tmp_text2);
        
                        for (int i = 0; i < cal_entries_cnt; i++) {
                            if (cal_entries[i].c0_today != 0) {
                                PaintText(font_table[CAL_LINE_FONT], 1, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c0_today);
                            } else {
                                PaintText(font_table[CAL_LINE_FONT], 1, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c1_weekday);
                                PaintText(font_table[CAL_LINE_FONT], CAL_LINE_FSIZE * 3, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c2_day);
                                PaintText(font_table[CAL_LINE_FONT], CAL_LINE_FSIZE * 6, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c3_month);
                            }

                            PaintText(font_table[CAL_LINE_FONT], CAL_LINE_FSIZE * 10, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c4_summary);
                            if (cal_entries[i].c5_time != 0) {
                                PaintText(font_table[CAL_LINE_FONT], 800 - 5 * CAL_LINE_FSIZE, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c5_time);
                            }
                        }

                        UpdateDisplay();

                        global_status = GLOBAL_WAIT_FOR_ESP_COMMANDS;
                        break;

                    case ESP_CMD_SHUTDOWN:
                        global_status = GLOBAL_SHUTDOWN;
                        break;

                    default:
                        sprintf(global_error,"Unknown ESP Command: %i",cmd);
                        global_status = GLOBAL_ERROR;
                }

                break;

            case GLOBAL_SHUTDOWN:
                // Turn ourselves OFF, hopefully save some power before final power gate off
                PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
                break;


            case GLOBAL_ERROR:
                ClearDisplay();
                PaintText(font_table[1], 1, 1, "ERROR\0");
                PaintText(font_table[1], 1, 30, global_error);
                UpdateDisplay();
                global_status = GLOBAL_SHUTDOWN;
                break;

            default:
                sprintf(global_error,"Unknown Global Status: %i",global_status);
                global_status = GLOBAL_ERROR;

        }
        break;
    }

    return 0;
}


