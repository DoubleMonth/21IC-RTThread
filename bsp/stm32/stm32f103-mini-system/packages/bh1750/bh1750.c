#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#define DBG_ENABLE
#define DBG_SECTION_NAME "BH1750"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR 
#include <rtdbg.h>
#include "bh1750/bh1750.h"

static bh1750_device_t bh1750_dev=RT_NULL;
static rt_err_t bh1750_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t len,rt_uint8_t *buf)
{
	struct rt_i2c_msg msgs[1];
	
	msgs[0].addr=BH1750_ADDR;
	msgs[0].flags = RT_I2C_RD;
	msgs[0].buf = buf;
	msgs[0].len = len;
	if(rt_i2c_transfer(bus,msgs,1)==1)
	{
		return RT_EOK;
	}
	else
	{
		return -RT_ERROR;
	}
}
static rt_err_t bh1750_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t data)
{
	struct rt_i2c_msg msgs;
	rt_uint8_t buf[1]={reg};
	
	msgs.addr = BH1750_ADDR;
	msgs.flags = RT_I2C_WR;
	msgs.buf = buf;
	msgs.len = 1;
	
	if(rt_i2c_transfer(bus,&msgs,1)==1)
	{
		return RT_EOK;
	}
	else
	{
		return -RT_ERROR;
	}
}
rt_uint32_t bh1750_read_illuminace(void)
{
	rt_err_t result;
	rt_uint32_t illuminace;
	rt_uint8_t illuminace_buf[2];
	RT_ASSERT(bh1750_dev);
	result = rt_mutex_take(bh1750_dev->lock, RT_WAITING_FOREVER);
	bh1750_write_reg(bh1750_dev->i2c,BH1750_READ_ILLUMINACE_ADDR,0);
	rt_thread_mdelay(200);
	if (result == RT_EOK)
	{
		bh1750_read_reg(bh1750_dev->i2c,BH1750_READ_ILLUMINACE_ADDR,2,&illuminace_buf[0]);
		illuminace=(uint32_t)(((illuminace_buf[0]<<8)+illuminace_buf[1])/1.2*10);					//Êý¾Ý×ª»»
//		rt_kprintf("illuminace=%d\n",illuminace);
	}
	else
	{
		LOG_E("The bh1750 could not respond time read at this time. Please try again");
		rt_mutex_release(bh1750_dev->lock);
	}
	return illuminace;
}

bh1750_device_t bh1750_init(void)
{ 
	rt_kprintf("bh1750_init...\n");
	const char *i2c_bus_name = BH1750_DEVICE_NAME;
	RT_ASSERT(i2c_bus_name);
	bh1750_dev = rt_calloc(1, sizeof(struct bh1750_device));
	if (bh1750_dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for bh1750 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }
	bh1750_dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
	if(bh1750_dev->i2c == RT_NULL )
	{
		LOG_E ("Can't find i2c device for bh1750 device on '%s' ",i2c_bus_name);
		rt_free(bh1750_dev);
		return RT_NULL;
	}
	bh1750_dev->lock = rt_mutex_create("mutex_bh1750",RT_IPC_FLAG_FIFO);
	if(bh1750_dev->lock == RT_NULL)
	{
		LOG_E("Can't create mutex for bh1750 device on '%s' ",i2c_bus_name);
		return RT_NULL;
	}
	bh1750_write_reg(bh1750_dev->i2c,BH1750_POWER_ON,0);
	rt_kprintf("bh1750_init finshed!\n");
    return bh1750_dev;
}
MSH_CMD_EXPORT(bh1750_read_illuminace,read humi);
