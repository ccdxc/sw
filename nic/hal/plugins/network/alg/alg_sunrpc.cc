#include <string.h>
#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>
#include "nic/third-party/libtirpc/export/rpcb_prot.h"
#include "nic/hal/src/session.hpp"
#include "nic/include/fte_db.hpp"
#include "nic/p4/nw/include/defines.h"

#define WORD_BYTES   4
#define ADDR_NETID_BYTES 128
#define CALL_HDR_LEN (sizeof(struct call_body) + (2*WORD_BYTES))
#define REPLY_HDR_LEN (sizeof(struct reply_body) + (2*WORD_BYTES))
#define LAST_RECORD_FRAG 0x80

typedef struct rp__list rpcblist;

namespace hal {
namespace net {

void
__parse_rpcb_res_hdr(const uint8_t *pkt, uint8_t offset, char *uaddr)
{
    uint32_t len = 0;
    
    len = __pack_uint32(pkt, offset);
    memcpy(uaddr, &pkt[offset], len);
}

uint32_t
__parse_rpcb_entry(const uint8_t *pkt, uint8_t offset, struct rpcb *rpcb)
{
    uint32_t len = 0;

    rpcb->r_prog = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    rpcb->r_vers = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    len = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    memcpy(rpcb->r_netid, &pkt[offset], len);
    // Even if the netid or Uaddr is not a multiple of 4
    // zeroes are written to make it a multiple of 4
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    memcpy(rpcb->r_netid, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, offset);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;

    return offset;
}

uint32_t
__parse_pmap_hdr(const uint8_t *pkt, uint8_t offset, struct pmaplist *pmap)
{
    pmap->pml_map.pm_prog = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    pmap->pml_map.pm_vers = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    pmap->pml_map.pm_prot = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    pmap->pml_map.pm_port = __pack_uint32(pkt, offset);    
    offset += WORD_BYTES;
   
    return (offset);
}

uint32_t
__parse_call_hdr(const uint8_t *pkt, uint8_t offset, struct rpc_msg *cmsg)
{
    uint32_t len = 0;
   
    cmsg->rm_call.cb_rpcvers = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_prog = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_vers = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_proc = __pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    // Move the offset after Auth credentials and verif
    len = __pack_uint32(pkt, offset);
    if (len == 0) 
        len = 4;
    offset += WORD_BYTES;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, offset);
    if (len == 0)
        len = 4;
    offset += WORD_BYTES;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len; 

    return offset;
}

uint32_t
__parse_reply_hdr(const uint8_t *pkt, uint8_t offset, struct rpc_msg *rmsg)
{
    uint32_t len = 0;

    rmsg->rm_reply.rp_stat = (reply_stat)__pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    if (rmsg->rm_reply.rp_stat == MSG_ACCEPTED) {

        // Move the offset after Auth credentials and verif
        len = __pack_uint32(pkt, offset);
        if (len == 0)
            len = 4;
        offset += WORD_BYTES;
        offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
        rmsg->acpted_rply.ar_stat = (accept_stat)__pack_uint32(pkt, offset);
        offset += WORD_BYTES;
    }
 
    return offset;
}

uint32_t
__parse_rpc_msg(const uint8_t *pkt, uint8_t offset, uint32_t pkt_len, struct rpc_msg *msg)
{
    uint32_t call_hdr_offset=0, reply_hdr_offset=0;

    msg->rm_xid = __pack_uint32(pkt, offset);
    offset += WORD_BYTES; 
    msg->rm_direction = (msg_type)__pack_uint32(pkt, offset);
    offset += WORD_BYTES;
    
    if (msg->rm_direction == CALL) {
        call_hdr_offset = (offset + CALL_HDR_LEN);
        if (pkt_len >= call_hdr_offset) {
            return (__parse_call_hdr(pkt, offset, msg));
        } else {
            HAL_TRACE_ERR("RPC Message parsing failed -- packet len: {} \
                           is smaller than call hdr offset: {}", pkt_len,
                           call_hdr_offset);
        }        
    } else {
        reply_hdr_offset = (offset + REPLY_HDR_LEN);
        if (pkt_len >= reply_hdr_offset) { 
            return (__parse_reply_hdr(pkt, offset, msg));
        } else {
            HAL_TRACE_ERR("RPC Message parsing failed -- packet len: {} \
                           is smaller than call hdr offset: {}", pkt_len,
                           reply_hdr_offset);
        }
    }

    return 0;
}

static inline uint32_t
netid2proto(char *netid)
{
    if (strstr(netid, "tcp") != NULL) {
        return IP_PROTO_TCP;
    } else if (strstr(netid, "udp") != NULL) {
        return IP_PROTO_UDP;
    }
    
    return 0;
}

static inline fte::alg_addr_family_t 
netid2addrfamily(char *netid)
{
    if (strstr(netid, "6") != NULL) {
        return fte::ALG_ADDRESS_FAMILY_IPV6;
    } else {
        return fte::ALG_ADDRESS_FAMILY_IPV4;
    }
}
 
static inline uint32_t
uaddr2dport(char *uaddr) {
    unsigned int port = 0, porthi = 0, portlo = 0;
    char *p = NULL;
    char addrstr[128];

    memcpy(addrstr, uaddr, strlen(uaddr));
    if (addrstr[0] != '/') {
        p = strrchr(addrstr, '.');
        if (p == NULL)
            goto done;
        portlo = (unsigned)atoi(p + 1);
        *p = '\0';

        p = strrchr(addrstr, '.');
        if (p == NULL)
            goto done;
        porthi = (unsigned)atoi(p + 1);

        *p = '\0';
        port = (porthi << 8) | portlo;
    }

done:
    return port;
}

hal_ret_t
parse_sunrpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t                ret = HAL_RET_OK;
    const uint8_t           *pkt = ctx.pkt();
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    uint8_t                  pgm_offset = 0;
    fte::alg_entry_t         newentry = ctx.alg_entry();
    struct rpc_msg           rpc_msg;
    fte::RPCMap             *map = NULL;
    pmaplist                 pmap_list;
    rpcblist                 rpcb_list;
    uint8_t                  idx = 0, update_entry = 0;
    char                     netid[128], uaddr[128];

    // Payload offset from CPU header
    if (ctx.pkt_len() == rpc_msg_offset && 
        ctx.key().proto == IP_PROTO_TCP) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (ctx.pkt_len() <= (unsigned int)(rpc_msg_offset + 3*WORD_BYTES)) {
        // Packet length is smaller than the RPC common header
        // Size. We cannot process this packet.
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {} rpc_msg size: {}", \
                      ctx.pkt_len(),  rpc_msg_offset);
        return HAL_RET_ERR;
    }

    newentry.rpc_frag_cont = 0;
    if (ctx.key().proto == IP_PROTO_TCP) {
        // Do we need to maintain the record tracking state for TCP ?
        if (!(pkt[rpc_msg_offset] & LAST_RECORD_FRAG)) {
            HAL_TRACE_DEBUG("Fragmented packet detected");
            newentry.rpc_frag_cont = 1;
        } 
        update_entry = 1;
        rpc_msg_offset += WORD_BYTES;
    }

    pgm_offset = __parse_rpc_msg(pkt, rpc_msg_offset, ctx.pkt_len(), &rpc_msg);
    if (!pgm_offset) {
        return HAL_RET_ERR;
    }

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            HAL_TRACE_DEBUG("RPC Proc: {}", rpc_msg.rm_call.cb_proc);
            if (rpc_msg.rm_direction == 0 && \
                rpc_msg.rm_call.cb_prog == PMAPPROG && \
                (rpc_msg.rm_call.cb_proc == PMAPPROC_GETPORT || \
                 rpc_msg.rm_call.cb_proc == PMAPPROC_DUMP)) {
                 map = &newentry.rpc_map.maps[newentry.rpc_map.num_map];
                //Calculate the offset of the call header alone
                switch (rpc_msg.rm_call.cb_proc) {
                    case PMAPPROC_GETPORT:
                        if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                            rpc_msg.rm_call.cb_vers == RPCBVERS_4) {
                            rpcb_list.rpcb_map.r_netid = &netid[0];
                            rpcb_list.rpcb_map.r_addr = &uaddr[0];
                            __parse_rpcb_entry(pkt, pgm_offset, 
                                                   &rpcb_list.rpcb_map);
                            map->xid = rpc_msg.rm_xid;
                            map->prog_num = rpcb_list.rpcb_map.r_prog;
                            map->vers = rpcb_list.rpcb_map.r_vers;
                            map->prot = netid2proto(rpcb_list.rpcb_map.r_netid);
                            map->dport = uaddr2dport(rpcb_list.rpcb_map.r_addr);     
                        } else {
                            __parse_pmap_hdr(pkt, pgm_offset, &pmap_list);
                            map->xid = rpc_msg.rm_xid;
                            map->prog_num = pmap_list.pml_map.pm_prog; 
                            map->prot  = pmap_list.pml_map.pm_prot;
                            map->dport = pmap_list.pml_map.pm_port;
                            map->vers  = pmap_list.pml_map.pm_vers;
                        }

                        newentry.rpc_map.num_map++;
                        newentry.alg_proto_state = fte::ALG_PROTO_STATE_RPC_GETPORT;
                        break;
                  
                    case PMAPPROC_DUMP:
                        HAL_TRACE_DEBUG("Dump request received");
                        newentry.alg_proto_state = fte::ALG_PROTO_STATE_RPC_DUMP;
                        break;
 
                    default:
                        break;
                };
            } 
            break;

        case fte::ALG_PROTO_STATE_RPC_GETPORT:
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.rm_reply.rp_acpt.ar_stat == 0) {
                uint32_t dport = 0;
 
                // Parse the header to save the details
                if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                    rpc_msg.rm_call.cb_vers == RPCBVERS_4) {
                    __parse_rpcb_res_hdr(pkt, pgm_offset, &uaddr[0]);
                    dport =  uaddr2dport(uaddr);
                } else {
                    dport = (uint16_t)__pack_uint32(pkt, pgm_offset); 
                }

                map = ctx.alg_entry().rpc_map.maps; 
                for (idx = 0; idx < ctx.alg_entry().rpc_map.num_map; idx++) {
                     if (map[idx].xid == rpc_msg.rm_xid) { 
                         map[idx].dport = dport;
                         break;
                     } 
                }

                update_entry = 1; 
                // Insert an ALG entry for the DIP, Dport
                insert_rpc_entry(ctx, &map[idx]);
                newentry.alg_proto_state = fte::ALG_PROTO_STATE_RPC_INIT;
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_DUMP:
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.acpted_rply.ar_stat == 0) {
           
                map = newentry.rpc_map.maps;
           
                HAL_TRACE_DEBUG("Dump reply detected: offset1: {} offset2: {} offset3: {} offset4: {}", 
                               pkt[pgm_offset], pkt[pgm_offset+1], pkt[pgm_offset+2], pkt[pgm_offset+3]);

                rpcb_list.rpcb_map.r_netid = &netid[0];
                rpcb_list.rpcb_map.r_addr = &uaddr[0]; 
                while (__pack_uint32(pkt, pgm_offset) == 1) {
                    pgm_offset += WORD_BYTES;
                    // Todo (Pavithra) search if the program already
                    // exists in the list
                    if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                        rpc_msg.rm_call.cb_vers == RPCBVERS_4) { 
                        memset(&rpcb_list, 0, sizeof(rpcb_list));
                        memset(&uaddr, 0, ADDR_NETID_BYTES);
                        memset(&netid, 0, ADDR_NETID_BYTES);
                        pgm_offset = __parse_rpcb_entry(pkt, pgm_offset,
                                                    &rpcb_list.rpcb_map);
                        map[newentry.rpc_map.num_map].prog_num = 
                                                    rpcb_list.rpcb_map.r_prog;
                        map[newentry.rpc_map.num_map].vers = 
                                                   rpcb_list.rpcb_map.r_vers;
                        map[newentry.rpc_map.num_map].prot = 
                                      netid2proto(rpcb_list.rpcb_map.r_netid);
                        map[newentry.rpc_map.num_map].dport = 
                                       uaddr2dport(rpcb_list.rpcb_map.r_addr); 
                    } else {
                        __parse_pmap_hdr(pkt, pgm_offset, &pmap_list);
                        map[newentry.rpc_map.num_map].xid = rpc_msg.rm_xid;
                        map[newentry.rpc_map.num_map].prog_num = pmap_list.pml_map.pm_prog;
                        map[newentry.rpc_map.num_map].prot  = pmap_list.pml_map.pm_prot;
                        map[newentry.rpc_map.num_map].dport = pmap_list.pml_map.pm_port;
                        map[newentry.rpc_map.num_map].vers  = pmap_list.pml_map.pm_vers;    
                    }
                    update_entry = 1;
                    // Insert ALG Entry for DIP, Dport
                    insert_rpc_entry(ctx, &map[newentry.rpc_map.num_map]);
                    newentry.rpc_map.num_map++;        
                };
                if (!newentry.rpc_frag_cont) {
                    newentry.alg_proto_state = fte::ALG_PROTO_STATE_RPC_INIT;
                }
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_CALLIT:
            return HAL_RET_NOOP;

        default:
            break;
    }

    if (update_entry) {
        HAL_TRACE_DEBUG("Updating the existing entry: {}", newentry);
        //Update the existing entry
        ret = fte::update_alg_entry(ctx.key(), (void *)&newentry, sizeof(fte::alg_entry_t));
    } else {
        HAL_TRACE_DEBUG("Setting a new entry: {}", newentry);
        // Set the entry
        ctx.set_alg_entry(newentry);
    }

    return ret;
}

hal_ret_t
process_sunrpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;

    if (ctx.alg_proto() == nwsec::APP_SVC_SUN_RPC && \
        ctx.role() == hal::FLOW_ROLE_INITIATOR) {

        ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_INIT);

        // UDP could have the portmapper queries at the 
        // start of the session
        if (ctx.key().proto == IP_PROTO_UDP) {
            ret = parse_sunrpc_control_flow(ctx); 
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
        if (ret == HAL_RET_OK) {
            // Update Responder flow also to do a Mcast copy (redirect for now)
            ret = ctx.update_flow(flowupd, hal::FLOW_ROLE_RESPONDER);
        }
    } 

    return ret;
}

hal_ret_t
process_sunrpc_data_flow(fte::ctx_t& ctx)
{
    // Get the Firewall data and make sure that the program
    // is still allowed in the config

    return HAL_RET_OK;
}

}
}
