#include "rtc.h"
#include "../kernel/port.h"

#define CMOS_ADDR_PORT 0x70
#define CMOS_DATA_PORT 0x71

#define RTC_REG_SECOND  0x00
#define RTC_REG_MINUTE  0x02
#define RTC_REG_HOUR    0x04
#define RTC_REG_DAY     0x07
#define RTC_REG_MONTH   0x08
#define RTC_REG_YEAR    0x09
#define RTC_REG_STATUS_A 0x0A
#define RTC_REG_STATUS_B 0x0B

#define RTC_UPDATE_IN_PROGRESS 0x80
#define RTC_24H_FORMAT         0x02
#define RTC_BINARY_MODE        0x04

static uint8_t cmos_read(uint8_t reg) {
    outb(CMOS_ADDR_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static int is_update_in_progress(void) {
    return cmos_read(RTC_REG_STATUS_A) & RTC_UPDATE_IN_PROGRESS;
}

void rtc_init(void) {
    while (is_update_in_progress());
}

void rtc_read(rtc_time_t* time) {
    uint8_t status_b;

    while (is_update_in_progress());

    time->second = cmos_read(RTC_REG_SECOND);
    time->minute = cmos_read(RTC_REG_MINUTE);
    time->hour   = cmos_read(RTC_REG_HOUR);
    time->day    = cmos_read(RTC_REG_DAY);
    time->month  = cmos_read(RTC_REG_MONTH);
    time->year   = cmos_read(RTC_REG_YEAR);

    status_b = cmos_read(RTC_REG_STATUS_B);

    if (!(status_b & RTC_BINARY_MODE)) {
        time->second = bcd_to_bin(time->second);
        time->minute = bcd_to_bin(time->minute);
        time->hour   = bcd_to_bin(time->hour & 0x7F);
        time->day    = bcd_to_bin(time->day);
        time->month  = bcd_to_bin(time->month);
        time->year   = bcd_to_bin(time->year);
    }

    time->year += 2000;

    if (!(status_b & RTC_24H_FORMAT) && (time->hour & 0x80)) {
        time->hour = ((time->hour & 0x7F) + 12) % 24;
    }
}

uint8_t rtc_get_second(void) {
    rtc_time_t t;
    rtc_read(&t);
    return t.second;
}

uint8_t rtc_get_minute(void) {
    rtc_time_t t;
    rtc_read(&t);
    return t.minute;
}

uint8_t rtc_get_hour(void) {
    rtc_time_t t;
    rtc_read(&t);
    return t.hour;
}
