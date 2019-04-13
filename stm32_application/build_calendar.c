//
// Created by development on 31.03.19.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <string.h>
#include <integer.h>
#include "time.h"
#include "jsmn.h"
#include "json_parser.h"
#include "build_calendar.h"
#include "iso_iconv.h"

void BuildCalenarRequest(char *calendar_request, const char *calender_link, char *str_time_min, char *str_time_max) {

    // "/calendar/v3/calendars/neuhaus.info@gmail.com/events?maxResults=2500&orderBy=startTime&singleEvents=true&timeMax=2019-03-31T00:00:00-01:00&timeMin=2019-03-20T00:00:00-01:00&\0");
    sprintf(calendar_request,"/calendar/v3/calendars/%s/events?maxResults=2500&orderBy=startTime&singleEvents=true&timeMax=%sT00:00:00-01:00&timeMin=%sT00:00:00-01:00&",calender_link,str_time_max,str_time_min);
}






bool BuildCalendar(char *buffer, uint32_t counter, struct cal_entry_type *cal_entries, int *cal_cnt, struct tm *CurrentDate,
              char *summary_format) {
    jsmn_parser parser;
    jsmn_init(&parser);
    char *js = (char *) buffer;
    size_t fsize = counter;
    int r = 0;

    // Check how many tokens are needed - "r" tokens need to be allocated
    r = jsmn_parse(&parser, js, strlen(js), NULL, 0);
    jsmn_init(&parser);

    // Allocate storage for the tokens
    jsmntok_t *tokens = (jsmntok_t *) calloc((size_t) r, sizeof(jsmntok_t));
    r = jsmn_parse(&parser, js, strlen(js), tokens, (u_int16_t) r);

    char value[100];
    char value2[100];
    char searchstr[50];
    char cal_entry[200];
    int items = 1;

    while (1) {

        sprintf(searchstr, "-Root-items[%i]-kind", items);

        if (search_json(js, tokens, r, searchstr, value) == 0) break;

        if (strcmp(value, "calendar#event") == 0) {

            memset(cal_entry, 32, sizeof(cal_entry));

            cal_entries[*cal_cnt].tm = calloc(sizeof(struct tm),1);

            bool b_found_time=false;

            // Find Start-Date or Start-Date and Time
            sprintf(searchstr, "-Root-items[%i]-start-date", items);
            if (search_json(js, tokens, r, searchstr, value) != 0) {
                b_found_time=false;
                // Format 2019-01-07
            } else {
                // Format: 2018-10-08T09:07:30.000Z
                sprintf(searchstr, "-Root-items[%i]-start-dateTime", items);
                if (search_json(js, tokens, r, searchstr, value) != 0) {
                    b_found_time=true;
                }
            }

            GetDate(value, b_found_time, cal_entries[*cal_cnt].tm);

            // Check for today or tomorrow
            if ((CurrentDate->tm_mday==cal_entries[*cal_cnt].tm->tm_mday) && (CurrentDate->tm_mon==cal_entries[*cal_cnt].tm->tm_mon) ) {
                cal_entries[*cal_cnt].c0_today=strdup("Heute");

            } else if ((CurrentDate->tm_mday+1==cal_entries[*cal_cnt].tm->tm_mday) && (CurrentDate->tm_mon==cal_entries[*cal_cnt].tm->tm_mon) ) {
                cal_entries[*cal_cnt].c0_today=strdup("Morgen");
            } else {

                cal_entries[*cal_cnt].c1_weekday=strdup(WEEKDAY[cal_entries[*cal_cnt].tm->tm_wday-1]);

                //Day: 1.
                cal_entries[*cal_cnt].c2_day=calloc(sizeof(char),4);
                sprintf(cal_entries[*cal_cnt].c2_day, "%i.",cal_entries[*cal_cnt].tm->tm_mday );

                //Month: Feb.
                cal_entries[*cal_cnt].c3_month=strdup(MONTH[cal_entries[*cal_cnt].tm->tm_mon]);

                //Zeit:
                if (b_found_time) {
                    cal_entries[*cal_cnt].c5_time=calloc(sizeof(char),6);
                    strftime(cal_entries[*cal_cnt].c5_time,6, "%H:%M", cal_entries[*cal_cnt].tm);
                }

            }

            // Find Summary
            sprintf(searchstr, "-Root-items[%i]-summary", items);

            if (search_json(js, tokens, r, searchstr, value) != 0) {

                utf8_to_latin9(value2, value,strlen(value)+1);
                size_t needed = snprintf(NULL, 0, summary_format, value2);
                cal_entries[*cal_cnt].c4_summary= malloc(needed+1);
                sprintf(cal_entries[*cal_cnt].c4_summary, summary_format, value2);
            } else return true;


            *cal_cnt=*cal_cnt+1;
            items++;
        }

    }


    // Sort
    qsort(cal_entries, (size_t ) *cal_cnt, sizeof *cal_entries, cmp_dates_descend);

    free(tokens);
    return false;

}