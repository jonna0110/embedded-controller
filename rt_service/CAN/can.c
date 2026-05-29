#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "can.h"

int can_socket;

int can_init(const char *ifname) {
    struct sockaddr_can addr;
    struct ifreq ifr;

    can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, ifname);
    ioctl(can_socket, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(can_socket, (struct sockaddr *)&addr, sizeof(addr));

    // non-blocking
    fcntl(can_socket, F_SETFL, O_NONBLOCK);

    return 0;
}


int can_read(struct can_frame *frame) {
    return read(can_socket, frame, sizeof(struct can_frame));
}


void can_send(uint32_t id, uint8_t *data, uint8_t dlc) {
    struct can_frame frame;

    frame.can_id = id;
    frame.can_dlc = dlc;
    memcpy(frame.data, data, dlc);

    write(can_socket, &frame, sizeof(frame));
}
