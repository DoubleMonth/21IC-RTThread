#ifndef __TM1638_H_
#define __TM1638_H_
#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>

#define DIO_PIN 	GET_PIN(B, 7)
#define CLK_PIN 	GET_PIN(B, 8)
#define STB_PIN 	GET_PIN(B, 9)

void tm1638Init(void);
void tm1638Setluminance(rt_uint8_t lumi);
static void tm1638Write(rt_uint8_t data);
static void writeCMD(rt_uint8_t cmd);
static void writeData(rt_uint8_t add,rt_uint8_t data);
void writeAllLED(rt_uint8_t LED_Flag);
void display(rt_uint8_t *number,rt_uint8_t pointFlash, rt_uint8_t pointPosition_1, rt_uint8_t pointPosition_2);//number为要显示的数字，依次从1到10。

#endif
