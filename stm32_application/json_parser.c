//
// Created by development on 23.03.19.
//

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "jsmn.h"
#include "json_parser.h"

#include "string.h"
#include "time.h"
#include "build_calendar.h"


int cmp_dates_descend(const void *d1, const void *d2) {

    struct cal_entry_type date_1 = *(const struct cal_entry_type *) d1;
    struct cal_entry_type date_2 = *(const struct cal_entry_type *) d2;

    double d = difftime(mktime(date_1.start_tm), mktime(date_2.start_tm));

    return (d > 0) - (d < 0);

}

int dayofweek(int d, int m, int y) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    y -= m < 3;
    return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

void GetDate(char *value, bool with_time, struct tm *my_time) {

    // no time format:   2019-01-07
    // with time format: Format: 2019-01-21T09:00:00+01:00

    memset(my_time, 0, sizeof(struct tm));

    if (with_time) {
        strptime(value, "%Y-%m-%dT%H:%M:%S+%z", my_time);
    } else {
        strptime(value, "%Y-%m-%dT", my_time);
    }

    my_time->tm_wday = dayofweek(my_time->tm_mday, my_time->tm_mon + 1, my_time->tm_year + 1990);

}

int search_json(char *js, jsmntok_t *tokens, int end_line, char *search_text, char *value) {

#define MAX_LENGTH_OF_KEY 50
#define MAX_STACK_SIZE 8
#define PRINT_JSON_TREE 0


// #define DBT(fmt, ...)  do { if (PRINT_JSON_TREE) fprintf(stdout, fmt, __VA_ARGS__); } while (0)
#define DBT(fmt, ...)

    int st_count = 0;


    jsmntok_t *t;

    struct st_type {
        jsmntok_t s;
        char key[MAX_LENGTH_OF_KEY];
        int array_count;
    };

    struct st_type st[MAX_STACK_SIZE];

    memset(&st[0], 0, sizeof(struct st_type));
    strcpy(st[0].key, "Root\0");
    st[0].s.size = tokens->size;

    char token_text[100];


    for (int i = 0; i < end_line; i++) {

        t = &tokens[i];

        // key found in token ************************
        if ((t->type == JSMN_STRING) && (t->size == 1)) {
            memset(st[st_count].key, 0, MAX_LENGTH_OF_KEY);
            memcpy(st[st_count].key, js + t->start, (size_t) (t->end - t->start));
        }

            // value found in token ************************
        else {

            st[st_count].s.size--;

            // counting array to have it available  on output
            if (st[st_count].s.type == JSMN_ARRAY) {
                st[st_count].array_count++;
            }

            // if token is a printable value - print the value and entire stack
            if (((t->type == JSMN_STRING) && (t->size == 0)) || (t->type == JSMN_PRIMITIVE)) {

                DBT("%i-Stack[%i] Size[%i]:", i, st_count, st[st_count].s.size);


                // build the token-string, to be compared with the search-string
                int len = 0;

                for (int k = 0; k <= st_count; k++) {
                    if (st[k].s.type == JSMN_ARRAY) {
                        len += sprintf(token_text + len, "%s[%i]", st[k].key, st[k].array_count);
                    } else {
                        len += sprintf(token_text + len, "-%s", st[k].key);
                    }
                }

                DBT("%s", token_text);
                DBT(" = '%.*s'\n", t->end - t->start, js + t->start);

                if (strcmp(search_text, token_text) == 0) {
                    snprintf(value, MAX_CHAR_JSON_LINE, "%.*s", t->end - t->start, js + t->start);
                    return i;
                }

            }

            // if token is a object or array - add to stack
            if ((t->type == JSMN_ARRAY) || (t->type == JSMN_OBJECT)) {
                st_count++;
                memset(&st[st_count], 0, sizeof(struct st_type));
                st[st_count].s = *t;

            }

            // clean the stack, if multiple objects are done
            while (st[st_count].s.size == 0 && st_count > 0) {
                st_count--;
            }

        }

    }

    return 0;
}

