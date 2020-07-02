#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "tm1638/tm1638.h"
uint8_t tab[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,
                           0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,0x00};
void tm1638Init(void)
{
	rt_uint8_t i;
	rt_pin_mode(DIO_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(CLK_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(STB_PIN, PIN_MODE_OUTPUT);
	
	rt_pin_write(DIO_PIN,PIN_HIGH);
	rt_pin_write(CLK_PIN,PIN_HIGH);
	rt_pin_write(STB_PIN,PIN_HIGH);
	
	writeCMD(0x8B);		//设置亮度
	writeCMD(0x40);		//采用地址自动加1
	rt_pin_write(STB_PIN,PIN_LOW);
	tm1638Write(0xC0);		//设置起始地址
	for(i=0;i<16;i++)	//初始全部不显示
	{
		tm1638Write(0x00);
	}
	rt_pin_write(STB_PIN,PIN_HIGH);
}
void tm1638Setluminance(rt_uint8_t lumi)
{
	writeCMD(lumi);		//设置亮度
}
static void tm1638Write(rt_uint8_t data)
{
	rt_uint8_t i;
	for(i=0;i<8;i++)
	{
		rt_pin_write(CLK_PIN,PIN_LOW);
		if(data&0x01)
		{
			rt_pin_write(DIO_PIN,PIN_HIGH);
		}
		else
			rt_pin_write(DIO_PIN,PIN_LOW);
		data>>=1;
		rt_pin_write(CLK_PIN,PIN_HIGH);
	}
}
static void writeCMD(rt_uint8_t cmd)
{
	rt_pin_write(STB_PIN,PIN_LOW);
	tm1638Write(cmd);
	rt_pin_write(STB_PIN,PIN_HIGH);
}
static void writeData(rt_uint8_t add,rt_uint8_t data)
{
	writeCMD(0x44);//普通模式，固定地址
	rt_pin_write(STB_PIN,PIN_LOW);
	tm1638Write(0xc0|add);
	tm1638Write(data);
	rt_pin_write(STB_PIN,PIN_HIGH);
}
void writeAllLED(rt_uint8_t LED_Flag)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(LED_Flag&(1<<i))
			writeData(2*i+1,1);
		else
			writeData(2*i+1,0);
	}
}
/********************************
* @number 需要显示的数字
* @pointFlash 秒闪烁标志，1时亮，0时灭
* @pointPosition_1 需要显示的小数点的位置。0：不显示小数点；4：温度1位后， 5：温度2位后， 6：温度3位后， 7：湿度1位后。 
* @pointPosition_2 需要显示的小数点的位置。0：不显示小数点；1：湿度2位后，2：湿度3位后 
********************************/
void display(rt_uint8_t *number,rt_uint8_t pointFlash, rt_uint8_t pointPosition_1, rt_uint8_t pointPosition_2)//number为要显示的数字，依次从1到10。
{
	rt_int8_t i;
	rt_uint8_t j=0;
	rt_uint8_t data[8]={0};
	
	for(j=0;j<8;j++)
	{
		for(i=7;i>=0;i--)
		{
			data[j] <<= 1;
			if((i==3)&&pointFlash)
			{
				if(((tab[*(number+i)]|0x80)>>j)&0x01)
				data[j] +=1;
			}
			else if ((pointPosition_1<=7)&&(i==pointPosition_1))
			{
				if(((tab[*(number+i)]|0x80)>>j)&0x01)
				data[j] +=1;
			}
			else
			{
				if(((tab[*(number+i)])>>j)&0x01)
				data[j] +=1;
			}
		}
		writeData(2*j,data[j]);
	}
	for(j=0;j<8;j++)
	{
		for(i=7;i>=0;i--)
		{
			data[j] <<= 1;
			if(((pointPosition_2!=0))&&(pointPosition_2-1)==i)
			{
				if(((tab[*(number+i+8)]|0x80)>>j)&0x01)
				data[j] +=1;
			}
			else
			{
				if(((tab[*(number+i+8)])>>j)&0x01)
				data[j] +=1;
			}
		}
		writeData(2*j+1,data[j]);
	}
	
}
