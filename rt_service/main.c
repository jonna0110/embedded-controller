#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/time.h>

#include "../shared_memory/process_image.h"
#include "../CAN/can.h"
#include "../Alarms/alarm.h"

#define ERR_CAN_LOST (1u << 0)
#define CAN_ID_INPUTS 0x100
#define CAN_ID_OUTPUTS 0x200

#define ALARM_CAN_LOST   0
#define ALARM_EMERGENCY  1
#define ALARM_STATE_MACHINE_ERROR 2

#define DI_STARTBTN  (1 << 0)
#define DI_STOPBTN  (1 << 1)
#define DI_EMERGENCY  (1 << 2)

#define DO_MOTOR_ON (1 << 0)



typedef enum {
    STATE_IDLE = 0,
    STATE_STARTING = 10,
    STATE_RUNNING = 100,
    STATE_ERROR = 900
} machine_state_t;

void handle_can_frame(process_image_t *p, struct can_frame *f) {

    switch (f->can_id) {

        case CAN_ID_INPUTS:
            p->di = f->data[0];
            p->ai = f->data[1];
            break;

        default:
            break;
    }
}

void sleep_until_next_cycle() {

struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    ts.tv_nsec += 100000000; // 100 ms

    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);

    }


void state_machine(process_image_t *p) {

    switch (p->state) {

        case STATE_IDLE:
            clear_alarm(p, ALARM_STATE_MACHINE_ERROR);

            if (p->di & DI_STARTBTN) {
                p->state = STATE_STARTING;
            }
            break;

        case STATE_STARTING:
            p->do_ |= DO_MOTOR_ON; // Tænd motor
            p->state = STATE_RUNNING;
            break;

        case STATE_RUNNING:
            if (p->di & DI_STOPBTN) {
                p->do_ &= ~DO_MOTOR_ON; // Sluk motor
                p->state = STATE_IDLE;
            }
            break;

        case STATE_ERROR:
            // Håndter fejl
            set_alarm(p, ALARM_STATE_MACHINE_ERROR);
            
            if (p->di & DI_STOPBTN) {
                p->state = STATE_IDLE;
            }

            break;

        default:
            break;
    }
}

// Compile=> gcc rt_service/main.c -o rt_service/rt
// Compile with can.c => gcc rt_service/main.c rt_service/can.c -o rt_service/rt
// Run => ./rt_service/rt

int main() {
    shm_unlink(SHM_NAME);  // Clear any old data
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(process_image_t));

    process_image_t *p = mmap(NULL, sizeof(process_image_t),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    srand(time(NULL));

    can_init("vcan0");

    struct can_frame frame;

    while (1) {

        p->version++;

   // 🔽 READ CAN
    while (can_read(&frame) > 0) {

        printf("RX ID: 0x%X DLC: %d\n", frame.can_id, frame.can_dlc);

        if (frame.can_id == CAN_ID_INPUTS) {
            // Opdater watchdog
            p->last_can_rx = p->heartbeat;
            
           handle_can_frame(p, &frame);

        }
    }
  

    /*   // Simuler inputs (senere CAN)
        p->di = rand() % 256;
        p->ai = rand() % 1000;
    */


  // timeout
    if ((p->heartbeat - p->last_can_rx) > 20) {
        set_alarm(p, ALARM_CAN_LOST); // Sæt alarm
    } 
    else {
        clear_alarm(p, ALARM_CAN_LOST); // Ryd alarm
    }

   // Emergency stop logik
    if (p->di & DI_EMERGENCY) {
        set_alarm(p, ALARM_EMERGENCY); // Sæt alarm
    } 
    else {
        clear_alarm(p, ALARM_EMERGENCY); // Ryd alarm
    }





    if (p->alarm_active != 0) {
        p->do_ = 0;  // fail safe
    }
    else{
    
        // 🔧 LOGIK
        if (p->di & 1) {
            p->do_ |= 1;
        } else {
            p->do_ &= ~1;
        }

    }

    
    state_machine(p);
    printf("STATE: %d DI: %d DO: %d\n", p->state, p->di, p->do_);


    // 🔼 WRITE CAN
    uint8_t tx[2];
    tx[0] = p->do_ & 0xFF;
    tx[1] = (p->do_ >> 8) & 0xFF;

    can_send(CAN_ID_OUTPUTS, tx, 2);
        p->heartbeat++;
        
        p->version_end = p->version;
           
        //printf("RT: Time=%ld DI=%d DO=%d AI=%d Watchdog=%d\n", (long int)time(NULL), p->di, p->do_, p->ai, p->heartbeat);

        //usleep(100000); // 100 ms
    
    sleep_until_next_cycle();
    
}

}