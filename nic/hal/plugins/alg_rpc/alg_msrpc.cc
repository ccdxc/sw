//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "alg_msrpc.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/plugins/alg_rpc/msrpc_proto_def.hpp"
#include "nic/p4/iris/include/defines.h"

#define UUID_BYTES (sizeof(uuid_t))
#define DREP_LENDIAN 0x10
static uint8_t data_rep = 0x0; //Big Endian

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

uuid_t epm_uuid = {0xe1af8308, 0x5d1f, 0x11c9, 0x91, 0xa4, {0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa}};
uuid_t ndr_64bit = {0x71710533, 0xbeba, 0x4937, 0x83, 0x19, {0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36}};
uuid_t ndr_32bit = {0x8a885d04, 0x1ceb, 0x11c9, 0x9f, 0xe8, {0x08, 0x00, 0x2b, 0x10, 0x48, 0x60}};

std::ostream& operator<<(std::ostream& os, const rpc_info_t& val) {
    os << "{skip_sfw=" << val.skip_sfw;
    os << " ,rpc_frag_cont=" << val.rpc_frag_cont;
    os << " ,ip=" << val.ip.v4_addr;
    os << " ,prot=" << val.prot;
    os << " ,dport=" << val.dport;
    os << " ,vers=" << val.vers;
    os << " ,pkt_type=" << val.pkt_type;
    os << " ,call_id=" << val.call_id;
    os << " ,msrpc_64bit=" << val.msrpc_64bit;
    os << " ,num_msrpc_ctxt=" << val.num_msrpc_ctxt;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const uuid_t& val) {
    os << "{time_lo=" << val.time_lo;
    os << " ,time_mid=" << val.time_mid;
    os << " ,time_hi_vers=" << val.time_hi_vers;
    os << " ,clock_seq_hi=" << (int)val.clock_seq_hi;
    os << " ,clock_seq_lo=" << (int)val.clock_seq_lo;
    for (int i=0; i<6; i++) {
        os << ",node " << i;
        os << "=" << (int)val.node[i];
    }
    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_cn_common_hdr_t& val) {
    os << "{rpc_ver=" << (int)val.rpc_ver;
    os << " ,rpc_ver_minor=" << (int)val.rpc_ver_minor;
    os << " ,ptype=" << (int)val.ptype;
    os << " ,flags=" << (int)val.flags;
    for (int i=0; i<4; i++) {
        os << "drep " << i;
        os << "=" << (int)val.drep[i];
    }
    os << " ,frag_len=" << val.frag_len;
    os << " ,auth_len=" << val.auth_len;
    os << " ,call_id=" << val.call_id;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_dg_common_hdr_t& val) {
    os << "{rpc_ver=" << (int)val.rpc_ver;
    os << " ,ptype=" << (int)val.ptype;
    os << " ,flags1=" << (int)val.flags1;
    os << " ,flags2=" << (int)val.flags2;
    for (int i=0; i<3; i++) {
        os << "drep " << i;
        os << "=" << (int)val.drep[i];
    }
    os << " ,serial_hi=" <<  (int)val.serial_hi;
    os << " ,obj_id=" << val.obj_id;
    os << " ,if_id=" << val.if_id;
    os << " ,act_id=" << val.act_id;
    os << " ,server_boot=" << val.server_boot;
    os << " ,if_ver=" << val.if_ver;
    os << " ,seqnum=" << val.seqnum;
    os << " ,opnum=" << val.opnum;
    os << " ,ihint=" << val.ihint;
    os << " ,ahint=" << val.ahint;
    os << " ,frag_len=" << val.frag_len;
    os << " ,frag_num=" << val.frag_num;
    os << " ,auth_proto=" << (int)val.auth_proto;
    os << " ,serial_lo=" << (int)val.serial_lo;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_epm_flr_t& val) {
    os << "{lhs_length=" << val.lhs_length;
    os << " ,protocol=" << (int)val.protocol;
    os << " ,uuid=" << val.uuid;
    os << " ,version=" << val.version;
    os << " ,rhs_length=" << val.rhs_length;
    os << " ,minor_vers=" << val.minor_vers;
    os << " ,port=" << val.port;
    os << " ,ip=" << val.ip.v4_addr;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_map_twr_t& val) {
    os << "{twr_ptr=" << val.twr_ptr;
    os << " ,twr_lgth=" << val.twr_lgth;
    os << " ,twr_arr_lgth=" << val.twr_arr.twr_arr_len;
    os << " ,num_floors=" << val.twr_arr.num_floors;
    for (int i=0; i<val.twr_arr.num_floors; i++) {
        os << " { floor " << i;
        os << ":=" << val.twr_arr.flrs[i];
        os << "}";
    }

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_epm_req_hdr_t& val) {
    os << "{msrpc_twr=" << val.twr;
    os << " {hdl: attr=" << val.hdl.attr;
    os << " ,uuid=" << val.hdl.uuid;
    os << "}";
    os << " ,max_twrs=" << val.max_twrs;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_epm_rsp_hdr_t& val) {
    os << "{{hdl: attr=" << val.hdl.attr;
    os << " ,uuid=" << val.hdl.uuid;
    os << "}";
    os << " ,num_twrs=" << val.num_twrs;
    os << " ,max_cnt=" << val.max_cnt;
    os << " ,offset=" << val.offset;
    os << " ,actual_cnt=" << val.actual_cnt;
    os << " ,msrpc_twr=" << val.twr;

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const msrpc_bind_ack_hdr_t& val) {
    os << "{max_xmit_frag=" << val.max_xmit_frag;
    os << " ,max_recv_frag=" << val.max_recv_frag;
    os << " ,assoc_group_id=" << val.assoc_group_id;
    os << " ,sec_addr_len=" << val.sec_addr.len;
    os << " ,num_rslt=" << (int)val.rlist.num_rslts;
    for (int i=0; i< val.rlist.num_rslts; i++) {
        os << "{rslt " << i;
        os << ": result=" << (int)val.rlist.rslts[i].result;
        os << " } ";
    }

    return os << "}";
}

void __parse_uuid(const uint8_t *pkt, uint32_t *offset, uuid_t *u) {
    u->time_lo = __pack_uint32(pkt, offset, data_rep);
    u->time_mid = __pack_uint16(pkt, offset, data_rep);
    u->time_hi_vers = __pack_uint16(pkt, offset, data_rep);
    u->clock_seq_hi = pkt[(*offset)++];
    u->clock_seq_lo = pkt[(*offset)++];
    for (int i=0; i<6; i++)
        u->node[i] = pkt[(*offset)++];
}

uint8_t __parse_dg_common_hdr(const uint8_t *pkt, uint32_t offset,
                              msrpc_dg_common_hdr_t *hdr) {
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

uint8_t __parse_cn_common_hdr(const uint8_t *pkt, uint32_t offset,
                              msrpc_cn_common_hdr_t *hdr) {
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

uint32_t __parse_msrpc_bind_hdr(const uint8_t *pkt, uint32_t dlen, 
                        msrpc_bind_hdr_t *hdr, rpc_info_t *rpc_info) {
    static uint32_t BIND_HDR_SZ = (sizeof(msrpc_bind_hdr_t) - 
                                   sizeof(hdr->context_list.cont_elem));
    uint32_t offset = 0;
    uint8_t ele = 0, xferele = 0;

    if (dlen < BIND_HDR_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than the Bind Hdr size {}",
                       dlen, BIND_HDR_SZ);
        return 0;
    }

    hdr->max_xmit_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->max_recv_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->assoc_group_id = __pack_uint32(pkt, &offset, data_rep);
    HAL_TRACE_DEBUG("Offset: {} num ctxt elem: {}", offset, pkt[offset]);
    hdr->context_list.num_elm = pkt[offset++];
    hdr->context_list.rsvd = pkt[offset++];
    hdr->context_list.rsvd2 = __pack_uint16(pkt, &offset, data_rep);
    hdr->context_list.cont_elem = ctxt_elem;
    memset(ctxt_elem, 0, sizeof(ctxt_elem));

    while (ele < hdr->context_list.num_elm) {
       xferele = 0;
       if ((dlen-offset) < sizeof(p_cont_elem_t)) {
           incr_parse_error(rpc_info);
           HAL_TRACE_ERR("Packet Len {} is smaller than ctxt elem size {}",
                  (dlen-offset), (sizeof(p_cont_elem_t)*hdr->context_list.num_elm));
           return 0;
       }
       hdr->context_list.cont_elem[ele].context_id = __pack_uint16(pkt, &offset, data_rep);
       hdr->context_list.cont_elem[ele].num_xfer_syn = pkt[offset++];
       hdr->context_list.cont_elem[ele].reserved = pkt[offset++];
       (void)__parse_uuid(pkt, &offset,
                           &hdr->context_list.cont_elem[ele].abs_syntax.if_uuid);
       hdr->context_list.cont_elem[ele].abs_syntax.if_vers =
                                            __pack_uint32(pkt, &offset, data_rep);
       while ((xferele < hdr->context_list.cont_elem[ele].num_xfer_syn) &&
              ((dlen-offset) >= sizeof(p_syntax_id_t))) {
           //Parse the pointer to move it
           uuid_t xferuuid;

           (void)__parse_uuid(pkt, &offset, &xferuuid);
           (void)__pack_uint32(pkt, &offset, data_rep);
           xferele++;
       };
       ele++;
    };

    return offset;
}

uint32_t __parse_msrpc_bind_ack_hdr(const uint8_t *pkt, uint32_t dlen, 
                          msrpc_bind_ack_hdr_t *hdr, rpc_info_t *rpc_info) {
    static uint32_t BIND_ACK_SZ = (sizeof(msrpc_bind_ack_hdr_t) - 4);
    uint32_t offset = 0;
    uint8_t ele = 0;

    if (dlen < BIND_ACK_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than bind ack size {}",
                       (dlen-offset), BIND_ACK_SZ);
        return 0;
    }
    hdr->max_xmit_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->max_recv_frag = __pack_uint16(pkt, &offset, data_rep);
    hdr->assoc_group_id = __pack_uint32(pkt, &offset, data_rep);
    hdr->sec_addr.len = __pack_uint16(pkt, &offset, data_rep);
    // It is padded to be word aligned
    offset += (hdr->sec_addr.len%WORD_BYTES)?(hdr->sec_addr.len+(\
         WORD_BYTES - hdr->sec_addr.len%WORD_BYTES)):hdr->sec_addr.len;
    offset += 2; // Padding
    hdr->rlist.num_rslts = pkt[offset++];
    hdr->rlist.rsvd = pkt[offset++];
    hdr->rlist.rsvd2 = __pack_uint16(pkt, &offset, data_rep);
    hdr->rlist.rslts = rslt;

    while (ele < hdr->rlist.num_rslts) {
        if ((dlen-offset) < sizeof(p_result_t)) {
            incr_parse_error(rpc_info);
            HAL_TRACE_ERR("Packet Len {} is smaller than bind rslt size {}",
                       (dlen-offset), (sizeof(p_result_t)*hdr->rlist.num_rslts));
            return 0;
        }
        hdr->rlist.rslts[ele].result = pkt[offset++];
        hdr->rlist.rslts[ele].fail_reason = pkt[offset++];
        (void)__parse_uuid(pkt, &offset, &hdr->rlist.rslts[ele].xfer_syn.if_uuid);
        hdr->rlist.rslts[ele].xfer_syn.if_vers = __pack_uint32(pkt, &offset, data_rep);
        ele++;
    };

    return offset;
}

uint32_t __parse_msrpc_req_hdr(const uint8_t *pkt, uint32_t dlen,
                               msrpc_req_hdr_t *hdr, uint8_t is64bit, 
                               rpc_info_t *rpc_info) {
    static uint32_t REQ_PDU_SZ = sizeof(msrpc_req_hdr_t);
    uint32_t offset = 0;

    if (dlen < REQ_PDU_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than msrpc req size {}",
                       (dlen-offset), REQ_PDU_SZ);
        return 0;
    }
    hdr->alloc_hint = __pack_uint32(pkt, &offset, data_rep);
    hdr->ctxt_id = __pack_uint16(pkt, &offset, data_rep);
    hdr->opnum = __pack_uint16(pkt, &offset, data_rep);
    if (is64bit)
        hdr->uuid_ptr = __pack_uint64(pkt, &offset, data_rep);
    else
        hdr->uuid_ptr = __pack_uint32(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->uuid);

    return offset;
}

uint32_t __parse_msrpc_epm_map_twr(const uint8_t *pkt, uint32_t dlen,
                                   msrpc_map_twr_t *twr, uint8_t is64bit, 
                                   rpc_info_t *rpc_info) {
    uint32_t offset = 0;

    if (is64bit) {
        if (dlen < 16) {
            incr_parse_error(rpc_info);       
            HAL_TRACE_ERR("Map twr len {} is smaller than the header size 16 bytes",
                       (dlen-offset));
            return 0;
        }
        twr->twr_ptr = __pack_uint64(pkt, &offset, data_rep);
        twr->twr_lgth = __pack_uint64(pkt, &offset, data_rep);
    } else {
        if (dlen < 8) {
            incr_parse_error(rpc_info);
            HAL_TRACE_ERR("Map twr Len {} is smaller than the header size 8 bytes",
                       (dlen-offset));
            return 0;
        }
        twr->twr_ptr = __pack_uint32(pkt, &offset, data_rep);
        twr->twr_lgth = __pack_uint32(pkt, &offset, data_rep);
    }
    if (!twr->twr_lgth)
        return offset;

    if ((dlen-offset) < 6) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Map floors len {} is smaller than the header size 6 bytes",
                       (dlen-offset));
        return 0;
    }
    twr->twr_arr.twr_arr_len = __pack_uint32(pkt, &offset, data_rep);
    twr->twr_arr.num_floors = __pack_uint16(pkt, &offset, data_rep);
    // Parse maximum of MAX_FLOORS as we only expect so many
    twr->twr_arr.num_floors = (twr->twr_arr.num_floors > MAX_FLOORS)?MAX_FLOORS:\
                                    twr->twr_arr.num_floors;
    for (int i=0; (i<twr->twr_arr.num_floors && (dlen-offset) > 6); i++) {
        HAL_TRACE_DEBUG("Dlen: {} offset:{}", dlen, offset);
        memset(&twr->twr_arr.flrs[i], 0, sizeof(msrpc_epm_flr_t));
        twr->twr_arr.flrs[i].lhs_length = __pack_uint16(pkt, &offset, data_rep);
        twr->twr_arr.flrs[i].protocol = pkt[offset++];
        switch (twr->twr_arr.flrs[i].protocol) {
            case EPM_PROTO_UUID:
                if ((dlen-offset) < UUID_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("Proto UUID Len {} is smaller than the header size {}",
                       (dlen-offset), UUID_PROTO_SZ);
                    return 0;
                }
                __parse_uuid(pkt, &offset, &twr->twr_arr.flrs[i].uuid);
                twr->twr_arr.flrs[i].version = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].minor_vers = __pack_uint16(pkt, &offset, data_rep);
                break;

            case EPM_PROTO_TCP:
            case EPM_PROTO_UDP:
                if ((dlen-offset) < L4_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("L4 Proto Len {} is smaller than the header size {}",
                       (dlen-offset), L4_PROTO_SZ);
                    return 0;
                }
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].port = __pack_uint16(pkt, &offset);
                break;

            case EPM_PROTO_IP:
                if ((dlen-offset) < L3_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("L3 Proto Len {} is smaller than the header size {}",
                       (dlen-offset), L3_PROTO_SZ);
                    return 0;
                }
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                twr->twr_arr.flrs[i].ip.v4_addr = __pack_uint32(pkt, &offset);
                break;

            default:
                // Move past anything we havent parsed apart from protocol
                // for the lhs.
                offset += (twr->twr_arr.flrs[i].lhs_length - 1);
                if ((dlen-offset) < (uint32_t)(\
                           (twr->twr_arr.flrs[i].lhs_length - 1) + DEFAULT_PROTO_SZ)) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("Default Proto Len {} is smaller than the header size {}",
                       (dlen-offset), (uint32_t)((twr->twr_arr.flrs[i].lhs_length - 1) + DEFAULT_PROTO_SZ));
                    return 0;
                }
                twr->twr_arr.flrs[i].rhs_length = __pack_uint16(pkt, &offset, data_rep);
                offset += twr->twr_arr.flrs[i].rhs_length;
                break;
        };
    }

    return offset;
}

uint32_t __parse_msrpc_epm_req_hdr(const uint8_t *pkt, uint32_t dlen, 
                                   msrpc_epm_req_hdr_t *hdr, uint8_t is64bit, 
                                   rpc_info_t *rpc_info) {
    static uint32_t MSRPC_EPM_REQ = (sizeof(msrpc_handle_t) + sizeof(hdr->max_twrs));
    uint32_t offset = 0;
    
    offset += __parse_msrpc_epm_map_twr(pkt, dlen, &hdr->twr, is64bit, rpc_info);
    if (!offset)
        return offset;

    if ((dlen-offset) < MSRPC_EPM_REQ) {
        incr_parse_error(rpc_info); 
        HAL_TRACE_ERR("MSRPC REQ Len {} is smaller than the header size {}",
                       (dlen-offset), MSRPC_EPM_REQ);
        return 0;
    }
    hdr->hdl.attr = __pack_uint32(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->hdl.uuid);
    hdr->max_twrs = __pack_uint32(pkt, &offset, data_rep);

    return offset;
}

uint32_t __parse_msrpc_rsp_hdr(const uint8_t *pkt, uint32_t dlen,
                               msrpc_rsp_hdr_t *hdr, rpc_info_t *rpc_info) {
    static uint32_t RSP_PDU_SZ = sizeof(msrpc_rsp_hdr_t);
    uint32_t offset = 0;

    if (dlen < RSP_PDU_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("PDU Response Len {} is smaller than the header size {}",
                       dlen, RSP_PDU_SZ);
        return 0;
    }

    hdr->alloc_hint = __pack_uint32(pkt, &offset, data_rep);
    hdr->ctxt_id = __pack_uint16(pkt, &offset, data_rep);
    hdr->cancel_cnt = pkt[offset++];
    hdr->rsvd = pkt[offset++];

    return offset;
}

uint32_t __parse_msrpc_epm_rsp_hdr(const uint8_t *pkt, uint32_t dlen,
                                   msrpc_epm_rsp_hdr_t *hdr, uint8_t is64bit, 
                                   rpc_info_t *rpc_info) {
    static uint32_t EPM_RSP_SZ = (sizeof(msrpc_epm_rsp_hdr_t) - \
                                  sizeof(msrpc_map_twr_t));
    static uint32_t EPM_32BIT_RSP_SZ = (EPM_RSP_SZ - 12);
    uint32_t  offset = 0, twr_offset=0;

    if ((is64bit && dlen < EPM_RSP_SZ) ||
        (!is64bit && dlen < EPM_32BIT_RSP_SZ)) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("EPM RESP Len {} is smaller than the header size {}",
                       dlen, (is64bit)?EPM_RSP_SZ:EPM_32BIT_RSP_SZ);
        return 0;
    }
    hdr->hdl.attr = __pack_uint32(pkt, &offset, data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->hdl.uuid);
    hdr->num_twrs = __pack_uint32(pkt, &offset, data_rep);
    if (is64bit) {
        hdr->max_cnt = __pack_uint64(pkt, &offset, data_rep);
        hdr->offset = __pack_uint64(pkt, &offset, data_rep);
        hdr->actual_cnt = __pack_uint64(pkt, &offset, data_rep);
    } else {
        hdr->max_cnt = __pack_uint32(pkt, &offset, data_rep);
        hdr->offset = __pack_uint32(pkt, &offset, data_rep);
        hdr->actual_cnt = __pack_uint32(pkt, &offset, data_rep);
    }

    twr_offset = __parse_msrpc_epm_map_twr(&pkt[offset], (dlen-offset), 
                                           &hdr->twr, is64bit, rpc_info);
    if (!twr_offset)
        return twr_offset;

    return (offset+twr_offset);
}

/*
 * MSRPC ALG completion handler - invoked when the session creation is done.
 */
static void msrpc_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t   *l4_sess = (l4_alg_status_t *)alg_status(\
                                 ctx.feature_session_state(FTE_FEATURE_ALG_RPC));
    l4_alg_status_t   *exp_flow = NULL;
    hal::flow_key_t    key;
    hal_ret_t          ret;

    HAL_ASSERT(l4_sess != NULL);

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            g_rpc_state->cleanup_app_session(l4_sess->app_session);
        } else { /* Cleanup data session */
            g_rpc_state->cleanup_l4_sess(l4_sess);
        }
    } else {
        l4_sess->sess_hdl = ctx.session()->hal_handle;
        if (l4_sess->isCtrl == true) {  /* Control session */
            if (ctx.key().proto == IP_PROTO_UDP) {
                // Connection-Less MSRPC
                // Set the responder flow key & mark sport as 0
                key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
                key.sport = 0;

                /*
                 * Add an expected flow here for control session
                 */
                ret = g_rpc_state->alloc_and_insert_exp_flow(l4_sess->app_session,
                                                             key, &exp_flow);
                HAL_ASSERT(ret == HAL_RET_OK);
                exp_flow->entry.handler = expected_flow_handler;
                exp_flow->alg = nwsec::APP_SVC_MSFT_RPC;
                /*
                 * Move the RPC info for expected flow from L4 session
                 * to Expected flow
                 */
                exp_flow->info = l4_sess->info;
                HAL_TRACE_DEBUG("Setting expected flow {:p}", (void *)exp_flow);
                l4_sess->info = (rpc_info_t *)g_rpc_state->alg_info_slab()->alloc();
                HAL_ASSERT(l4_sess->info != NULL);
            }
        } else { /* Data session */
            l4_alg_status_t   *ctrl_sess =  g_rpc_state->get_ctrl_l4sess(\
                                                 l4_sess->app_session);
 
            HAL_ASSERT(ctrl_sess);
            incr_data_sess((rpc_info_t *)ctrl_sess->info); 
            if (ctx.key().proto == IP_PROTO_UDP) {
                /*
                 * Connection-Less MSRPC - Data session flow has been
                 * installed sucessfully cleanup expected flow from the
                 * exp flow table and app session list and move it to
                 * l4 session list
                 */
                g_rpc_state->move_expflow_to_l4sess(l4_sess->app_session, l4_sess);
                memset(l4_sess->info, 0, sizeof(rpc_info_t));
                l4_sess->alg = nwsec::APP_SVC_MSFT_RPC;
                HAL_TRACE_DEBUG("Move expected flow to l4 session");
            }
        }
    }
}

hal_ret_t process_msrpc_data_flow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess) {
    hal_ret_t        ret = HAL_RET_OK;
    l4_alg_status_t *exp_flow = l4_sess;

    // Todo (Pavithra) Get the Firewall data and make sure that the UUID
    // is still allowed in the config
    /*
     * Alloc L4 Session. This is just to keep a backward reference to the
     * app session that created it.
     */
    ret = g_rpc_state->alloc_and_insert_l4_sess(l4_sess->app_session, &l4_sess);
    HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
    l4_sess->alg = nwsec::APP_SVC_MSFT_RPC;
    l4_sess->isCtrl = FALSE;

    // Register completion handler and session state
    ctx.register_completion_handler(msrpc_completion_hdlr);
    ctx.register_feature_session_state(&l4_sess->fte_feature_state);

    // Decrement the ref count for the expected flow
    dec_ref_count(&exp_flow->entry);
    return ret;
}

/*
 * Initialize RPC Info
 */
static void reset_rpc_info(rpc_info_t *rpc_info) {
    if (rpc_info->pkt_len && rpc_info->pkt != NULL) {
        HAL_FREE(hal::HAL_MEM_ALLOC_ALG, rpc_info->pkt);
    }
    //memset(rpc_info, 0, sizeof(rpc_info_t));
    rpc_info->pkt_type = PDU_NONE;
    rpc_info->callback = parse_msrpc_cn_control_flow;
}

hal_ret_t parse_msrpc_cn_control_flow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess) {
    uint32_t                 rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_cn_common_hdr_t    rpc_hdr;
    uint32_t                 pgm_offset = 0, idx = 0;
    rpc_info_t              *rpc_info = (rpc_info_t *)l4_sess->info;
    const uint8_t           *pkt = ctx.pkt();
    uint32_t                 pkt_len = ctx.pkt_len();

    HAL_TRACE_DEBUG("In parse_msrpc_cn_control_flow");
    if (pkt_len == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (pkt_len < (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t)));
        return HAL_RET_ERR;
    }

    pgm_offset = __parse_cn_common_hdr(pkt, rpc_msg_offset, &rpc_hdr);

    /*
     * L7 Fragment reassembly
     */
    if (rpc_hdr.rpc_ver_minor == 0 || rpc_hdr.flags & PFC_LAST_FRAG) {
        rpc_info->rpc_frag_cont = 0;

        // If this is the last frag of a multi-PDU
        // transmission and doesnt have the first & last frag set
        if (!(rpc_hdr.flags & PFC_FIRST_FRAG)) {
            if ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ) {
                memcpy(&rpc_info->pkt[rpc_info->pkt_len], &pkt[pgm_offset], (pkt_len-pgm_offset));
                rpc_info->pkt_len += (pkt_len-pgm_offset);
                pkt = rpc_info->pkt;
                pkt_len = rpc_info->pkt_len;
            } else {
                HAL_TRACE_ERR("Packet len execeeded the Max ALG Fragmented packet sz");
                incr_max_pkt_sz(rpc_info);
                reset_rpc_info(rpc_info);
            }
        }
    } else {
        if (rpc_hdr.ptype != PDU_BIND && rpc_hdr.ptype != PDU_ALTER_CTXT &&
            rpc_hdr.ptype != PDU_BIND_ACK && rpc_hdr.ptype != PDU_ALTER_CTXT_ACK) {
            // No other PDUs except for these can be L7 fragmented
            return HAL_RET_ERR;
        }

        // First fragment alloc memory and store the packet
        if (rpc_hdr.flags & PFC_FIRST_FRAG) {
            rpc_info->pkt = alloc_rpc_pkt();
            rpc_info->payload_offset = rpc_msg_offset;
        }

        rpc_info->rpc_frag_cont = 1;
        if ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ) {
            memcpy(&rpc_info->pkt[rpc_info->pkt_len], &pkt[pgm_offset], (pkt_len-pgm_offset));
            rpc_info->pkt_len += (pkt_len-pgm_offset);
        } else {
            HAL_TRACE_ERR("Packet len execeeded the Max ALG Fragmented packet sz");
            incr_max_pkt_sz(rpc_info);
            reset_rpc_info(rpc_info);
        }

        /*
         * Store the packet until all of it is received or we hit the
         * the MAX_ALG_RPC_PKT_SZ
         */
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("Parsed MSRPC Connection oriented header: {}", rpc_hdr);

    switch (rpc_info->pkt_type)
    {
        case PDU_NONE:
            if (rpc_hdr.ptype == PDU_BIND ||
                rpc_hdr.ptype == PDU_ALTER_CTXT) {
                msrpc_bind_hdr_t bind_hdr;
                uint8_t ctxt_id = 0;

                pgm_offset = __parse_msrpc_bind_hdr(&pkt[pgm_offset], 
                                        (pkt_len-pgm_offset), &bind_hdr, rpc_info);
                if (!pgm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }

                // Move to bind state if the interface UUID is
                // for endpoint mapper.
                for (idx = 0; idx < bind_hdr.context_list.num_elm; idx++) {
                    HAL_TRACE_DEBUG("UUID: {}", bind_hdr.context_list.cont_elem[idx].abs_syntax.if_uuid);
                    if (!memcmp(&epm_uuid,
                        &bind_hdr.context_list.cont_elem[idx].abs_syntax.if_uuid, UUID_BYTES)) {
                        HAL_TRACE_DEBUG("Received MSRPC BIND for EPM ctxt id: {}", idx);
                        rpc_info->msrpc_ctxt_id[ctxt_id++] = idx;
                        rpc_info->pkt_type = rpc_hdr.ptype;
                    }
                }
                rpc_info->num_msrpc_ctxt = ctxt_id;
            }
            break;

        case PDU_BIND:
        case PDU_ALTER_CTXT:
            if (rpc_hdr.ptype == PDU_BIND_ACK ||
                rpc_hdr.ptype == PDU_ALTER_CTXT_ACK) {
                msrpc_bind_ack_hdr_t bind_ack;
 
                pgm_offset = __parse_msrpc_bind_ack_hdr(&pkt[pgm_offset], 
                                             (pkt_len-pgm_offset), &bind_ack, rpc_info);
                if (!pgm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }

                HAL_TRACE_DEBUG("Received Bind ACK: {}", bind_ack);
                // Check if we the result was successful
                for (idx = 0; idx < rpc_info->num_msrpc_ctxt; idx++) {
                    p_result_t *rslt = &bind_ack.rlist.rslts[rpc_info->msrpc_ctxt_id[idx]];
                    if (!rslt->result) {
                        rpc_info->pkt_type = rpc_hdr.ptype;
                        if (!memcmp(&ndr_64bit, &rslt->xfer_syn.if_uuid, UUID_BYTES))
                            rpc_info->msrpc_64bit = 1;
                        break;
                    } else {
                        // Reset the PDU type to NONE as we didnt get a success response
                        rpc_info->pkt_type = PDU_NONE;
                    }
                }
            }
            break;

        case PDU_BIND_ACK:
        case PDU_ALTER_CTXT_ACK:
            if (rpc_hdr.ptype == PDU_REQ) {
                uint32_t            epm_offset=0;
                msrpc_req_hdr_t     msrpc_req;
                msrpc_epm_req_hdr_t epm_req;
                msrpc_twr_p_t       twr_arr;
          
                epm_offset = __parse_msrpc_req_hdr(&pkt[pgm_offset], (pkt_len-pgm_offset), 
                                              &msrpc_req, rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }
                epm_offset += pgm_offset;

                epm_offset = __parse_msrpc_epm_req_hdr(&pkt[epm_offset], (pkt_len-epm_offset), 
                                                       &epm_req, rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }

                HAL_TRACE_DEBUG("Parsed EPM REQ Header: {}", epm_req);

                twr_arr = epm_req.twr.twr_arr;
                if (twr_arr.num_floors > MSRPC_DEFAULT_FLOORS) {
                    for (idx=0; idx<twr_arr.num_floors; idx++) {
                        if (twr_arr.flrs[idx].protocol == EPM_PROTO_CN ||
                            twr_arr.flrs[idx].protocol == EPM_PROTO_DG) {
                            rpc_info->call_id = rpc_hdr.call_id;
                            memcpy(&rpc_info->uuid,
                                               &twr_arr.flrs[0].uuid, UUID_BYTES);
                            rpc_info->vers = twr_arr.flrs[0].version;
                            rpc_info->prot =
                                       (twr_arr.flrs[idx].protocol == EPM_PROTO_CN)?\
                                           IP_PROTO_TCP:IP_PROTO_UDP;
                        }
                    }
                }
                rpc_info->pkt_type = rpc_hdr.ptype;
            }
            break;

        case PDU_REQ:
            if (rpc_hdr.ptype == PDU_RESP) {
                uint32_t            epm_offset=0;
                msrpc_rsp_hdr_t     msrpc_rsp;
                msrpc_epm_rsp_hdr_t epm_rsp;
                msrpc_twr_p_t       twr_arr;

                epm_offset = __parse_msrpc_rsp_hdr(&pkt[pgm_offset], 
                                                 (pkt_len-pgm_offset), &msrpc_rsp, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }
                epm_offset += pgm_offset;

                epm_offset = __parse_msrpc_epm_rsp_hdr(&pkt[epm_offset], (pkt_len-epm_offset), 
                                               &epm_rsp, rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }
                HAL_TRACE_DEBUG("Parsed EPM RSP Header: {}", epm_rsp);

                twr_arr = epm_rsp.twr.twr_arr;
                if (rpc_info->call_id == rpc_hdr.call_id &&
                    (!memcmp(&twr_arr.flrs[0].uuid, &rpc_info->uuid, UUID_BYTES))) {
                    // Check only if we have information beyond the default headers
                    if (twr_arr.num_floors > MSRPC_DEFAULT_FLOORS) {
                        for (idx=0; idx<twr_arr.num_floors; idx++) {
                            if (twr_arr.flrs[idx].protocol == EPM_PROTO_TCP ||
                                twr_arr.flrs[idx].protocol == EPM_PROTO_UDP) {
                                rpc_info->dport = twr_arr.flrs[idx].port;
                                HAL_TRACE_DEBUG("Setting Dport: {}", rpc_info->dport);
                            } else if (twr_arr.flrs[idx].protocol == EPM_PROTO_IP) {
                               // If the IP address is not filled in we assume that the sender is the
                               // server and use that.
                               if (!twr_arr.flrs[idx].ip.v4_addr)
                                   twr_arr.flrs[idx].ip = ctx.key().sip;
                               rpc_info->ip.v4_addr = twr_arr.flrs[idx].ip.v4_addr;
                            }
                        }
                    }
                    HAL_TRACE_DEBUG("RPC INFO DPORT: {}", rpc_info->dport);
                    if (g_rpc_state && rpc_info->dport)
                        insert_rpc_expflow(ctx, l4_sess, process_msrpc_data_flow);
                }
                rpc_info->pkt_type = PDU_NONE;
            }
            break;

        default:
            break;
    };

    HAL_TRACE_DEBUG("Processed Connection-Oriented MSRPC Header");

    return HAL_RET_OK;
}

hal_ret_t parse_msrpc_dg_control_flow(fte::ctx_t& ctx, l4_alg_status_t *exp_flow) {
    hal_ret_t                ret = HAL_RET_OK;
    uint32_t                 rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    msrpc_dg_common_hdr_t    rpc_hdr;
    rpc_info_t              *rpc_info = NULL;

    if (ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return ret;

    if (ctx.pkt_len() == rpc_msg_offset) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("Payload offset: {}", rpc_msg_offset);
    rpc_info = (rpc_info_t *)exp_flow->info;
    if (ctx.pkt_len() < (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       ctx.pkt_len(), (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t)));
        incr_parse_error(rpc_info);
        return HAL_RET_ERR;
    }

    __parse_dg_common_hdr(ctx.pkt(), rpc_msg_offset, &rpc_hdr);

    HAL_TRACE_DEBUG("Parsed MSRPC Connectionless header: {}", rpc_hdr);

    if (rpc_info->pkt_type == PDU_NONE) {
        /*
         * Possibly the first packet received
         */
        if (rpc_hdr.ptype == PDU_REQ) {
            rpc_info->callback = parse_msrpc_dg_control_flow;
            rpc_info->skip_sfw = TRUE;
            rpc_info->pkt_type = PDU_REQ;
            rpc_info->call_id = rpc_hdr.seqnum;
            memcpy(&rpc_info->act_id, &rpc_hdr.act_id, UUID_BYTES);
            memcpy(&rpc_info->uuid, &rpc_hdr.if_id, UUID_BYTES);
            ctx.set_valid_rflow(false);
        }
    } else {
        rpc_info = (rpc_info_t *)exp_flow->info;
        if ((rpc_info->pkt_type == PDU_REQ && rpc_hdr.ptype == PDU_RESP) &&
             (rpc_info->call_id == rpc_hdr.seqnum &&
              (!memcmp(&rpc_info->act_id, &rpc_hdr.act_id, UUID_BYTES)))) {
            HAL_TRACE_DEBUG("Received matching PDU response key: {}", ctx.key());
            // Register completion handler
            ctx.register_completion_handler(msrpc_completion_hdlr);
        }
    }

    return ret;
}

/*
 * MSRPC ALG
 * Connection-less MSRPC Flow -
 *  (i)  Parse the first packet for PDU_REQ
 *  (ii) If parsing is successful - Add new app session, L4 session.
 *       Set FTE session state entry with L4 session and add completion handler.
 *  (iii) If FTE session creation is successful, add an expected flow with
 *       rflow key and wildcarded sport.
 *  (iv) When the reverse flow comes in, the expected flow is hit and if the
 *       parser matches req and resp, add a completion handler.
 *  (v)  If FTE session creation is successful for data flow, move the expected
 *       flow to L4 feature state and clean up RPC info -- we dont keep RPC info
 *       for data sessions.
 * Connection-Oriented MSRPC Flow -
 * (i)  Add new app session, l4 session. Set FTE session state entry with L4
 *      session Update flows to receive Mcast copies of control session to
 *      parse the Endpoint mapper request/response.
 * (ii) For any EPM req/response, parse the packets and add new expected flows
 *      with (flow_type, dir, vrf, dport and dip) along with UUID details in
 *      the RPC Info. Add a timer to these entries (TBD)
 * (iii) When a flow miss matches the added expected flow, skip firewall lookup
 *       and add a feature session state to the newly created session. (TBD) -
 *       validate against firewall again to see if the UUID is allowed or not.
 */
hal_ret_t alg_msrpc_exec(fte::ctx_t& ctx, sfw_info_t *sfw_info,
                         l4_alg_status_t *l4_sess) {
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;
    rpc_info_t           *rpc_info = NULL;
    app_session_t        *app_sess = NULL;

    HAL_TRACE_DEBUG("In alg_msrpc_exec {:p}", (void *)l4_sess);
    if (sfw_info->alg_proto == nwsec::APP_SVC_MSFT_RPC &&
        (!ctx.existing_session())) {
        /*
         * ALG is hit - install Mcast flows and process first packet for UDP
         */
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            HAL_TRACE_DEBUG("Parsing the first packet");
            /*
             * Alloc APP session, L4 Session and RPC info
             */
            ret = g_rpc_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
            ret = g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
            l4_sess->alg = nwsec::APP_SVC_MSFT_RPC;
            rpc_info = (rpc_info_t *)g_rpc_state->alg_info_slab()->alloc();
            HAL_ASSERT_RETURN((rpc_info != NULL), HAL_RET_OOM);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = rpc_info;
            reset_rpc_info(rpc_info);


            // Register completion handler and feature session state
            ctx.register_completion_handler(msrpc_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);

            /*
             * For connection-less MSRPC, the requested call
             * is with the PDU_REQ packet. The response from the
             * server would come back with a different sport.
             */
            if (ctx.key().proto == IP_PROTO_UDP) {
                ret = parse_msrpc_dg_control_flow(ctx, l4_sess);
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Failed to parse connection-less MSRPC header");
                    return ret;
                }
            } else {
                flowupd.type = fte::FLOWUPD_MCAST_COPY;
                flowupd.mcast_info.mcast_en = 1;
                flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
                flowupd.mcast_info.proxy_mcast_ptr = 0;
                ret = ctx.update_flow(flowupd);
            }
        } else if (ctx.key().proto == IP_PROTO_TCP) {
            /*
             * Install Mcast copy only for Connection-oriented MSRPC
             */
            flowupd.type = fte::FLOWUPD_MCAST_COPY;
            flowupd.mcast_info.mcast_en = 1;
            flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
            flowupd.mcast_info.proxy_mcast_ptr = 0;
            ret = ctx.update_flow(flowupd);
        }
    } else if (l4_sess && l4_sess->info) {
        rpc_info = (rpc_info_t *)l4_sess->info;
        HAL_TRACE_DEBUG("RPC Info {:p}", l4_sess->info);

        /*
         * Parse Control session data && Process Expected flows
         */
        rpc_info->callback(ctx, l4_sess);
    }

    return ret;
}

} // namespace alg_rpc
} // namespace plugins
} // namespace hal
