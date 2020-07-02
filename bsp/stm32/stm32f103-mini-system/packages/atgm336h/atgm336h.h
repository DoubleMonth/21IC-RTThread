#ifndef __ATGH336H_H__
#define __ATGH336H_H__

#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
extern char  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern rt_uint16_t USART_RX_STA;         		//接收状态标记	

#define false 0
#define true 1

//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 

typedef struct SaveDataType 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char isGetData;		//是否获取到GPS数据
	char isParseData;	//是否解析完成
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//定位信息是否有效
} _SaveData;

void printGpsBuffer(void);
void parseGpsBuffer(void);
void clrStruct(void);
int uart2_sample(void);
#endif
