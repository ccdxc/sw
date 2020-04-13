//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// Hardware FIFO
//-----------------------------------------------------------------------------


#ifndef __ELBA_FIFO_HPP__
#define __ELBA_FIFO_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace elba {

sdk_ret_t elba_init_hw_fifo(int fifo_num, uint64_t addr,
                            int n, hw_fifo_prof_t *prof);
sdk_ret_t elba_get_hw_fifo_info(int fifo_num, hw_fifo_stats_t *stats);
sdk_ret_t elba_set_hw_fifo_info(int fifo_num, hw_fifo_stats_t *stats);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    //__ELBA_FIFO_HPP__
