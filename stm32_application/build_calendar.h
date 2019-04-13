//
// Created by development on 31.03.19.
//

#ifndef STM32_DISPLAY_BUILD_CALENDAR_H
#define STM32_DISPLAY_BUILD_CALENDAR_H

#include <stdint-gcc.h>


extern const char *const WEEKDAY[];
extern const char *const MONTH[];


struct cal_entry_type {
    struct tm *tm;
    char *c0_today;
    char *c1_weekday;
    char *c2_day;
    char *c3_month;
    char *c4_summary;
    char *c5_time;
};


void BuildCalenarRequest(char *calendar_request, const char *calender_link, char *str_time_min, char *str_time_max);

bool BuildCalendar(char *buffer, uint32_t counter, struct cal_entry_type *cal_entries, int *cal_cnt, struct tm *CurrentDate,
              char *summary_format);

#endif //STM32_DISPLAY_BUILD_CALENDAR_H
