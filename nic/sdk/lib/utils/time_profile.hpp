// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

#ifndef __TIME_PROFILE_HPP__
#define __TIME_PROFILE_HPP__

namespace sdk {
namespace utils {
namespace time_profile {

class time_profile_info {
private:
    struct timespec before_;
    struct timespec after_;
    uint64_t total_;
public:
    time_profile_info() {
        total_ = 0;
    }
    void start();
    void stop();
    uint64_t total();
    string print_diff();
};

extern bool time_profile_enable;

//#define TIME_PROFILE_ENABLE
#ifdef TIME_PROFILE_ENABLE


#define TIME_PROFILE_FUNCTION_ENUMS(__handler) \
        __handler(TABLE_LIB_MEMHASH_INSERT) \
        __handler(TABLE_LIB_FTL_INSERT) \
        __handler(P4PD_HWKEY_HWMASK_BUILD) \
        __handler(P4PD_ENTRY_READ) \
        __handler(ASICPD_HBM_TABLE_ENTRY_READ) \
        __handler(ASIC_MEM_READ) \
        __handler(PAL_MEM_RD) \
        __handler(PAL_REG_READ) \
        __handler(P4PD_ENTRY_INSTALL) \
        __handler(ASICPD_HBM_TABLE_ENTRY_WRITE) \
        __handler(CAPRI_HBM_TABLE_ENTRY_WRITE) \
        __handler(CAPRI_HBM_TABLE_ENTRY_CACHE_INVALIDATE) \
        __handler(ASIC_MEM_WRITE) \
        __handler(PAL_MEM_WR) \
        __handler(PAL_REG_WRITE) \
        __handler(COMPUTE_CRC) \
        __handler(FTE_CTXT_INIT) \
        __handler(TIME_PROFILE_ID_MAX)

#define HANDLER_GENERATE_ENUM(ENUM) ENUM,
#define HANDLER_GENERATE_STRING(STRING) #STRING,

typedef enum time_profile_id_s {
    TIME_PROFILE_FUNCTION_ENUMS(HANDLER_GENERATE_ENUM)
} time_profile_id_t;

extern time_profile_info time_profile_db[];

void print();

#define time_profile_enable(v) time_profile_enable = v;
#define time_profile_begin(_id) \
        sdk::utils::time_profile::time_profile_db[_id].start()
#define time_profile_end(_id) \
        sdk::utils::time_profile::time_profile_db[_id].stop()
#define time_profile_total(_id) \
        sdk::utils::time_profile::time_profile_db[_id].total()
#define time_profile_print() \
        sdk::utils::time_profile::print();
#else
#define time_profile_enable(v)
#define time_profile_begin(_id)
#define time_profile_end(_id)
#define time_profile_total(_id) 0lu
#define time_profile_print()
#endif

} // namespace time_profile
} // namespace utils
} // namespace sdk

#endif // __TIME_PROFILE_HPP__
