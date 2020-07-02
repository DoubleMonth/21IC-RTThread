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
	
	writeCMD(0x8B);		//��������
	writeCMD(0x40);		//���õ�ַ�Զ���1
	rt_pin_write(STB_PIN,PIN_LOW);
	tm1638Write(0xC0);		//������ʼ��ַ
	for(i=0;i<16;i++)	//��ʼȫ������ʾ
	{
		tm1638Write(0x00);
	}
	rt_pin_write(STB_PIN,PIN_HIGH);
}
void tm1638Setluminance(rt_uint8_t lumi)
{
	writeCMD(lumi);		//��������
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
	writeCMD(0x44);//��ͨģʽ���̶���ַ
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
* @number ��Ҫ��ʾ������
* @pointFlash ����˸��־��1ʱ����0ʱ��
* @pointPosition_1 ��Ҫ��ʾ��С�����λ�á�0������ʾС���㣻4���¶�1λ�� 5���¶�2λ�� 6���¶�3λ�� 7��ʪ��1λ�� 
* @pointPosition_2 ��Ҫ��ʾ��С�����λ�á�0������ʾС���㣻1��ʪ��2λ��2��ʪ��3λ�� 
********************************/
void display(rt_uint8_t *number,rt_uint8_t pointFlash, rt_uint8_t pointPosition_1, rt_uint8_t pointPosition_2)//numberΪҪ��ʾ�����֣����δ�1��10��
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
