//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"

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

hal_ret_t
lif_get_pinned_if (lif_t *lif, if_t **uplink_if)
{
    hal_handle_t        uplink_hdl = HAL_HANDLE_INVALID;
    hal_ret_t           ret = HAL_RET_OK;

    uplink_hdl = lif->pinned_uplink;

    if (uplink_hdl != HAL_HANDLE_INVALID) {
        *uplink_if = find_if_by_handle(uplink_hdl);
        if (!*uplink_if) {
            HAL_TRACE_ERR("Unable to find if for if_hdl {}", uplink_hdl);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
    } else {
        HAL_TRACE_DEBUG("No pinned uplink for lif {}", lif->lif_id);
        ret = HAL_RET_INVALID_OP;
    }

end:
    return ret;
}

}
