
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
#include <stm32f10x/stm32f10x_flash.h>

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


#include "fonts/font-robo_8-8.h"
//#include "fonts/font-robo_bold_8-8.h"
#include "fonts/font-robo_12-12.h"
//#include "fonts/font-robo_bold_12-12.h"
#include "fonts/font-robo_20-20.h"
//#include "fonts/font-DejaVuSerif-16-rle.h"
#include "fonts/font-robo_40-40.h"
//#include "fonts/font-robo_10_rle-10.h"

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
#define ESP_CMD_SEND_WIFI_INFO 5
#define ESP_AWAKE_TEST 6
#define ESP_WRITE_REFRESH_TOKEN 7
#define ESP_READ_REFRESH_TOKEN 7




#define CALENDAR_READY  1
#define CALENDAR_MORE  2
#define CALENDAR_DONE  3


// Commands from the ESP
#define ESP_CMD_READ_CALENDAR 1
#define ESP_CMD_SHUTDOWN 2
#define ESP_CMD_SHOW_USERCODE 3
#define ESP_CMD_SHOW_ERRORMSG 4
#define ESP_CMD_SEND_WIFI 5


#define CAL_LINE_FONT 1
#define CAL_LINE_FSIZE 20
#define CAL_LINE_HEIGHT 42
#define CAL_HEADER_OFFSET 95
#define CAL_LEFT_OFFSET 10


#define CAL_DISPLAY_DAYS 14
#ifdef MYDEBUG
char mbuf[DB_BUFFER] = {0};
#endif

//************** Global VAriables
const struct font *font_table[3] =
        {
                &font_robo_12_12,
                &font_robo_20_20,
                &font_robo_40_40,
        };

const char *const WEEKDAY[] = {"So.", "Mo.", "Di.", "Mi.", "Do.", "Fr.", "Sa."};
const char *const WEEKDAY_LONG[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

const char *const MONTH[] = {"Jan.", "Feb.", "März", "Apr.", "Mai", "Jun.", "Jul.", "Aug.", "Sept.", "Okt.", "Nov.",
                             "Dez."};

FATFS FatFs;   /* Work area (file system object) for logical drive */
uint8_t global_status;
char global_error[100];
t_grafic_buffer_line *grafic_buffer_lines;
uint8_t *ptr_grafic_buffer;
struct tm global_time;
struct struct_esp_data {
    char refresh_token[10];
} ESPData;



// not working..seems not to be enough memory available
void ReadRecord(struct struct_esp_data *pSD, uint32_t address)
{
    uint32_t *ptr = (uint32_t* )pSD;
    uint32_t flash_address = address;

    for (int i = 0; i < sizeof(struct struct_esp_data); i+=4, ptr++, flash_address+=4 )
        *ptr = *(__IO uint32_t*) flash_address;
}

void WriteRecord(struct struct_esp_data *pSD, uint32_t address)
{
    int i;
    uint32_t *pRecord = (uint32_t* )pSD;
    uint32_t flash_address = address;
    FLASH_UnlockBank1();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Status FLASHStatus = FLASH_ErasePage(255);
    for(i=0; i<sizeof(struct struct_esp_data); i+=4, pRecord++, flash_address+=4)
        FLASH_ProgramWord(flash_address, *pRecord);
    FLASH_LockBank1();
}


UINT WriteESPData(struct struct_esp_data *pSD) {
    FIL file;
    UINT bw;
    FRESULT res1, res2;
    res1=f_open(&file, "esp.txt", FA_WRITE  | FA_CREATE_ALWAYS );
    res2= f_write(&file, pSD->refresh_token, sizeof(pSD->refresh_token), &bw);
    f_sync(&file);
    f_close(&file);
    return res2+res1;
}

UINT ReadESPData(struct struct_esp_data *pSD) {
    FIL file;
    UINT bw;
    FRESULT res1, res2;
    res1=f_open(&file, "esp.txt", FA_READ );
    res2= f_read(&file, pSD, sizeof(pSD->refresh_token), &bw);
    f_close(&file);
    return bw;
}

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

    global_status = GLOBAL_WAIT_FOR_ESP_COMMANDS;

    // Init System and Hardware
    initHW();
    FSMC_SRAM_Init();
    DMA_Configuration_USART();
    Serial_USART_Initialize();

    // Allocate Memory from SRAM (calloc over SRAM, linker.ld
    //display is 800x600
    //800 pixel = 100 bytes * 2 for grey colors = 200
    char *json_buffer = (char *) calloc((size_t) MAX_JSON_BUFFER, sizeof(char));  //json_buffer to read from esp

    char *tmp_text = (char *) calloc((size_t) 100, sizeof(char));  //json_buffer to read from esp
    char *tmp_text2 = (char *) calloc((size_t) 100, sizeof(char));  //json_buffer to read from esp
    char *tmp_text3 = (char *) calloc((size_t) 100, sizeof(char));  //json_buffer to read from esp

    char *calendar_request = (char *) calloc((size_t) 500, sizeof(char));  //json_buffer to read from esp

    struct cal_entry_type *cal_entries = (struct cal_entry_type *) calloc(MAX_CAL_ENTRIES,
                                                                          (sizeof(struct cal_entry_type)));
    grafic_buffer_lines = calloc(600, sizeof(t_grafic_buffer_line));
    ptr_grafic_buffer = (uint8_t *) grafic_buffer_lines;
    configuration config = {0};


    // Mount SD Card and Read configuration
    if (f_mount(&FatFs, "", 1)) {
        strcpy(global_error, "SD Card Mount Error\0");
        global_status = GLOBAL_ERROR;
    }

    if (ini_parse("cal.ini", handler, &config) < 0) {
        strcpy(global_error, "Ini File Read Error\0");
        global_status = GLOBAL_ERROR;
    }

/*
    strcpy(ESPData.refresh_token, "Hello\0");
    UINT bw=WriteESPData(&ESPData);
    struct struct_esp_data my_data;
    UINT br=ReadESPData(&my_data);
*/


    // ******************************** LOOP  *****************************************************************

    while (true) {

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

        char str_esp_time[22] = {0};
        char device_code[20] = {0};
        char error_msg[200] = {0};

        switch (cmd) {

            case ESP_AWAKE_TEST:
                // read the current time from ESP

                USART_WriteStatus(CALENDAR_READY);
                ClearDisplay();
                PaintText(font_table[1], 1, 1, "*** Welcome ***\0");
                UpdateDisplay();
                break;

            case ESP_CMD_SEND_WIFI_INFO:
                ClearDisplay();

                PaintText(font_table[1], 1, 1, "*** Init ***\0");
                sprintf(tmp_text, "Wifi SSID: %s", config.wifi_ssid);
                PaintText(font_table[1], 1, 30, tmp_text);
                sprintf(tmp_text, "Calendars available: %i", config.cal_number);
                PaintText(font_table[1], 1, 60, tmp_text);

                UpdateDisplay();

                // Update with Wifi Information
                USART_Write(config.wifi_ssid);
                delay_ms(300);
                USART_Write(config.wifi_pwd);

                break;

            case ESP_CMD_SHOW_USERCODE:

                USART_ReadString(device_code, sizeof(device_code));

                //** Init eInk Display and update with start information*
                ClearDisplay();

                PaintText(font_table[2], 1, 1, "** Google OAuthRegistration **\0");
                sprintf(tmp_text, "Code: %s", device_code);
                PaintText(font_table[2], 1, 100, tmp_text);
                PaintText(font_table[1], 1, 250, "Register your device with Google OAuth.\0");
                PaintText(font_table[1], 1, 290, "Check: https://www.google.com/device\0");

                UpdateDisplay();
                delay_ms(1000);

                break;

            case ESP_CMD_SHOW_ERRORMSG:
                DB(90);
                USART_ReadString(error_msg, sizeof(error_msg));

                //** Init eInk Display and update with start information*
                ClearDisplay();

                PaintText(font_table[2], 1, 1, "** ERROR **\0");
                PaintText(font_table[1], 1, 100, error_msg);

                UpdateDisplay();
                delay_ms(1000);

                break;


            case ESP_CMD_READ_CALENDAR:
                DB(1);
                USART_ReadString(str_esp_time, sizeof(str_esp_time));
                strptime(str_esp_time, "%Y %m %d %H:%M:%S", &global_time);
                // Prepare time-min and time-max for calendar
                struct tm my_time;
                memcpy(&my_time, &global_time, sizeof(struct tm));
                char time_min[11];
                char time_max[11];
                strftime(time_min, sizeof(time_min), "%Y-%m-%d", &my_time);
                my_time.tm_mday = my_time.tm_mday + CAL_DISPLAY_DAYS;
                mktime(&my_time);
                strftime(time_max, sizeof(time_min), "%Y-%m-%d", &my_time);

                int cal_entries_cnt = 0;

                // ask ESP to read calendars from Google and build calendar
                for (int cal = 0; cal < config.cal_number; cal++) {
//                for (int cal = 0; cal < 2; cal++) {
                    //Ask ESP to return calendar information
                    BuildCalendarRequest(calendar_request, config.calendars[cal].link, time_min, time_max);

                    USART_WriteStatus(CALENDAR_MORE);
                    USART_Write(calendar_request);

                    uint32_t buffer_counter;
                    buffer_counter = USART_ReadString(json_buffer, MAX_JSON_BUFFER);

                    BuildCalendar(json_buffer, buffer_counter, cal_entries, &cal_entries_cnt, &global_time,
                                  (char *) config.calendars[cal].text);

                    // Tell ESP we want to send more requests
                    if (cal_entries_cnt > MAX_CAL_DISPLAY_ENTRIES) {
                        break;
                    }

                }

                if (global_status == GLOBAL_ERROR) break;

                ClearDisplay();

                strftime(tmp_text, 15, "%d.%m.%Y", &global_time);
                sprintf(tmp_text2, "%s, %s", WEEKDAY_LONG[global_time.tm_wday], tmp_text);
                PaintText(font_table[2], CAL_LEFT_OFFSET, 5, tmp_text2);

                for (int i = 0; ((i < cal_entries_cnt) && (i < MAX_CAL_DISPLAY_ENTRIES)); i++) {
                    if (cal_entries[i].c0_today != 0) {
                        PaintText(font_table[CAL_LINE_FONT], CAL_LEFT_OFFSET, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET,
                                  cal_entries[i].c0_today);
                    } else {
                        PaintText(font_table[CAL_LINE_FONT], CAL_LEFT_OFFSET, (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET,
                                  cal_entries[i].c1_weekday);
                        PaintText(font_table[CAL_LINE_FONT], CAL_LEFT_OFFSET + (CAL_LINE_FSIZE * 3),
                                  (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c2_day);
                        PaintText(font_table[CAL_LINE_FONT], CAL_LEFT_OFFSET + (CAL_LINE_FSIZE * 6),
                                  (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c3_month);
                    }

                    PaintText(font_table[CAL_LINE_FONT], CAL_LEFT_OFFSET + (CAL_LINE_FSIZE * 10),
                              (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c4_summary);
                    if (cal_entries[i].c5_time != 0) {
                        PaintText(font_table[CAL_LINE_FONT], 795 - 5 * CAL_LINE_FSIZE,
                                  (CAL_LINE_HEIGHT * i) + CAL_HEADER_OFFSET, cal_entries[i].c5_time);
                    }
                }

                strftime(tmp_text, 19, "%d.%m.%Y %H:%M", &global_time);
        #ifdef MYDEBUG
                sprintf(tmp_text2, "V2-1 %s D:%s ", tmp_text, mbuf);
                PaintText(font_table[0], 1, CANVAS_Y - 20, tmp_text2);
        #else
                PaintText(font_table[0], 1, CANVAS_Y - 20, tmp_text);
        #endif
                UpdateDisplay();
                delay_ms(500);

                USART_WriteStatus(CALENDAR_READY);
                break;

            case ESP_CMD_SHUTDOWN:
                DB(80);
                global_status = GLOBAL_SHUTDOWN;
                break;

            default:
                DB(91);
                sprintf(global_error, "Unknown ESP Command:  %i", cmd);
                global_status = GLOBAL_ERROR;
                break;
        }


    if (global_status == GLOBAL_ERROR) {
        ClearDisplay();
        PaintText(font_table[1], 1, 1, "Display ERROR\0");
        PaintText(font_table[1], 1, 30, global_error);
        UpdateDisplay();
        global_status = GLOBAL_SHUTDOWN;
    }

    if (global_status == GLOBAL_SHUTDOWN) {
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
        while (1) {}
    }

}


// Turn ourselves OFF, hopefully save some power before final power gate off

return 0;
}


