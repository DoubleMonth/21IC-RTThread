#ifndef __SI702X_H__
#define __SI702X_H__
#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#define SI702X_DEVICE_NAME "i2c3"
#define SI702X_ADDR 0x40
#define SI702X_MEASURE_HUMI_HOLD_CMD 		0XE0
#define SI702X_MEASURE_HUMI_NO_HOLD_CMD 	0XF5
#define SI702X_MEASURE_TEMP_HOLD_CMD		0XE3
#define SI702X_MEASURE_TEMP_NO_HOLD_CMD 	0XF3
#define SI702X_RESET_CMD					0XFE

#ifdef SI702X_USING_SOFT_FILTER

typedef struct filter_data
{
    float buf[AHT10_AVERAGE_TIMES];
    float average;

    rt_off_t index;
    rt_bool_t is_full;

} filter_data_t;
#endif /* AHT10_USING_SOFT_FILTER */
struct si702x_device 
{
	struct rt_i2c_bus_device *i2c;
#ifdef SI702X_USING_SOFT_FILTER
	filter_data_t temp_filter;
	filter_data_t humi_filter;
	
	rt_thread_t thread;
	rt_thread_t period;
#endif
	rt_mutex_t lock;
	
};
typedef struct si702x_device *si702x_device_t;
static rt_err_t si702x_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t len,rt_uint8_t *buf);
si702x_device_t si702x_init(void);
rt_int16_t si702x_read_humidity(void);
rt_int16_t si702x_read_temperature(void);
rt_err_t si702x_read_temp_humi(void);
#endif
