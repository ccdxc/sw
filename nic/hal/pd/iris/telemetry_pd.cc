#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <lif_pd.hpp>
#include <pd_api.hpp>
#include <interface_api.hpp>
#include <if_pd_utils.hpp>
#include <p4pd.h>
#include <p4pd_api.hpp>

namespace hal {
namespace pd {

hal_ret_t
pd_mirror_update_hw(uint32_t id, mirror_actiondata *action_data)
{
    hal_ret_t       ret = HAL_RET_OK;
    DirectMap       *session = NULL;
    p4pd_error_t    p4_err;
    char            buff[4096] = {0};

    session = g_hal_state_pd->dm_table(P4TBL_ID_MIRROR);
    HAL_ASSERT_RETURN((session != NULL), HAL_RET_ERR);

    ret = session->update(id, action_data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-MIRROR-SESSION::{}: programming sesion {} failed ({})", 
                __FUNCTION__, id, ret);
    } else {
        p4_err =  p4pd_table_ds_decoded_string_get(P4TBL_ID_MIRROR, NULL, NULL,
                action_data, buff, sizeof(buff));
        HAL_ASSERT(p4_err == P4PD_SUCCESS);
        HAL_TRACE_DEBUG("PD-MIRROR-SESSION::{}: programmed session {}: {}",
                __FUNCTION__, id, buff);
    }
    return ret;
}


hal_ret_t
pd_mirror_session_create(pd_mirror_session_args_t *args)
{
    uint32_t dst_lport;
    mirror_actiondata action_data; 
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("PD-MIRROR-SESSION:: NULL argument");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("PD-MIRROR-SESSION::{}: Create call for session {}",
            __FUNCTION__, args->session->id);

    // Add to a PD datastructure instead of stack.
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id < 7));

    switch (args->session->dest_if.if_type) {
        case intf::IF_TYPE_ENIC:
        case intf::IF_TYPE_TUNNEL:
            // Not supported yet.
            return HAL_RET_INVALID_OP;
        case intf::IF_TYPE_UPLINK_PC:
        case intf::IF_TYPE_UPLINK:
            dst_lport = if_get_lport_id(&(args->session->dest_if));
            break;
        default:
            return HAL_RET_INVALID_OP;
    }

    switch (args->session->type) {
        case MIRROR_DEST_LOCAL:
            action_data.actionid = MIRROR_LOCAL_SPAN_ID;
            action_data.mirror_action_u.mirror_local_span.truncate_len = args->session->truncate_len;
            action_data.mirror_action_u.mirror_local_span.dst_lport = dst_lport;
            break;
        case MIRROR_DEST_RSPAN:
            action_data.actionid = MIRROR_REMOTE_SPAN_ID;
            action_data.mirror_action_u.mirror_remote_span.truncate_len = args->session->truncate_len;
            action_data.mirror_action_u.mirror_remote_span.dst_lport = dst_lport;
            action_data.mirror_action_u.mirror_remote_span.vlan = args->session->mirror_destination_u.r_span_dest.vlan;
            action_data.mirror_action_u.mirror_remote_span.tunnel_rewrite_index = g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
            break;
        case MIRROR_DEST_ERSPAN:
            HAL_TRACE_ERR("PD-MIRROR-SESSION:: session type is ERSPAN (not supported yet)");
            return HAL_RET_INVALID_OP;
            break;
        default:
            HAL_TRACE_ERR("PD-MIRROR-SESSION:: unknown session type {}", args->session->type);
            return HAL_RET_INVALID_ARG;
    }
    
    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_delete(pd_mirror_session_args_t *args)
{
    mirror_actiondata action_data;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("PD-MIRROR-SESSION:: NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id < 7));
    action_data.actionid = MIRROR_DROP_MIRROR_ID;

    return pd_mirror_update_hw(args->session->id, &action_data);
}

hal_ret_t
pd_mirror_session_get(pd_mirror_session_args_t *args)
{
    mirror_actiondata action_data;
    if ((args == NULL) || (args->session == NULL)) {
        HAL_TRACE_ERR("PD-MIRROR-SESSION:: NULL argument");
        return HAL_RET_INVALID_ARG;
    }
    memset(&action_data, 0, sizeof(mirror_actiondata));
    HAL_ASSERT((args->session->id >= 0) && (args->session->id < 7));
   
    p4pd_error_t pdret; 
    pdret = p4pd_entry_read(P4TBL_ID_MIRROR, args->session->id, NULL, NULL, (void *)&action_data);
    if (pdret == P4PD_SUCCESS) {
        switch (action_data.actionid) {
            case MIRROR_LOCAL_SPAN_ID:
                args->session->type = MIRROR_DEST_LOCAL;
                args->session->truncate_len = action_data.mirror_action_u.mirror_local_span.truncate_len;
                // args-> dst_if // TBD
            case MIRROR_REMOTE_SPAN_ID:
                args->session->type = MIRROR_DEST_RSPAN;
                args->session->truncate_len = action_data.mirror_action_u.mirror_remote_span.truncate_len;
                args->session->mirror_destination_u.r_span_dest.vlan = action_data.mirror_action_u.mirror_remote_span.vlan;
            case MIRROR_ERSPAN_MIRROR_ID:
                args->session->type = MIRROR_DEST_ERSPAN;
                args->session->truncate_len = action_data.mirror_action_u.mirror_erspan_mirror.truncate_len;
                // Get tunnel if ID - TBD
                //args->session->mirror_destination_u.r_span_dest.tunnel_if_id = 
            case MIRROR_NOP_ID:
                args->session->type = MIRROR_DEST_NONE;
            default:
                return HAL_RET_INVALID_OP;
        }
    } else {
        return HAL_RET_HW_PROG_ERR;
    }
    return HAL_RET_OK;
}

} // namespace pd
} // namespace hal

