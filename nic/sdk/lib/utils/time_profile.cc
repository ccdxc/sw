// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "time_profile.hpp"

#ifdef PERF
namespace sdk {
namespace utils {
namespace time_profile {

time_profile_info time_profile_db[TIME_PROFILE_ID_MAX];

static const char *time_profile_str_list[] = {
    TIME_PROFILE_FUNCTION_ENUMS(HANDLER_GENERATE_STRING)
};

static uint64_t
timespec_diff(struct timespec *before, struct timespec *after) {
    uint64_t nseconds = after->tv_sec - before->tv_sec;
    nseconds = nseconds * 1000000000lu;
    if (after->tv_nsec < before->tv_nsec) {
        nseconds -= 1000000000lu;
        after->tv_nsec += 1000000000lu;
    }
    nseconds += (after->tv_nsec - before->tv_nsec);
    return nseconds;
}

void
time_profile_info::start() {
    clock_gettime(CLOCK_REALTIME, &before_);
}

void
time_profile_info::stop() {
    clock_gettime(CLOCK_REALTIME, &after_);
    total_ += timespec_diff(&before_, &after_);
}

uint64_t
time_profile_info::total() {
    return total_;
}

void
print() {
    uint64_t seconds;
    uint64_t mseconds;
    uint64_t useconds;
    uint64_t nseconds;
    uint64_t delta;

    printf("%-40s %-16s\n", "TimeProfileID", "TotalTime");
    printf("------------------------------------------------------------\n");
    for (uint32_t i = 0; i < TIME_PROFILE_ID_MAX; i++) {
        delta = time_profile_db[i].total();
        if (delta) {
            nseconds = delta % 1000;
            delta = delta / 1000;
            useconds =  delta % 1000;
            delta = delta / 1000;
            mseconds = delta % 1000;
            delta = delta / 1000;
            seconds = delta % 1000;
            printf("%-40s %ld.%03ld.%03ld.%03ld\n", time_profile_str_list[i],
                   seconds, mseconds, useconds, nseconds);               
        }
    }
    return;
}

} // namespace time_profile
} // namespace utils
} // namespace sdk
#endif
