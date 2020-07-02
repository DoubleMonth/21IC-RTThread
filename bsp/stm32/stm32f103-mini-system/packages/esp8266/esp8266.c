#include "esp8266/esp8266.h"
rt_uint16_t point1 = 0;
rt_uint16_t USART_RX_STA=0;       //接收状态标记	 
static struct rt_semaphore rx_sem;
static rt_device_t esp8266_serial;
char  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
#define SAMPLE_UART_NAME "uart2"
static rt_err_t uart2_input(rt_device_t dev,rt_size_t size)
{
	if(size>0)
	{
		rt_sem_release(&rx_sem);
	}
	return RT_EOK;
}
char cldstart[]={"AT+CLDSTART\r\n"};  //启动云服务
char rst[]={"AT+RST\r\n"}; //复位模块/停止云服务
char cldstate[]={"AT+CLDTATE\r\n"}; //复位模块/停止云服务
char cldtime[]={"AT+CLDTIME\r\n"}; //获取时间
char linkwifi[]={"AT+CWJAP=\"TP-LINK_B4DE\",\"jiuhe431102.\"\r\n"};   //连接WIFI

/*数据解析线程*/
static void thread_wifi_link(void *parameter)
{
	rt_thread_mdelay(3000);
	rt_device_write(esp8266_serial,0,rst,(sizeof(rst)-1));
	rt_thread_mdelay(3000);
	rt_device_write(esp8266_serial,0,linkwifi,(sizeof(linkwifi)-1));
	rt_thread_mdelay(5000);
	rt_device_write(esp8266_serial,0,cldstart,(sizeof(cldstart)-1));
	rt_thread_mdelay(5000);
	rt_device_write(esp8266_serial,0,cldtime,(sizeof(cldtime)-1));
	
}
static void thread_receive_data(void *parameter)
{
	char ch;
	char data[USART_REC_LEN];
	static  char i=0;
	while(1)
	{
		while(rt_device_read(esp8266_serial,0,&ch,1)==0)
		{
			rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, RT_NULL); //重置信号量
			rt_sem_take(&rx_sem,RT_WAITING_FOREVER);
		}
////		rt_device_write(serial ,0,&ch,1);
//		if(ch=='A')
//		{
//			point1=0;
//		}
		USART_RX_BUF[point1++] = ch;
		rt_strcmp(USART_RX_BUF,"WIFI GOT IP");
		{
			rt_kprintf("WIFI haved connected\n");
			point1=0;
			rt_memset(USART_RX_BUF, 0, USART_REC_LEN);      //清空
		}
		
//		if(USART_RX_BUF[0] == 'A' && USART_RX_BUF[1] == 'T' && USART_RX_BUF[3] == '+')			//确定是否收到"GPRMC/GNRMC"这一帧数据
//		{
//			if(ch == '\n')									   
//			{
//				rt_memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
//				rt_memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, point1); 	//保存数据
//				Save_Data.isGetData = true;
//				point1 = 0;
//				rt_memset(USART_RX_BUF, 0, USART_REC_LEN);      //清空				
//			}	
//					
//		}
		if(point1 >= USART_REC_LEN)
		{
			point1 = USART_REC_LEN;
		}	
	}	
}
int uart2_sample(void)
{
	rt_err_t ret=RT_EOK;
	struct serial_configure config=RT_SERIAL_CONFIG_DEFAULT;
	char uart_name[RT_NAME_MAX];
	char str[]="Hello RT-Thread!\r\n";
	
	esp8266_serial = rt_device_find("uart2");
	if(!esp8266_serial)
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
	rt_device_open(esp8266_serial,RT_DEVICE_FLAG_INT_RX);
	/* 设置波特率等参数 */
	config.baud_rate=BAUD_RATE_115200;
	config.data_bits=DATA_BITS_8;
	config.stop_bits=STOP_BITS_1;
	config.parity=PARITY_NONE;
	rt_device_control(esp8266_serial,RT_DEVICE_CTRL_CONFIG,&config);
	/* 设置回调函数 */
	rt_device_set_rx_indicate(esp8266_serial,uart2_input);
	/* 发送字符 */
	rt_thread_t td_link_wifi=rt_thread_create("serial",thread_wifi_link,RT_NULL,1024,25,10);
	rt_thread_t td_receive_data=rt_thread_create("serial",thread_receive_data,RT_NULL,1024,25,10);
	if(td_receive_data != RT_NULL)
	{
		rt_thread_startup(td_receive_data);
	}
	else
	{
		ret=RT_ERROR;
	}
	if(td_link_wifi != RT_NULL)
	{
		rt_thread_startup(td_link_wifi);
	}
	else
	{
		ret=RT_ERROR;
	}
	
	return ret;
}

MSH_CMD_EXPORT(uart2_sample,uart device sample);