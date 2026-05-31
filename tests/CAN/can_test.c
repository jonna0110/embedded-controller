// Simple integration-style unit test for CAN module.
// Requires a Linux host with `vcan0` configured (sudo modprobe vcan; sudo ip link add dev vcan0 type vcan; sudo ip link set up vcan0).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#include "../rt_service/CAN/can.h"
#include <linux/can.h>

int main(void) {
    if (can_init("vcan0") != 0) {
        fprintf(stderr, "can_init failed\n");
        return 2;
    }

    uint8_t tx[2] = {0xAA, 0x55};
    can_send(0x200, tx, 2);

    struct can_frame rx;
    int success = 0;

    // wait up to 1s for the looped-back frame
    for (int i = 0; i < 100; ++i) {
        int n = can_read(&rx);
        if (n > 0) {
            printf("RX ID: 0x%X DLC: %d\n", rx.can_id, rx.can_dlc);
            if (rx.can_id == 0x200 && rx.can_dlc == 2 && rx.data[0] == tx[0] && rx.data[1] == tx[1]) {
                success = 1;
                break;
            }
        }
        usleep(10000); // 10ms
    }

    can_close();

    if (success) {
        printf("CAN test: PASS\n");
        return 0;
    } else {
        printf("CAN test: FAIL\n");
        return 1;
    }
}
