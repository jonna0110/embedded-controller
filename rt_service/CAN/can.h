#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include <linux/can.h>

int can_init(const char *ifname);
int can_read(struct can_frame *frame);
void can_send(uint32_t id, uint8_t *data, uint8_t dlc);

#endif
