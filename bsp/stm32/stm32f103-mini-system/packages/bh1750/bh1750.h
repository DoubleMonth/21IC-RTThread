#ifndef __BH1750_H__
#define __BH1750_H__
#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#define BH1750_DEVICE_NAME "i2c2"
#define BH1750_ADDR 0X23
#define BH1750_POWER_ON 0X01
#define BH1750_READ_ILLUMINACE_ADDR 0X10
//#define BH1750_MINUTE_ADDR 0X03
//#define BH1750_HOUR_ADDR   0X04
//#define BH1750_DAY_ADDR 0X05
//#define BH1750_WEEK_ADDR 0X06
//#define BH1750_MONTH_ADDR 0X07
//#define BH1750_YEAR_ADDR 0X08
//#define BH1750_CONTROL_ADDR 0X00

struct bh1750_device 
{
	struct rt_i2c_bus_device *i2c;
	rt_mutex_t lock;
};
typedef struct bh1750_device *bh1750_device_t;
bh1750_device_t bh1750_init(void);
rt_uint32_t bh1750_read_illuminace(void);;
#endif
