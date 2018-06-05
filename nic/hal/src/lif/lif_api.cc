//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/lif/lif.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Returns if vlan_insert_en is set on the lif
// ----------------------------------------------------------------------------
hal_ret_t
lif_get_vlan_insert_en (lif_id_t lif_id, bool *insert_en)
{
    hal_ret_t   ret = HAL_RET_OK;
    lif_t       *lif = NULL;

    *insert_en = false;

    lif = find_lif_by_id(lif_id);
    if (lif == NULL) {
        HAL_TRACE_ERR("Lif(id):{} not found", lif_id);
        ret = HAL_RET_LIF_NOT_FOUND;
        goto end;
    }

    *insert_en = lif->vlan_insert_en;

end:
    return ret;
}

}
