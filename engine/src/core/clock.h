#pragma once

#include "defines.h"

typedef struct clock
{
    f64 start_time;
    f64 elapsed;
} clock;

HAPI void clock_start(clock* clock);

HAPI void clock_update(clock* clock);

HAPI void clock_stop(clock* clock);