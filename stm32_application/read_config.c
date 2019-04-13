//
// Created by development on 07.04.19.
//
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "read_config.h"


static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* cal_config = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0


    if (MATCH("main", "calendars")) {
        cal_config->cal_number = atoi(value);
        return 0;
    }

    char searchstr[15];
    for (int i=0;i<cal_config->cal_number;i++) {
        sprintf(searchstr, "calendar-%i", i+1);
        printf("%s - %s\n",searchstr,value);
        if (MATCH(searchstr, "link")) {
            cal_config->calendars[i].link = strdup(value);
            return 0;
        }
        if (MATCH(searchstr, "text")) {
            cal_config->calendars[i].text = strdup(value);
            return 0;
        }

    }


    if (MATCH("wifi", "pwd")) {
        cal_config->wifi_pwd = strdup(value);
        return 0;
    }

    if (MATCH("wifi", "ssid")) {
        cal_config->wifi_ssid = strdup(value);
        return 0;
    }

    return 1;

}
