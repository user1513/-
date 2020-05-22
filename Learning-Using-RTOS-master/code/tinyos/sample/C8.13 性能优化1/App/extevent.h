#ifndef EXTEVENT_H
#define EXTEVENT_H

#include "Button.h"
#include "extio.h"

#define EXTEVENT_TASK_PRIO          0
#define EXTEVENT_TASK_ENV_SIZE      512

void ExtEventInit (void);
void ExtEventButtonPress (ButtonId id);
void ExtEventExtIOInt (ExtIntIOPin pin);

tTask * ExtEvnetTask (void);

#endif
