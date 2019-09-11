//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __FILTER_HPP__
#define __FILTER_HPP__

#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/eplearn.pb.h"
#include "nic/include/pd.hpp"
#include <netinet/ether.h>

using kh::FilterKeyHandle;
using kh::FilterType;
using endpoint::FilterSpec;
using endpoint::FilterStatus;
using endpoint::FilterResponse;
using endpoint::FilterRequestMsg;
using endpoint::FilterResponseMsg;
using endpoint::FilterDeleteRequestMsg;
using endpoint::FilterDeleteRequest;
using endpoint::FilterDeleteResponseMsg;
using endpoint::FilterDeleteResponse;
using endpoint::FilterGetRequest;
using endpoint::FilterGetRequestMsg;
using endpoint::FilterGetResponse;
using endpoint::FilterGetResponseMsg;

using kh::FilterKey;

namespace hal {

#define HAL_MAX_FILTERS (1 << 10)

typedef struct filter_key_s {
    FilterType    type;
    hal_handle_t  lif_handle;
    vlan_id_t     vlan;
    mac_addr_t    mac_addr;
} __PACK__ filter_key_t;

typedef struct filter_s {
    sdk_spinlock_t  slock;   // lock to protect this structure
    filter_key_t    key;

    hal_handle_t hal_handle;
} __PACK__ filter_t;

typedef struct filter_create_app_ctxt_s {
    lif_t *lif;
} __PACK__ filter_create_app_ctxt_t;

typedef struct filter_delete_app_ctxt_s {
    lif_t *lif;
} __PACK__ filter_delete_app_ctxt_t;

hal_ret_t filter_check_enic_with_filter (filter_key_t *upd_key, lif_t *lif, if_t *hal_if,
                                         bool egress_en, bool *update_enic);
const char *filter_key_to_str (filter_key_t *key);
const char *filter_keyhandle_to_str (filter_t *filter);
hal_ret_t filter_check_enic (lif_t *lif, if_t *hal_if, bool *egress_en);
void *filter_get_key_func (void *entry);
uint32_t filter_compute_key_hash_func (void *key, uint32_t ht_size);
bool filter_compare_key_func (void *key1, void *key2);

}    // namespace hal

#endif    // __FILTER_HPP__

