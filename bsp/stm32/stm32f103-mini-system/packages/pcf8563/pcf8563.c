#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#define DBG_ENABLE
#define DBG_SECTION_NAME "PCF8563"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR 
#include <rtdbg.h>
#include "pcf8563/pcf8563.h"

pcf8563_device_t pcf8563_dev;
static rt_err_t pcf8563_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t len,rt_uint8_t *buf)
{
	struct rt_i2c_msg msgs[2];
	msgs[0].addr = PCF8563_ADDR;
	msgs[0].flags = RT_I2C_WR;
	msgs[0].buf = &reg;
	msgs[0].len = 1;
	
	msgs[1].addr = PCF8563_ADDR;
	msgs[1].flags = RT_I2C_RD;
	msgs[1].buf = buf;
	msgs[1].len = len;
	if(rt_i2c_transfer(bus,msgs,2)==2)
	{
		return RT_EOK;
	}
	else
	{
		return -RT_ERROR;
	}
}
static rt_err_t pcf8563_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg,rt_uint8_t data)
{
	struct rt_i2c_msg msgs;
	rt_uint8_t buf[2]={reg,data};
	
	msgs.addr = PCF8563_ADDR;
	msgs.flags = RT_I2C_WR;
	msgs.buf = buf;
	msgs.len = 2;
	
	if(rt_i2c_transfer(bus,&msgs,1)==1)
	{
		return RT_EOK;
	}
	else
	{
		return -RT_ERROR;
	}
}
rt_err_t pcf8563_start(void)
{
	rt_uint8_t cmd =0x08,val;
	rt_err_t result;
	rt_kprintf("pcf8563 start.");
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if(result==RT_EOK)
	{
		val= pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_CONTROL_ADDR,cmd);
		rt_mutex_release(pcf8563_dev->lock);
		return val;
	}
	else
	{
		LOG_E("The pcf8563 can't start. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
		return -RT_ERROR;
	}
}
rt_err_t pcf8563_stop(void)
{
	rt_uint8_t cmd =0x28,val;
	rt_err_t result;
	rt_kprintf("pcf8563 stop.");
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if(result==RT_EOK)
	{
		val= pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_CONTROL_ADDR,cmd);
		rt_mutex_release(pcf8563_dev->lock);
		return val;
	}
	else
	{
		LOG_E("The pcf8563 can't stop. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
		return -RT_ERROR;
	}
}
struct pcf8563_time pcf8563_read_time(void)
{
	rt_err_t result;
	struct pcf8563_time time;
	rt_uint8_t time_buf[3];
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if (result == RT_EOK)
	{
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_SECOND_ADDR,1,&time_buf[0]);
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_MINUTE_ADDR,1,&time_buf[1]);
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_HOUR_ADDR,1,&time_buf[2]);
		time.second=((time_buf[0]>>4)&0x07)*10+(time_buf[0]&0x0f);
		time.minute=((time_buf[1]>>4)&0x07)*10+(time_buf[1]&0x0f);
		time.hour=((time_buf[2]>>4)&0x03)*10+(time_buf[2]&0x0f);
		rt_mutex_release(pcf8563_dev->lock);
//		rt_kprintf("Now time %d:%d:%d\n",time.hour,time.minute,time.second);
	}
	else
	{
		LOG_E("The pcf8563 could not respond time read at this time. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
	}
	return time;
}
struct pcf8563_day pcf8563_read_day(void)
{
	rt_err_t result;
	struct pcf8563_day day;
	rt_uint8_t time_buf[4];
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if (result == RT_EOK)
	{
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_DAY_ADDR,1,&time_buf[0]);
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_WEEK_ADDR,1,&time_buf[1]);
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_MONTH_ADDR,1,&time_buf[2]);
		pcf8563_read_reg(pcf8563_dev->i2c,PCF8563_YEAR_ADDR,1,&time_buf[3]);
		day.day=((time_buf[0]>>4)&0x03)*10+(time_buf[0]&0x0f);
		day.week=time_buf[1]&0x07;
		day.month=((time_buf[2]>>4)&0x01)*10+(time_buf[2]&0x0f);
		day.year=((time_buf[3]>>4)&0x0f)*10+(time_buf[3]&0x0f);
		rt_mutex_release(pcf8563_dev->lock);
//		rt_kprintf("The day 20%d-%d-%d,week %d\n",day.year,day.month,day.day,day.week);
	}
	else
	{
		LOG_E("The pcf8563 could not respond day read at this time. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
	}
	return day;
}
void pcf8563WriteTime(struct pcf8563_time time_temp)
{
	rt_err_t result;
	rt_uint8_t time_buf[3];
	time_buf[0]=((time_temp.second/10)<<4)+(time_temp.second%10);  //秒
	time_buf[1]=((time_temp.minute /10)<<4)+(time_temp.minute%10);  //分
	time_buf[2]=((time_temp.hour /10)<<4)+(time_temp.hour%10);  //时
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if (result == RT_EOK)
	{
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_SECOND_ADDR,time_buf[0]);
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_MINUTE_ADDR,time_buf[1]);
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_HOUR_ADDR,time_buf[2]);
		rt_mutex_release(pcf8563_dev->lock);
		rt_kprintf("Write Time %d:%d:%d\n",time_buf[0],time_buf[1],time_buf[2]);
	}
	else
	{
		LOG_E("The pcf8563 could not respond day read at this time. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
	}
}
void pcf8563WriteDay(struct pcf8563_day day_temp)
{
	rt_err_t result;
	rt_uint8_t time_buf[4];
	time_buf[0]=((day_temp.day /10)<<4)+(day_temp.day%10);  //日
	time_buf[1]=((day_temp.month /10)<<4)+(day_temp.month%10);  //月
	time_buf[2]=((day_temp.year /10)<<4)+(day_temp.year%10);  //年
	time_buf[3]=day_temp.week%7;  //周
	RT_ASSERT(pcf8563_dev);
	result = rt_mutex_take(pcf8563_dev->lock, RT_WAITING_FOREVER);
	if (result == RT_EOK)
	{
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_DAY_ADDR,time_buf[0]);
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_MONTH_ADDR,time_buf[1]);
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_YEAR_ADDR,time_buf[2]);
		pcf8563_write_reg(pcf8563_dev->i2c,PCF8563_WEEK_ADDR,time_buf[3]);
		rt_mutex_release(pcf8563_dev->lock);
		rt_kprintf("Write Day 20%d-%d-%d,Week %d \n",time_buf[2],time_buf[1],time_buf[0],time_buf[3]);
	}
	else
	{
		LOG_E("The pcf8563 could not respond day read at this time. Please try again");
		rt_mutex_release(pcf8563_dev->lock);
	}
}
pcf8563_device_t pcf8563_init(void)
{ 
	rt_kprintf("pcf8563_init..\n");
	const char *i2c_bus_name=PCF8563_DEVICE_NAME;
	RT_ASSERT(i2c_bus_name);
	pcf8563_dev = rt_calloc(1, sizeof(struct pcf8563_device));
	if (pcf8563_dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for pcf8563 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }
	pcf8563_dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
	if(pcf8563_dev->i2c == RT_NULL )
	{
		LOG_E ("Can't create mutex for pcf8563 device on '%s' ",i2c_bus_name);
		rt_free(pcf8563_dev);
		return RT_NULL;
	}
	pcf8563_dev->lock = rt_mutex_create("mutex_pcf8563",RT_IPC_FLAG_FIFO);
	if(pcf8563_dev->lock == RT_NULL)
	{
		LOG_E("Can't create mutex for pcf8563 device on '%s' ",i2c_bus_name);
		return RT_NULL;
	}
	rt_kprintf("pcf8563_init finshed!\n");
    return pcf8563_dev;
}
MSH_CMD_EXPORT(pcf8563_read_time,read time);
