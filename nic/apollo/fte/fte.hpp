//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains external interfaces of FTE module
///
//----------------------------------------------------------------------------

#ifndef __FTE_FTE_HPP__
#define __FTE_FTE_HPP__

namespace fte {

struct stats_t {
    uint64_t tx;
    uint64_t rx;
    uint64_t drop;
};

void fte_init(void);

void fte_flow_prog(struct rte_mbuf *m);
void fte_ftl_set_core_id(unsigned int core_id);
sdk_ret_t fte_ftl_init(void);

}    // namespace fte

#endif    // __FTE_FTE_HPP__

