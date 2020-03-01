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

void BuildCalendarRequest(char *calendar_request, const char *calender_link, char *str_time_min, char *str_time_max) {

    // "/calendar/v3/calendars/neuhaus.info@gmail.com/events?maxResults=2500&orderBy=startTime&singleEvents=true&timeMax=2019-03-31T00:00:00-01:00&timeMin=2019-03-20T00:00:00-01:00&\0");
    sprintf(calendar_request,"/calendar/v3/calendars/%s/events?maxResults=%i&orderBy=startTime&singleEvents=true&timeMax=%sT00:00:00-01:00&timeMin=%sT00:00:00-01:00&",calender_link,MAX_CAL_DISPLAY_ENTRIES,str_time_max,str_time_min);
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

    char value[MAX_CHAR_JSON_LINE];
    char value2[MAX_CHAR_JSON_LINE+10];//UTF8 conversion
    char searchstr[50];
//    char cal_entry[200];
    int items = 1;

    while (1) {

        sprintf(searchstr, "-Root-items[%i]-kind", items);

        if (search_json(js, tokens, r, searchstr, value) == 0) break;

        if (strcmp(value, "calendar#event") == 0) {

  //          memset(cal_entry, 32, sizeof(cal_entry));

            // ***************+ Read Start Time of the Event

            cal_entries[*cal_cnt].start_tm = calloc(sizeof(struct tm),1);
            cal_entries[*cal_cnt].end_tm = calloc(sizeof(struct tm),1);

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

            GetDate(value, b_found_time, cal_entries[*cal_cnt].start_tm);

            // ***************+ Read End  Time of the Event
            cal_entries[*cal_cnt].end_tm = calloc(sizeof(struct tm),1);

            b_found_time=false;

            // Find Start-Date or Start-Date and Time
            sprintf(searchstr, "-Root-items[%i]-end-date", items);
            if (search_json(js, tokens, r, searchstr, value) != 0) {
                b_found_time=false;
                // Format 2019-01-07
            } else {
                // Format: 2018-10-08T09:07:30.000Z
                sprintf(searchstr, "-Root-items[%i]-end-dateTime", items);
                if (search_json(js, tokens, r, searchstr, value) != 0) {
                    b_found_time=true;
                }
            }

            GetDate(value, b_found_time, cal_entries[*cal_cnt].end_tm);
            
            // For all day events google returns end-date +1, we have to subtract 1 day

            if (!b_found_time) {
                cal_entries[*cal_cnt].end_tm->tm_mday = cal_entries[*cal_cnt].end_tm->tm_mday - 1;
                mktime(cal_entries[*cal_cnt].end_tm);
            }

            // to  look nice
            struct cal_entry_type *ce;
            ce=&cal_entries[*cal_cnt];


            time_t t_current, t_start, t_end;

            t_current=mktime(CurrentDate);
            t_start=mktime(ce->start_tm);
            t_end=mktime(ce->end_tm);

            // Check for today or tomorrow, events with duration are displayed also when starting in the past and still valid
            if ((CurrentDate->tm_mday==ce->start_tm->tm_mday) && (CurrentDate->tm_mon==ce->start_tm->tm_mon)) {
                ce->c0_today=strdup("Heute");

            } else if ((CurrentDate->tm_mday+1==ce->start_tm->tm_mday) && (CurrentDate->tm_mon==ce->start_tm->tm_mon) ) {
                ce->c0_today=strdup("Morgen");
            } else {

                ce->c1_weekday=strdup(WEEKDAY[ce->start_tm->tm_wday]);

                //Day: 1.
                ce->c2_day=calloc(sizeof(char),4);
                sprintf(ce->c2_day, "%i.",ce->start_tm->tm_mday );

                //Month: Feb.
                ce->c3_month=strdup(MONTH[ce->start_tm->tm_mon]);

                //Zeit:
                if (b_found_time) {
                    ce->c5_time=calloc(sizeof(char),6);
                    strftime(ce->c5_time,6, "%H:%M", ce->start_tm);
                }

            }

            // Find Summary
            sprintf(searchstr, "-Root-items[%i]-summary", items);

            if (search_json(js, tokens, r, searchstr, value) != 0) {

                size_t length;
                length=strlen(value);
                utf8_to_latin9(value2, value,length+1);


                char str_end_date[30]={0};

                if (difftime(t_end, t_start)>86400 ) {
                    sprintf(str_end_date, " bis %i. %s",ce->end_tm->tm_mday,MONTH[ce->start_tm->tm_mon] );
                    strcat(value2,str_end_date);
                }

                //limit to 35 char
                value2[MAX_CHAR_CALENDAR_ENTRY]=0; //
                size_t needed = snprintf(NULL, 0, summary_format, value2);
                ce->c4_summary= malloc(needed+1);
                snprintf(ce->c4_summary,MAX_CHAR_CALENDAR_ENTRY, summary_format, value2);
            } else return true;


            *cal_cnt=*cal_cnt+1;

            if (*cal_cnt==MAX_CAL_ENTRIES) break;

            items++;
        }

    }


    // Sort
    qsort(cal_entries, (size_t ) *cal_cnt, sizeof *cal_entries, cmp_dates_descend);

    free(tokens);
    return false;

}