#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <rtthread.h>
#include <stdint.h>
#include <rtdevice.h>

typedef struct
{
	rt_uint16_t si702x_temperature;
	rt_uint16_t si702x_humidity;
	
	rt_uint16_t bh1750_illuminace;
	
	rt_uint8_t pcf8563_second;
	rt_uint8_t pcf8563_minute;
	rt_uint8_t pcf8563_hour;
	
	rt_uint8_t pcf8563_year;
	rt_uint8_t pcf8563_month;
	rt_uint8_t pcf8563_day;
	rt_uint8_t pcf8563_week;
}sensor_data_t;
void sensorDataUpdate(sensor_data_t *in_data,sensor_data_t *out_data);
int thread_led_update(void);
int thread_time_update(void);
int thread_temp_humi_update(void);
int thread_illuminace_update(void);
int thread_keySetPcf8563Time(void);

#endif
