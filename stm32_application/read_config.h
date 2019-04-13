//
// Created by development on 07.04.19.
//

#ifndef STM32_DISPLAY_READ_CONFIG_H
#define STM32_DISPLAY_READ_CONFIG_H



typedef struct
{
    const char *text;
    const char *link;
} t_calendars;


typedef struct
{
    int cal_number;
    const char *wifi_pwd;
    const char *wifi_ssid;
    t_calendars calendars[10];

} configuration;


#endif //STM32_DISPLAY_READ_CONFIG_H
