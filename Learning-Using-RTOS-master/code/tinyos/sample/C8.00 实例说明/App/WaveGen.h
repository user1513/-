/**
 * @brief 波形生成器实现
 * @details
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef WAVEGEN_H
#define WAVEGEN_H

typedef enum {
    WaveSquare,
    WaveUnknown,
}WaveType;

void WaveGenInit (void);
uint32_t WaveSelectType (WaveType type);
uint32_t WaveSquareSet (uint32_t highMs, uint32_t lowMs);
uint32_t WaveStartOutput (void);
void WaveStopOutput (void);

#endif //WAVEGEN_H
