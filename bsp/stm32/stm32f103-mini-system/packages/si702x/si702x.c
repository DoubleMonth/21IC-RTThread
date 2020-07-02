#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#define DBG_ENABLE
#define DBG_SECTION_NAME "SI702X"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR 
#include <rtdbg.h>

#include "si702x/si702x.h"

#ifdef PKG_USING_SI702X 

static si702x_device_t si702x_dev=RT_NULL;

static rt_err_t si702x_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t len,rt_uint8_t *buf)
{
	struct rt_i2c_msg msgs[2];
	msgs[0].addr = SI702X_ADDR;
	msgs[0].flags = RT_I2C_WR;
	msgs[0].buf = &reg;
	msgs[0].len = 1;
	
	msgs[1].addr=SI702X_ADDR;
	msgs[1].flags = RT_I2C_RD;
	msgs[1].buf = buf;
	msgs[1].len = 2;
	if(rt_i2c_transfer(bus,msgs,2)==2)
	{
		return RT_EOK;
	}
	else
	{
		return -RT_ERROR;
	}
}
rt_int16_t si702x_read_temperature(void)
{
	rt_uint8_t temp_buf[2];
	rt_int16_t temp;
	if(RT_EOK == si702x_read_reg(si702x_dev->i2c,SI702X_MEASURE_TEMP_HOLD_CMD,2,temp_buf))
	{
		temp=(rt_uint16_t)((((temp_buf[0]<<8)+temp_buf[1])*175.72/65536.0-46.85)*10);
//		rt_kprintf("temperature=%d\n",temp);
		return temp;
	}
	else
	{
		rt_kprintf("Error!!!,read temperature error!\n",temp);
		return -500;
	}
}
rt_int16_t si702x_read_humidity(void)
{
	rt_uint8_t temp_buf[2];
	rt_int16_t temp;
	if(RT_EOK==si702x_read_reg(si702x_dev->i2c,SI702X_MEASURE_HUMI_HOLD_CMD,2,temp_buf))
	{
		temp=(rt_uint16_t)((((temp_buf[0]<<8)+temp_buf[1])*125.0/65536.0-6.0)*10);
//		rt_kprintf("humidity=%d\n",temp);
		return temp;
	}
	else
	{
		rt_kprintf("Error!!!,read humidity error!\n");
		return -500;
	}
}
rt_err_t si702x_read_temp_humi(void)
{
	si702x_read_temperature();
	si702x_read_humidity();
	return RT_EOK;
}

si702x_device_t si702x_init(void)
{
	char *i2c_bus_name=SI702X_DEVICE_NAME;
	RT_ASSERT(i2c_bus_name);
	si702x_dev=rt_calloc(1,sizeof(struct si702x_device));
	if(si702x_dev == RT_NULL)
	{
		LOG_E("Can't allocate memory for si702x device on '%s',si702x initializes failure!",i2c_bus_name);
		return RT_NULL ;
	}
	si702x_dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
	if(si702x_dev->i2c == RT_NULL )
	{
		LOG_E ("Can't create mutex for si702x device on '%s' ",i2c_bus_name);
		rt_free(si702x_dev);
		return RT_NULL;
	}
	si702x_dev->lock = rt_mutex_create("mutex_si702x",RT_IPC_FLAG_FIFO);
	if(si702x_dev->lock == RT_NULL)
	{
		LOG_E("Can't create mutex for si702x device on '%s' ",i2c_bus_name);
		rt_free(si702x_dev);
		return RT_NULL;
	}
#ifdef SI702X_USING_SOFT_FILTER
    dev->period = SI702X_SAMPLE_PERIOD;
    dev->thread = rt_thread_create("si702x", si702x_filter_entry, (void *)dev, 1024, 15, 10);
    if (dev->thread != RT_NULL)
    {
        rt_thread_startup(dev->thread);
    }
    else
    {
        LOG_E("Can't start filtering function for si702x device on '%s' ", i2c_bus_name);
        rt_mutex_delete(dev->lock);
        rt_free(dev);
    }
#endif /* SI702X_USING_SOFT_FILTER */

//    sensor_init(dev);
	rt_kprintf("si702x using %s finshed\n",SI702X_DEVICE_NAME);
    return si702x_dev;
	
}
MSH_CMD_EXPORT(si702x_read_temp_humi,read temperature humidity);


#endif /*PKG_USING_SI702X*/
