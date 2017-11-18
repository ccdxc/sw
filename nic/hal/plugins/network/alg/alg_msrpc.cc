#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include "nic/hal/plugins/network/alg/msrpc_proto_def.hpp"
#include "nic/p4/nw/include/defines.h"

#define UUID_BYTES (sizeof(uuid_t))
#define DREP_LENDIAN 0x10
static uint8_t data_rep = 0x0; //Big Endian

namespace hal {
namespace net {

uuid_t epm_uuid = {0xe1af8308, 0x5d1f, 0x11c9, 0x91, 0xa4, {0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa}};


uint8_t
__parse_uuid(const uint8_t *pkt, uint8_t offset, uuid_t *u)
{
    u->time_lo = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    u->time_mid = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    u->time_hi_vers = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    u->clock_seq_hi = pkt[offset++];
    u->clock_seq_lo = pkt[offset++];
    for (int i=0; i<6; i++)
        u->node[i] = pkt[offset++];  

    return offset; 
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
    offset += __parse_uuid(pkt, offset, &hdr->obj_id);
    offset += __parse_uuid(pkt, offset, &hdr->if_id);
    offset += __parse_uuid(pkt, offset, &hdr->act_id);
    hdr->server_boot = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->if_ver = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->seqnum = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;  
    hdr->opnum = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->ihint = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->ahint = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->frag_len = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->frag_num = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
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
 
    hdr->frag_len = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->auth_len = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->call_id = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;

    return offset; 
}

uint8_t
__parse_msrpc_bind_hdr(const uint8_t *pkt, uint8_t offset, msrpc_bind_hdr_t *hdr)
{
    uint8_t ele = 0;

    hdr->max_xmit_frag = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;   
    hdr->max_recv_frag = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->assoc_group_id = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->context_list.num_elm = pkt[offset++];
    hdr->context_list.rsvd = pkt[offset++];
    hdr->context_list.rsvd2 = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    
    // Parse one element to get the UUID
    hdr->context_list.cont_elem[ele].context_id = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->context_list.cont_elem[ele].num_xfer_syn = pkt[offset++];
    hdr->context_list.cont_elem[ele].reserved = pkt[offset++];
    offset += __parse_uuid(pkt, offset, 
                           &hdr->context_list.cont_elem[ele].abs_syntax.if_uuid);
    hdr->context_list.cont_elem[ele].abs_syntax.if_vers = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;

    //We dont really need to look beyond this point

    return offset;
}
 
uint8_t 
__parse_msrpc_req_hdr(const uint8_t *pkt, uint8_t offset,
                       msrpc_req_hdr_t *hdr)
{
    hdr->alloc_hint = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->ctxt_id = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->opnum = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    offset += __parse_uuid(pkt, offset, &hdr->uuid);
    
    return offset;
}

uint8_t
__parse_msrpc_epm_map_twr(const uint8_t *pkt, uint8_t offset,
                          msrpc_map_twr_t *twr)
{
    twr->twr_ptr = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    twr->twr_lgth = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    twr->twr_arr.twr_arr_len = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    twr->twr_arr.num_floors = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    for (int i=0; i<twr->twr_arr.num_floors; i++) {
        twr->twr_arr.flrs[i].lhs_length = __pack_uint16(pkt, offset, data_rep);
        offset += SHORT_BYTES;
        twr->twr_arr.flrs[i].protocol = pkt[offset++];
        switch (twr->twr_arr.flrs[i].protocol) {
            case EPM_PROTO_UUID:
                offset += __parse_uuid(pkt, offset, &twr->twr_arr.flrs[i].uuid);
                twr->twr_arr.flrs[i].version = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
                twr->twr_arr.flrs[i].minor_vers = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
                break;

            case EPM_PROTO_TCP_PORT:
            case EPM_PROTO_UDP_PORT:
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
                twr->twr_arr.flrs[i].port = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
                break;

            default:
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, offset, data_rep);
                offset += SHORT_BYTES;
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
    hdr->hdl.attr = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    offset += __parse_uuid(pkt, offset, &hdr->hdl.uuid);
    hdr->max_twrs = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES; 

    return offset;
} 

uint8_t
__parse_msrpc_rsp_hdr(const uint8_t *pkt, uint8_t offset,
                      msrpc_rsp_hdr_t *hdr)
{
    hdr->alloc_hint = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->ctxt_id = __pack_uint16(pkt, offset, data_rep);
    offset += SHORT_BYTES;
    hdr->cancel_cnt = pkt[offset++];
    hdr->rsvd = pkt[offset++];

    return offset;   
}

uint8_t
__parse_msrpc_epm_rsp_hdr(const uint8_t *pkt, uint8_t offset,
                      msrpc_epm_rsp_hdr_t *hdr)
{
    hdr->hdl.attr = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    offset += __parse_uuid(pkt, offset, &hdr->hdl.uuid); 
    hdr->num_twrs = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->max_cnt = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->offset = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    hdr->actual_cnt = __pack_uint32(pkt, offset, data_rep);
    offset += WORD_BYTES;
    offset += __parse_msrpc_epm_map_twr(pkt, offset, &hdr->twr);
 
    return offset;
}
 
hal_ret_t 
parse_msrpc_cn_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t                ret = HAL_RET_OK;
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_cn_common_hdr_t    rpc_hdr;
    fte::alg_entry_t         newentry = ctx.alg_entry();
    uint8_t                  pgm_offset = 0;

    if (ctx.pkt_len() == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (ctx.pkt_len() <= (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t)));
        return HAL_RET_ERR;
    }

    pgm_offset = __parse_cn_common_hdr(ctx.pkt(), rpc_msg_offset,  &rpc_hdr);
    if (rpc_hdr.rpc_ver_minor == 0 || rpc_hdr.flags & PFC_LAST_FRAG) {
        newentry.rpc_frag_cont = 0;
    } else {
        newentry.rpc_frag_cont = 1;
    }

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            if (rpc_hdr.ptype == PDU_BIND) {
                msrpc_bind_hdr_t bind_hdr;

                __parse_msrpc_bind_hdr(ctx.pkt(), pgm_offset, &bind_hdr);
                // Move to bind state if the interface UUID is
                // for endpoint mapper.
                if (!memcmp(&epm_uuid, 
                      &bind_hdr.context_list.cont_elem[0].abs_syntax.if_uuid, UUID_BYTES)) {
                    newentry.alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_BIND;
                }
            }
            break;

        case fte::ALG_PROTO_STATE_MSRPC_BIND:
            if (rpc_hdr.ptype == PDU_BIND_ACK) {
                newentry.alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_BOUND;
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
                    newentry.rpc_map.maps[newentry.rpc_map.num_map].xid = rpc_hdr.call_id;
                    memcpy(&newentry.rpc_map.maps[newentry.rpc_map.num_map].uuid, 
                            &twr_arr.flrs[0].uuid, UUID_BYTES);
                    newentry.rpc_map.maps[newentry.rpc_map.num_map].vers = twr_arr.flrs[0].version;
                    newentry.rpc_map.maps[newentry.rpc_map.num_map].prot = 
                      (twr_arr.flrs[2].protocol == EPM_PROTO_TCP_PORT)?IP_PROTO_TCP:IP_PROTO_UDP;
                    newentry.rpc_map.maps[newentry.rpc_map.num_map].dport = twr_arr.flrs[3].port;
                    newentry.alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_EPM;
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
                for (int i=0; i< (int)newentry.rpc_map.num_map; i++) {
                    if (newentry.rpc_map.maps[i].xid == rpc_hdr.call_id &&
                        (!memcmp(&twr_arr.flrs[0].uuid, &newentry.rpc_map.maps[i].uuid, UUID_BYTES))) {
                        newentry.rpc_map.maps[i].dport = twr_arr.flrs[3].port;
                        insert_rpc_entry(ctx, &newentry.rpc_map.maps[i]);
                    }
                }
                newentry.alg_proto_state = fte::ALG_PROTO_STATE_RPC_INIT;
            }
            break;

        default:
            break;
    };   
        
    HAL_TRACE_DEBUG("Updating the existing entry: {}", newentry);

    //Update the existing entry
    ret = fte::update_alg_entry(ctx.key(), (void *)&newentry, sizeof(fte::alg_entry_t));

    return ret;
}

hal_ret_t
parse_msrpc_dg_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t                ret = HAL_RET_OK;
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_dg_common_hdr_t    rpc_hdr;
    fte::alg_entry_t         newentry = ctx.alg_entry(), *entry = NULL;
    hal::flow_key_t          key = ctx.key();
    uint8_t                  idx = 0;

    if (ctx.pkt_len() == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (ctx.pkt_len() <= (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t)));
        return HAL_RET_ERR;
    }

    __parse_dg_common_hdr(ctx.pkt(), rpc_msg_offset,  &rpc_hdr);
    if (rpc_hdr.flags1 & PFC_LAST_FRAG) {
        newentry.rpc_frag_cont = 0;
    } else {
        newentry.rpc_frag_cont = 1;
    }

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            if (rpc_hdr.ptype == PDU_REQ) {
                newentry.rpc_map.maps[0].xid = rpc_hdr.seqnum;    
                memcpy(&newentry.rpc_map.maps[0].act_id, &rpc_hdr.act_id, UUID_BYTES);
                memcpy(&newentry.rpc_map.maps[0].uuid, &rpc_hdr.if_id, UUID_BYTES);
                newentry.rpc_map.num_map++;
                newentry.key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
                newentry.alg_proto_state = fte::ALG_PROTO_STATE_MSRPC_EPM;
                ctx.set_alg_entry(newentry); 
            }
            break;

       case fte::ALG_PROTO_STATE_MSRPC_EPM:
            if (rpc_hdr.ptype == PDU_RESP) {
                do {
                    if (newentry.rpc_map.maps[idx].xid == rpc_hdr.seqnum && 
                        (!memcmp(&newentry.rpc_map.maps[idx].act_id, &rpc_hdr.act_id, UUID_BYTES))) {
                        newentry.rpc_map.maps[idx].dport = ctx.key().sport;
                        insert_rpc_entry(ctx, &newentry.rpc_map.maps[idx]);
                        key.sport = 0;
                        entry = (fte::alg_entry_t *)fte::remove_alg_entry(key);
                        HAL_FREE(hal::HAL_MEM_ALLOC_ALG, entry);
                        break;
                    }
                } while (idx < newentry.rpc_map.num_map); 
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
    hal_ret_t ret = HAL_RET_OK;

    fte::flow_update_t    flowupd;

    if (ctx.alg_proto() == nwsec::APP_SVC_MSFT_RPC) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {

            ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_INIT);

            // UDP could have the portmapper queries at the
            // start of the session
            if (ctx.key().proto == IP_PROTO_UDP) {
                ret = parse_msrpc_control_flow(ctx);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("SUN RPC ALG parse for UDP frame failed");
                    return ret;
                }
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
