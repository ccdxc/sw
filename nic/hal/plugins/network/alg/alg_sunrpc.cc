#include <string.h>
#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>
#include "nic/third-party/libtirpc/export/rpcb_prot.h"
#include "nic/hal/src/session.hpp"
#include "nic/include/fte_db.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/include/ip.h"

#define WORD_BYTES   4
#define ADDR_NETID_BYTES 128

// Mininum length of call and reply header required
// for parsing. Auth/cred len is variable so taking
// minimal required for those.
#define CALL_HDR_LEN (8*WORD_BYTES)
#define REPLY_HDR_LEN (5*WORD_BYTES)
#define LAST_RECORD_FRAG 0x80
#define RPC_CALL 0
#define RPC_REPLY 1

typedef struct rp__list rpcblist;

namespace hal {
namespace net {

void
__parse_rpcb_res_hdr(const uint8_t *pkt, uint32_t offset, char *uaddr)
{
    uint32_t len = 0;
    
    len = __pack_uint32(pkt, &offset);
    memcpy(uaddr, &pkt[offset], len);
}

uint32_t
__parse_rpcb_entry(const uint8_t *pkt, uint32_t offset, struct rpcb *rpcb)
{
    uint32_t len = 0;

    rpcb->r_prog = __pack_uint32(pkt, &offset);
    rpcb->r_vers = __pack_uint32(pkt, &offset);
    // Even if the netid or Uaddr is not a multiple of 4
    // zeroes are written to make it a multiple of 4
    len = __pack_uint32(pkt, &offset);
    memcpy(rpcb->r_netid, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, &offset);
    memcpy(rpcb->r_addr, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, &offset);
    memcpy(rpcb->r_owner, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;

    return offset;
}

uint32_t
__parse_pmap_hdr(const uint8_t *pkt, uint32_t offset, struct pmaplist *pmap)
{
    pmap->pml_map.pm_prog = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_vers = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_prot = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_port = __pack_uint32(pkt, &offset);    
   
    return (offset);
}

uint32_t
__parse_call_hdr(const uint8_t *pkt, uint32_t offset, struct rpc_msg *cmsg)
{
    uint32_t len = 0;
   
    cmsg->rm_call.cb_rpcvers = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_prog = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_vers = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_proc = __pack_uint32(pkt, &offset);
    // Move the offset after Auth credentials and verif
    len = __pack_uint32(pkt, &offset);
    if (len == 0) 
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    len = __pack_uint32(pkt, &offset);
    if (len == 0)
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len; 

    return offset;
}

uint32_t
__parse_reply_hdr(const uint8_t *pkt, uint32_t offset, struct rpc_msg *rmsg)
{
    uint32_t len = 0;

    rmsg->rm_reply.rp_stat = (reply_stat)__pack_uint32(pkt, &offset);
    if (rmsg->rm_reply.rp_stat == MSG_ACCEPTED) {

        // Move the offset after Auth credentials and verif
        len = __pack_uint32(pkt, &offset);
        if (len == 0)
            len = 4;
        offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
        rmsg->acpted_rply.ar_stat = (accept_stat)__pack_uint32(pkt, &offset);
    }
 
    return offset;
}

uint32_t
__parse_rpc_msg(const uint8_t *pkt, uint32_t offset, uint32_t pkt_len, struct rpc_msg *msg)
{
    uint32_t call_hdr_offset=0, reply_hdr_offset=0;

    msg->rm_xid = __pack_uint32(pkt, &offset);
    msg->rm_direction = (msg_type)__pack_uint32(pkt, &offset);
   
    HAL_TRACE_DEBUG("xid: {} direction: {}", msg->rm_xid, msg->rm_direction); 
    if (msg->rm_direction == RPC_CALL) {
        call_hdr_offset = (offset + CALL_HDR_LEN);
        if (pkt_len >= call_hdr_offset) {
            return (__parse_call_hdr(pkt, offset, msg));
        } else {
            HAL_TRACE_ERR("RPC Message parsing failed -- packet len: {} "\
                          "is smaller than call hdr offset: {}",\
                          pkt_len, call_hdr_offset);
        }        
    } else {
        reply_hdr_offset = (offset + REPLY_HDR_LEN);
        if (pkt_len >= reply_hdr_offset) { 
            return (__parse_reply_hdr(pkt, offset, msg));
        } else {
            HAL_TRACE_ERR("RPC Message parsing failed -- packet len: {}" \
                           " is smaller than reply hdr offset: {}", pkt_len,
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

static inline uint8_t
netid2addrfamily(char *netid)
{
    if (strstr(netid, "6") != NULL) {
        return IP_PROTO_IPV6;
    } else {
        return IP_PROTO_IPV4;
    }
}

static inline uint8_t 
isNullip(ipvx_addr_t ip, uint8_t addr_family) {
    uint8_t null_ipv6[16];
  
    memset(null_ipv6, 0, sizeof(null_ipv6));
    if ((addr_family == IP_PROTO_IPV6 && 
        memcmp(ip.v6_addr.addr8, null_ipv6, sizeof(null_ipv6))) ||
        (addr_family == IP_PROTO_IPV4 && !ip.v4_addr)) {
        return 1;
    }

    return 0;
}

static inline void
decodeuaddr(char *uaddr, ipvx_addr_t *ip, 
            uint32_t *dport, uint8_t addr_family) {
    unsigned int port = 0, porthi = 0, portlo = 0;
    char *p = NULL;
    char addrstr[128];
    ipvx_addr_t ipaddr;

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
      
        HAL_TRACE_DEBUG("Addr string: {}", addrstr);
        if (addr_family == IP_PROTO_IPV6) {
            inet_pton(AF_INET6, addrstr, (void *)ipaddr.v6_addr.addr8); 
        } else {
            inet_pton(AF_INET, addrstr, (void *)&ipaddr.v4_addr);
        }
    }

done:
    *dport = port;
    *ip    = ipaddr;
    return;
}

hal_ret_t
parse_sunrpc_control_flow(fte::ctx_t& ctx)
{
    const uint8_t           *pkt = ctx.pkt();
    uint32_t                 rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    uint32_t                 pgm_offset = 0;
    fte::alg_entry_t        *alg_entry = NULL;
    struct rpc_msg           rpc_msg;
    fte::RPCMap             *map = NULL;
    pmaplist                 pmap_list;
    rpcblist                 rpcb_list;
    uint8_t                  idx = 0, addr_family = 0;
    char                     netid[128], uaddr[128], owner[128];

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }

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

    alg_entry->rpcinfo.rpc_frag_cont = 0;
    if (ctx.key().proto == IP_PROTO_TCP) {
        // Do we need to maintain the record tracking state for TCP ?
        if (!(pkt[rpc_msg_offset] & LAST_RECORD_FRAG)) {
            HAL_TRACE_DEBUG("Fragmented packet detected");
            alg_entry->rpcinfo.rpc_frag_cont = 1;
        } 
        rpc_msg_offset += WORD_BYTES;
    }

    pgm_offset = __parse_rpc_msg(pkt, rpc_msg_offset, ctx.pkt_len(), &rpc_msg);
    if (!pgm_offset) {
        return HAL_RET_ERR;
    }

    switch (alg_entry->alg_proto_state)
    {
        case fte::ALG_PROTO_STATE_SUNRPC_INIT:
            HAL_TRACE_DEBUG("RPC Proc: {}", rpc_msg.rm_call.cb_proc);
            if (rpc_msg.rm_direction == 0 && \
                rpc_msg.rm_call.cb_prog == PMAPPROG && \
                (rpc_msg.rm_call.cb_proc == PMAPPROC_GETPORT || \
                 rpc_msg.rm_call.cb_proc == PMAPPROC_DUMP)) {
                 map = &alg_entry->rpcinfo.rpc_map;
                switch (rpc_msg.rm_call.cb_proc) {
                    case PMAPPROC_GETPORT:
                        if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                            rpc_msg.rm_call.cb_vers == RPCBVERS_4) {
                            rpcb_list.rpcb_map.r_netid = &netid[0];
                            rpcb_list.rpcb_map.r_addr = &uaddr[0];
                            rpcb_list.rpcb_map.r_owner = &owner[0];
                            __parse_rpcb_entry(pkt, pgm_offset, 
                                                   &rpcb_list.rpcb_map);
                            map->xid = rpc_msg.rm_xid;
                            alg_entry->rpcinfo.rpcvers = rpc_msg.rm_call.cb_vers;
                            map->prog_num = rpcb_list.rpcb_map.r_prog;
                            map->vers = rpcb_list.rpcb_map.r_vers;
                            map->prot = netid2proto(rpcb_list.rpcb_map.r_netid);
                            addr_family = netid2addrfamily(rpcb_list.rpcb_map.r_netid);
                            decodeuaddr(rpcb_list.rpcb_map.r_addr, &map->ip, &map->dport, 
                                        addr_family);
                            if (isNullip(map->ip, addr_family)) {
                                map->ip = ctx.key().dip;
                            } 
                        } else {
                            __parse_pmap_hdr(pkt, pgm_offset, &pmap_list);
                            map->xid = rpc_msg.rm_xid;
                            alg_entry->rpcinfo.rpcvers = 2;
                            map->prog_num = pmap_list.pml_map.pm_prog; 
                            map->prot  = pmap_list.pml_map.pm_prot;
                            map->dport = pmap_list.pml_map.pm_port;
                            map->vers  = pmap_list.pml_map.pm_vers;
                            map->ip    = ctx.key().dip;
                        }
                        alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_GETPORT;
                        break;
                  
                    case PMAPPROC_DUMP:
                        alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_RPC_DUMP;
                        alg_entry->rpcinfo.rpcvers = rpc_msg.rm_call.cb_vers;
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
 
                map = &alg_entry->rpcinfo.rpc_map; 
                if (map[idx].xid == rpc_msg.rm_xid) {
                    // Parse the header to save the details
                    if (map->vers == RPCBVERS_3 ||
                        map->vers == RPCBVERS_4) { 
                        __parse_rpcb_res_hdr(pkt, pgm_offset, &uaddr[0]);
                        decodeuaddr(rpcb_list.rpcb_map.r_addr, &map->ip, &map->dport,
                                       addr_family);
                        if (isNullip(map->ip, addr_family)) {
                            map->ip = ctx.key().dip;
                        }
                    } else {
                        map[idx].dport = __pack_uint32(pkt, &pgm_offset);
                    } 
                }

                // Insert an ALG entry for the DIP, Dport
                insert_rpc_entry(ctx, map, fte::ALG_PROTO_STATE_SUNRPC_DATA);
                alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_SUNRPC_INIT;
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_DUMP:
            HAL_TRACE_DEBUG("Reply state: {} accp state: {}", rpc_msg.rm_reply.rp_stat, 
                             rpc_msg.acpted_rply.ar_stat);
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.acpted_rply.ar_stat == 0) {
           
                map = &alg_entry->rpcinfo.rpc_map;
                while (__pack_uint32(pkt, &pgm_offset) == 1) {
                    HAL_TRACE_DEBUG("cb vers: {}", alg_entry->rpcinfo.rpcvers);
                    // Todo (Pavithra) search if the program already
                    // exists in the list
                    if (alg_entry->rpcinfo.rpcvers == RPCBVERS_3 ||
                        alg_entry->rpcinfo.rpcvers == RPCBVERS_4) { 
                        memset(&rpcb_list, 0, sizeof(rpcb_list));
                        rpcb_list.rpcb_map.r_netid = &netid[0];
                        rpcb_list.rpcb_map.r_addr = &uaddr[0];
                        rpcb_list.rpcb_map.r_owner = &owner[0];
                        memset(uaddr, 0, ADDR_NETID_BYTES);
                        memset(netid, 0, ADDR_NETID_BYTES);
                        memset(owner, 0, ADDR_NETID_BYTES);
                        pgm_offset = __parse_rpcb_entry(pkt, pgm_offset,
                                                    &rpcb_list.rpcb_map);
                        map->prog_num = rpcb_list.rpcb_map.r_prog;
                        map->vers = rpcb_list.rpcb_map.r_vers;
                        map->prot = netid2proto(rpcb_list.rpcb_map.r_netid);
                        addr_family = netid2addrfamily(rpcb_list.rpcb_map.r_netid);
                        decodeuaddr(rpcb_list.rpcb_map.r_addr, &map->ip, &map->dport,
                                       addr_family);
                        if (isNullip(map->ip, addr_family)) {
                            map->ip = ctx.key().dip;
                        }
                        HAL_TRACE_DEBUG("Dump entry dport: {}", map->dport);
                    } else {
                        __parse_pmap_hdr(pkt, pgm_offset, &pmap_list);
                        map->xid = rpc_msg.rm_xid;
                        map->prog_num = pmap_list.pml_map.pm_prog;
                        map->prot  = pmap_list.pml_map.pm_prot;
                        map->dport = pmap_list.pml_map.pm_port;
                        map->vers  = pmap_list.pml_map.pm_vers;    
                    }
                    // Insert ALG Entry for DIP, Dport
                    insert_rpc_entry(ctx, map, fte::ALG_PROTO_STATE_SUNRPC_DATA);
                };
                if (!alg_entry->rpcinfo.rpc_frag_cont) {
                    alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_SUNRPC_INIT;
                }
            }
            break;

        case fte::ALG_PROTO_STATE_RPC_CALLIT:
            return HAL_RET_NOOP;

        default:
            break;
    }

    HAL_TRACE_DEBUG("Updated the existing entry: {}", *alg_entry);

    return HAL_RET_OK;
}

hal_ret_t
process_sunrpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;
    fte::alg_entry_t     *alg_entry = NULL;
    hal::plugins::sfw::sfw_info_t *sfw_info =
        (hal::plugins::sfw::sfw_info_t*)ctx.feature_state(hal::plugins::sfw::FTE_FEATURE_SFW);

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }

    if (sfw_info->alg_proto == nwsec::APP_SVC_SUN_RPC) { 
        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_info.mcast_en = 1;
        flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
     
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_SUNRPC_INIT;

            // UDP could have the portmapper queries at the 
            // start of the session
            if (ctx.key().proto == IP_PROTO_UDP) {
                ret = parse_sunrpc_control_flow(ctx); 
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("SUN RPC ALG parse for UDP frame failed");
                    return ret;
                }
            } else {
                alg_entry->alg_proto_state = fte::ALG_PROTO_STATE_SUNRPC_INIT;
                ret = ctx.update_flow(flowupd);
           }
           ctx.register_completion_handler(fte::alg_completion_hdlr);
        } else { /* Responder flow */
           if (ctx.key().proto == IP_PROTO_UDP)
               alg_entry->entry.key = ctx.get_key(hal::FLOW_ROLE_RESPONDER); 
           ret = ctx.update_flow(flowupd);
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
