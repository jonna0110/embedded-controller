#include "rt_profiler.h"
#include <time.h>

static uint64_t start_time;
static uint64_t last_cycle_time;

static uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void rt_profiler_init(rt_stats_t *s) {
    s->min_exec_time_us = 999999;
    s->max_exec_time_us = 0;
    s->max_jitter_us = 0;
    s->deadline_miss = 0;

    last_cycle_time = get_time_ns();
}

void rt_profiler_start() {
    start_time = get_time_ns();
}

void rt_profiler_end(rt_stats_t *s, uint32_t expected_cycle_ns) {

    uint64_t now = get_time_ns();

    uint64_t exec_time = now - start_time;
    uint64_t cycle_time = now - last_cycle_time;

    last_cycle_time = now;

    uint32_t exec_us = exec_time / 1000;
    int32_t jitter_us = (cycle_time - expected_cycle_ns) / 1000;

    s->exec_time_us = exec_us;
    s->jitter_us = jitter_us;

    if (exec_us > s->max_exec_time_us)
        s->max_exec_time_us = exec_us;

    if (exec_us < s->min_exec_time_us)
        s->min_exec_time_us = exec_us;

    if (jitter_us > s->max_jitter_us)
        s->max_jitter_us = jitter_us;

    if (exec_time > expected_cycle_ns)
        s->deadline_miss++;
}