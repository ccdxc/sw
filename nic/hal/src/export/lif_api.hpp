//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __LIF_API_HPP__
#define __LIF_API_HPP__

#include "nic/include/base.h"

namespace hal {

hal_ret_t lif_get_vlan_insert_en (lif_id_t lif_id, bool *insert_en);

} // namespace hal


#endif // __LIF_API_HPP__
