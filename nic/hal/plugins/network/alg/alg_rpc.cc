#include <string.h>
#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>
#include "nic/third-party/libtirpc/export/rpcb_prot.h"
#include "nic/hal/src/session.hpp"
#include "nic/include/fte_db.hpp"

#define WORD_BYTES   4
#define ADDR_NETID_BYTES 128

typedef struct rp__list rpcblist;

namespace hal {
namespace net {

static inline uint32_t
__pack_uint32(const uint8_t *buf)
{
    uint8_t idx = 24;
    uint32_t val = 0;
 
    do {
       val |= (val<<idx);
       idx -= 8;   
    } while (idx >= 0);    
 
    return val;
}

void
__parse_rpcb_res_hdr(const uint8_t *pkt, char *uaddr)
{
    uint32_t offset = 0, len = 0;
    
    len = __pack_uint32(&pkt[offset]);
    memcpy(uaddr, &pkt[offset], len);
}

uint32_t
__parse_rpcb_entry(const uint8_t *pkt, struct rpcb *rpcb)
{
    uint32_t offset = 0, len = 0;

    rpcb->r_prog = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    rpcb->r_vers = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    len = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    memcpy(rpcb->r_netid, &pkt[offset], len);
    // Even if the netid or Uaddr is not a multiple of 4
    // zeroes are written to make it a multiple of 4
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    memcpy(rpcb->r_netid, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(&pkt[offset]);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;

    return offset;
}

uint32_t
__parse_pmap_hdr(const uint8_t *pkt, struct pmaplist *pmap)
{
    uint32_t offset = 0;

    pmap->pml_map.pm_prog = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    pmap->pml_map.pm_vers = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    pmap->pml_map.pm_prot = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    pmap->pml_map.pm_port = __pack_uint32(&pkt[offset]);    
    offset += WORD_BYTES;
   
    return (offset);
}

uint32_t
__parse_call_hdr(const uint8_t *pkt, struct rpc_msg *cmsg)
{
    uint32_t offset = 0, len = 0;
   
    cmsg->rm_call.cb_rpcvers = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_prog = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_vers = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    cmsg->rm_call.cb_proc = __pack_uint32(&pkt[offset]);

    // Move the offset after Auth credentials and verif
    len = __pack_uint32(&pkt[offset]);
    if (len == 0) 
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(&pkt[offset]);
    if (len == 0)
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len; 

    return offset;
}

uint32_t
__parse_reply_hdr(const uint8_t *pkt, struct rpc_msg *rmsg)
{
    uint32_t offset = 0, len = 0;

    rmsg->rm_reply.rp_stat = (reply_stat)__pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    if (rmsg->rm_reply.rp_stat == MSG_ACCEPTED) {

        // Move the offset after Auth credentials and verif
        len = __pack_uint32(&pkt[offset]);
        if (len == 0)
            len = 4;
        offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
        rmsg->acpted_rply.ar_stat = (accept_stat)__pack_uint32(&pkt[offset]);
        offset += WORD_BYTES;
        if (rmsg->acpted_rply.ar_stat == SUCCESS) {
            rmsg->acpted_rply.ar_vers.low = (uint16_t)__pack_uint32(&pkt[offset]);
        }
        offset += WORD_BYTES;
    }
 
    return offset;
}

uint32_t
__parse_rpc_msg(const uint8_t *pkt, struct rpc_msg *msg)
{
    uint32_t offset = 0;

    msg->rm_xid = __pack_uint32(&pkt[offset]);
    offset += WORD_BYTES; 
    msg->rm_direction = (msg_type)__pack_uint32(&pkt[offset]);
    offset += WORD_BYTES;
    
    if (msg->rm_direction == CALL) {
        return (__parse_call_hdr(&pkt[offset], msg));
    }
    
    return (__parse_reply_hdr(&pkt[offset], msg));
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

static void 
insert_rpc_entry(fte::ctx_t& ctx, fte::RPCMap *map)
{
    fte::alg_entry_t *entry = NULL;
    hal::flow_key_t key;

    entry = (fte::alg_entry_t *)HAL_CALLOC(alg_entry_t, sizeof(fte::alg_entry_t));
    if (!entry) {
        HAL_TRACE_ERR("Could not allocated memory for RPC ALG entry");
        return;
    }

    memset(&key, 0, sizeof(hal::flow_key_t));
    key.dip = ctx.key().dip;
    key.dport = map->dport;
    key.proto = map->prot;
    key.flow_type = (map->addr_family == fte::ALG_ADDRESS_FAMILY_IPV6)?FLOW_TYPE_V6:FLOW_TYPE_V4;    
    entry->key = key;
    entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_DATA;

    // Save the program number and SUN RPC control dport (could be user specified)
    // We would replace this with the incoming one for Firewall lookup.
    entry->rpc_map.num_map = 1;
    entry->rpc_map.maps[entry->rpc_map.num_map-1].prog_num = map->prog_num;
    entry->rpc_map.maps[entry->rpc_map.num_map-1].vers = map->vers;
    entry->rpc_map.maps[entry->rpc_map.num_map-1].dport = ctx.key().dport;
  
    // Need to alloc and insert here 
    insert_alg_entry(entry); 
}

hal_ret_t
parse_rpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t                ret = HAL_RET_OK;
    const uint8_t           *pkt = ctx.pkt();
    uint8_t                  rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    uint8_t                  pgm_offset = 0;
    fte::alg_entry_t         newentry;
    struct rpc_msg           rpc_msg;
    fte::RPCMap             *map = NULL;
    pmaplist                 pmap_list;
    rpcblist                 rpcb_list;
    uint8_t                  idx = 0;
    char                     netid[128], uaddr[128];

    // Payload offset from CPU header
    if (ctx.pkt_len() == rpc_msg_offset && 
        ctx.key().proto == IP_PROTO_TCP) {
        // The first iflow packet that get mcast copied could be an
        // ACK from the TCP handshake.
        HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
        return HAL_RET_OK;
    }

    if (ctx.pkt_len() < (rpc_msg_offset + sizeof(rpc_msg))) {
        // We cannot process this packet
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  rpc_msg_offset);
        return HAL_RET_ERR;
    }

    if (ctx.key().proto == IP_PROTO_TCP) {
        // Do we need to maintain the record tracking state for TCP ?
        rpc_msg_offset += WORD_BYTES;
        newentry = ctx.alg_entry();
    } else {
        memset(&newentry, 0, sizeof(fte::alg_entry_t));
    }

    pgm_offset = __parse_call_hdr(&pkt[rpc_msg_offset], &rpc_msg);
 
    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            if (rpc_msg.rm_direction == 0 && \
                rpc_msg.rm_call.cb_prog == PMAPPROG && \
                (rpc_msg.rm_call.cb_proc == PMAPPROC_GETPORT || \
                 rpc_msg.rm_call.cb_proc == PMAPPROC_DUMP)) {
                //Calculate the offset of the call header alone
                switch (rpc_msg.rm_call.cb_proc) {
                    case PMAPPROC_GETPORT:
                        if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                            rpc_msg.rm_call.cb_vers == RPCBVERS_4) {
                            rpcb_list.rpcb_map.r_netid = &netid[0];
                            rpcb_list.rpcb_map.r_addr = &uaddr[0];
                            __parse_rpcb_entry(&pkt[pgm_offset], 
                                                   &rpcb_list.rpcb_map);
                        } else {
                            __parse_pmap_hdr(&pkt[pgm_offset], &pmap_list);
                            map = &map[newentry.rpc_map.num_map];
                            map->xid = rpc_msg.rm_xid;
                            map->prog_num = pmap_list.pml_map.pm_prog; 
                            map->prot  = pmap_list.pml_map.pm_prot;
                            map->dport = pmap_list.pml_map.pm_port;
                            map->vers  = pmap_list.pml_map.pm_vers;
                        }

                        newentry.rpc_map.num_map++;
                        ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_GETPORT);
                        break;
                  
                    case PMAPPROC_DUMP:
                        ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_DUMP);
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
                    dport = (uint32_t)rpc_msg.acpted_rply.ar_vers.low; 
                } else {
                    __parse_rpcb_res_hdr(&pkt[pgm_offset], &uaddr[0]);
                    dport =  uaddr2dport(uaddr);
                }

                map = newentry.rpc_map.maps; 
                for (idx = 0; idx < newentry.rpc_map.num_map; idx++) {
                     if (map[idx].xid == rpc_msg.rm_xid) { 
                         map[idx].dport = dport;
                         break;
                     } 
                } 
                // Insert an ALG entry for the DIP, Dport
                insert_rpc_entry(ctx, &map[idx]);
                ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_INIT);
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_DUMP:
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.acpted_rply.ar_stat == 0) {
           
                map = newentry.rpc_map.maps;
            
                rpcb_list.rpcb_map.r_netid = &netid[0];
                rpcb_list.rpcb_map.r_addr = &uaddr[0]; 
                while (__pack_uint32(&pkt[pgm_offset]) == 1) {
                    pgm_offset += WORD_BYTES;
                    if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                        rpc_msg.rm_call.cb_vers == RPCBVERS_4) { 
                        memset(&rpcb_list, 0, sizeof(rpcb_list));
                        memset(&uaddr, 0, ADDR_NETID_BYTES);
                        memset(&netid, 0, ADDR_NETID_BYTES);
                        pgm_offset = __parse_rpcb_entry(&pkt[pgm_offset],
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
                         __parse_pmap_hdr(&pkt[pgm_offset], &pmap_list);
                        map = &map[newentry.rpc_map.num_map];
                        map->xid = rpc_msg.rm_xid;
                        map->prog_num = pmap_list.pml_map.pm_prog;
                        map->prot  = pmap_list.pml_map.pm_prot;
                        map->dport = pmap_list.pml_map.pm_port;
                        map->vers  = pmap_list.pml_map.pm_vers;    
                    }
                    // Insert ALG Entry for DIP, Dport
                    insert_rpc_entry(ctx, &map[newentry.rpc_map.num_map]);
                    newentry.rpc_map.num_map++;        
                };
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_CALLIT:
            return HAL_RET_NOOP;

        default:
            break;
    }

    if (ctx.alg_entry().alg_proto_state == ctx.alg_proto_state()) {
        //Update the existing entry
        fte::update_alg_entry(ctx.key(), (void *)&newentry, sizeof(fte::alg_entry_t));
    } else {
        // Set the entry
        ctx.set_alg_entry(newentry);
    }

    return ret;
}

hal_ret_t
process_rpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;

    if (ctx.alg_proto() == nwsec::APP_SVC_SUN_RPC && \
        ctx.role() == hal::FLOW_ROLE_INITIATOR) {

        ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_INIT);

        // UDP could have the portmapper queries at the 
        // start of the session
        if (ctx.key().proto == IP_PROTO_UDP) {
            ret = parse_rpc_control_flow(ctx); 
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("SUN RPC ALG parse for UDP frame failed");
                return ret;
            }
        }

        ctx.register_completion_handler(fte::alg_completion_hdlr);
        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_copy_en = 1;
        ret = ctx.update_flow(flowupd);
        if (ret == HAL_RET_OK) {
            // Update Responder flow also to do a Mcast copy (redirect for now)
            ret = ctx.update_flow(flowupd, hal::FLOW_ROLE_RESPONDER);
        }
    } 

    return ret;
}

}
}
