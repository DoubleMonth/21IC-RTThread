

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <rtdbg.h>

#include "sensor/sensor.h"
#include "si702x/si702x.h"
#include "tm1638/tm1638.h"
#include "pcf8563/pcf8563.h"
#include "bh1750/bh1750.h"
#include "example_agile_button.h"

static rt_thread_t led_tid=RT_NULL;
static rt_thread_t update_time_tid=RT_NULL;
static rt_thread_t temp_humi_tid=RT_NULL;
static rt_thread_t illu_tid=RT_NULL;
static rt_thread_t setTime_tid=RT_NULL;
rt_uint8_t displayNumber[10];
static rt_uint8_t pointFlag =0;
static sensor_data_t sensor_data_result=
{
	.si702x_temperature = 0,
	.si702x_humidity = 0,
	
	.bh1750_illuminace = 0,
	
	.pcf8563_second = 0,
	.pcf8563_minute = 0,
	.pcf8563_hour = 0,
	
	.pcf8563_year = 0,
	.pcf8563_month = 0,
	.pcf8563_day = 0,
	.pcf8563_week = 0,
};
void sensorDataUpdate(sensor_data_t *in_data,sensor_data_t *out_data)
{
	out_data->bh1750_illuminace=in_data->bh1750_illuminace;
	out_data->si702x_temperature=in_data->si702x_temperature;
	out_data->si702x_humidity=in_data->si702x_humidity;
	out_data->pcf8563_second=in_data->pcf8563_second;
	out_data->pcf8563_minute=in_data->pcf8563_minute;
	out_data->pcf8563_hour=in_data->pcf8563_hour;
	out_data->pcf8563_day=in_data->pcf8563_day;
	out_data->pcf8563_month=in_data->pcf8563_month;
	out_data->pcf8563_year=in_data->pcf8563_year;
	out_data->pcf8563_week=in_data->pcf8563_week;
} 
//按键使用的互斥量
rt_mutex_t key_mutex = RT_NULL;
int mutex_key(void)
{
	key_mutex=rt_mutex_create("key_mutex",RT_IPC_FLAG_FIFO); //创建按键互斥量
	if(key_mutex==RT_NULL)
	{
		rt_kprintf("creat dynamic mutex failed.\n");
		return -1;
	}
	return 0;
}
static void led_update(void *parameter)
{
	while(1)
	{
		display(displayNumber,pointFlag,5,1);
		rt_thread_mdelay(50);
	}
}
extern struct pcf8563_time UTC_time;
extern rt_uint8_t update_UTC_time_Flag;
static void time_update(void *parameter)
{
	struct pcf8563_time temp;
	
	while(1)
	{
		if(update_UTC_time_Flag)   //更新本地时间为UTC时间
		{
			pcf8563WriteTime(UTC_time);
			update_UTC_time_Flag=0;
		}
		rt_mutex_take(key_mutex,10000);	//获取设置时间互斥量
		temp=pcf8563_read_time();
		displayNumber[0]=temp.hour/10;
		displayNumber[1]=temp.hour%10;
		displayNumber[2]=temp.minute /10;
		displayNumber[3]=temp.minute%10;
		
		pointFlag=temp.second&0x01;  // 秒闪烁标志
		rt_mutex_release(key_mutex);   //释放设置时间互斥量
		rt_thread_mdelay(500);
	}
}
static void temp_humi_update(void *parameter)
{
	rt_int16_t temp=0;
	rt_int16_t humi=0;
	while(1)
	{
		
		rt_mutex_take(key_mutex,10000);	//获取设置时间互斥量
		temp=si702x_read_temperature();
		humi=si702x_read_humidity();
		displayNumber[4]=temp/100;
		displayNumber[5]=temp%100/10;
		displayNumber[6]=temp%10;
		
		displayNumber[7]=humi/100;
		displayNumber[8]=humi%100/10;
		displayNumber[9]=humi%10;
		rt_mutex_release(key_mutex);   //释放设置时间互斥量
		rt_thread_mdelay(3000);
	}
}
static void illuminace_update(void *parameter)
{
	rt_uint32_t illuminace=0;
	rt_int16_t humi=0;
	while(1)
	{
		illuminace=bh1750_read_illuminace();
		if (illuminace<=20)
		{
			tm1638Setluminance(0x88);   // 三段调光 最暗
		}
		else if (illuminace<=400)
		{
			tm1638Setluminance(0x8A);		//  中亮  
		}
		else 
		{
			tm1638Setluminance(0x8F);		//最亮
		}
			
		rt_thread_mdelay(5000);
	}
}
extern rt_sem_t set_short_state;   //set 按键短按状态 1:按下,0:未按下。
extern rt_sem_t set_long_state;		//set 按键长按状态 1:按下,0:未按下。
extern rt_sem_t add_short_state;	//add 按键短按状态 1:按下,0:未按下。
extern rt_sem_t add_long_state;		//add 按键长按状态 1:按下,0:未按下。
static void keySetPcf8563Time(void *parameter)
{
	rt_uint16_t counter=0;
	rt_uint8_t setTimeFlag=0;  //0:不进行设置，1：设置分，2：设置时，3：设置年，4：设置月，5：设置日，6：设置周,7:设置完成、开始时钟
	rt_uint8_t showDayFlag=0;
	rt_err_t rt_sus;
	struct pcf8563_time temp_time;
	struct pcf8563_day temp_day;
	while(1)
	{
		if (setTimeFlag>0 || showDayFlag>0)
		{
			counter++;
			rt_kprintf("counter=%d\n",counter);
		}
		if(RT_EOK == rt_sem_trytake(set_short_state))  //set 按键按下
		{
			counter=0;
			setTimeFlag++;
			pcf8563_stop();  
			temp_time=pcf8563_read_time();
			temp_day=pcf8563_read_day();
			displayNumber[0]=temp_time.hour/10;
			displayNumber[1]=temp_time.hour%10;
			displayNumber[2]=temp_time.minute /10;
			displayNumber[3]=temp_time.minute%10;
			pointFlag=0;  					// 暂停秒闪烁
			rt_kprintf("set key pressed setTimeFlag = %d\n",setTimeFlag);
		}
		if((setTimeFlag==1) && (counter==0))
		{
			rt_mutex_take(key_mutex,5);  //获取按键互斥量  只获取一次
		}
		if((setTimeFlag==1) && (RT_EOK == rt_sem_trytake(add_short_state)))   //设置时
		{
			if((temp_time.hour++)>=23)
				temp_time.hour=0;
			temp_time.second=0;   //秒置0
			counter=0;
			setTimeFlag=1;
			pcf8563WriteTime(temp_time);
			rt_kprintf("add hour=\n",temp_time.hour);
		}
		if((setTimeFlag==2) && (RT_EOK == rt_sem_trytake(add_short_state)))  //设置 分
		{
			if((temp_time.minute++)>=59)
				temp_time.minute=0;
			temp_time.second=0;   //秒置0
			counter=0;
			setTimeFlag=2;
			pcf8563WriteTime(temp_time);
			rt_kprintf("add minute=%d\n",temp_time.minute);
		}
		
		if((setTimeFlag==3) && (RT_EOK == rt_sem_trytake(add_short_state)))  //设置 年
		{
			if((temp_day.year++)>=99)
				temp_day.year=0;
			counter=0;
			setTimeFlag=3;
			pcf8563WriteDay(temp_day); 
			rt_kprintf("add year =%d\n",temp_day.year);
		}
		if((setTimeFlag==4) && (RT_EOK == rt_sem_trytake(add_short_state)))  //设置 月
		{
			if((temp_day.month++)>=12)
				temp_day.month=1;
			counter=0;
			setTimeFlag=4;
			pcf8563WriteDay(temp_day); 
			rt_kprintf("add month =%d\n",temp_day.month);
		}
		if((setTimeFlag==5) && (RT_EOK == rt_sem_trytake(add_short_state)))  //设置 日
		{
			if((temp_day.month==1)||(temp_day.month==3)||(temp_day.month==5)||(temp_day.month==7)||(temp_day.month==8)||(temp_day.month==10)||(temp_day.month==12))
			{
				if((temp_day.day++)>=31)    //闰月
					temp_day.day=1;
			}
			else if(temp_day.month==2)    
			{
				if(temp_day.year%4==0)
				{
					if((temp_day.day++)>=29)   //闰年的二月
						temp_day.day=1;
				}
				else
				{
					if((temp_day.day++)>=28)   //平年的二月
						temp_day.day=1;
				}
			}
			else
			{
				if((temp_day.day++)>=30)   //平月
						temp_day.day=1;
			}
			counter=0;
			setTimeFlag=5;
			pcf8563WriteDay(temp_day); 
			rt_kprintf("add day =%d\n",temp_day.day);
		}
		if((setTimeFlag==6) && (RT_EOK == rt_sem_trytake(add_short_state)))  //设置 周
		{
			if((temp_day.week++)>=6)
				temp_day.week=0;
			counter=0;
			setTimeFlag=6;
			pcf8563WriteDay(temp_day); 
			rt_kprintf("add week =%d\n",temp_day.week);
		}
		if(setTimeFlag==7)
		{
			counter=0;
			setTimeFlag=0;
			pcf8563_start();
			rt_mutex_release(key_mutex);
			rt_kprintf("key start update\n");
		}
		if(setTimeFlag==1 && (counter%50)<=25)     //设置时
		{
			displayNumber[0]=temp_time.hour/10;
			displayNumber[1]=temp_time.hour%10;
			displayNumber[2]=temp_time.minute/10;
			displayNumber[3]=temp_time.minute%10;
			pointFlag=1;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==1 && (counter%50)<50)
		{
			displayNumber[0]=16;
			displayNumber[1]=16;
			displayNumber[2]=temp_time.minute/10;
			displayNumber[3]=temp_time.minute%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		
		if(setTimeFlag==2 && (counter%50)<=25)     //设置分
		{
			displayNumber[0]=temp_time.hour/10;
			displayNumber[1]=temp_time.hour%10;
			displayNumber[2]=temp_time.minute/10;
			displayNumber[3]=temp_time.minute%10;
			pointFlag=1;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==2 && (counter%50)<50)
		{
			displayNumber[0]=temp_time.hour/10;
			displayNumber[1]=temp_time.hour%10;
			displayNumber[2]=16;
			displayNumber[3]=16;
			pointFlag=0;  					// 暂停秒闪烁
		}
		
		if(setTimeFlag==3 && (counter%50)<=25)     //设置年
		{
			displayNumber[0]=temp_day.year/10;
			displayNumber[1]=temp_day.year%10;
			displayNumber[2]=temp_day.month /10;
			displayNumber[3]=temp_day.month%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==3 && (counter%50)<50)
		{
			displayNumber[0]=16;
			displayNumber[1]=16;
			displayNumber[2]=temp_day.month /10;
			displayNumber[3]=temp_day.month%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		
		if(setTimeFlag==4 && (counter%50)<=25)     //设置月
		{
			displayNumber[0]=temp_day.year/10;
			displayNumber[1]=temp_day.year%10;
			displayNumber[2]=temp_day.month /10;
			displayNumber[3]=temp_day.month%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==4 && (counter%50)<50)
		{
			displayNumber[0]=temp_day.year/10;
			displayNumber[1]=temp_day.year%10;
			displayNumber[2]=16;
			displayNumber[3]=16;
			pointFlag=0;  					// 暂停秒闪烁
		}
		
		if(setTimeFlag==5 && (counter%50)<=25)     //设置日
		{
			displayNumber[0]=temp_day.day/10;
			displayNumber[1]=temp_day.day%10;
			displayNumber[2]=16;
			displayNumber[3]=temp_day.week%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==5 && (counter%50)<50)
		{
			displayNumber[0]=16;
			displayNumber[1]=16;
			displayNumber[2]=16;
			displayNumber[3]=temp_day.week%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		
		if(setTimeFlag==6 && (counter%50)<=25)    //设置周
		{
			displayNumber[0]=temp_day.day/10;
			displayNumber[1]=temp_day.day%10;
			displayNumber[2]=16;
			displayNumber[3]=temp_day.week%10;
			pointFlag=0;  					// 暂停秒闪烁
		}
		else if(setTimeFlag==6 && (counter%50)<50)
		{
			displayNumber[0]=temp_day.day/10;
			displayNumber[1]=temp_day.day%10;
			displayNumber[2]=16;
			displayNumber[3]=16;
			pointFlag=0;  					// 暂停秒闪烁
		}
		if(setTimeFlag && (counter>=500))   //闪烁10秒钟后停止
		{
			counter=0;
			setTimeFlag=0;
			pcf8563_start();
			rt_mutex_release(key_mutex);
			rt_kprintf("time out start update\n");
		}
		if(showDayFlag && (counter>=250))
		{
			counter=0;
			showDayFlag=0;
			rt_mutex_release(key_mutex);
		}
		if((showDayFlag==0)&& (setTimeFlag==0) && (RT_EOK == rt_sem_trytake(add_short_state)))
		{
			showDayFlag=1;
		}
		if(showDayFlag==1 && counter ==0)
		{
			rt_mutex_take(key_mutex,50);  //获取按键互斥量  只获取一次
			rt_kprintf("add take mutex");
			temp_day=pcf8563_read_day();
			displayNumber[0]=2;
			displayNumber[1]=0;
			displayNumber[2]=temp_day.year/10;
			displayNumber[3]=temp_day.year%10;
			displayNumber[4]=temp_day.month/10;
			displayNumber[5]=temp_day.month%10;
			displayNumber[6]=16;
			displayNumber[7]=temp_day.day/10;
			displayNumber[8]=temp_day.day%10;
			pointFlag=0;  					// 暂停秒闪烁
			if(temp_day.week==0)
				displayNumber[9]=7;
			else				
				displayNumber[9]=temp_day.week%10;
		}
		rt_thread_mdelay(20);
	}
}

int thread_led_update(void)
{
	led_tid=rt_thread_create("led_update",led_update,RT_NULL,256,25,50);  //数码管更新线程
	if(led_tid!=RT_NULL)
	{
		rt_thread_startup(led_tid);
	}
	return 0;
}
int thread_time_update(void)
{
	update_time_tid=rt_thread_create("time_update",time_update,RT_NULL,512,15,50);  //时间更新线程
	if(led_tid!=RT_NULL)
	{
		rt_thread_startup(update_time_tid);
	}
	return 0;
}
int thread_temp_humi_update(void)
{
	temp_humi_tid=rt_thread_create("temp_humi_update",temp_humi_update,RT_NULL,512,25,50);  //温湿度更新线程
	if(temp_humi_tid!=RT_NULL)
	{
		rt_thread_startup(temp_humi_tid);
	}
	return 0;
}
int thread_illuminace_update(void)
{
	illu_tid=rt_thread_create("temp_illuminace_update",illuminace_update,RT_NULL,512,25,50);   //光照度更新线程
	if(illu_tid!=RT_NULL)
	{
		rt_thread_startup(illu_tid);
	}
	return 0;
}
int thread_keySetPcf8563Time(void)
{
	setTime_tid=rt_thread_create("keySetPcf8563Time",keySetPcf8563Time,RT_NULL,512,25,50);   //设置时间线程
	if(setTime_tid!=RT_NULL)
	{
		rt_thread_startup(setTime_tid);
	}
	return 0;
}


//MSH_CMD_EXPORT(time_update, create time_update);





