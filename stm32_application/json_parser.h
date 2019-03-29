//
// Created by development on 23.03.19.
//

#ifndef STM32_DISPLAY_JSON_PARSER_H
#define STM32_DISPLAY_JSON_PARSER_H
struct cal_entry_type {
    struct tm *tm;
    char *summary;
};

void GetDate(char *value,bool with_time, struct tm *my_time);
int  search_json(char *js, jsmntok_t *tokens, int end_line, char *search_text, char *value);
int cmp_dates_descend(const void *d1, const void *d2);
int dayofweek(int d, int m, int y);
#endif //STM32_DISPLAY_JSON_PARSER_H
