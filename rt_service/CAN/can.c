#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
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
    if (can_socket < 0) {
        perror("can: socket");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0) {
        perror("can: ioctl");
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("can: bind");
        close(can_socket);
        can_socket = -1;
        return -1;
    }

    // non-blocking
    if (fcntl(can_socket, F_SETFL, O_NONBLOCK) < 0) {
        perror("can: fcntl");
        close(can_socket);
        can_socket = -1;
        return -1;
    }

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

int can_close(void) {
    if (can_socket >= 0) {
        close(can_socket);
        can_socket = -1;
    }
    return 0;
}
