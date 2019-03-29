
// STM32F103ZE ARM CortexM3 SoC

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <stm32f10x/stm32f10x.h>
#include <stm32f10x/stm32f10x_pwr.h>
#include <stm32f10x/stm32f10x_dma.h>
#include <stm32f10x/stm32f10x_sdio.h>
#include <ff.h>

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
#include <stdlib.h>
#include "painter.h"
#include "usart_dma.h"
#include "sdcard.h"
#include "jsmn.h"
#include "json_parser.h"

#define RESULT_INIT 1
#define RESULT_READY 2
#define RESULT_ERROR 3

const struct font *font_table[4] =
        {
                &font_robo_12_12,
                &font_robo_20_20,
                &font_robo_40_40,
                &font_DejaVuSerif_16_rle
        };


/*

// Interrupt routine make some advance to the
void SPI1_Handler(void)
{
	scratch[RxIdx++] = SPI_I2S_ReceiveData(SPI1);
}
*/
FATFS FatFs;   /* Work area (file system object) for logical drive */
uint8_t global_status;

int main() {
    // Init HW for the micro

    global_status=RESULT_INIT;

    initHW();

    FSMC_SRAM_Init();
    DMA_Configuration_USART();
    Serial_USART_Initialize();
    

    char *buffer = (char *) calloc((size_t ) 51200,sizeof(char));


    DP("Welcome\r\n");
    global_status=RESULT_READY;

    /* Register work area to the default drive */
/*
    if(f_mount(&FatFs, "", 1)) {
        printf("Fail to mount SD driver!\n");
        return 1;
    }

    FRESULT res;
    uint8_t path[13] = "cn.txt";
    FIL testFile;
    path[12] = '\0';
    res = f_open(&testFile, (char*)path, FA_READ);

    char my_buffer[20];
    u_int b_read;

    f_read(&testFile,my_buffer,5,&b_read);
*/

while(1) {

    InitDisplayMemory();
    
    char stat[2];
    stat[0]=(char)global_status;
    stat[1]=0;
            
    USART_Write(stat);

    uint8_t cmd = 0; //USART_ReadByteSync(USART1, 0);
    uint32_t counter = 0;

    // Wait for the magic word
    while (1) {
        while (cmd != 0x2d) {
            cmd = USART_ReadByteSync(USART1, 0);
        }
        cmd = USART_ReadByteSync(USART1, 0);
        if (cmd == 0x5a) break;
    }

    // Then read command
#define CMD_READ_CALENDAR 1

    cmd = USART_ReadByteSync(USART1, 0);

    char txt[] = "Unkown  Command\0";

    switch (cmd) {
        case CMD_READ_CALENDAR:
//              USART_Write("/calendar/v3/users/me/calendarList\0");
//            USART_Write("/calendar/v3/calendars/neuhaus.info@gmail.com/events\0");
              USART_Write("/calendar/v3/calendars/neuhaus.info@gmail.com/events?maxResults=2500&orderBy=startTime&singleEvents=true&timeMax=2019-03-31T00:00:00-01:00&timeMin=2019-03-20T00:00:00-01:00&\0");


            while (1) {
                cmd = USART_ReadByteSync(USART1, 0);
                buffer[counter++] = cmd;
                if (cmd == 0) {
                    break;
                }
            }

            jsmn_parser parser;
            jsmn_init(&parser);
            char *js = (char *) buffer;
            size_t fsize = counter;
            int r = 0;

            // Check how many tokens are needed - "r" tokens need to be allocated
            r = jsmn_parse(&parser, js, strlen(js), NULL, 0);
            jsmn_init(&parser);

            // Allocate storage for the tokens
            // after the buffer -- no clue if correct
            //jsmntok_t *tokens = *(uint16_t *) ((uint32_t)0x6800F100);
            jsmntok_t *tokens = (jsmntok_t *) calloc((size_t) r, sizeof(jsmntok_t));

            r = jsmn_parse(&parser, js, strlen(js), tokens, (u_int) r);

            char value[100];
            char searchstr[50];
            char cal_entry[200];
            int cal_entries_cnt = 0;
            int items = 1;
            struct cal_entry_type cal_entries[100];


            while (1) {

                sprintf(searchstr, "-Root-items[%i]-kind", items);

                if (search_json(js, tokens, r, searchstr, value) == 0) break;

                if (strcmp(value, "calendar#event") == 0) {

                    memset(cal_entry, 32, sizeof(cal_entry));

                    cal_entries[cal_entries_cnt].tm = malloc(sizeof(struct tm));

                    // Find Start-Date
                    sprintf(searchstr, "-Root-items[%i]-start-date", items);
                    if (search_json(js, tokens, r, searchstr, value) != 0) {
                        // Format 2019-01-07

                        GetDate(value, false, cal_entries[cal_entries_cnt].tm);

                        strftime(cal_entry, 80, "%A", cal_entries[cal_entries_cnt].tm);
                        cal_entry[strlen(cal_entry)] = ' ';

                        strftime(cal_entry + 10, 80, "%d.%m.%Y", cal_entries[cal_entries_cnt].tm);
                        cal_entry[strlen(cal_entry)] = ' ';

                    } else {
                        // Format: 2018-10-08T09:07:30.000Z
                        sprintf(searchstr, "-Root-items[%i]-start-dateTime", items);
                        if (search_json(js, tokens, r, searchstr, value) != 0) {
                            GetDate(value, true, cal_entries[cal_entries_cnt].tm);

                            strftime(cal_entry, 80, "%A", cal_entries[cal_entries_cnt].tm);
                            cal_entry[strlen(cal_entry)] = ' ';

                            strftime(cal_entry + 10, 80, "%d.%m.%Y   %H:%M", cal_entries[cal_entries_cnt].tm);
                            cal_entry[strlen(cal_entry)] = ' ';

                        } else return -1;

                    }

                    // Find Summary
                    sprintf(searchstr, "-Root-items[%i]-summary", items);
                    if (search_json(js, tokens, r, searchstr, value) != 0) {
                        sprintf(cal_entry + 28, ": %s", value);
                    } else return -1;

                    // Copy into array of events
                    cal_entries[cal_entries_cnt].summary = strdup(cal_entry);

                    cal_entries_cnt++;
                    items++;
                }

            }


            // Sort
            // qsort(cal_entries, (size_t ) cal_entries_cnt, sizeof *cal_entries, cmp_dates_descend);


            // Print the entries

            PaintText(font_table[2], 1, 1, "My First Calendar");

            for (int i = 0; i < cal_entries_cnt; i++) {
                PaintText(font_table[1], 1, (30 * i) + 60, cal_entries[i].summary);
            }

            global_status=RESULT_READY;

            break;


        default:
            PaintText(font_table[1], 700, 1, "ERROR\0");
            global_status=RESULT_ERROR;


    }


    einkd_init(1);
    einkd_PowerOn();     // Power ON, draw and OFF again!
    einkd_refresh_from_sram(); //    InitDisplayMemory();

    einkd_PowerOff();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, DISABLE);
    einkd_deinit();

    DPL("Done my  job /r/n");
    DP("/r/n");


}

    // Turn ourselves OFF, hopefully save some power before final power gate off
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    return 0;
}


