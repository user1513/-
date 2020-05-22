#ifndef MONITOR_H
#define MONITOR_H

#define MONITOR_TASK_PRIO           3
#define MONITOR_TASK_ENV_SIZE       512
#define MONITOR_MAX_CMD             10
#define MONITOR_PERIOD              100
#define REPORT_BUFFER_SIZE          128

typedef enum {
    MonitorCPUUsage,
    MonitorExtIOPin,
}MonitorTarget;

typedef struct _MonitorCmd {
    tNode linkNode;
    
    uint8_t isAddr;
    MonitorTarget target;
    
    union MonitorOption {
        struct {
            uint8_t warning;
            float warnPercent;
        }cpu;
        
        struct {
            uint8_t pinNum;
        }pin;
    }options;
    
}MonitorCmd;

tTask * MonitorTask (void);

void MonitorInit (void);
void MonitorOn (void);
void MonitorOff (void);
uint8_t MonitorIsOn (void);

MonitorCmd * MonitorAllocCmd (void);
void MonitorSendCmd (MonitorCmd * cmd);
void MonitorFreeCmd (MonitorCmd * cmd);

#endif
