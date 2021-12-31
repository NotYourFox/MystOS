#include "io/vgaio/vgaio.h"
#include "io/io.h"
#include "cmos.h"
#include "config.h"
#include "idt/idt.h"
#include "mem/heap/kheap.h"
#include "status.h"

enum {
      cmos_address = 0x70,
      cmos_data = 0x71
};

int get_update_in_progress_flag(){
      outb(cmos_address, 0x0A);
      return (insb(cmos_data) & 0x80);
}
 
unsigned char get_RTC_register(int reg){
      interrupt_flag(cli);
      outb(cmos_address, reg);
      return insb(cmos_data);
      interrupt_flag(sti);
}

int check_century_register(){
    int century_register = 0x32;
    int res = (int)get_RTC_register(century_register);
    if (!res){
        century_register = 0x00;
    }
    return century_register;
}
 
struct rtc* time(){
      struct rtc* rtc = kzalloc(sizeof(struct rtc));
      int century_register = check_century_register();
 
      // Note: This uses the "read registers until you get the same values twice in a row" technique
      //       to avoid getting dodgy/inconsistent values due to RTC updates
 
      while (get_update_in_progress_flag());                // Make sure an update isn't in progress
      rtc -> second = get_RTC_register(0x00);
      rtc -> minute = get_RTC_register(0x02);
      rtc -> hour = get_RTC_register(0x04);
      rtc -> day = get_RTC_register(0x07);
      rtc -> month = get_RTC_register(0x08);
      rtc -> year = get_RTC_register(0x09);
      if(century_register != 0) {
            rtc -> century = get_RTC_register(century_register);
      }
 
      do {
            rtc -> last_second = rtc -> second;
            rtc -> last_minute = rtc -> minute;
            rtc -> last_hour = rtc -> hour;
            rtc -> last_day = rtc -> day;
            rtc -> last_month = rtc -> month;
            rtc -> last_year = rtc -> year;
            rtc -> last_century = rtc -> century;
 
            while (get_update_in_progress_flag());           // Make sure an update isn't in progress
            rtc -> second = get_RTC_register(0x00);
            rtc -> minute = get_RTC_register(0x02);
            rtc -> hour = get_RTC_register(0x04);
            rtc -> day = get_RTC_register(0x07);
            rtc -> month = get_RTC_register(0x08);
            rtc -> year = get_RTC_register(0x09);
            if(century_register != 0) {
                  rtc -> century = get_RTC_register(century_register);
            }
      } while( (rtc -> last_second != rtc -> second) || (rtc -> last_minute != rtc -> minute) || (rtc -> last_hour != rtc -> hour) ||
               (rtc -> last_day != rtc -> day) || (rtc -> last_month != rtc -> month) || (rtc -> last_year != rtc -> year) ||
               (rtc -> last_century != rtc -> century) );
 
      rtc -> registerB = get_RTC_register(0x0B);
 
      // Convert BCD to binary values if necessary
 
      if (!(rtc -> registerB & 0x04)) {
            rtc -> second = (rtc -> second & 0x0F) + ((rtc -> second / 16) * 10);
            rtc -> minute = (rtc -> minute & 0x0F) + ((rtc -> minute / 16) * 10);
            rtc -> hour = ( (rtc -> hour & 0x0F) + (((rtc -> hour & 0x70) / 16) * 10) ) | (rtc -> hour & 0x80);
            rtc -> day = (rtc -> day & 0x0F) + ((rtc -> day / 16) * 10);
            rtc -> month = (rtc -> month & 0x0F) + ((rtc -> month / 16) * 10);
            rtc -> year = (rtc -> year & 0x0F) + ((rtc -> year / 16) * 10);
            if(century_register != 0) {
                  rtc -> century = (rtc -> century & 0x0F) + ((rtc -> century / 16) * 10);
            }
      }
 
      // Convert 12 rtc -> hour clock to 24 rtc -> hour clock if necessary
 
      if (!(rtc -> registerB & 0x02) && (rtc -> hour & 0x80)) {
            rtc -> hour = ((rtc -> hour & 0x7F) + 12) % 24;
      }
 
      // Calculate the full (4-digit) rtc -> year
 
      if(century_register != 0) {
            rtc -> year += rtc -> century * 100;
      } else {
            rtc -> year += (CURRENT_CENTURY - 1) * 100;
      }
      return rtc;
}

int get_num_of_days_in_year(int year){
      int days = 365;
      if (year % 400 == 0){
            goto out;
      }
      if (year % 100 == 0){
            days = 366;
            goto out;
      }
      if (year % 4 == 0){
            days = 366;
            goto out;
      }
out:
      return days;
}

int get_num_of_days_in_month(int month, int year){
      int days;
      if (month < 1 || month > 12){
            days = -EINVARG;
            goto out;
      }
      switch (month){
            case 2: days = get_num_of_days_in_year(year) == 365 ? 28 : 29; break;
            case 4:
            case 6:
            case 9:
            case 11: days = 30; break;
            default: days = 31; break;
      }
out:
      return days;
}

int utime(){
      struct rtc* rtc = time();
      int years_to_days = 0;
      int months_to_days = 0;
      int days = rtc -> day - 1;
      for (int year = 1970; year < rtc -> year; year++){
            int num = get_num_of_days_in_year(year);
            years_to_days += num;
      }
      for (int month = 1; month < rtc -> month; month++){
            int num = get_num_of_days_in_month(month, rtc -> year);
            months_to_days += num;
      }
      int total_days = years_to_days + months_to_days + days;
      int days_to_seconds = total_days * 86400;
      int hours_to_seconds = rtc -> hour * 3600;
      int minutes_to_seconds = rtc -> minute * 60;
      int seconds = rtc -> second;
      kfree(rtc);
      return days_to_seconds + hours_to_seconds + minutes_to_seconds + seconds;
}

void sleep(int secs){
      int cur_utime = utime();
      int end_utime = cur_utime + secs;
      while (cur_utime < end_utime){
            cur_utime = utime();
      }
}