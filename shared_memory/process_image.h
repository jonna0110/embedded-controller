#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#include <stdint.h>

typedef struct {
    uint32_t version;

// 🔥 RT: process image
    struct {

        uint16_t di;
        uint16_t ai;

        uint16_t do_;
        
        uint32_t heartbeat;
        
        uint32_t last_can_rx;
        
        uint32_t error_flags;
    } io;

   // 🧠 config data
    struct {
        float setpoint;
        float kp, ki, kd;

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
    } rt_stats;


    uint32_t version_end;

} process_image_t;

#define SHM_NAME "/process_image"

#endif