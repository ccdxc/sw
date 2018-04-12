#ifndef __L4LB_HPP__
#define __L4LB_HPP__

#include "nic/include/base.h"
#include "sdk/list.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/ip.h"
#include "sdk/ht.hpp"
#include "nic/gen/proto/hal/l4lb.pb.h"
#include <netinet/ether.h>
#include "nic/hal/src/nw/vrf.hpp"

using l4lb::L4LbServiceKeyHandle;
using l4lb::L4LbServiceGetResponse;

using sdk::lib::ht_ctxt_t;

namespace hal {

// l4lb service key
typedef struct l4lb_key_s {
    vrf_id_t             vrf_id;      // vrf id
    ip_addr_t               service_ip;     // l4lb service ip address
    uint8_t                 proto;          // l4lb service proto
    uint32_t                service_port;   // l4lb service port
} __PACK__ l4lb_key_t;

// l4lb service entry
typedef struct l4lb_service_entry_s {
    hal_spinlock_t          slock;                  // lock to protect this structure
    l4lb_key_t              l4lb_key;               // l4lb key
    mac_addr_t              serv_mac_addr;          // service mac address
    uint16_t                proxy_arp_en:1;         // proxy arp enable
    l4lb::SessionAffinity   sess_aff;               // session affinity

    // operational state of endpoint
    hal_handle_t         hal_handle;                // HAL allocated handle

    void                *pd;                        // all PD specific state

    ht_ctxt_t           l4lbkey_ht_ctxt;            // l4lb key hash table ctxt
    ht_ctxt_t           hal_handle_ht_ctxt;         // hal handle based hash table ctxt
} __PACK__ l4lb_service_entry_t;

// max. number of l4lb services supported  (TODO: we can take this from cfg file)
#define HAL_MAX_L4LB_SERVICES                (1 << 10)

static inline l4lb_service_entry_t *
find_l4lb_by_key (l4lb_key_t *key)
{
    l4lb_service_entry_t    *l4lb_entry;

    HAL_ASSERT(key != NULL);
    l4lb_entry =
        (l4lb_service_entry_t *)g_hal_state->l4lb_ht()->lookup(key);
    if (l4lb_entry == NULL) {
        return NULL;
    }
    return l4lb_entry;
}

const char *l4lb_to_str(l4lb_service_entry_t *l4lb);
void *l4lb_get_key_func(void *entry);
uint32_t l4lb_compute_key_hash_func(void *key, uint32_t ht_size);
bool l4lb_compare_key_func(void *key1, void *key2);
void *l4lb_get_handle_key_func(void *entry);
uint32_t l4lb_compute_handle_hash_func(void *key, uint32_t ht_size);
bool l4lb_compare_handle_key_func(void *key1, void *key2);
hal_ret_t hal_l4lb_init_cb(hal_cfg_t *hal_cfg);
hal_ret_t hal_l4lb_cleanup_cb(void);

hal_ret_t l4lbservice_create(l4lb::L4LbServiceSpec& spec,
                             l4lb::L4LbServiceResponse *rsp);
hal_ret_t l4lbservice_update(l4lb::L4LbServiceSpec& spec,
                             l4lb::L4LbServiceResponse *rsp);
hal_ret_t l4lbservice_get(l4lb::L4LbServiceSpec& spec,
                          l4lb::L4LbServiceGetResponseMsg *rsp);
}    // namespace hal

#endif    // __L4LB_HPP__

