#ifndef __KEY_H_
#define __KEY_H_
#include <rtthread.h>
#include "FlexibleButton/flexible_button.h"

#define IOTB_KEY_SCAN_CYCLE (20)
rt_err_t iotb_key_process_start(void);
void iotb_key_thr_set_cycle(uint16_t time);
void iotb_btn_scan_enable(void);
void iotb_btn_scan_disable(void);
#endif
