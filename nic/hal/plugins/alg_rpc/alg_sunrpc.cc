#include "core.hpp"
#include "alg_sunrpc.hpp"
#include <string.h>
#include <rpc/rpc_msg.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_rmt.h>
#include "nic/include/hal_mem.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "rpcb_prot.h"
#include "nic/p4/iris/include/defines.h"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/alg_utils/alg_db.hpp"

#define ADDR_NETID_BYTES 128
#define LAST_RECORD_FRAG 0x80
#define RPC_CALL 0
#define RPC_REPLY 1

typedef struct rp__list rpcblist;

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

uint32_t __parse_rpcb_res_hdr(const uint8_t *pkt, uint32_t dlen, char *uaddr) {
    uint32_t len = 0, offset = 0;
   
    if (dlen < WORD_BYTES) {
        HAL_TRACE_ERR("Packet len {} is less than a word to parse rpcb response",
                      dlen);
        return 0;
    }
    len = __pack_uint32(pkt, &offset);
    if ((dlen-offset) < len) {
        HAL_TRACE_ERR("Packet len {} is less than Uaddr len {}",
                      (dlen-offset), len);
        return 0;
    }
    memcpy(uaddr, &pkt[offset], len);

    return offset; 
}

uint32_t __parse_rpcb_entry(const uint8_t *pkt, uint32_t dlen, 
                            struct rpcb *rpcb) {
    uint32_t len = 0, offset = 0;

    if (dlen < sizeof(struct rpcb)) {
        HAL_TRACE_ERR("Packet len {} is smaller than RPCB Entry len {}",
                      dlen, sizeof(struct rpcb));
        return 0;
    }
    rpcb->r_prog = __pack_uint32(pkt, &offset);
    rpcb->r_vers = __pack_uint32(pkt, &offset);
    // Even if the netid or Uaddr is not a multiple of 4
    // zeroes are written to make it a multiple of 4
    len = __pack_uint32(pkt, &offset);
    if ((dlen-offset) < len) {
        HAL_TRACE_ERR("Packet len {} is smaller than Netid len {}",
                      (dlen-offset), len);
        return 0;
    }
    memcpy(rpcb->r_netid, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;
    if ((dlen-offset) < WORD_BYTES) {
        HAL_TRACE_ERR("Packet len {} is smaller to parse addr len",
                      (dlen-offset));
        return 0;
    }
    len = __pack_uint32(pkt, &offset);
    if ((dlen-offset) < len) {
        HAL_TRACE_ERR("Packet len {} is smaller than Addr len {}",
                      (dlen-offset), len);
        return 0;
    }
    memcpy(rpcb->r_addr, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;
    if ((dlen-offset) < WORD_BYTES) {
        HAL_TRACE_ERR("Packet len {} is smaller to parse owner len",
                      (dlen-offset));
        return 0;
    }
    len = __pack_uint32(pkt, &offset);
    if ((dlen-offset) < len) {
        HAL_TRACE_ERR("Packet len {} is smaller than owner len {}",
                      (dlen-offset), len);
        return 0;
    }
    memcpy(rpcb->r_owner, &pkt[offset], len);
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES - len%WORD_BYTES)):len;

    return offset;
}

uint32_t __parse_pmap_hdr(const uint8_t *pkt, uint32_t dlen, 
                          struct pmaplist *pmap) {
    static uint32_t PMAP_HDR_SZ = 4 * WORD_BYTES;
    uint32_t offset = 0;

    if (dlen < PMAP_HDR_SZ) {
        HAL_TRACE_ERR("Packet len {} is smaller than pmap hdr {}",
                      dlen, PMAP_HDR_SZ);
        return 0;
    }

    pmap->pml_map.pm_prog = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_vers = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_prot = __pack_uint32(pkt, &offset);
    pmap->pml_map.pm_port = __pack_uint32(pkt, &offset);    
   
    return (offset);
}

uint32_t __parse_callit_req(const uint8_t *pkt, uint32_t dlen,
                            struct rmtcallargs *msg) {
    uint32_t offset = 0;

    if (dlen < (3*WORD_BYTES)) {
        HAL_TRACE_ERR("Packet len {} is smaller than rmtcall arg {}",
                      dlen, (3*WORD_BYTES));
        return 0;
    }
    msg->prog = __pack_uint32(pkt, &offset);
    msg->vers = __pack_uint32(pkt, &offset);
    msg->proc = __pack_uint32(pkt, &offset);
 
    /* We dont care about the args */
 
    return offset;
}

uint32_t __parse_call_hdr(const uint8_t *pkt, uint32_t dlen, 
                          struct rpc_msg *cmsg) {
    uint32_t len = 0, offset = 0;
  
    if (dlen < sizeof(struct call_body)) {
        HAL_TRACE_ERR("Packet len {} is smaller than call hdr {}",
                      dlen, sizeof(struct call_body));
        return 0;
    } 

    cmsg->rm_call.cb_rpcvers = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_prog = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_vers = __pack_uint32(pkt, &offset);
    cmsg->rm_call.cb_proc = __pack_uint32(pkt, &offset);
    // Move the offset after Auth credentials and verif
    len = __pack_uint32(pkt, &offset);
    if (len == 0) 
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
    if ((dlen-offset) < WORD_BYTES) {
        HAL_TRACE_ERR("Cannot parse further dlen {} is smaller than a word",
                      (dlen-offset));
        return 0;
    }
    len = __pack_uint32(pkt, &offset);
    if (len == 0)
        len = 4;
    offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len; 

    return offset;
}

uint32_t __parse_reply_hdr(const uint8_t *pkt, uint32_t dlen, 
                           struct rpc_msg *rmsg) {
    uint32_t len = 0, offset = 0;

    if (dlen < WORD_BYTES) {
        HAL_TRACE_ERR("Packet len {} is smaller to parse response",
                       dlen);
        return 0;
    }
    rmsg->rm_reply.rp_stat = (reply_stat)__pack_uint32(pkt, &offset);
    if (rmsg->rm_reply.rp_stat == MSG_ACCEPTED) {
        if ((dlen-offset) < sizeof(struct accepted_reply)) {
            HAL_TRACE_ERR("Packet len {} is smaller to parse accepted rsp {}",
                          (dlen-offset), sizeof(struct accepted_reply));
            return 0;
        }
        // Move the offset after Auth credentials and verif
        len = __pack_uint32(pkt, &offset);
        if (len == 0)
            len = 4;
        offset += (len%WORD_BYTES)?(len+(WORD_BYTES + len%WORD_BYTES)):len;
        if ((dlen-offset) < WORD_BYTES) {
            HAL_TRACE_ERR("Packet len {} is smaller than a word",
                          (dlen-offset));
            return 0;
        }
        rmsg->acpted_rply.ar_stat = (accept_stat)__pack_uint32(pkt, &offset);
    }

    return offset;
}

uint32_t __parse_rpc_msg(const uint8_t *pkt, uint32_t payload_offset, 
                         uint32_t dlen, struct rpc_msg *msg) {
    uint32_t offset = 0, hdr_offset = 0;

    pkt = &pkt[payload_offset];
    msg->rm_xid = __pack_uint32(pkt, &offset);
    msg->rm_direction = (msg_type)__pack_uint32(pkt, &offset);
   
    HAL_TRACE_DEBUG("xid: {} direction: {}", msg->rm_xid, msg->rm_direction); 
    if (msg->rm_direction == RPC_CALL) {
        hdr_offset = __parse_call_hdr(&pkt[offset], (dlen-offset), msg);
    } else {
        hdr_offset = __parse_reply_hdr(&pkt[offset], (dlen-offset), msg);
    }

    if (!hdr_offset)
        return 0;

    return (offset + hdr_offset);
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

/*
 * SUNRPC ALG completion handler - invoked when the session creation is done.
 */
static void sunrpc_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t   *l4_sess = (l4_alg_status_t *)alg_status(\
                                 ctx.feature_session_state(FTE_FEATURE_ALG_RPC));

    HAL_ASSERT(l4_sess != NULL);

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            g_rpc_state->cleanup_app_session(l4_sess->app_session);
        } else { /* Cleanup data session */
            g_rpc_state->cleanup_l4_sess(l4_sess);            
        }
    } else {
        l4_sess->session = ctx.session();
    }
}

hal_ret_t process_sunrpc_data_flow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess) {
    hal_ret_t          ret = HAL_RET_OK;
    l4_alg_status_t   *exp_flow = l4_sess;

    // Todo (Pavithra) Get the Firewall data and make sure that the UUID
    // is still allowed in the config
    /*
     * Alloc L4 Session. This is just to keep a backward reference to the
     * app session that created it.
     */

    HAL_TRACE_DEBUG("In process_sunrpc_data_flow alg: {}", l4_sess->alg);
    ret = g_rpc_state->alloc_and_insert_l4_sess(l4_sess->app_session, &l4_sess);
    HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
    l4_sess->alg = nwsec::APP_SVC_SUN_RPC;
    l4_sess->isCtrl = FALSE;

    // Register completion handler and session state
    ctx.register_completion_handler(sunrpc_completion_hdlr);
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
    memset(rpc_info, 0, sizeof(rpc_info_t));
    rpc_info->pkt_type = PMAPPROC_NULL;
    rpc_info->callback = parse_sunrpc_control_flow;
}

hal_ret_t parse_sunrpc_control_flow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess) {
    const uint8_t           *pkt = ctx.pkt();
    uint32_t                 pkt_len = ctx.pkt_len();
    uint32_t                 rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    uint32_t                 pgm_offset = 0, offset=0;
    struct rpc_msg           rpc_msg;
    rpc_info_t              *rpc_info = (rpc_info_t *)l4_sess->info;
    pmaplist                 pmap_list;
    rpcblist                 rpcb_list;
    struct rmtcallargs       rmtcallargs;
    uint8_t                  addr_family = 0;
    char                     netid[128], uaddr[128], owner[128];

    if (pkt_len <= (uint32_t)(rpc_msg_offset + 3*WORD_BYTES)) {
        // Packet length is smaller than the RPC common header
        // Size. We cannot process this packet.
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {} rpc_msg size: {}", \
                      ctx.pkt_len(),  rpc_msg_offset);
        return HAL_RET_ERR;
    }

    pgm_offset = __parse_rpc_msg(pkt, (ctx.key().proto==IP_PROTO_TCP)?\
                    (rpc_msg_offset+WORD_BYTES):rpc_msg_offset, pkt_len, &rpc_msg);
    if (!pgm_offset) {
        return HAL_RET_ERR;
    }
    pgm_offset += rpc_msg_offset;
 
    /*
     * L7 Fragment reassembly
     */
    if (ctx.key().proto == IP_PROTO_TCP) {
        // Offset adjustment for Record Fragment byte
        pgm_offset += WORD_BYTES;
        if (!(pkt[rpc_msg_offset] & LAST_RECORD_FRAG)) {
            HAL_TRACE_DEBUG("Fragmented packet detected");
            rpc_info->rpc_frag_cont = 1;
            if (rpc_info->pkt == NULL && !rpc_info->pkt_len) {
                rpc_info->pkt = alloc_rpc_pkt();
                rpc_info->payload_offset = rpc_msg_offset + WORD_BYTES;
            }
            if ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ) {
                memcpy(&rpc_info->pkt[rpc_info->pkt_len], &pkt[pgm_offset], (pkt_len-pgm_offset));
                rpc_info->pkt_len += pkt_len;
            } else {
                HAL_TRACE_ERR("Packet len execeeded the Max ALG Fragmented packet sz");
                reset_rpc_info(rpc_info);
            } 
            return HAL_RET_OK;
        } else {
            if (rpc_info->pkt != NULL && rpc_info->pkt_len) {
                if ((rpc_info->pkt_len + (pkt_len-pgm_offset)) < MAX_ALG_RPC_PKT_SZ) {
                    memcpy(&rpc_info->pkt[rpc_info->pkt_len], 
                                             &pkt[pgm_offset], (pkt_len-pgm_offset));
                    rpc_info->pkt_len += pkt_len;
                    pkt = rpc_info->pkt;
                    pkt_len = rpc_info->pkt_len;
                } else {
                    HAL_TRACE_ERR("Packet len execeeded the Max ALG Fragmented packet sz");
                    reset_rpc_info(rpc_info);
                }
            }
            rpc_info->rpc_frag_cont = 0;
        } 
    }

    switch (rpc_info->pkt_type)
    {
        case PMAPPROC_NULL:
            HAL_TRACE_DEBUG("RPC Proc: {}", rpc_msg.rm_call.cb_proc);
            if (rpc_msg.rm_direction == 0 && \
                rpc_msg.rm_call.cb_prog == PMAPPROG && \
                (rpc_msg.rm_call.cb_proc == PMAPPROC_GETPORT || \
                 rpc_msg.rm_call.cb_proc == PMAPPROC_DUMP || \
                 rpc_msg.rm_call.cb_proc == PMAPPROC_CALLIT)) {

                rpc_info->pkt_type = rpc_msg.rm_call.cb_proc;
                switch (rpc_msg.rm_call.cb_proc) {
                    case PMAPPROC_GETPORT:
                        if (rpc_msg.rm_call.cb_vers == RPCBVERS_3 ||
                            rpc_msg.rm_call.cb_vers == RPCBVERS_4) {
                            rpcb_list.rpcb_map.r_netid = &netid[0];
                            rpcb_list.rpcb_map.r_addr = &uaddr[0];
                            rpcb_list.rpcb_map.r_owner = &owner[0];
                            offset = __parse_rpcb_entry(&pkt[pgm_offset], (pkt_len-pgm_offset), 
                                                   &rpcb_list.rpcb_map);
                            if (!offset) {
                                reset_rpc_info(rpc_info);
                                return HAL_RET_ERR;
                            }

                            rpc_info->xid = rpc_msg.rm_xid;
                            rpc_info->rpcvers = rpc_msg.rm_call.cb_vers;
                            rpc_info->prog_num = rpcb_list.rpcb_map.r_prog;
                            rpc_info->vers = rpcb_list.rpcb_map.r_vers;
                            rpc_info->prot = netid2proto(rpcb_list.rpcb_map.r_netid);
                            addr_family = netid2addrfamily(rpcb_list.rpcb_map.r_netid);
                            decodeuaddr(rpcb_list.rpcb_map.r_addr, &rpc_info->ip, 
                                        &rpc_info->dport, addr_family);
                            if (isNullip(rpc_info->ip, addr_family)) {
                                rpc_info->ip = ctx.key().dip;
                            } 
                        } else {
                            offset = __parse_pmap_hdr(&pkt[pgm_offset], 
                                                       (pkt_len-pgm_offset), &pmap_list);
                            if (!offset) {
                                reset_rpc_info(rpc_info);
                                return HAL_RET_ERR;
                            }
                            rpc_info->xid = rpc_msg.rm_xid;
                            rpc_info->rpcvers = 2;
                            rpc_info->prog_num = pmap_list.pml_map.pm_prog; 
                            rpc_info->prot  = pmap_list.pml_map.pm_prot;
                            rpc_info->dport = pmap_list.pml_map.pm_port;
                            rpc_info->vers  = pmap_list.pml_map.pm_vers;
                            HAL_TRACE_DEBUG("Prog num: {} proto: {}", 
                                                   rpc_info->prog_num, rpc_info->prot);
                            rpc_info->ip    = ctx.key().dip;
                        }
                        break;
                  
                    case PMAPPROC_DUMP:
                        rpc_info->rpcvers = rpc_msg.rm_call.cb_vers;
                        break;

                    case PMAPPROC_CALLIT:
                        //Parse Callit message
                        offset = __parse_callit_req(&pkt[pgm_offset], 
                                                    (pkt_len-pgm_offset), &rmtcallargs);
                        if (!offset) {
                            reset_rpc_info(rpc_info);
                            return HAL_RET_ERR;
                        } 
                        rpc_info->prog_num = rmtcallargs.prog;
                        rpc_info->vers     = rmtcallargs.vers;
                        break; 

                    default:
                        break;
                };
            } 
            return HAL_RET_OK;

        case PMAPPROC_GETPORT:
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.rm_reply.rp_acpt.ar_stat == 0) {
 
                if (rpc_info->xid == rpc_msg.rm_xid) {
                    // Parse the header to save the details
                    if (rpc_info->vers == RPCBVERS_3 ||
                        rpc_info->vers == RPCBVERS_4) { 
                        offset = __parse_rpcb_res_hdr(&pkt[pgm_offset], 
                                                       (pkt_len-pgm_offset), &uaddr[0]);
                        if (!offset) {
                            reset_rpc_info(rpc_info);
                            return HAL_RET_ERR;
                        }
                        decodeuaddr(rpcb_list.rpcb_map.r_addr, &rpc_info->ip, &rpc_info->dport,
                                       addr_family);
                        if (isNullip(rpc_info->ip, addr_family)) {
                            rpc_info->ip = ctx.key().dip;
                        }
                    } else {
                        rpc_info->dport = __pack_uint32(pkt, &pgm_offset);
                        HAL_TRACE_DEBUG("RPC Info Dport: {}", rpc_info->dport);
                    } 
                }
                // Insert an ALG entry for the DIP, Dport
                if (rpc_info->dport)
                    insert_rpc_expflow(ctx, l4_sess, process_sunrpc_data_flow);
            }
            break;

        case PMAPPROC_DUMP:
            HAL_TRACE_DEBUG("Reply state: {} accp state: {}", rpc_msg.rm_reply.rp_stat, 
                             rpc_msg.acpted_rply.ar_stat);
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.acpted_rply.ar_stat == 0) {
          
                while (__pack_uint32(pkt, &pgm_offset) == (uint32_t)1) {
                    HAL_TRACE_DEBUG("cb vers: {}", rpc_info->rpcvers);
                    // Todo (Pavithra) search if the program already
                    // exists in the list
                    if (rpc_info->rpcvers == RPCBVERS_3 ||
                        rpc_info->rpcvers == RPCBVERS_4) { 
                        memset(&rpcb_list, 0, sizeof(rpcb_list));
                        rpcb_list.rpcb_map.r_netid = &netid[0];
                        rpcb_list.rpcb_map.r_addr = &uaddr[0];
                        rpcb_list.rpcb_map.r_owner = &owner[0];
                        memset(uaddr, 0, ADDR_NETID_BYTES);
                        memset(netid, 0, ADDR_NETID_BYTES);
                        memset(owner, 0, ADDR_NETID_BYTES);
                        offset = __parse_rpcb_entry(&pkt[pgm_offset], (pkt_len-pgm_offset),
                                                    &rpcb_list.rpcb_map);
                        if (!offset) {
                            reset_rpc_info(rpc_info);
                            return HAL_RET_ERR;
                        }
                        rpc_info->prog_num = rpcb_list.rpcb_map.r_prog;
                        rpc_info->vers = rpcb_list.rpcb_map.r_vers;
                        rpc_info->prot = netid2proto(rpcb_list.rpcb_map.r_netid);
                        addr_family = netid2addrfamily(rpcb_list.rpcb_map.r_netid);
                        decodeuaddr(rpcb_list.rpcb_map.r_addr, &rpc_info->ip, &rpc_info->dport,
                                       addr_family);
                        if (isNullip(rpc_info->ip, addr_family)) {
                            rpc_info->ip = ctx.key().dip;
                        }
                        HAL_TRACE_DEBUG("Dump entry dport: {}", rpc_info->dport);
                    } else {
                        offset = __parse_pmap_hdr(&pkt[pgm_offset], 
                                                    (pkt_len-pgm_offset), &pmap_list);
                        if (!offset) {
                            reset_rpc_info(rpc_info);
                            return HAL_RET_ERR;
                        }
                        rpc_info->xid = rpc_msg.rm_xid;
                        rpc_info->prog_num = pmap_list.pml_map.pm_prog;
                        rpc_info->prot  = pmap_list.pml_map.pm_prot;
                        rpc_info->dport = pmap_list.pml_map.pm_port;
                        rpc_info->vers  = pmap_list.pml_map.pm_vers;    
                    }
                    if (rpc_info->dport)
                        insert_rpc_expflow(ctx, l4_sess, process_sunrpc_data_flow);
                    pgm_offset += offset;
                }
            }
            break;

        case PMAPPROC_CALLIT:
            if (rpc_msg.rm_direction == 1 && \
                rpc_msg.rm_reply.rp_stat == 0 && \
                rpc_msg.rm_reply.rp_acpt.ar_stat == 0) {
                //Parse Callit response
                if ((pkt_len-pgm_offset) < WORD_BYTES) {
                    HAL_TRACE_ERR("Packet len is too small to parse callit response");
                    reset_rpc_info(rpc_info);
                    return HAL_RET_ERR;
                }
                rpc_info->dport = __pack_uint32(&pkt[pgm_offset], &offset);
                if (rpc_info->dport)
                    insert_rpc_expflow(ctx, l4_sess, process_sunrpc_data_flow);
            }
            break;

        default:
            break;
    }
    
    /* Reset RPC Info */
    reset_rpc_info(rpc_info);

    return HAL_RET_OK;
}

hal_ret_t alg_sunrpc_exec(fte::ctx_t& ctx, sfw_info_t *sfw_info,
                          l4_alg_status_t *l4_sess) {
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;
    rpc_info_t           *rpc_info = NULL;
    app_session_t        *app_sess = NULL;
    uint32_t              payload_offset = 0;

    HAL_TRACE_DEBUG("In alg_sunrpc_exec {:p}", (void *)l4_sess);
    payload_offset = ctx.cpu_rxhdr()->payload_offset;
    if (sfw_info->alg_proto == nwsec::APP_SVC_SUN_RPC &&
        (!ctx.existing_session())) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            HAL_TRACE_DEBUG("Parsing the first packet");
            /*
             * Alloc APP session, L4 Session and RPC info
             */
            ret = g_rpc_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
            ret = g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), ret);
            l4_sess->alg = nwsec::APP_SVC_SUN_RPC;
            rpc_info = (rpc_info_t *)g_rpc_state->alg_info_slab()->alloc();
            HAL_ASSERT_RETURN((rpc_info != NULL), HAL_RET_OOM);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = rpc_info;
            reset_rpc_info(rpc_info);

            // Register completion handler and feature session state
            ctx.register_completion_handler(sunrpc_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);

            /*
             * Connectionless SUNRPC would have the portmapper query
             * in the first packet so start parsing.
             */
            if (ctx.key().proto == IP_PROTO_UDP) {
                ret = parse_sunrpc_control_flow(ctx, l4_sess); 
                if (ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("SUN RPC ALG parse for UDP frame failed");
                    return ret;
                }
                rpc_info->skip_sfw = TRUE;
            } else {
                flowupd.type = fte::FLOWUPD_MCAST_COPY;
                flowupd.mcast_info.mcast_en = 1;
                flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;    
                flowupd.mcast_info.proxy_mcast_ptr = 0;
                ret = ctx.update_flow(flowupd);
            }
        } else { /* Responder flow */
           flowupd.type = fte::FLOWUPD_MCAST_COPY;
           flowupd.mcast_info.mcast_en = 1;
           flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
           flowupd.mcast_info.proxy_mcast_ptr = 0;
           ret = ctx.update_flow(flowupd);
        }
    } else if (l4_sess && l4_sess->info && (ctx.pkt_len() > payload_offset)) {
        rpc_info = (rpc_info_t *)l4_sess->info;
        HAL_TRACE_DEBUG("RPC Info {:p}", l4_sess->info);

        /*
         * Parse Control session data && process Expected flows
         */
        rpc_info->callback(ctx, l4_sess);
    } 

    return ret;
}

} // namespace alg_rpc
} // namespace plugins
} // namespace hal
