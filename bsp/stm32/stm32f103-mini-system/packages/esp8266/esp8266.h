#ifndef __ESP8266_H__
#define __ESP8266_H__

#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#define USART_REC_LEN  			200  	//�����������ֽ��� 200

extern rt_uint16_t USART_RX_STA;         		//����״̬���	

#define false 0
#define true 1
int uart2_sample(void);


#endif
