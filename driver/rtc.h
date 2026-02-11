#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

void     rtc_init(void);
void     rtc_read(rtc_time_t* time);
uint8_t  rtc_get_second(void);
uint8_t  rtc_get_minute(void);
uint8_t  rtc_get_hour(void);

#endif
