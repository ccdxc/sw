//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines debug APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_DEBUG_HPP__
#define __INCLUDE_API_PDS_DEBUG_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/include/sdk/base.hpp"

typedef enum pds_clock_freq_e {
    PDS_CLOCK_FREQUENCY_833  = 0,
    PDS_CLOCK_FREQUENCY_900  = 1,
    PDS_CLOCK_FREQUENCY_957  = 2,
    PDS_CLOCK_FREQUENCY_1033 = 3,
    PDS_CLOCK_FREQUENCY_1100 = 4,
} pds_clock_freq_t;

typedef struct pds_system_temperature_e {
    uint32_t dietemp;
    uint32_t localtemp;
    uint32_t hbmtemp;
} pds_system_temperature_t;

typedef struct pds_system_power_e {
    uint32_t pin;
    uint32_t pout1;
    uint32_t pout2;
} pds_system_power_t;

namespace debug {

sdk_ret_t pds_clock_frequency_update(pds_clock_freq_t freq);
sdk_ret_t pds_get_system_temperature(pds_system_temperature_t *temp);
sdk_ret_t pds_get_system_power(pds_system_power_t *pow);

} // namespace debug

#endif    // __INCLUDE_API_PDS_DEBUG_HPP__
