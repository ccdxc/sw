#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include "nic/hal/plugins/network/alg/msrpc_proto_def.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/plugins/firewall/firewall.hpp"

#define UUID_BYTES (sizeof(uuid_t))
#define DREP_LENDIAN 0x10
static uint8_t data_rep = 0x0; //Big Endian

namespace hal {
namespace net {

uuid_t epm_uuid = {0xe1af8308, 0x5d1f, 0x11c9, 0x91, 0xa4, {0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa}};


void
__parse_uuid(const uint8_t *pkt, uint8_t *offset, uuid_t *u)
{
    u->time_lo = __pack_uint32(pkt, offset, data_rep);
    u->time_mid = __pack_uint16(pkt, offset, data_rep);
    u->time_hi_vers = __pack_uint16(pkt, offset, data_rep);
    u->clock_seq_hi = pkt[(*offset)++];
    u->clock_seq_lo = pkt[(*offset)++];
    for (int i=0; i<6; i++)
        u->node[i] = pkt[(*offset)++];  
}

uint8_t
__parse_dg_common_hdr(const uint8_t *pkt, uint8_t offset, msrpc_dg_common_hdr_t *hdr)
{
    hdr->rpc_ver = pkt[offset++];
    hdr->ptype = pkt[offset++];
    hdr->flags1 = pkt[offset++];
    hdr->flags2 = pkt[offset++];
    for (int i=0; i<3; i++) hdr->drep[i] = pkt[offset++];
    if (hdr->drep[0] & DREP_LENDIAN) data_rep = 0x1;
    hdr->serial_hi = pkt[offset++];
    (void)__parse_uuid(pkt, &offset, &hdr->obj_id);
    (void)__parse_uuid(pkt, &offset, &hdr->if_id);
    (void)__parse_uuid(pkt, &offset, &hdr->act_id);
    hdr->server_boot = __pack_uint32(pkt, &offset, data_rep);
    hdr->if_ver = __pack_uint32(pkt, &offset, data_rep);
    hdr->seqnum = __pack_uint32(pkt, &offset, data_rep);
    hdr->opnum = __pack_uint16(pkt, &offset, data_rep);
    hdr->ihint = __pack_uint16(pkt, &offset, data_rep);
    hdr->ahint = __pack_uint16(pkt, &offset, data_rep);
    hdr->frag_len = __pack_uint16(pkt, &offset, data_rep);
    hdr->frag_num = __pack_uint16(pkt, &offset, data_rep);
    hdr->auth_proto = pkt[offset++];
    hdr->serial_lo = pkt[offset++];
 
    return offset; 
}

uint8_t
__parse_cn_common_hdr(const uint8_t *pkt, uint8_t offset, msrpc_cn_common_hdr_t *hdr)
{
    hdr->rpc_ver = pkt[offset++];
    hdr->rpc_ver_minor = pkt[offset++];
    hdr->ptype = pkt[offset++];
    hdr->flags = pkt[offset++];
    for (int i=0; i<4; i++) hdr->drep[i] = pkt[offset++];
    if (hdr->drep[0] & DREP_LENDIAN) data_rep = 0x1;
 
    hdr->frag_len = __pack_uint16(pkt, &offset, data_rep);
    hdr->auth_len = __pack_uint16(pkt, &offset, data_rep);
    hdr->call_id = __pack_uint32(pkt, &offset, data_rep);

    return offset; 
}

static p_cont_elem_t ctxt_elem[MAX_CONTEXT];
static p_result_t    rslt[MAX_CONTEXT];

uint8_t
__parse_msrpc_bind_hdr(const uint8_t *pkt,  
                       uint8_t offset, msrpc_bind_hdr_t *hdr)
{
    uint8_t ele = 0, xferele = 0;

    hdr->max_xmit_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->max_recv_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->assoc_group_id = __pack_uint32(pkt, &offset, data_rep);
    hdr->context_list.num_elm = pkt[offset++];
    hdr->context_list.rsvd = pkt[offset++];
    hdr->context_list.rsvd2 = __pack_uint16(pkt, &offset, data_rep);
    hdr->context_list.cont_elem = ctxt_elem;
 
    // Parse one element to get the UUID
    do {
       hdr->context_list.cont_elem[ele].context_id = __pack_uint16(pkt, &offset, data_rep);
       hdr->context_list.cont_elem[ele].num_xfer_syn = pkt[offset++];
       hdr->context_list.cont_elem[ele].reserved = pkt[offset++];
       (void)__parse_uuid(pkt, &offset, 
                           &hdr->context_list.cont_elem[ele].abs_syntax.if_uuid);
       hdr->context_list.cont_elem[ele].abs_syntax.if_vers = __pack_uint32(pkt, &offset, data_rep);
       do {
           //We dont really care about the xfer syntax
           //Parse the pointer to move it
           uuid_t xferuuid;
           //uint32_t xfer_vers;

           (void)__parse_uuid(pkt, &offset, &xferuuid);
           (void)__pack_uint32(pkt, &offset, data_rep);
           xferele++;
       } while(xferele < hdr->context_list.cont_elem[ele].num_xfer_syn);
       ele++;
    } while (ele < hdr->context_list.num_elm);

    return offset;
}

uint8_t
__parse_msrpc_bind_ack_hdr(const uint8_t *pkt, uint8_t offset, 
                            msrpc_bind_ack_hdr_t *hdr)
{
    uint8_t ele = 0;

    hdr->max_xmit_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->max_recv_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->assoc_group_id = __pack_uint32(pkt, &offset, data_rep);
    hdr->sec_addr.len = __pack_uint16(pkt, &offset, data_rep);
    // It is padded to be word aligned
    offset += (hdr->sec_addr.len%WORD_BYTES)?(hdr->sec_addr.len+(\
         WORD_BYTES + hdr->sec_addr.len%WORD_BYTES)):hdr->sec_addr.len;    
    hdr->rlist.num_rslts = pkt[offset++];
    hdr->rlist.rsvd = pkt[offset++];
    hdr->rlist.rsvd2 = __pack_uint16(pkt, &offset, data_rep);
    hdr->rlist.rslts = rslt;

    do {
        hdr->rlist.rslts[ele].result = pkt[offset++];
        hdr->rlist.rslts[ele].fail_reason = pkt[offset++];
        (void)__parse_uuid(pkt, &offset, &hdr->rlist.rslts[ele].xfer_syn.if_uuid);
        hdr->rlist.rslts[ele].xfer_syn.if_vers = __pack_uint32(pkt, &offset, data_rep);
        ele++;
    } while (ele < hdr->rlist.num_rslts);

    return offset;
}
 
uint8_t 
__parse_msrpc_req_hdr(const uint8_t *pkt, uint8_t offset,
                       msrpc_req_hdr_t *hdr)
{
    hdr->alloc_hint = __pack_uint32(pkt, &offset, data_rep);
    hdr->ctxt_id = __pack_uint16(pkt, &offset, data_rep);
    hdr->opnum = __pack_uint16(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->uuid);
    
    return offset;
}

uint8_t
__parse_msrpc_epm_map_twr(const uint8_t *pkt, uint8_t offset,
                          msrpc_map_twr_t *twr)
{
    twr->twr_ptr = __pack_uint32(pkt, &offset, data_rep);
    twr->twr_lgth = __pack_uint32(pkt, &offset, data_rep);
    twr->twr_arr.twr_arr_len = __pack_uint32(pkt, &offset, data_rep);
    twr->twr_arr.num_floors = __pack_uint16(pkt, &offset, data_rep);
    for (int i=0; i<twr->twr_arr.num_floors; i++) {
        twr->twr_arr.flrs[i].lhs_length = __pack_uint16(pkt, &offset, data_rep);
        twr->twr_arr.flrs[i].protocol = pkt[offset++];
        switch (twr->twr_arr.flrs[i].protocol) {
            case EPM_PROTO_UUID:
                __parse_uuid(pkt, &offset, &twr->twr_arr.flrs[i].uuid);
                twr->twr_arr.flrs[i].version = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].minor_vers = __pack_uint16(pkt, &offset, data_rep);
                break;

            case EPM_PROTO_TCP_PORT:
            case EPM_PROTO_UDP_PORT:
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].port = __pack_uint16(pkt, &offset, data_rep);
                break;

            default:
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                break;
        };
    }

    return offset;
}

uint8_t
__parse_msrpc_epm_req_hdr(const uint8_t *pkt, uint8_t offset, 
                          msrpc_epm_req_hdr_t *hdr)
{
    offset += __parse_msrpc_epm_map_twr(pkt, offset, &hdr->twr);
    hdr->hdl.attr = __pack_uint32(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->hdl.uuid);
    hdr->max_twrs = __pack_uint32(pkt, &offset, data_rep);

    return offset;
} 

uint8_t
__parse_msrpc_rsp_hdr(const uint8_t *pkt, uint8_t offset,
                      msrpc_rsp_hdr_t *hdr)
{
    hdr->alloc_hint = __pack_uint32(pkt, &offset, data_rep);
    hdr->ctxt_id = __pack_uint16(pkt, &offset, data_rep);
    hdr->cancel_cnt = pkt[offset++];
    hdr->rsvd = pkt[offset++];

    return offset;   
}

uint8_t
__parse_msrpc_epm_rsp_hdr(const uint8_t *pkt, uint8_t offset,
                      msrpc_epm_rsp_hdr_t *hdr)
{
    hdr->hdl.attr = __pack_uint32(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->hdl.uuid); 
    hdr->num_twrs = __pack_uint32(pkt, &offset, data_rep);
    hdr->max_cnt = __pack_uint32(pkt, &offset, data_rep);
    hdr->offset = __pack_uint32(pkt, &offset, data_rep);
    hdr->actual_cnt = __pack_uint32(pkt, &offset, data_rep);
    offset += __parse_msrpc_epm_map_twr(pkt, offset, &hdr->twr);
 
    return offset;
}
 
hal_ret_t 
parse_msrpc_cn_control_flow(fte::ctx_t& ctx)
{
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_cn_common_hdr_t    rpc_hdr;
    fte::alg_entry_t         *alg_entry = NULL;
    uint8_t                  pgm_offset = 0, idx = 0;

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }
 
    if (ctx.pkt_len() == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (ctx.pkt_len() < (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t)));
        return HAL_RET_ERR;
    }

    pgm_offset = __parse_cn_common_hdr(ctx.pkt(), rpc_msg_offset,  &rpc_hdr);
    if (rpc_hdr.rpc_ver_minor == 0 || rpc_hdr.flags & PFC_LAST_FRAG) {
        alg_entry->rpcinfo.rpc_frag_cont = 0;
    } else {
        // Todo (Pavithra) we need to save the packet until all the
        // L7 Fragments are received before we decode the header
        alg_entry->rpcinfo.rpc_frag_cont = 1;
    }

    switch (alg_entry->alg_proto_state)
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            if (rpc_hdr.ptype == PDU_BIND || 
                rpc_hdr.ptype == PDU_ALTER_CTXT) {
                msrpc_bind_hdr_t bind_hdr;

                __parse_msrpc_bind_hdr(ctx.pkt(), pgm_offset, &bind_hdr);
                // Move to bind state if the interface UUID is
                // for endpoint mapper.
                for (idx = 0; idx < bind_hdr.context_list.num_elm; idx++) { 
                    if (!memcmp(&epm_uuid, 
                        &bind_hdr.context_list.cont_elem[0].abs_syntax.if_uuid, UUID_BYTES)) {
                        alg_entry->rpcinfo.msrpc_ctxt_id = idx;
                        alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_BIND;
                    }
                }
            }
            break;

        case fte::ALG_PROTO_STATE_MSRPC_BIND:
            if (rpc_hdr.ptype == PDU_BIND_ACK ||
                rpc_hdr.ptype == PDU_ALTER_CTXT_ACK) {
                msrpc_bind_ack_hdr_t bind_ack;
 
                __parse_msrpc_bind_ack_hdr(ctx.pkt(), pgm_offset, &bind_ack);
                // Check if we the result was successful
                if (bind_ack.rlist.num_rslts >= alg_entry->rpcinfo.msrpc_ctxt_id && 
                    !bind_ack.rlist.rslts[alg_entry->rpcinfo.msrpc_ctxt_id].result) {
                     alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_BOUND;
                }
            }
            break;

        case fte::ALG_PROTO_STATE_MSRPC_BOUND:
            if (rpc_hdr.ptype == PDU_REQ) {
                uint8_t             epm_offset=0;
                msrpc_req_hdr_t     msrpc_req;
                msrpc_epm_req_hdr_t epm_req;
                msrpc_twr_p_t   twr_arr;
          
                epm_offset = __parse_msrpc_req_hdr(ctx.pkt(), pgm_offset, &msrpc_req);
                __parse_msrpc_epm_req_hdr(ctx.pkt(), epm_offset, &epm_req);
 
                twr_arr = epm_req.twr.twr_arr;
                if (twr_arr.flrs[2].protocol == EPM_PROTO_CN || 
                    twr_arr.flrs[2].protocol == EPM_PROTO_DG) {
                    uint8_t num_map = alg_entry->rpcinfo.rpc_map.num_map;

                    alg_entry->rpcinfo.rpc_map.maps[num_map].call_id = rpc_hdr.call_id;
                    memcpy(&alg_entry->rpcinfo.rpc_map.maps[num_map].uuid, 
                            &twr_arr.flrs[0].uuid, UUID_BYTES);
                    alg_entry->rpcinfo.rpc_map.maps[num_map].vers = twr_arr.flrs[0].version;
                    alg_entry->rpcinfo.rpc_map.maps[num_map].prot = 
                      (twr_arr.flrs[2].protocol == EPM_PROTO_TCP_PORT)?IP_PROTO_TCP:IP_PROTO_UDP;
                    alg_entry->rpcinfo.rpc_map.maps[num_map].dport = twr_arr.flrs[3].port;
                    alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_EPM;
                }
            }
            break;

        case fte::ALG_PROTO_STATE_MSRPC_EPM:
            if (rpc_hdr.ptype == PDU_RESP) {
                uint8_t             epm_offset=0;
                msrpc_rsp_hdr_t     msrpc_rsp;
                msrpc_epm_rsp_hdr_t epm_rsp;
                msrpc_twr_p_t       twr_arr;

                epm_offset = __parse_msrpc_rsp_hdr(ctx.pkt(), pgm_offset, &msrpc_rsp);
                __parse_msrpc_epm_rsp_hdr(ctx.pkt(), epm_offset, &epm_rsp);
             
                twr_arr = epm_rsp.twr.twr_arr;
                for (int i=0; i< (int)alg_entry->rpcinfo.rpc_map.num_map; i++) {
                    if (alg_entry->rpcinfo.rpc_map.maps[i].xid == rpc_hdr.call_id &&
                        (!memcmp(&twr_arr.flrs[0].uuid, &alg_entry->rpcinfo.rpc_map.maps[i].uuid, UUID_BYTES))) {
                        alg_entry->rpcinfo.rpc_map.maps[i].dport = twr_arr.flrs[3].port;
                        insert_rpc_entry(ctx, &alg_entry->rpcinfo.rpc_map.maps[i]);
                    }
                }
                alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_INIT;
            }
            break;

        default:
            break;
    };   
        
    HAL_TRACE_DEBUG("Updated the existing entry: {}", *alg_entry);

    return HAL_RET_OK;
}

hal_ret_t
parse_msrpc_dg_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t                ret = HAL_RET_OK;
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_dg_common_hdr_t    rpc_hdr;
    hal::flow_key_t          key = ctx.key();
    uint8_t                  idx = 0, insert_entry = 0;
    fte::alg_entry_t         *alg_entry = NULL;

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }
    
    if (ctx.pkt_len() == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("Payload offset: {}", rpc_msg_offset);
    if (ctx.pkt_len() < (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t)));
        return HAL_RET_ERR;
    }

    __parse_dg_common_hdr(ctx.pkt(), rpc_msg_offset,  &rpc_hdr);
    if (alg_entry->alg_proto_state == fte::ALG_PROTO_STATE_RPC_INIT) {
        insert_entry = 1;
    } 

    if (rpc_hdr.flags1 & PFC_LAST_FRAG) {
        alg_entry->rpcinfo.rpc_frag_cont = 0;
    } else {
        alg_entry->rpcinfo.rpc_frag_cont = 1;
    }

    switch (alg_entry->alg_proto_state)
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            if (rpc_hdr.ptype == PDU_REQ) {
                if ((insert_entry) ||
                    (!alg_entry->rpcinfo.rpc_frag_cont && 
                     alg_entry->rpcinfo.rpc_map.maps[0].call_id == rpc_hdr.seqnum &&
                     !memcmp(&alg_entry->rpcinfo.rpc_map.maps[0].act_id, &rpc_hdr.act_id, UUID_BYTES) &&
                     !memcmp(&alg_entry->rpcinfo.rpc_map.maps[0].uuid, &rpc_hdr.if_id, UUID_BYTES))) {

                    if (insert_entry) {
                        alg_entry->rpcinfo.rpc_map.maps[0].call_id = rpc_hdr.seqnum;
                        memcpy(&alg_entry->rpcinfo.rpc_map.maps[0].act_id, &rpc_hdr.act_id, UUID_BYTES);
                        memcpy(&alg_entry->rpcinfo.rpc_map.maps[0].uuid, &rpc_hdr.if_id, UUID_BYTES);
                        alg_entry->rpcinfo.rpc_map.num_map = 1;
                        if (alg_entry->rpcinfo.rpc_frag_cont) {
                            alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_FRAG_REQ;
                        } else {
                            alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_EPM;
                        }
                        ctx.set_valid_rflow(false);
                    } 
                   
                    if (!alg_entry->rpcinfo.rpc_frag_cont) {
                        alg_entry->entry.key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
                        alg_entry->entry.key.sport = 0;
                        alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_EPM;
                        alg_entry->skip_firewall = TRUE;

                        // Insert an entry for Iflow when the first fragment is seen in case 
                        // of fragmented packet. Insert an entry for Rflow in case of Last 
                        // fragment/un-fragmented packet.
                        ctx.register_completion_handler(fte::alg_completion_hdlr);
                    }
                }
            }
            break;

       case fte::ALG_PROTO_STATE_MSRPC_EPM:
            if (rpc_hdr.ptype == PDU_RESP) {
                do {
                    if (alg_entry->rpcinfo.rpc_map.maps[idx].call_id == rpc_hdr.seqnum && 
                        (!memcmp(&alg_entry->rpcinfo.rpc_map.maps[idx].act_id, &rpc_hdr.act_id, UUID_BYTES))) {
                        alg_entry->rpcinfo.rpc_map.maps[idx].dport = ctx.key().sport;
                        HAL_TRACE_DEBUG("Received matching PDU response key: {}", ctx.key());
                        // Todo - register completion handler to link this session to control session
                        // Remove the ALG entry from wildcard table
                        // as we have processed the flow already and
                        // installed/dropped.
                        key.sport = 0;
                        alg_entry = (fte::alg_entry_t *)fte::remove_expected_flow(key);
                        HAL_FREE(hal::HAL_MEM_ALLOC_ALG, alg_entry);
                        break;
                    }
                } while (idx < alg_entry->rpcinfo.rpc_map.num_map); 
               
            }
            break;

       default:
            break;
    };
 
    return ret;
}

hal_ret_t
parse_msrpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    if (ctx.key().proto == IP_PROTO_TCP) { 
        return (parse_msrpc_cn_control_flow(ctx)); 
    } else {
        return (parse_msrpc_dg_control_flow(ctx));
    }

    return ret;
}

hal_ret_t
process_msrpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    fte::alg_entry_t     *alg_entry = NULL;
    fte::flow_update_t    flowupd;
    hal::firewall::firewall_info_t *firewall_info =
        (hal::firewall::firewall_info_t*)ctx.feature_state(hal::firewall::FTE_FEATURE_FIREWALL);

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }

    if (firewall_info->alg_proto == nwsec::APP_SVC_MSFT_RPC) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {

            alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_INIT;
            // UDP could have the portmapper queries at the
            // start of the session
            if (ctx.key().proto == IP_PROTO_UDP) {
                ret = parse_msrpc_control_flow(ctx);
                return ret;
            }

            ctx.register_completion_handler(fte::alg_completion_hdlr);
            flowupd.type = fte::FLOWUPD_MCAST_COPY;
            flowupd.mcast_info.mcast_en = 1;
            flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
            ret = ctx.update_flow(flowupd);
        } else {
            flowupd.type = fte::FLOWUPD_MCAST_COPY;
            flowupd.mcast_info.mcast_en = 1;
            flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
            ret = ctx.update_flow(flowupd);
        }
    }

    return ret;
}

hal_ret_t
process_msrpc_data_flow(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    // Get the Firewall data and make sure that the UUID
    // is still allowed in the config

    return ret;
}

}
}
