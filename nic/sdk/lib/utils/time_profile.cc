// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "time_profile.hpp"

namespace sdk {
namespace utils {
namespace time_profile {

bool time_profile_enable = false;

#ifdef TIME_PROFILE_ENABLE
time_profile_info time_profile_db[TIME_PROFILE_ID_MAX];

static const char *time_profile_str_list[] = {
    TIME_PROFILE_FUNCTION_ENUMS(HANDLER_GENERATE_STRING)
};

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
            printf("%-40s %s\n", time_profile_str_list[i],
                   time_profile_db[i].print_diff().to_str());
    }
}

#endif

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
    if (time_profile_enable) {
        clock_gettime(CLOCK_REALTIME, &before_);
    }
}

void
time_profile_info::stop() {
    if (time_profile_enable) {
        clock_gettime(CLOCK_REALTIME, &after_);
        total_ += timespec_diff(&before_, &after_);
    }
}

uint64_t
time_profile_info::total() {
    return total_;
}

string
time_profile_info::print_diff() {
    uint64_t seconds = 0;
    uint64_t mseconds = 0;
    uint64_t useconds = 0;
    uint64_t nseconds = 0;
    uint64_t delta = total_;
    char buff[500];

    if (delta) {
        nseconds = delta % 1000;
        delta = delta / 1000;
        useconds =  delta % 1000;
        delta = delta / 1000;
        mseconds = delta % 1000;
        delta = delta / 1000;
        seconds = delta % 1000;
    }
    snprintf(buff, 500,"%ld.%03ld.%03ld.%03ld",
             seconds, mseconds, useconds, nseconds);
    return string(buff);
}

} // namespace time_profile
} // namespace utils
} // namespace sdk
