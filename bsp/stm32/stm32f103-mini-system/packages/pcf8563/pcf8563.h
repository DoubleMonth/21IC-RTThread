#ifndef __PCF8563_H__
#define __PCF8563_H__
#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#define PCF8563_DEVICE_NAME "i2c1"
#define PCF8563_ADDR 0X51
#define PCF8563_SECOND_ADDR 0X02
#define PCF8563_MINUTE_ADDR 0X03
#define PCF8563_HOUR_ADDR   0X04
#define PCF8563_DAY_ADDR 0X05
#define PCF8563_WEEK_ADDR 0X06
#define PCF8563_MONTH_ADDR 0X07
#define PCF8563_YEAR_ADDR 0X08
#define PCF8563_CONTROL_ADDR 0X00

struct pcf8563_device 
{
	struct rt_i2c_bus_device *i2c;
	rt_mutex_t lock;
};
typedef struct pcf8563_device *pcf8563_device_t;
struct pcf8563_time
{
	rt_uint8_t second;
	rt_uint8_t minute;
	rt_uint8_t hour;
};
typedef struct pcf8563_time *pcf8563_time_t;
struct pcf8563_day
{
	rt_uint8_t year;
	rt_uint8_t month;
	rt_uint8_t day;
	rt_uint8_t week;
};
typedef struct pcf8563_day *pcf8563_day_t;
pcf8563_device_t pcf8563_init(void);
struct pcf8563_time pcf8563_read_time(void);
rt_err_t pcf8563_start(void);
rt_err_t pcf8563_stop(void);
struct pcf8563_day pcf8563_read_day(void);
void pcf8563WriteTime(struct pcf8563_time time_temp);
void pcf8563WriteDay(struct pcf8563_day day_temp);
#endif
