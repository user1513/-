/**
 * @brief tinyOS 回调接口
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include "tHooks.h"
#include "tinyOS.h"

#if TINYOS_ENABLE_HOOKS == 1

uint32_t idleFlag = 0;

void tHooksCpuIdle (void) {
	idleFlag ^= 0x1;
	
	targetEnterSleep();
}

void tHooksSysTick (void) {

}

void tHooksTaskSwitch (tTask *from, tTask *to) {

}

void tHooksTaskInit (tTask *task) {

}

#endif
