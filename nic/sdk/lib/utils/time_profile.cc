// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "time_profile.hpp"
#include "include/sdk/timestamp.hpp"

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
print(void) {
    printf("%-40s %-16s\n", "TimeProfileID", "TotalTime");
    printf("------------------------------------------------------------\n");
    for (uint32_t i = 0; i < TIME_PROFILE_ID_MAX; i++) {
            printf("%-40s %s\n", time_profile_str_list[i],
                   time_profile_db[i].print_diff().to_str());
    }
}

#endif

void
time_profile_info::start(void) {
    if (time_profile_enable) {
        clock_gettime(CLOCK_REALTIME, &before_);
    }
}

void
time_profile_info::stop(void) {
    struct timespec ts_diff;

    if (time_profile_enable) {
        clock_gettime(CLOCK_REALTIME, &after_);
        ts_diff = sdk::timestamp_diff(&after_, &before_);
        sdk::timestamp_to_nsecs(&ts_diff, &total_);
    }
}

uint64_t
time_profile_info::total(void) {
    return total_;
}

string
time_profile_info::print_diff(void) {
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
    snprintf(buff, 500,"%lu.%03lu.%03lu.%03lu",
             seconds, mseconds, useconds, nseconds);
    return string(buff);
}

} // namespace time_profile
} // namespace utils
} // namespace sdk
