#ifndef ALARM_H
#define ALARM_H

#include "../shared_memory/process_image.h"

void set_alarm(process_image_t *p, int id);
void clear_alarm(process_image_t *p, int id);
void ack_alarm(process_image_t *p, int id);

#endif
