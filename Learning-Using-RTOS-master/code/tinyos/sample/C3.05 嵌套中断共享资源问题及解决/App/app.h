/**
 * @brief tOS应用示例
 * @details
 * @author 01课堂 lishutong
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef PROJECT_APP_H_H
#define PROJECT_APP_H_H

#include <stdio.h>
#include "tinyOS.h"
#include "hal.h"

#define TASK1_ENV_SIZE              512             // Task1的堆栈空间单元数
#define TASK2_ENV_SIZE              512             // Task2的堆栈空间单元数
#define TASK3_ENV_SIZE              512             // Task3的堆栈空间单元数
#define TASK4_ENV_SIZE              512             // Task4的堆栈空间单元数

#define TASK1_PRIO                   1              // Task1的任务优先级
#define TASK2_PRIO                   2              // Task2的任务优先级
#define TASK3_PRIO                   3              // Task3的任务优先级
#define TASK4_PRIO                   4              // Task4的任务优先级

// 各个任务
void task1Entry (void *param);

void task2Entry (void *param);

void task3Entry (void *param);

void task4Entry (void *param);

#endif //PROJECT_APP_H_H
