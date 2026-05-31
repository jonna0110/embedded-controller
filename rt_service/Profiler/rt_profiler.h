#ifndef RT_PROFILER_H
#define RT_PROFILER_H

#include <stdint.h>

typedef struct {
    uint32_t exec_time_us;
    uint32_t min_exec_time_us;
    uint32_t max_exec_time_us;

    int32_t jitter_us;
    int32_t max_jitter_us;

    uint32_t deadline_miss;

} rt_stats_t;

void rt_profiler_init(rt_stats_t *s);
void rt_profiler_start(void);
void rt_profiler_end(rt_stats_t *s, uint32_t expected_cycle_ns);

#endif