/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"
#include "sdk/list.hpp"

namespace hal {
namespace plugins {
namespace nat {

/*
 * Constants
 */
const std::string FTE_FEATURE_NAT("pensando.io/nat:nat");

/*
 * NAT feature fte state (stored in the fte ctx for programming rflow)
 */
typedef struct nat_info_s {
    vrf_id_t  nat_svrf;
    ip_addr_t nat_sip;
    uint16_t  nat_sport;
    vrf_id_t  nat_dvrf;
    ip_addr_t nat_dip;
    uint16_t  nat_dport;
} nat_info_t;

/*
 * Function prototypes
 */

// exec.cc
fte::pipeline_action_t nat_exec (fte::ctx_t &ctx);

}  // namespace nat
}  // namespace plugins
}  // namespace hal
