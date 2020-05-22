//
// Created by lishutong on 2018/2/5.
//

#ifndef PROJECT_APPCONFIG_H
#define PROJECT_APPCONFIG_H

#include "hal.h"


//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(str, arg)
#else
#define DEBUG_PRINT(str, arg) xprintf(str, arg)
#endif

#endif //PROJECT_APPCONFIG_H
