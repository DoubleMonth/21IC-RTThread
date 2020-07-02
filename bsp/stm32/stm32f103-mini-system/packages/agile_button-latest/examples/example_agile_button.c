#include <agile_button.h>
#include <drv_gpio.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif
#include "sensor/sensor.h"
//#define WK_UP_KEY_PIN  GET_PIN(A, 0)
#define KEY0_PIN       GET_PIN(B, 4)
#define KEY1_PIN       GET_PIN(B, 5)

//static agile_btn_t *wk_up_key = RT_NULL;
static agile_btn_t *key0 = RT_NULL;
static agile_btn_t *key1 = RT_NULL;

rt_sem_t set_short_state;   //set 按键短按状态 1:按下,0:未按下。
rt_sem_t set_long_state;		//set 按键长按状态 1:按下,0:未按下。
rt_sem_t add_short_state;	//add 按键短按状态 1:按下,0:未按下。
rt_sem_t add_long_state;		//add 按键长按状态 1:按下,0:未按下。



static void btn_click_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button click event] pin:%d   repeat:%d, hold_time:%d\r\n", btn->pin, btn->repeat_cnt, btn->hold_time);
	if (btn->pin == KEY0_PIN)
	{
		if(RT_EOK == rt_sem_release(add_short_state))  //释放ADD信号量
		{
			rt_kprintf("Release ADD KEY SEM\n");
		} 
		
	}
	else if (btn->pin == KEY1_PIN)
	{
		if(RT_EOK == rt_sem_release(set_short_state) ) //释放SET信号量
		{
			rt_kprintf("Release SET KEY sem\n");
		}
	}
		
	
}
void keySemInit(void)
{
	set_short_state=rt_sem_create("setShort",0,RT_IPC_FLAG_FIFO);
	set_long_state=rt_sem_create("setLong",0,RT_IPC_FLAG_FIFO);
	add_short_state=rt_sem_create("addShort",0,RT_IPC_FLAG_FIFO);
	add_long_state=rt_sem_create("addLong",0,RT_IPC_FLAG_FIFO);
}
static void btn_hold_event_cb(agile_btn_t *btn)
{
    rt_kprintf("[button hold event] pin:%d   hold_time:%d\r\n", btn->pin, btn->hold_time);
}

static void key_create(void)
{
    if(key0 == RT_NULL)
    {
        key0 = agile_btn_create(KEY0_PIN, PIN_LOW, PIN_MODE_INPUT_PULLUP);
        agile_btn_set_event_cb(key0, BTN_CLICK_EVENT, btn_click_event_cb);
        agile_btn_set_event_cb(key0, BTN_HOLD_EVENT, btn_hold_event_cb);
        agile_btn_start(key0);
    }

    if(key1 == RT_NULL)
    {
        key1 = agile_btn_create(KEY1_PIN, PIN_LOW, PIN_MODE_INPUT_PULLUP);
        agile_btn_set_event_cb(key1, BTN_CLICK_EVENT, btn_click_event_cb);
        agile_btn_set_event_cb(key1, BTN_HOLD_EVENT, btn_hold_event_cb);
        agile_btn_start(key1);
    }
}
static void key_delete(void)
{
    if(key0)
    {
        agile_btn_delete(key0);
        key0 = RT_NULL;
    }

    if(key1)
    {
        agile_btn_delete(key1);
        key1 = RT_NULL;
    }
}
void startKeyPress(void)
{
	key_create();
}
//#ifdef RT_USING_FINSH
//MSH_CMD_EXPORT(key_create, create key);
//MSH_CMD_EXPORT(key_delete, delete key);
//#endif
