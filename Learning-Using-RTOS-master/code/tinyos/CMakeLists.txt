cmake_minimum_required(VERSION 3.7)
project(Project)

set(CMAKE_CXX_STANDARD 11)

include_directories(tinyOS Target Tools)

set(SOURCE_FILES
        Target/target.c
        Target/target.h
        tinyOS/tBitmap.c
        tinyOS/tCore.c
        tinyOS/tEvent.c
        tinyOS/tEvent.h
        tinyOS/tFlagGroup.c
        tinyOS/tFlagGroup.h
        tinyOS/tHooks.h
        tinyOS/tinyOS.h
        tinyOS/tLib.h
        tinyOS/tList.c
        tinyOS/tMBox.c
        tinyOS/tMBox.h
        tinyOS/tMemBlock.c
        tinyOS/tMemBlock.h
        tinyOS/tMutex.c
        tinyOS/tMutex.h
        tinyOS/tSem.c
        tinyOS/tSem.h
        tinyOS/tSlist.c
        tinyOS/tSwitch.c
        tinyOS/tTask.c
        tinyOS/tTask.h
        tinyOS/tTime.c
        tinyOS/tTimer.c
        tinyOS/tTimer.h)

add_executable(Project ${SOURCE_FILES})