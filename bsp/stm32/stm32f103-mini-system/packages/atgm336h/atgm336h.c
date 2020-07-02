#include "atgm336h/atgm336h.h"
#include "pcf8563/pcf8563.h"
char rxdatabufer;
rt_uint16_t point1 = 0;
_SaveData  Save_Data;

char USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
rt_uint16_t USART_RX_STA=0;       //接收状态标记	 
static struct rt_semaphore rx_sem;
static rt_device_t serial;
#define SAMPLE_UART_NAME "uart2"
static rt_err_t uart2_input(rt_device_t dev,rt_size_t size)
{
	if(size>0)
	{
		rt_sem_release(&rx_sem);
	}
	return RT_EOK;
}
/*数据解析线程*/
static void serial_thread_entry(void *parameter)
{
	char ch;
	char data[USART_REC_LEN];
	static  char i=0;
	while(1)
	{
		while(rt_device_read(serial,0,&ch,1)==0)
		{
			rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, RT_NULL); //重置信号量
			rt_sem_take(&rx_sem,RT_WAITING_FOREVER);
		}
//		rt_device_write(serial ,0,&ch,1);
		if(ch=='$')
		{
			point1=0;
		}
		USART_RX_BUF[point1++] = ch;
		if(USART_RX_BUF[0] == '$' && USART_RX_BUF[4] == 'M' && USART_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
		{
			if(ch == '\n')									   
			{
				rt_memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
				rt_memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, point1); 	//保存数据
				Save_Data.isGetData = true;
				point1 = 0;
				rt_memset(USART_RX_BUF, 0, USART_REC_LEN);      //清空				
			}	
					
		}
		if(point1 >= USART_REC_LEN)
		{
			point1 = USART_REC_LEN;
		}	
	}
}
static void receive_gps(void *parameter)
{
	while(1)
	{
		parseGpsBuffer();
		printGpsBuffer();
	}
}
int uart2_sample(void)
{
	rt_err_t ret=RT_EOK;
	struct serial_configure config=RT_SERIAL_CONFIG_DEFAULT;
	char uart_name[RT_NAME_MAX];
	char str[]="Hello RT-Thread!\r\n";
	
	serial = rt_device_find("uart2");
	if(!serial)
	{
		rt_kprintf("find %s failed!\n",uart_name);
		return RT_ERROR;
	}
	else
	{
		rt_kprintf("find %s ok!\n",uart_name);
	}
	/*初始化信号量*/
	rt_sem_init(&rx_sem,"rx_sem",0,RT_IPC_FLAG_FIFO);
	/* 以中断接收及轮询发送模式打开串口设备 */
	rt_device_open(serial,RT_DEVICE_FLAG_INT_RX);
	/* 设置波特率等参数 */
	config.baud_rate=BAUD_RATE_9600;
	config.data_bits=DATA_BITS_8;
	config.stop_bits=STOP_BITS_1;
	config.parity=PARITY_NONE;
	rt_device_control(serial,RT_DEVICE_CTRL_CONFIG,&config);
	/* 设置回调函数 */
	rt_device_set_rx_indicate(serial,uart2_input);
	/* 发送字符 */
	rt_device_write(serial,0,str,(sizeof(str)-1));
	rt_thread_t thread=rt_thread_create("serial",serial_thread_entry,RT_NULL,1024,25,10);
	rt_thread_t thread_receive_gps=rt_thread_create("receive_gps",receive_gps,RT_NULL,256,25,10);  //建立一个线程
	if(thread != RT_NULL)
	{
		rt_thread_startup(thread);
	}
	else
	{
		ret=RT_ERROR;
	}
	if(thread_receive_gps != RT_NULL)
	{
		rt_thread_startup(thread_receive_gps);
	}
	else
	{
		ret=RT_ERROR;
	}
	return ret;
}


void clrStruct(void)
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	rt_memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
	rt_memset(Save_Data.UTCTime, 0, UTCTime_Length);
	rt_memset(Save_Data.latitude, 0, latitude_Length);
	rt_memset(Save_Data.N_S, 0, N_S_Length);
	rt_memset(Save_Data.longitude, 0, longitude_Length);
	rt_memset(Save_Data.E_W, 0, E_W_Length);
}
struct pcf8563_time UTC_time;
rt_uint8_t update_UTC_time_Flag=0;
void parseGpsBuffer(void)
{
	char *subString;
	char *subStringNext;
	char i = 0;
	
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		rt_kprintf("**************\r\n");
		rt_kprintf(Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = rt_strstr(Save_Data.GPS_Buffer, ",")) == NULL)  //判断接收到的数据中是否有","。
					rt_kprintf("ERROR_1\n");	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = rt_strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:
						{
							rt_memcpy(Save_Data.UTCTime, subString, subStringNext - subString);
							UTC_time.hour=(Save_Data.UTCTime[0]-48)*10+(Save_Data.UTCTime[1]-48)+8;
							UTC_time.minute=(Save_Data.UTCTime[2]-48)*10+(Save_Data.UTCTime[3]-48);
							UTC_time.second=(Save_Data.UTCTime[4]-48)*10+(Save_Data.UTCTime[5]-48);
//							if((UTC_time.hour==23) && (UTC_time.minute ==0) && (UTC_time.second ==0))
//							{
								update_UTC_time_Flag=1;
//							}
						}
						break;	//获取UTC时间
						case 2:rt_memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:rt_memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:rt_memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:rt_memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:rt_memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')  //定位
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')	//导航
						Save_Data.isUsefull = false;
				}
				else
				{
					rt_kprintf("ERROR_2\n");	//解析错误
				}
			}
		}
	}
}

void printGpsBuffer(void)
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		rt_kprintf("Save_Data.UTCTime = ");
		rt_kprintf(Save_Data.UTCTime);
		rt_kprintf("\r\n");

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
			rt_kprintf("Save_Data.latitude = ");
			rt_kprintf(Save_Data.latitude);
			rt_kprintf("\r\n");


			rt_kprintf("Save_Data.N_S = ");
			rt_kprintf(Save_Data.N_S);
			rt_kprintf("\r\n");

			rt_kprintf("Save_Data.longitude = ");
			rt_kprintf(Save_Data.longitude);
			rt_kprintf("\r\n");

			rt_kprintf("Save_Data.E_W = ");
			rt_kprintf(Save_Data.E_W);
			rt_kprintf("\r\n");
		}
		else
		{
			rt_kprintf("GPS DATA is not usefull!\r\n");
		}
		
	}
}

MSH_CMD_EXPORT(uart2_sample,uart device sample);
