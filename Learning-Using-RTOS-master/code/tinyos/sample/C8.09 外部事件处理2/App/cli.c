/**
 * @brief 命令解释器设计
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#include <string.h>
#include "tinyOS.h"
#include "cli.h"
#include "uart.h"
#include "extio.h"
#include "WaveGen.h"

#define TERMINAL_BS     0x08
#define TERMINAL_SPACE  ' '
#define TERMINAL_CR     '\r'
#define TERMINAL_LF     '\n'
#define TERMINAL_TAB    '\t'

// 空白字符
static const char spaceCh[] = {TERMINAL_SPACE, TERMINAL_CR, TERMINAL_LF, TERMINAL_TAB, '\0'};
static const char * unknownPinMsg = "Unknown Pin\r\n";
static const char * unknownCmdMsg = "Unknown Command!\r\n";
static const char * noEnoughParamMsg = "No Enough Param!\r\n";

// 任务相关
static tTaskStack cliTaskEnv[CLI_TASK_ENV_SIZE];     // 任务1的堆栈空间
static tTask cliTask;

// 命令行提示缓冲区
static char promoteBuffer[CLI_CMD_PROMATE_SIZE];

/**
 * 显示消息
 */
static void showMsg (const char * msg) {
    UartWrite(msg, strlen(msg));
}

/**
 * 输出显示字符
 * @param ch
 */
static void showCh (const char ch) {
    UartWrite(&ch, 1);
}

/**
 * 显示提示符
 */
static void showPromote (void) {
    UartWrite(promoteBuffer, strlen(promoteBuffer));
}

/**
 * 更改提示符
 */
static void setPromote (const char * newPromote) {
    strncpy(promoteBuffer, newPromote, sizeof(promoteBuffer));
    promoteBuffer[sizeof(promoteBuffer) - 1] = '\0';
}

/**
 * 提示符修改命令
 */
static void promoteCmd (void) {
    char * promote = strtok(NULL, spaceCh);
    if (promote == NULL) {
        showMsg(noEnoughParamMsg);
        return;
    }

    setPromote(promote);
}

/**
 * 将引脚名转换为内部序号
 * @param pinCh
 * @return
 */
static ExtIOPin convertPinNum (char * pinCh) {
    ExtIOPin pinNum;

    if (pinCh == NULL) {
        return ExtIOPinEnd;
    }

    pinNum = (ExtIOPin)(*pinCh - '0');
    if (pinNum >= ExtIOPinEnd) {
        return ExtIOPinEnd;
    }

    return pinNum;
}

/**
 * 外部IO命令解析
 */
static void extioCmd () {
    char * type = strtok(NULL, spaceCh);
    if (type == NULL) {
        showMsg(noEnoughParamMsg);
        return;
    }

    if (strstr(type, "get")) {      // 命令extio get pin
        ExtIOPin pin;
        ExtIOState state;

        pin = convertPinNum(strtok(NULL, spaceCh));
        if (pin == ExtIOPinEnd) {
            showMsg(unknownPinMsg);
            return;
        }

        state = ExtIOGetState(pin);
        showMsg((state == ExtIOHigh) ? "1\r\n" : "0\r\n");
    } else if (strstr(type, "set")) {   // 命令extio set pin value
        ExtIOPin pin;
        char * value;

        pin = convertPinNum(strtok(NULL, spaceCh));
        if (pin == ExtIOPinEnd) {
            showMsg(unknownPinMsg);
            return;
        }

        value = strtok(NULL, spaceCh);
        if (value == NULL) {
            showMsg(noEnoughParamMsg);
            return;
        }

        ExtIOSetState(pin, *value == '0' ? ExtIOLow : ExtIOHigh);
    } else if (strstr(type, "dir")) {   // 命令extio dir pin in/out
        ExtIOPin pin;
        char *outType;

        pin = convertPinNum(strtok(NULL, spaceCh));
        if (pin == ExtIOPinEnd) {
            showMsg(unknownPinMsg);
            return;
        }

        outType = strtok(NULL, spaceCh);
        if (outType == NULL) {
            showMsg(noEnoughParamMsg);
            return;
        }

        ExtIOSetDir(pin, strstr(outType, "in") ? 1 : 0);
    } else {
        showMsg(noEnoughParamMsg);
    }
}

/**
 * 波形输出命令解析
 */
static void waveCmd () {
    char *type = strtok(NULL, spaceCh);
    if (type == NULL) {
        showMsg(noEnoughParamMsg);
        return;
    }

    if (strstr(type, "square")) {      // 命令wave square
        WaveSelectType(WaveSquare);
    } else if (strstr(type, "start")) {
        WaveStartOutput();
    } else if (strstr(type, "stop")) {
        WaveStopOutput();
    } else {
        showMsg(noEnoughParamMsg);
    }
}

/**
 * 未知命令处理
 */
static void unknowCmd (void) {
    showMsg(unknownCmdMsg);
}

/**
 * 读取一行数据，如果命令超过则截断
 */
static void readLine (char * buffer, uint32_t maxLen) {
    uint32_t index = 0;

    while (index < maxLen) {
        char ch;

        UartRead(&ch, 1);
        switch (ch) {
            case TERMINAL_BS:   // 退格键
                if (index > 0) {
                    buffer[index--] = '\0';
                    showCh(TERMINAL_BS);
                    showCh(TERMINAL_SPACE);
                    showCh(TERMINAL_BS);
                }
                break;
            case TERMINAL_CR:
                showCh(TERMINAL_LF);
            default:
                showCh(ch);

                buffer[index++] = ch;
                if ((ch == '\n') || (ch == '\r') || (index >= maxLen)) {
                    buffer[index] = '\0';
                    return;
                }
                break;
        }
    }
}

/**
 * 解析命令
 */
static void processCmd (char * cmdLine) {
    char * cmdStart;

    // 获取开头
    cmdStart = strtok(cmdLine, spaceCh);
    if (cmdStart == NULL) {
        return;
    }

    // 识别命令
    if (strstr(cmdStart, "extio")) {
        extioCmd();
    } else if (strstr(cmdStart, "wave")) {
        waveCmd();
    } else if (strstr(cmdStart, "promote")){
        promoteCmd();
    } else {
        unknowCmd();
    }
}

/**
 * 解释器任务
 * @param param
 */
void cliTaskEntry (void * param) {
    static char cmdBuffer[CLI_CMD_BUFFER_SIZE];

    for (;;) {
        showPromote();
        readLine(cmdBuffer, sizeof(cmdBuffer));
        processCmd(cmdBuffer);
    }
}

/**
 * 命令解释器设计
 */
void CLIInit (void) {
    strcpy(promoteBuffer, ">>");
    tTaskInit(&cliTask, cliTaskEntry, (void *) 0x0, CLI_TASK_PRIO, cliTaskEnv, sizeof(cliTaskEnv));
}

