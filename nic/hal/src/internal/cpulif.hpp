//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __CPULIF_HPP__
#define __CPULIF_HPP__

namespace hal {

hal_ret_t program_cpu_lif(void);
hal_ret_t cpucb_create(uint8_t cpucb_id);

}    // namespace hal

#endif    // __CPULIF_HPP__

