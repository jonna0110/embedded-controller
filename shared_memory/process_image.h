#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#include <stdint.h>

typedef struct {
    uint32_t version;
    uint32_t timestamp_ms;
    
    // CAN Communication
    struct {
        uint16_t rx_count;
        uint16_t tx_count;
        uint16_t error_count;
        uint32_t last_rx_time;
        
        struct {
            uint16_t di;      // Digital inputs from CAN ID 0x100
            uint16_t ai_1;      // Analog inputs from CAN ID 0x100
        } in;
        
        struct {
            uint16_t do_;     // Digital outputs to CAN ID 0x200
            uint16_t ao_1;     // Analog outputs to CAN ID 0x200
        } out;

    } can;

   // 🧠 config data
    struct {
        float setpoint;
        float kp, ki, kd;
        uint16_t cycle_time_ms;
    } config;

   // 🧠 Control data
    struct {
        float pid_output;

        uint32_t state;
    
        uint32_t alarm_active;
        uint32_t alarm_latched;

    } app;

    struct {
        uint32_t exec_time;
        uint32_t max_exec_time;
        int32_t jitter;
        int32_t max_jitter;
        uint32_t deadline_miss;
    } metrics;


    uint32_t version_end;

} process_image_t;

#define SHM_NAME "/process_image"

#endif