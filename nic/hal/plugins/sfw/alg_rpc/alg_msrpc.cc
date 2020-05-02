//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "alg_msrpc.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/plugins/sfw/alg_rpc/msrpc_proto_def.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

#define UUID_BYTES (sizeof(uuid_t))
#define DREP_LENDIAN 0x10

namespace hal {
namespace plugins {
namespace alg_rpc {

thread_local msrpc_epm_req_hdr_t epm_req;
thread_local msrpc_epm_rsp_hdr_t epm_rsp;
thread_local char uuid_str[128];

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

uuid_t epm_uuid = {0xe1af8308, 0x5d1f, 0x11c9, 0x91, 0xa4, {0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa}};
uuid_t ndr_64bit = {0x71710533, 0xbeba, 0x4937, 0x83, 0x19, {0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36}};
uuid_t ndr_32bit = {0x8a885d04, 0x1ceb, 0x11c9, 0x9f, 0xe8, {0x08, 0x00, 0x2b, 0x10, 0x48, 0x60}};

static inline bool is_unsupported_ptype(msrpc_cn_common_hdr_t rpc_hdr) {
    return ((rpc_hdr.ptype != PDU_REQ && rpc_hdr.ptype != PDU_RESP && rpc_hdr.ptype != PDU_REJECT
             && rpc_hdr.ptype != PDU_BIND && rpc_hdr.ptype != PDU_BIND_ACK && rpc_hdr.ptype != PDU_BIND_NAK
             && rpc_hdr.ptype != PDU_ALTER_CTXT && rpc_hdr.ptype != PDU_ALTER_CTXT_ACK));
}

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

static inline void
__uuid_to_str(uuid_t *u, char *str) {
    uint8_t offset=0;

    sprintf(str, "%01x-%01x-%01x-%01x%01x-", u->time_lo, u->time_mid,
            u->time_hi_vers, u->clock_seq_hi, u->clock_seq_lo);
    offset = strlen(str);
    for (int i=0; i<6; i++) {
         sprintf(&str[offset], "%01x", u->node[i]);
         offset += 2;
    }
}

static inline void
__parse_uuid(const uint8_t *pkt, uint32_t *offset, uuid_t *u, uint8_t data_rep) {
    u->time_lo = __pack_uint32(pkt, offset, data_rep);
    u->time_mid = __pack_uint16(pkt, offset, data_rep);
    u->time_hi_vers = __pack_uint16(pkt, offset, data_rep);
    u->clock_seq_hi = pkt[(*offset)++];
    u->clock_seq_lo = pkt[(*offset)++];
    for (int i=0; i<6; i++)
        u->node[i] = pkt[(*offset)++];
}

static inline uint8_t
__parse_dg_common_hdr(const uint8_t *pkt, uint32_t offset,
                      msrpc_dg_common_hdr_t *hdr, rpc_info_t *rpc_info) {
    hdr->rpc_ver = pkt[offset++];
    hdr->ptype = pkt[offset++];
    hdr->flags1 = pkt[offset++];
    hdr->flags2 = pkt[offset++];
    for (int i=0; i<3; i++) hdr->drep[i] = pkt[offset++];
    if (hdr->drep[0] & DREP_LENDIAN) rpc_info->data_rep = 0x1;
    hdr->serial_hi = pkt[offset++];
    (void)__parse_uuid(pkt, &offset, &hdr->obj_id, rpc_info->data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->if_id, rpc_info->data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->act_id, rpc_info->data_rep);
    hdr->server_boot = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    hdr->if_ver = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    hdr->seqnum = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    hdr->opnum = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->ihint = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->ahint = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->frag_len = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->frag_num = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->auth_proto = pkt[offset++];
    hdr->serial_lo = pkt[offset++];

    return offset;
}

static inline uint8_t
__parse_cn_common_hdr(const uint8_t *pkt, uint32_t offset,
                      msrpc_cn_common_hdr_t *hdr, rpc_info_t *rpc_info) {
    hdr->rpc_ver = pkt[offset++];
    hdr->rpc_ver_minor = pkt[offset++];
    hdr->ptype = pkt[offset++];
    hdr->flags = pkt[offset++];
    for (int i=0; i<4; i++) hdr->drep[i] = pkt[offset++];
    if (hdr->drep[0] & DREP_LENDIAN) rpc_info->data_rep = 0x1;

    hdr->frag_len = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->auth_len = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr->call_id = __pack_uint32(pkt, &offset, rpc_info->data_rep);

    return offset;
}

static inline uint32_t
__parse_msrpc_bind_hdr(const uint8_t *pkt, uint32_t dlen,
                       rpc_info_t *rpc_info) {
    static uint32_t  BIND_HDR_SZ = (sizeof(msrpc_bind_hdr_t) -
                                   sizeof(p_cont_elem_t));
    uint32_t         offset = 0;
    uint8_t          ele = 0, xferele = 0;
    msrpc_bind_hdr_t bind_hdr = {};
    uint8_t          ctxt_id = 0;

    if (dlen < BIND_HDR_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than the Bind Hdr size {}",
                       dlen, BIND_HDR_SZ);
        return 0;
    }

    bind_hdr.max_xmit_frag = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    bind_hdr.max_recv_frag = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    bind_hdr.assoc_group_id = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    bind_hdr.context_list.num_elm = pkt[offset++];
    if (bind_hdr.context_list.num_elm > MAX_CONTEXT) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Num context {} is bigger than {}",
                       bind_hdr.context_list.num_elm, MAX_CONTEXT);
    }
    bind_hdr.context_list.rsvd = pkt[offset++];
    bind_hdr.context_list.rsvd2 = __pack_uint16(pkt, &offset, rpc_info->data_rep);

    while (ele < bind_hdr.context_list.num_elm &&
           bind_hdr.context_list.num_elm <= MAX_CONTEXT) {
       p_cont_elem_t  ctxt_elem;

       xferele = 0;
       if ((dlen-offset) < sizeof(p_cont_elem_t)) {
           incr_parse_error(rpc_info);
           HAL_TRACE_ERR("Packet Len {} is smaller than ctxt elem size {}",
                  (dlen-offset), (sizeof(p_cont_elem_t)*bind_hdr.context_list.num_elm));
           return 0;
       }
       ctxt_elem.context_id = __pack_uint16(pkt, &offset, rpc_info->data_rep);
       ctxt_elem.num_xfer_syn = pkt[offset++];
       ctxt_elem.reserved = pkt[offset++];
       (void)__parse_uuid(pkt, &offset,
                           &ctxt_elem.abs_syntax.if_uuid, rpc_info->data_rep);
       HAL_TRACE_VERBOSE("UUID: {}", ctxt_elem.abs_syntax.if_uuid);
       if (!memcmp(&epm_uuid,
                     &ctxt_elem.abs_syntax.if_uuid, UUID_BYTES)) {
           rpc_info->msrpc_ctxt_id[ctxt_id++] = ele;
       }
       ctxt_elem.abs_syntax.if_vers =
                            __pack_uint32(pkt, &offset, rpc_info->data_rep);
       while ((xferele < ctxt_elem.num_xfer_syn) &&
              ((dlen-offset) >= sizeof(p_syntax_id_t))) {
           //Parse the pointer to move it
           uuid_t xferuuid;

           (void)__parse_uuid(pkt, &offset, &xferuuid, rpc_info->data_rep);
           (void)__pack_uint32(pkt, &offset, rpc_info->data_rep);
           xferele++;
       };
       ele++;
    };
    rpc_info->num_msrpc_ctxt = ctxt_id;

    return offset;
}

static inline uint32_t
__parse_msrpc_bind_ack_hdr(const uint8_t *pkt, uint32_t dlen,
                           msrpc_cn_common_hdr_t rpc_hdr, rpc_info_t *rpc_info) {
    static uint32_t      BIND_ACK_SZ = (sizeof(msrpc_bind_ack_hdr_t) - 4);
    uint32_t             offset = 0;
    uint8_t              ele = 0;
    msrpc_bind_ack_hdr_t bind_ack= {};
    p_result_t           rslt[MAX_CONTEXT];

    bind_ack.rlist.rslts = &rslt[0];
    if (dlen < BIND_ACK_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than bind ack size {}",
                       (dlen-offset), BIND_ACK_SZ);
        return 0;
    }
    bind_ack.max_xmit_frag = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    bind_ack.max_recv_frag = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    bind_ack.assoc_group_id = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    bind_ack.sec_addr.len = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    // It is padded to be word aligned
    offset += (bind_ack.sec_addr.len%WORD_BYTES)?(bind_ack.sec_addr.len+(\
         WORD_BYTES - bind_ack.sec_addr.len%WORD_BYTES)):bind_ack.sec_addr.len;
    if (offset > dlen) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than offset size {}",
                       dlen, offset);
        return 0;
    }
    offset += 2; // Padding
    bind_ack.rlist.num_rslts = pkt[offset++];
    if (bind_ack.rlist.num_rslts > MAX_CONTEXT) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Num context {} is bigger than {}",
                       bind_ack.rlist.num_rslts, MAX_CONTEXT);
    }
    bind_ack.rlist.rsvd = pkt[offset++];
    bind_ack.rlist.rsvd2 = __pack_uint16(pkt, &offset, rpc_info->data_rep);

    while (ele < bind_ack.rlist.num_rslts &&
           bind_ack.rlist.num_rslts <= MAX_CONTEXT) {
        if ((dlen-offset) < sizeof(p_result_t)) {
            incr_parse_error(rpc_info);
            HAL_TRACE_ERR("Packet Len {} is smaller than bind rslt size {}",
                       (dlen-offset), (sizeof(p_result_t)*bind_ack.rlist.num_rslts));
            return 0;
        }
        bind_ack.rlist.rslts[ele].result = __pack_uint16(pkt, &offset, rpc_info->data_rep);
        bind_ack.rlist.rslts[ele].fail_reason = __pack_uint16(pkt, &offset, rpc_info->data_rep);
        HAL_TRACE_VERBOSE("Parsing UUID at offset: {} byte: {}", offset, pkt[offset]);
        (void)__parse_uuid(pkt, &offset, &bind_ack.rlist.rslts[ele].xfer_syn.if_uuid, rpc_info->data_rep);
        bind_ack.rlist.rslts[ele].xfer_syn.if_vers = __pack_uint32(pkt, &offset, rpc_info->data_rep);
        ele++;
    };

    // Check if we the result was successful
    for (uint8_t idx = 0; idx < rpc_info->num_msrpc_ctxt; idx++) {
         p_result_t *rslt = &bind_ack.rlist.rslts[rpc_info->msrpc_ctxt_id[idx]];
         if (!rslt->result) {
             rpc_info->pkt_type = rpc_hdr.ptype;
             if (!memcmp(&ndr_64bit, &rslt->xfer_syn.if_uuid, UUID_BYTES))
                  rpc_info->msrpc_64bit = 1;
             break;
         }
    }

    HAL_TRACE_VERBOSE("NDR 64bit: {} If uuid: {}", ndr_64bit, rslt->xfer_syn.if_uuid);
    HAL_TRACE_VERBOSE("Received Bind ACK: {} NDR 64bit: {}", bind_ack, rpc_info->msrpc_64bit);

    return offset;
}

static inline uint32_t
__parse_msrpc_req_hdr (const uint8_t *pkt, uint32_t dlen,
                       uint8_t is64bit, rpc_info_t *rpc_info) {
    static uint32_t     REQ_PDU_SZ = sizeof(msrpc_req_hdr_t);
    uint32_t            offset = 0;
    msrpc_req_hdr_t     hdr = {};

    if (dlen < REQ_PDU_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Packet Len {} is smaller than msrpc req size {}",
                       (dlen), REQ_PDU_SZ);
        return 0;
    }

    hdr.alloc_hint = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    hdr.ctxt_id = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    hdr.opnum = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    if (is64bit)
        hdr.uuid_ptr = __pack_uint64(pkt, &offset, rpc_info->data_rep);
    else
        hdr.uuid_ptr = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr.uuid, rpc_info->data_rep);

    HAL_TRACE_VERBOSE("Ctxt id: {}", hdr.ctxt_id);

    return offset;
}

static inline uint32_t
__parse_msrpc_epm_map_twr(const uint8_t *pkt, uint32_t dlen,
                          msrpc_map_twr_t *twr, uint8_t is64bit,
                          rpc_info_t *rpc_info, uint8_t pdu_type) {
    uint32_t offset = 0;
    bool     set_dport = false;
    uint32_t uuid_offset = 0, tmp_offset=0;

    if (is64bit) {
        if (dlen < 16) {
            incr_parse_error(rpc_info);
            HAL_TRACE_ERR("Map twr len {} is smaller than the header size 16 bytes",
                       (dlen-offset));
            return 0;
        }
        twr->twr_ptr = __pack_uint64(pkt, &offset, rpc_info->data_rep);
        twr->twr_lgth = __pack_uint64(pkt, &offset, rpc_info->data_rep);
    } else {
        if (dlen < 8) {
            incr_parse_error(rpc_info);
            HAL_TRACE_ERR("Map twr Len {} is smaller than the header size 8 bytes",
                       (dlen-offset));
            return 0;
        }
        twr->twr_ptr = __pack_uint32(pkt, &offset, rpc_info->data_rep);
        twr->twr_lgth = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    }
    if (!twr->twr_lgth)
        return offset;

    if ((dlen-offset) < 6) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("Map floors len {} is smaller than the header size 6 bytes",
                       (dlen-offset));
        return 0;
    }
    twr->twr_arr.twr_arr_len = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    twr->twr_arr.num_floors = __pack_uint16(pkt, &offset, rpc_info->data_rep);
    // Parse maximum of MAX_FLOORS as we only expect so many
    twr->twr_arr.num_floors = (twr->twr_arr.num_floors > MAX_FLOORS)?MAX_FLOORS:\
                                    twr->twr_arr.num_floors;
    HAL_TRACE_VERBOSE("Num floors: {}", twr->twr_arr.num_floors);
    for (int i=0; (i<twr->twr_arr.num_floors && (offset < dlen) && (dlen-offset) > 6); i++) {
        msrpc_epm_flr_t flr = {};
        flr.lhs_length = __pack_uint16(pkt, &offset, rpc_info->data_rep);
        flr.protocol = pkt[offset++];
        switch (flr.protocol) {
            case EPM_PROTO_UUID:
                if ((dlen-offset) < UUID_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("Proto UUID Len {} is smaller than the header size {}",
                       (dlen-offset), UUID_PROTO_SZ);
                    return 0;
                }
                tmp_offset = offset;
                __parse_uuid(pkt, &offset, &flr.uuid, rpc_info->data_rep);
                flr.version = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                flr.rhs_length = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                flr.minor_vers = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                if (pdu_type == PDU_REQ &&
                    (memcmp(&flr.uuid, &ndr_64bit, UUID_BYTES) != 0) &&
                    (memcmp(&flr.uuid, &ndr_32bit, UUID_BYTES) != 0)) {
                    uuid_offset = tmp_offset;
                }
                if (pdu_type == PDU_RESP &&
                    !memcmp(&pkt[tmp_offset], rpc_info->uuid, UUID_SZ)) {
                    set_dport = true;
                }
                break;

            case EPM_PROTO_TCP:
            case EPM_PROTO_UDP:
                if ((dlen-offset) < L4_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("L4 Proto Len {} is smaller than the header size {}",
                       (dlen-offset), L4_PROTO_SZ);
                    return 0;
                }
                flr.rhs_length = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                flr.port = __pack_uint16(pkt, &offset);
                if (set_dport) {
                    rpc_info->dport = flr.port;
                    HAL_TRACE_VERBOSE("Setting Dport: {}", rpc_info->dport);
                }
                break;

            case EPM_PROTO_IP:
                if ((dlen-offset) < L3_PROTO_SZ) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("L3 Proto Len {} is smaller than the header size {}",
                       (dlen-offset), L3_PROTO_SZ);
                    return 0;
                }
                flr.rhs_length = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                flr.ip.v4_addr = __pack_uint32(pkt, &offset);
                rpc_info->ip.v4_addr = flr.ip.v4_addr;
                break;

            case EPM_PROTO_CN:
            case EPM_PROTO_DG:
                if (pdu_type == PDU_REQ) {
                    uuid_t *uuid = (uuid_t*)rpc_info->uuid;
                    memcpy(rpc_info->uuid, &pkt[uuid_offset], UUID_SZ);
                    HAL_TRACE_VERBOSE("RPC info UUID: {} floor proto: {} UUID Bytes: {}", *uuid, flr.protocol, UUID_BYTES);
                }
                rpc_info->vers = flr.version;
                rpc_info->prot = (flr.protocol == EPM_PROTO_CN)?\
                                    IP_PROTO_TCP:IP_PROTO_UDP;
            default:
                // Move past anything we havent parsed apart from protocol
                // for the lhs.
                offset += (flr.lhs_length - 1);
                if ((dlen-offset) < (uint32_t)(\
                           (flr.lhs_length - 1) + DEFAULT_PROTO_SZ)) {
                    incr_parse_error(rpc_info);
                    HAL_TRACE_ERR("Default Proto Len {} is smaller than the header size {}",
                       (dlen-offset), (uint32_t)((flr.lhs_length - 1) + DEFAULT_PROTO_SZ));
                    return 0;
                }
                flr.rhs_length = __pack_uint16(pkt, &offset, rpc_info->data_rep);
                offset += flr.rhs_length;
                break;
        };
    }

    return offset;
}

static inline uint32_t
__parse_msrpc_epm_req_hdr(const uint8_t *pkt, uint32_t dlen,
                          uint8_t is64bit, rpc_info_t *rpc_info) {
   // static uint32_t     MSRPC_EPM_REQ = (sizeof(msrpc_handle_t) + sizeof(uint32_t));
    uint32_t            offset = 0;

    bzero(&epm_req, sizeof(msrpc_epm_req_hdr_t));
    offset += __parse_msrpc_epm_map_twr(pkt, dlen, &epm_req.twr, is64bit, rpc_info, PDU_REQ);

    HAL_TRACE_VERBOSE("Parsed EPM REQ Header: {}", epm_req);

#if 0

    twr_arr = &epm_req.twr.twr_arr;
    if (twr_arr->num_floors > MSRPC_DEFAULT_FLOORS) {
        for (uint8_t idx=0; idx<twr_arr->num_floors; idx++) {
             if (twr_arr->flrs[idx].protocol == EPM_PROTO_CN ||
                 twr_arr->flrs[idx].protocol == EPM_PROTO_DG) {
                  memcpy(&rpc_info->uuid,
                                  &twr_arr->flrs[0].uuid, UUID_BYTES);
                  rpc_info->vers = twr_arr->flrs[0].version;
                  rpc_info->prot = (twr_arr->flrs[idx].protocol == EPM_PROTO_CN)?\
                                    IP_PROTO_TCP:IP_PROTO_UDP;
            }
        }
    }

    if ((dlen-offset) < MSRPC_EPM_REQ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("MSRPC REQ Len {} is smaller than the header size {}",
                       (dlen-offset), MSRPC_EPM_REQ);
        return 0;
    }
    hdr->hdl.attr = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    (void)__parse_uuid(pkt, &offset, &hdr->hdl.uuid, rpc_info->data_rep);
    hdr->max_twrs = __pack_uint32(pkt, &offset, rpc_info->data_rep);
#endif

    return offset;
}

static inline uint32_t
__parse_msrpc_rsp_hdr(const uint8_t *pkt, uint32_t dlen,
                      rpc_info_t *rpc_info) {
    static uint32_t  RSP_PDU_SZ = sizeof(msrpc_rsp_hdr_t);
    uint32_t         offset = 0;

    if (dlen < RSP_PDU_SZ) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("PDU Response Len {} is smaller than the header size {}",
                       dlen, RSP_PDU_SZ);
        return 0;
    }

    __pack_uint32(pkt, &offset, rpc_info->data_rep);
    __pack_uint16(pkt, &offset, rpc_info->data_rep);
    offset += 2;

    return offset;
}

static inline uint32_t
__parse_msrpc_epm_rsp_hdr(const uint8_t *pkt, uint32_t dlen,
                          msrpc_cn_common_hdr_t rpc_hdr, uint8_t is64bit,
                          rpc_info_t *rpc_info) {
    static uint32_t      EPM_RSP_SZ = (sizeof(msrpc_epm_rsp_hdr_t) - \
                                  sizeof(msrpc_map_twr_t));
    static uint32_t      EPM_32BIT_RSP_SZ = (EPM_RSP_SZ - 12);
    uint32_t             offset = 0, twr_offset=0;

    bzero(&epm_rsp, sizeof(msrpc_epm_rsp_hdr_t));
    if ((is64bit && dlen < EPM_RSP_SZ) ||
        (!is64bit && dlen < EPM_32BIT_RSP_SZ)) {
        incr_parse_error(rpc_info);
        HAL_TRACE_ERR("EPM RESP Len {} is smaller than the header size {}",
                       dlen, (is64bit)?EPM_RSP_SZ:EPM_32BIT_RSP_SZ);
        return 0;
    }
    epm_rsp.hdl.attr = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    (void)__parse_uuid(pkt, &offset, &epm_rsp.hdl.uuid, rpc_info->data_rep);
    epm_rsp.num_twrs = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    if (is64bit) {
        epm_rsp.max_cnt = __pack_uint64(pkt, &offset, rpc_info->data_rep);
        epm_rsp.offset = __pack_uint64(pkt, &offset, rpc_info->data_rep);
        epm_rsp.actual_cnt = __pack_uint64(pkt, &offset, rpc_info->data_rep);
    } else {
        epm_rsp.max_cnt = __pack_uint32(pkt, &offset, rpc_info->data_rep);
        epm_rsp.offset = __pack_uint32(pkt, &offset, rpc_info->data_rep);
        epm_rsp.actual_cnt = __pack_uint32(pkt, &offset, rpc_info->data_rep);
    }

    if (rpc_info->call_id == rpc_hdr.call_id) {
        twr_offset = __parse_msrpc_epm_map_twr(&pkt[offset], (dlen-offset),
                                           &epm_rsp.twr, is64bit, rpc_info, PDU_RESP);
        if (!twr_offset)
            return twr_offset;
    }
#if 0
    msrpc_twr_p_t       *twr_arr = &epm_rsp.twr.twr_arr;
    if (rpc_info->call_id == rpc_hdr.call_id &&
        (!memcmp(&twr_arr->flrs[0].uuid, &rpc_info->uuid, UUID_BYTES))) {
        // Check only if we have information beyond the default headers
        if (twr_arr->num_floors > MSRPC_DEFAULT_FLOORS) {
            for (uint8_t idx=0; idx<twr_arr->num_floors; idx++) {
                if (twr_arr->flrs[idx].protocol == EPM_PROTO_TCP ||
                    twr_arr->flrs[idx].protocol == EPM_PROTO_UDP) {
                    rpc_info->dport = twr_arr->flrs[idx].port;
                    HAL_TRACE_DEBUG("Setting Dport: {}", rpc_info->dport);
                 } else if (twr_arr->flrs[idx].protocol == EPM_PROTO_IP) {
                    rpc_info->ip.v4_addr = twr_arr->flrs[idx].ip.v4_addr;
                 }
            }
       }
    }
#endif

    HAL_TRACE_VERBOSE("Parsed EPM RSP Header: {}", epm_rsp);

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

    SDK_ASSERT(l4_sess != NULL);

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == true) {
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
                SDK_ASSERT(ret == HAL_RET_OK);
                exp_flow->entry.handler = expected_flow_handler;
                exp_flow->alg = nwsec::APP_SVC_MSFT_RPC;
                /*
                 * Move the RPC info for expected flow from L4 session
                 * to Expected flow
                 */
                exp_flow->info = l4_sess->info;
                l4_sess->info = (rpc_info_t *)g_rpc_state->alg_info_slab()->alloc();
                SDK_ASSERT(l4_sess->info != NULL);
            }
        } else { /* Data session */
            l4_alg_status_t   *ctrl_sess =  g_rpc_state->get_ctrl_l4sess(\
                                                 l4_sess->app_session);

            SDK_ASSERT(ctrl_sess);
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
            }
        }
    }
}

size_t process_msrpc_data_flow(void *ctxt, uint8_t *pkt, size_t pkt_len) {
    fte::ctx_t      *ctx = (fte::ctx_t *)ctxt;
    l4_alg_status_t *exp_flow = (l4_alg_status_t *)alg_status(
                                              ctx->feature_session_state());
    hal_ret_t        ret = HAL_RET_OK;
    l4_alg_status_t *l4_sess = NULL;

    // Todo (Pavithra) Get the Firewall data and make sure that the UUID
    // is still allowed in the config
    /*
     * Alloc L4 Session. This is just to keep a backward reference to the
     * app session that created it.
     */
    ret = g_rpc_state->alloc_and_insert_l4_sess(exp_flow->app_session, &l4_sess);
    SDK_ASSERT_RETURN((ret == HAL_RET_OK), ret);
    l4_sess->alg = nwsec::APP_SVC_MSFT_RPC;
    l4_sess->isCtrl = FALSE;

    // Register completion handler and session state
    ctx->register_completion_handler(msrpc_completion_hdlr);
    ctx->register_feature_session_state(&l4_sess->fte_feature_state);

    // Decrement the ref count for the expected flow
    dec_ref_count(&exp_flow->entry);
    return ret;
}

static inline int get_is_uuid_allowed(rpc_info_t *rpc_info) {
    rpc_uuid_t *rpc = (rpc_uuid_t *)rpc_info->pgm_ids;
    uuid_t     *uuid = (uuid_t *)rpc_info->uuid;

    for (uint8_t idx=0; idx<rpc_info->pgmid_sz; idx++) {
        HAL_TRACE_VERBOSE("RPC allowed uuid: {}  received uuid: {}", *((uuid_t *)rpc[idx].uuid), *uuid);
        if (!memcmp(rpc_info->uuid, (uint8_t *)&rpc[idx].uuid, UUID_BYTES)) {
            return rpc[idx].timeout;
        }
    }
    return -1;
}

/*
 * Initialize RPC Info
 */
static void reset_rpc_info(rpc_info_t *rpc_info) {
    if (rpc_info->pkt_len && rpc_info->pkt != NULL) {
        HAL_FREE(hal::HAL_MEM_ALLOC_ALG, rpc_info->pkt);
        rpc_info->pkt_len = 0;
        rpc_info->pkt = NULL;
    }
    //memset(rpc_info, 0, sizeof(rpc_info_t));
    rpc_info->pkt_type = PDU_NONE;
    rpc_info->callback = parse_msrpc_cn_control_flow;
}

size_t parse_msrpc_cn_control_flow(void *ctxt, uint8_t *pkt, size_t pkt_len) {
    fte::ctx_t              *ctx = (fte::ctx_t *)ctxt;
    l4_alg_status_t         *l4_sess = (l4_alg_status_t *)alg_status(
                                                ctx->feature_session_state());
    uint32_t                 rpc_msg_offset = 0;
    msrpc_cn_common_hdr_t    rpc_hdr;
    uint32_t                 pgm_offset = 0;
    rpc_info_t              *rpc_info = (rpc_info_t *)l4_sess->info;

    HAL_TRACE_VERBOSE("In parse_msrpc_cn_control_flow {:p}", (void *)ctxt);

    if (pkt_len < (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       pkt_len, (rpc_msg_offset + sizeof(msrpc_cn_common_hdr_t)));
        return pkt_len;
    }

    pgm_offset = __parse_cn_common_hdr(pkt, rpc_msg_offset, &rpc_hdr, rpc_info);
    if (rpc_hdr.rpc_ver != MSRPC_CN_RPC_VERS || is_unsupported_ptype(rpc_hdr)) {
        return pkt_len;
    }

    /*
     * L7 Fragment reassembly
     */
    if (rpc_hdr.rpc_ver_minor == 0 || rpc_hdr.flags & PFC_LAST_FRAG) {
        rpc_info->rpc_frag_cont = 0;

        // If this is the last frag of a multi-PDU
        // transmission and doesnt have the first & last frag set
        if (!(rpc_hdr.flags & PFC_FIRST_FRAG)) {
            if ((rpc_info->pkt != NULL) &&
                ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ)) {
                memcpy(&rpc_info->pkt[rpc_info->pkt_len], &pkt[pgm_offset], (pkt_len-pgm_offset));
                rpc_info->pkt_len += (pkt_len-pgm_offset);
                pkt = rpc_info->pkt;
                pkt_len = rpc_info->pkt_len;
            } else {
                HAL_TRACE_ERR("Packet len execeeded the Max ALG Fragmented packet sz");
                incr_max_pkt_sz(rpc_info);
                reset_rpc_info(rpc_info);
                return pkt_len;
            }
        }
    } else {
        if (rpc_hdr.ptype != PDU_BIND && rpc_hdr.ptype != PDU_ALTER_CTXT &&
            rpc_hdr.ptype != PDU_BIND_ACK && rpc_hdr.ptype != PDU_ALTER_CTXT_ACK) {
            // No other PDUs except for these can be L7 fragmented
            return pkt_len;
        }

        // First fragment alloc memory and store the packet
        if (rpc_hdr.flags & PFC_FIRST_FRAG) {
            rpc_info->pkt = alloc_rpc_pkt();
            rpc_info->payload_offset = rpc_msg_offset;
        }

        rpc_info->rpc_frag_cont = 1;
        if ((rpc_info->pkt != NULL) &&
            ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ)) {
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
        return pkt_len;
    }

    HAL_TRACE_VERBOSE("Parsed MSRPC Connection oriented header: {}", rpc_hdr);

    switch (rpc_info->pkt_type)
    {
        case PDU_NONE:
            if (rpc_hdr.ptype == PDU_BIND ||
                rpc_hdr.ptype == PDU_ALTER_CTXT) {
                pgm_offset = __parse_msrpc_bind_hdr(&pkt[pgm_offset],
                                        (pkt_len-pgm_offset), rpc_info);
                if (!pgm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }
                rpc_info->pkt_type = rpc_hdr.ptype;
            }
            break;

        case PDU_BIND:
        case PDU_ALTER_CTXT:
            if (rpc_hdr.ptype == PDU_BIND_ACK ||
                rpc_hdr.ptype == PDU_ALTER_CTXT_ACK) {

                rpc_info->pkt_type = PDU_NONE;
                pgm_offset = __parse_msrpc_bind_ack_hdr(&pkt[pgm_offset],
                                             (pkt_len-pgm_offset), rpc_hdr, rpc_info);
                if (!pgm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }
            }
            break;

        case PDU_BIND_ACK:
        case PDU_ALTER_CTXT_ACK:
            if (rpc_hdr.ptype == PDU_REQ) {
                uint32_t            epm_offset=0;

                epm_offset = __parse_msrpc_req_hdr(&pkt[pgm_offset], (pkt_len-pgm_offset),
                                                   rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }
                epm_offset += pgm_offset;

                epm_offset = __parse_msrpc_epm_req_hdr(&pkt[epm_offset], (pkt_len-epm_offset),
                                                       rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }
                rpc_info->call_id = rpc_hdr.call_id;
                rpc_info->pkt_type = rpc_hdr.ptype;
            }
            break;

        case PDU_REQ:
            if (rpc_hdr.ptype == PDU_RESP) {
                uint32_t            epm_offset=0;

                epm_offset = __parse_msrpc_rsp_hdr(&pkt[pgm_offset],
                                                 (pkt_len-pgm_offset), rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }
                epm_offset += pgm_offset;

                epm_offset = __parse_msrpc_epm_rsp_hdr(&pkt[epm_offset], (pkt_len-epm_offset),
                                                       rpc_hdr, rpc_info->msrpc_64bit, rpc_info);
                if (!epm_offset) {
                    reset_rpc_info(rpc_info);
                    return 0;
                }

                HAL_TRACE_VERBOSE("RPC INFO DPORT: {}", rpc_info->dport);
                // If the IP address is not filled in we assume that the sender is the
                // server and use that.
                if (!rpc_info->ip.v4_addr)
                    rpc_info->ip.v4_addr = ctx->key().sip.v4_addr;

                int timeout = get_is_uuid_allowed(rpc_info);
                if (g_rpc_state && rpc_info->dport && timeout != -1)
                    insert_rpc_expflow(*ctx, l4_sess, process_msrpc_data_flow,
                                              (uint32_t)timeout);

                rpc_info->pkt_type = PDU_NONE;
            }
            break;

        default:
            break;
    };

    HAL_TRACE_VERBOSE("Processed Connection-Oriented MSRPC Header");

    return pkt_len;
}

size_t parse_msrpc_dg_control_flow(void *ctxt, uint8_t *pkt, size_t pkt_len) {
    fte::ctx_t              *ctx = (fte::ctx_t *)ctxt;
    l4_alg_status_t         *exp_flow = (l4_alg_status_t *)alg_status(
                                          ctx->feature_session_state());
    uint32_t                 rpc_msg_offset = 0;
    msrpc_dg_common_hdr_t    rpc_hdr;
    rpc_info_t              *rpc_info = NULL;

    rpc_info = (rpc_info_t *)exp_flow->info;
    if (pkt_len < (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t))) {
        HAL_TRACE_ERR("Cannot process further -- packet len: {} is smaller than expected: {}",
                       pkt_len, (rpc_msg_offset + sizeof(msrpc_dg_common_hdr_t)));
        incr_parse_error(rpc_info);
        return pkt_len;
    }

    __parse_dg_common_hdr(pkt, rpc_msg_offset, &rpc_hdr, rpc_info);
    if (rpc_hdr.rpc_ver != MSRPC_DG_RPC_VERS || 
        (rpc_hdr.ptype != PDU_REQ && rpc_hdr.ptype != PDU_RESP)) {
        return pkt_len;
    }

    HAL_TRACE_VERBOSE("Parsed MSRPC Connectionless header: {}", rpc_hdr);

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
            ctx->set_valid_rflow(false);

            // Register completion handler and feature session state
            ctx->register_completion_handler(msrpc_completion_hdlr);
            ctx->register_feature_session_state(&exp_flow->fte_feature_state);
        }
    } else {
        rpc_info = (rpc_info_t *)exp_flow->info;
        if ((rpc_info->pkt_type == PDU_REQ && rpc_hdr.ptype == PDU_RESP) &&
             (rpc_info->call_id == rpc_hdr.seqnum &&
              (!memcmp(&rpc_info->act_id, &rpc_hdr.act_id, UUID_BYTES)))) {
            HAL_TRACE_VERBOSE("Received matching PDU response key: {}", ctx->key());
            // Register completion handler
            ctx->register_completion_handler(msrpc_completion_hdlr);
        }
    }

    return pkt_len;
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
    uint32_t              payload_offset = ctx.cpu_rxhdr()->payload_offset;
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;
    rpc_info_t           *rpc_info = NULL;
    app_session_t        *app_sess = NULL;
    uint8_t               rc = 0;

    HAL_TRACE_VERBOSE("In alg_msrpc_exec {:p}", (void *)l4_sess);
    if (sfw_info->alg_proto == nwsec::APP_SVC_MSFT_RPC &&
        (!ctx.existing_session())) {
        /*
         * ALG is hit - install Mcast flows and process first packet for UDP
         */
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            /*
             * Alloc APP session, L4 Session and RPC info
             */
            ret = g_rpc_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK || ret == HAL_RET_ENTRY_EXISTS), ret);

            ret = g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK), ret);
            SDK_ASSERT(l4_sess != NULL);
            l4_sess->alg = nwsec::APP_SVC_MSFT_RPC;
            rpc_info = (rpc_info_t *)g_rpc_state->alg_info_slab()->alloc();
            SDK_ASSERT_RETURN((rpc_info != NULL), HAL_RET_OOM);
            l4_sess->isCtrl = true;
            l4_sess->info = rpc_info;
            reset_rpc_info(rpc_info);
            copy_sfw_info(sfw_info, l4_sess);
            //Register feature session state
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);

            /*
             * For connection-less MSRPC, the requested call
             * is with the PDU_REQ packet. The response from the
             * server would come back with a different sport.
             */
            if (ctx.key().proto == IP_PROTO_UDP) {
                uint8_t *pkt = ctx.pkt();
                rc = parse_msrpc_dg_control_flow(&ctx, &pkt[payload_offset],
                                            (ctx.pkt_len()-payload_offset));
                if (!rc) {
                    HAL_TRACE_ERR("Failed to parse connection-less MSRPC header");
                    return HAL_RET_ERR;
                }
            } else {
                flowupd.type = fte::FLOWUPD_MCAST_COPY;
                flowupd.mcast_info.mcast_en = 1;
                flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
                flowupd.mcast_info.proxy_mcast_ptr = 0;
                ret = ctx.update_flow(flowupd);

                if ((ctx.cpu_rxhdr()->tcp_flags & (TCP_FLAG_SYN)) == TCP_FLAG_SYN) {
                    HAL_TRACE_VERBOSE("Setting up buff for Iflow");
                    // Setup TCP buffer for IFLOW
                    l4_sess->tcpbuf[DIR_IFLOW] = tcp_buffer_t::factory(
                                              htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1,
                                              NULL, parse_msrpc_cn_control_flow,
                                              g_rpc_tcp_buffer_slabs);
                }
                // Register completion handler
                ctx.register_completion_handler(msrpc_completion_hdlr);
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
    } else if (l4_sess && l4_sess->info && (ctx.role() == hal::FLOW_ROLE_INITIATOR)) {
        uint8_t *pkt = ctx.pkt();
        rpc_info = (rpc_info_t *)l4_sess->info;

        if (l4_sess->isCtrl == true && ctx.key().proto == IP_PROTO_TCP) {
            if (!l4_sess->tcpbuf[DIR_RFLOW] && ctx.is_flow_swapped() &&
                (ctx.pkt_len() == payload_offset)) {
                HAL_TRACE_VERBOSE("Setting up buffer for rflow");
                // Set up TCP buffer for RFLOW
                l4_sess->tcpbuf[DIR_RFLOW] = tcp_buffer_t::factory(
                                          htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1,
                                           NULL, parse_msrpc_cn_control_flow,
                                           g_rpc_tcp_buffer_slabs);
            }

            /*
             * This will only be executed for control channel packets that
             * would lead to opening up pinholes for FTP data sessions.
             */
            uint8_t buff = ctx.is_flow_swapped()?1:0;
            if (ctx.payload_len() &&
                (l4_sess->tcpbuf[0] && l4_sess->tcpbuf[1])) {
                l4_sess->tcpbuf[buff]->insert_segment(ctx, rpc_info->callback);
            }
        } else {
            /*
             * Parse Control session data && Process Expected flows
             */
            rpc_info->callback(&ctx, &pkt[payload_offset],
                                  ctx.payload_len());
        }
    }

    return ret;
}

} // namespace alg_rpc
} // namespace plugins
} // namespace hal
