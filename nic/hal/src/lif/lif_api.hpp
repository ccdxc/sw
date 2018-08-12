//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __LIF_API_HPP__
#define __LIF_API_HPP__

#include "nic/hal/src/lif/lif.hpp"

namespace hal {

hal_ret_t lif_get_vlan_insert_en (lif_id_t lif_id, bool *insert_en);
hal_ret_t lif_handle_egress_en (lif_t *lif, filter_key_t *key,
                                bool egress_en);
} // namespace hal
#endif    // __LIF_HPP__
