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
 * Key for address entry
 */
typedef struct addr_entry_key_s {
    vrf_id_t     vrf_id;               /* VRF where the IP belongs */
    ip_addr_t    ip_addr;              /* Original IP address */
} __PACK__ addr_entry_key_t;

/*
 * Address mapping entry
 */
typedef struct addr_entry_s {
    addr_entry_key_t    key;           /* Entry key */
    ip_addr_t           tgt_ip_addr;   /* Target (mapped) IP address */
    ht_ctxt_t           db_node;       /* Node for hash table operations */
} __PACK__ addr_entry_t;

/*
 * Function prototypes
 */

// exec.cc
fte::pipeline_action_t nat_exec (fte::ctx_t &ctx);

}  // namespace nat
}  // namespace plugins
}  // namespace hal
