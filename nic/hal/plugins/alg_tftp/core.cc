/*
 * core.cc
 */

#include "core.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_tftp {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

static void incr_parse_error(l4_alg_status_t *sess) {
    HAL_ATOMIC_INC_UINT32(&((tftp_info_t *)sess->info)->parse_errors, 1);
}

static void incr_unknown_opcode(l4_alg_status_t *sess) {
    HAL_ATOMIC_INC_UINT32(&((tftp_info_t *)sess->info)->unknown_opcode, 1);
}

/*
 * TFTP info cleanup handler
 */ 
void tftpinfo_cleanup_hdlr(l4_alg_status_t *l4_sess) {
    if (l4_sess->info != NULL)
        g_tftp_state->alg_info_slab()->free((tftp_info_t *)l4_sess->info);
}

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, expected_flow_t *wentry) {
    l4_alg_status_t               *entry = NULL;
    tftp_info_t                   *tftp_info = NULL;
    sfw_info_t                    *sfw_info =
                             (sfw_info_t*)ctx.feature_state(FTE_FEATURE_SFW);

    entry = (l4_alg_status_t *)wentry;
    tftp_info = (tftp_info_t *)entry->info;
    if (entry->isCtrl != TRUE) {
        sfw_info->skip_sfw = tftp_info->skip_sfw;
        HAL_TRACE_DEBUG("Expected flow handler - skip sfw {}", sfw_info->skip_sfw);
    }
    ctx.set_feature_name(FTE_FEATURE_ALG_TFTP.c_str());
    ctx.register_feature_session_state(&entry->fte_feature_state);

    return HAL_RET_OK;
}

/*
 * TFTP ALG completion handler - invoked when the session creation is done.
 */
static void tftp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t   *l4_sess = (l4_alg_status_t *)alg_status(\
                                 ctx.feature_session_state(FTE_FEATURE_ALG_TFTP));
    l4_alg_status_t   *exp_flow = NULL;
    hal::flow_key_t    key;
    hal_ret_t          ret;

    HAL_ASSERT(l4_sess != NULL);

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            g_tftp_state->cleanup_app_session(l4_sess->app_session);
        }
    } else {
        HAL_TRACE_DEBUG("In TFTP Completion handler ctrl");
        l4_sess->session = ctx.session();
        if (l4_sess->isCtrl == TRUE) { /* Control session */
            // Set the responder flow key & mark sport as 0
            key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
            key.sport = 0;
            /*
             * Add an expected flow here for control session
             */
            ret = g_tftp_state->alloc_and_insert_exp_flow(l4_sess->app_session,
                                                         key, &exp_flow);
            HAL_ASSERT(ret == HAL_RET_OK);
            exp_flow->entry.handler = expected_flow_handler;
            exp_flow->alg = nwsec::APP_SVC_TFTP;
            exp_flow->info = l4_sess->info;
            HAL_TRACE_DEBUG("Setting expected flow {:p}", (void *)exp_flow);
            l4_sess->info = (tftp_info_t *)g_tftp_state->alg_info_slab()->alloc();
            HAL_ASSERT(l4_sess->info != NULL);
        } else { /* Data session */
            /*
             * Data session flow has been installed sucessfully
             * Cleanup expected flow from the exp flow table and app
             * session list and move it to l4 session list
             */
            g_tftp_state->move_expflow_to_l4sess(l4_sess->app_session, l4_sess);
            memset(&(l4_sess->info), 0, sizeof(tftp_info_t));
            HAL_TRACE_DEBUG("Move expected flow to l4 session");
        }
    }
}

hal_ret_t process_tftp(fte::ctx_t& ctx, l4_alg_status_t *exp_flow) {
    hal_ret_t             ret = HAL_RET_OK;
    l4_alg_status_t      *l4_sess = NULL;
    const uint8_t        *pkt = ctx.pkt();
    uint32_t              offset = 0;
    uint16_t              tftpop = 0;
    tftp_info_t          *info = NULL;

    HAL_ASSERT(exp_flow != NULL);
    info = (tftp_info_t *)exp_flow->info;
    HAL_ASSERT(info != NULL);

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        l4_sess = g_tftp_state->get_ctrl_l4sess(exp_flow->app_session);
        incr_parse_error(l4_sess);
        return ret;
    }

    // Fetch 2-byte opcode
    tftpop = __pack_uint16(pkt, &offset);

    switch (info->tftpop)
    {
        case TFTP_RRQ:
            HAL_TRACE_DEBUG("Received response for RRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (tftpop != 3 && /* DATA */
                tftpop != 6 && /* OACK */
                tftpop != 5) { /* ERROR */
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
                l4_sess = g_tftp_state->get_ctrl_l4sess(exp_flow->app_session);
                HAL_ASSERT(l4_sess != NULL);
                incr_unknown_opcode(l4_sess);
            }
            break;

        case TFTP_WRQ:
            HAL_TRACE_DEBUG("Received response for WRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (tftpop != 4 && /* ACK */
                tftpop != 6 && /* OACK */
                tftpop != 5) { /* ERROR */
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
                l4_sess = g_tftp_state->get_ctrl_l4sess(exp_flow->app_session);
                HAL_ASSERT(l4_sess != NULL);
                incr_unknown_opcode(l4_sess);
            }
            break;

        default:
           // Shouldnt be here
           HAL_ASSERT(0);
           return ret;
    }
    ctx.register_completion_handler(tftp_completion_hdlr);

    return ret;
}

hal_ret_t process_tftp_first_packet(fte::ctx_t& ctx, uint16_t *tftpop) {
    hal_ret_t               ret = HAL_RET_OK;
    const uint8_t          *pkt = ctx.pkt();
    uint32_t                offset = 0;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        return HAL_RET_INVALID_ARG;
    }

    // Fetch 2-byte opcode
    *tftpop = __pack_uint16(pkt, &offset);

    // Only act on it if there is a known opcode
    if (*tftpop != TFTP_RRQ && *tftpop != TFTP_WRQ) { 
        HAL_TRACE_DEBUG("Unknown Opcode -- parse error");
        return HAL_RET_INVALID_OP;
    } 

    HAL_TRACE_DEBUG("Received Opcode:{}", (*tftpop==TFTP_RRQ)?"TFTP_RRQ":"TFTP_WRQ");

    // Set Rflow to be invalid and ALG proto state
    // We want the flow miss to happen on Rflow
    ctx.set_valid_rflow(false);

    return ret;
}

/*
 * TFTP Exec 
 */
fte::pipeline_action_t alg_tftp_exec(fte::ctx_t &ctx) {
    hal_ret_t                      ret = HAL_RET_OK;
    app_session_t                 *app_sess = NULL;
    l4_alg_status_t               *l4_sess = NULL;
    tftp_info_t                   *tftp_info = NULL;
    uint16_t                       tftpop = 0;
    sfw_info_t                    *sfw_info = (sfw_info_t*)\
                                  ctx.feature_state(FTE_FEATURE_SFW);
    fte::feature_session_state_t  *alg_state = NULL;

    if (ctx.protobuf_request() || 
        ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        return fte::PIPELINE_CONTINUE;
    }
 
    alg_state = ctx.feature_session_state();
    if (sfw_info->alg_proto == nwsec::APP_SVC_TFTP &&
        (!ctx.existing_session())) { 
        HAL_TRACE_DEBUG("Alg Proto TFTP is set");

        ret = process_tftp_first_packet(ctx, &tftpop); 
        if (ret == HAL_RET_OK) {
            /*
             * Alloc APP session, EXP flow and TFTP info
             */
            ret = g_tftp_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            ret = g_tftp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            l4_sess->alg = nwsec::APP_SVC_TFTP;
            tftp_info = (tftp_info_t *)g_tftp_state->alg_info_slab()->alloc();
            HAL_ASSERT_RETURN((tftp_info != NULL), fte::PIPELINE_CONTINUE);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = tftp_info;
            tftp_info->tftpop = tftpop;
            tftp_info->callback = process_tftp;
            tftp_info->skip_sfw = TRUE;
            
            /*
             * Register Feature session state & completion handler
             */
            ctx.register_completion_handler(tftp_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);
        }
    } else if (alg_state != NULL) {
        l4_sess = (l4_alg_status_t *)alg_status(alg_state); 
        if (l4_sess != NULL && l4_sess->alg == nwsec::APP_SVC_TFTP) {
            HAL_TRACE_DEBUG("TFTP ALG invoking callback");
            tftp_info = (tftp_info_t *)l4_sess->info;
            HAL_ASSERT(tftp_info);
    
            tftp_info->callback(ctx, l4_sess);
        }
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace alg_tftp
} // namespace plugins
} // namespace hal
