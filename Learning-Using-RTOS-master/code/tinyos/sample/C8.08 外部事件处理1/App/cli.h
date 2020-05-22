/**
 * @brief 命令解释器设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef CLI_H
#define CLI_H

#define CLI_TASK_PRIO                     0
#define CLI_CMD_PROMATE_SIZE            64
#define CLI_TASK_ENV_SIZE               512
#define CLI_CMD_HISTORY_SIZE            1024
#define CLI_CMD_BUFFER_SIZE             128

void CLIInit (void);

#endif //CLI_H
