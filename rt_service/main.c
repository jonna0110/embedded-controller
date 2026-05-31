#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#include "../shared_memory/process_image.h"
#include "../CAN/can.h"
#include "../Alarms/alarm.h"

#define cycle_time_ms 10
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


// monotonic time in milliseconds
static uint64_t monotonic_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

// Pack outputs into a CAN payload. Returns DLC used.
static int pack_outputs(process_image_t *p, uint8_t *tx) {
    tx[0] = p->can.out.do_ & 0xFF;
    tx[1] = (p->can.out.do_ >> 8) & 0xFF;
    return 2;
}

// Unpack CAN input payload into the process image.
static void unpack_inputs(process_image_t *p, const uint8_t *data, uint8_t dlc) {
    if (dlc >= 1) p->can.in.di = data[0];
    if (dlc >= 2) p->can.in.ai_1 = data[1];
}

// Watchdog configuration
#define CAN_TIMEOUT_MS 500
#define CAN_RECONNECT_INTERVAL_MS 2000

void handle_can_frame(process_image_t *p, struct can_frame *f) {
    switch (f->can_id) {
        case CAN_ID_INPUTS:
            unpack_inputs(p, f->data, f->can_dlc);
            break;
        default:
            break;
    }
}

void sleep_until_next_cycle() {

    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    ts.tv_nsec += cycle_time_ms * 1000000; // cycle time in nanoseconds

    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);

    }


void state_machine(process_image_t *p) {
    switch (p->app.state) {
        case STATE_IDLE:
            clear_alarm(p, ALARM_STATE_MACHINE_ERROR);

            if (p->can.in.di & DI_STARTBTN) {
                p->app.state = STATE_STARTING;
            }
            break;

        case STATE_STARTING:
            p->can.out.do_ |= DO_MOTOR_ON; // Tænd motor
            p->app.state = STATE_RUNNING;
            break;

        case STATE_RUNNING:
            if (p->can.in.di & DI_STOPBTN) {
                p->can.out.do_ &= ~DO_MOTOR_ON; // Sluk motor
                p->app.state = STATE_IDLE;
            }
            break;

        case STATE_ERROR:
            // Håndter fejl
            set_alarm(p, ALARM_STATE_MACHINE_ERROR);
            if (p->can.in.di & DI_STOPBTN) {
                p->app.state = STATE_IDLE;
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
            // Update watchdog timestamp + counters
            p->can.last_rx_time = monotonic_ms();
            p->can.rx_count++;
            handle_can_frame(p, &frame);
        }
    }
  

    /*   // Simuler inputs (senere CAN)
        p->di = rand() % 256;
        p->ai = rand() % 1000;
    */


    // CAN watchdog (monotonic ms)
    {
        static uint64_t last_reconnect_attempt = 0;
        uint64_t now = monotonic_ms();
        uint64_t last_rx = p->can.last_rx_time;

        if (last_rx == 0) last_rx = now; // initialize

        if ((now - last_rx) > CAN_TIMEOUT_MS) {
            set_alarm(p, ALARM_CAN_LOST); // Sæt alarm
            p->can.out.do_ = 0; // fail-safe outputs
            p->can.error_count++;

            if ((now - last_reconnect_attempt) >= CAN_RECONNECT_INTERVAL_MS) {
                last_reconnect_attempt = now;
                can_close();
                if (can_init("vcan0") == 0) {
                    // keep alarm until frames resume, but clear transient errors if any
                }
            }
        } else {
            clear_alarm(p, ALARM_CAN_LOST); // Ryd alarm
        }
    }

   // Emergency stop logik
    if (p->can.in.di & DI_EMERGENCY) {
        set_alarm(p, ALARM_EMERGENCY); // Sæt alarm
    } else {
        clear_alarm(p, ALARM_EMERGENCY); // Ryd alarm
    }

    if (p->app.alarm_active != 0) {
        p->can.out.do_ = 0;  // fail safe
    } else {
        // 🔧 LOGIK
        if (p->can.in.di & 1) {
            p->can.out.do_ |= 1;
        } else {
            p->can.out.do_ &= ~1;
        }
    }

    
    state_machine(p);
    printf("STATE: %d DI: %d DO: %d\n", p->app.state, p->can.in.di, p->can.out.do_);


    // 🔼 WRITE CAN
    uint8_t tx[8];

    int tx_dlc = pack_outputs(p, tx);
    can_send(CAN_ID_OUTPUTS, tx, tx_dlc);
    p->can.tx_count++;
        
        p->version_end = p->version;
    
    sleep_until_next_cycle();
    
}

}