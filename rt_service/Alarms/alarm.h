#ifndef ALARM_H
#define ALARM_H

#define ALARM_CAN_LOST   0
#define ALARM_EMERGENCY  1
#define ALARM_STATE_MACHINE_ERROR 2

#include "../shared_memory/process_image.h"

void set_alarm(process_image_t *p, int id);
void clear_alarm(process_image_t *p, int id);
void ack_alarm(process_image_t *p, int id);

#endif
