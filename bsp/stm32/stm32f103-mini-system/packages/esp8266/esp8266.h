#ifndef __ESP8266_H__
#define __ESP8266_H__

#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#define USART_REC_LEN  			200  	//定义最大接收字节数 200

extern rt_uint16_t USART_RX_STA;         		//接收状态标记	

#define false 0
#define true 1
int uart2_sample(void);


#endif
