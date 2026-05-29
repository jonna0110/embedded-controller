#include "../shared_memory/process_image.h"

void set_alarm(process_image_t *p, int id) {
    p->alarm_active |= (1 << id);    // aktiv nu
    p->alarm_latched |= (1 << id);   // husk alarm
}

void clear_alarm(process_image_t *p, int id) {
    p->alarm_active &= ~(1 << id);   // fjern aktiv
}

void ack_alarm(process_image_t *p, int id) {
    p->alarm_latched &= ~(1 << id);
}
