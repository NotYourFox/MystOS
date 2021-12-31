#ifndef CMOS_H
#define CMOS_H

#define CURRENT_CENTURY 21

struct rtc{
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;
};

struct rtc* time();
int utime();
void sleep();

#endif