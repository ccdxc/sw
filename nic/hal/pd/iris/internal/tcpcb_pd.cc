#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/tcpcb_pd.hpp"
#include "nic/hal/pd/iris/p4pd_tcp_proxy_api.h"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "asic/cmn/asic_common.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/tls_common.h"
#include "nic/include/app_redir_shared.h"
#include "nic/hal/pd/iris/internal/tlscb_pd.hpp"
#ifdef ELBA
#include "nic/sdk/platform/elba/elba_txs_scheduler.hpp"
#else
#include "nic/sdk/platform/capri/capri_txs_scheduler.hpp"
#include "nic/sdk/platform/capri/capri_barco_crypto.hpp"
#endif

#ifdef ELBA
using namespace sdk::platform::elba;
/* TBD-ELBA-REBASE: get these from elba_barco_crypto.hpp */
#define ELBA_MAX_TLS_PAD_SIZE              512
#define ELBA_GC_GLOBAL_TABLE               (ELBA_MAX_TLS_PAD_SIZE + 128)
#define ELBA_GC_GLOBAL_OOQ_TX2RX_FP_PI     (ELBA_GC_GLOBAL_TABLE + 8)
#else
using namespace sdk::platform::capri;
#endif

namespace hal {
namespace pd {
// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

void *
tcpcb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_tcpcb_t *)entry)->hw_id);
}

uint32_t
tcpcb_pd_hw_key_size ()
{
    return sizeof(tcpcb_hw_id_t);
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_tcp_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "tcp_rx_stage0";

    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    *offset >>= MPU_PC_ADDR_SHIFT;
    return HAL_RET_OK;
}

static hal_ret_t
p4pd_add_or_del_tcp_rx_read_tx2rx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d data = {0};
    hal_ret_t ret = HAL_RET_OK;
    //uint64_t                        pc_offset;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);

    if(!del) {
        uint64_t pc_offset;

        // get pc address
        if(p4pd_get_tcp_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG("Received pc address {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.read_tx2rx_d.debug_dol = htons(tcpcb_pd->tcpcb->debug_dol);
        data.u.read_tx2rx_d.snd_nxt = htonl(tcpcb_pd->tcpcb->snd_nxt);
        data.u.read_tx2rx_d.rcv_wup = htonl(tcpcb_pd->tcpcb->rcv_wup);
        data.u.read_tx2rx_d.rcv_wnd_adv = htons(tcpcb_pd->tcpcb->rcv_wnd);
        data.u.read_tx2rx_d.l7_proxy_type = tcpcb_pd->tcpcb->l7_proxy_type;
        data.u.read_tx2rx_d.serq_cidx = htons((uint16_t)tcpcb_pd->tcpcb->serq_ci);
        HAL_TRACE_DEBUG("TCPCB snd_nxt: {:#x}", data.u.read_tx2rx_d.snd_nxt);
        HAL_TRACE_DEBUG("TCPCB rcv_wup: {:#x}", data.u.read_tx2rx_d.rcv_wup);
        HAL_TRACE_DEBUG("TCPCB l7_proxy_type: {:#x}", data.u.read_tx2rx_d.l7_proxy_type);
        HAL_TRACE_DEBUG("TCPCB _debug_dol: {:#x}", data.u.read_tx2rx_d.debug_dol);
        HAL_TRACE_DEBUG("TCPCB serq_ci: {}", ntohs(data.u.read_tx2rx_d.serq_cidx));
    }
    HAL_TRACE_DEBUG("Programming tx2rx at hw-id: {:#x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: read_tx2rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s1_t0_tcp_rx_d              data = {0};
    hal_ret_t                   ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(!del) {
        data.u.tcp_rx_d.rcv_nxt = htonl(tcpcb_pd->tcpcb->rcv_nxt);
        data.u.tcp_rx_d.snd_una = htonl(tcpcb_pd->tcpcb->snd_una);
        data.u.tcp_rx_d.snd_wnd = htons((uint16_t)tcpcb_pd->tcpcb->snd_wnd);
        data.u.tcp_rx_d.serq_pidx = htons((uint16_t)tcpcb_pd->tcpcb->serq_pi);
        data.u.tcp_rx_d.state = (uint8_t)tcpcb_pd->tcpcb->state;
        data.u.tcp_rx_d.pred_flags = htonl(tcpcb_pd->tcpcb->pred_flags);
        data.u.tcp_rx_d.rcv_wscale = tcpcb_pd->tcpcb->rcv_wscale;
        if (tcpcb_pd->tcpcb->delay_ack) {
            data.u.tcp_rx_d.cfg_flags |= TCP_OPER_FLAG_DELACK;
            data.u.tcp_rx_d.ato = htons(tcpcb_pd->tcpcb->ato / TCP_TIMER_TICK);
            data.u.tcp_rx_d.quick = TCP_QUICKACKS;
            HAL_TRACE_DEBUG("TCPCB ato: {} us", tcpcb_pd->tcpcb->ato);
        }
        if (tcpcb_pd->tcpcb->ooo_queue) {
            data.u.tcp_rx_d.cfg_flags |= TCP_OPER_FLAG_OOO_QUEUE;
        }
        switch (data.u.tcp_rx_d.state) {
            case TCP_SYN_SENT:
            case TCP_SYN_RECV:
            case TCP_CLOSE:
            case TCP_LISTEN:
            case TCP_NEW_SYN_RECV:
                data.u.tcp_rx_d.parsed_state |= TCP_PARSED_STATE_HANDLE_IN_CPU;
                break;
            default:
                data.u.tcp_rx_d.parsed_state &= ~TCP_PARSED_STATE_HANDLE_IN_CPU;
        }
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            data.u.tcp_rx_d.consumer_ring_shift = ASIC_SESQ_RING_SLOTS_SHIFT;
        } else {
            data.u.tcp_rx_d.consumer_ring_shift = ASIC_SERQ_RING_SLOTS_SHIFT;
        }

        HAL_TRACE_DEBUG("TCPCB rcv_nxt: {:#x}", data.u.tcp_rx_d.rcv_nxt);
        HAL_TRACE_DEBUG("TCPCB snd_una: {:#x}", data.u.tcp_rx_d.snd_una);
        HAL_TRACE_DEBUG("TCPCB ts_recent: {:#x}", data.u.tcp_rx_d.ts_recent);
        HAL_TRACE_DEBUG("TCPCB snd_wnd: {:#x}", data.u.tcp_rx_d.snd_wnd);
        HAL_TRACE_DEBUG("TCPCB serq_pi: {:#x}", data.u.tcp_rx_d.serq_pidx);
        HAL_TRACE_DEBUG("TCPCB state: {:#x}", data.u.tcp_rx_d.state);
        HAL_TRACE_DEBUG("TCPCB pred_flags: {:#x}", data.u.tcp_rx_d.pred_flags);
        HAL_TRACE_DEBUG("TCPCB cfg_flags: {:#x}", data.u.tcp_rx_d.cfg_flags);
    }
    int size = sizeof(s1_t0_tcp_rx_d);
    HAL_TRACE_DEBUG("Programming tcp_rx at hw-id: {:#x}", hwid);
    HAL_TRACE_DEBUG("Programming tcp_rx at size: {:#x}", size);

    if(!p4plus_hbm_write(hwid, (uint8_t *)&data, size,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create rx: tcp_rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_rx_tcp_rtt_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s3_t0_tcp_rx_d      data = {0};
    hal_ret_t           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RTT);

    if(!del) {
#define OLD_RTO 1
#ifdef OLD_RTO
        data.u.tcp_rtt_d.rto = htonl(100);
        data.u.tcp_rtt_d.srtt_us = htonl(0x80);
#else
        data.u.tcp_rtt_d.rto = htonl(100000);
        data.u.tcp_rtt_d.srtt_us = 0;
#endif
        data.u.tcp_rtt_d.curr_ts = htonl(0xf0);
        data.u.tcp_rtt_d.rttvar_us = 0;
        data.u.tcp_rtt_d.rtt_updated = 0;
        data.u.tcp_rtt_d.rtt_seq_tsoffset = htonl(tcpcb_pd->tcpcb->rtt_seq_tsoffset);
        data.u.tcp_rtt_d.rtt_time = htonl(tcpcb_pd->tcpcb->rtt_time);
        data.u.tcp_rtt_d.ts_learned = tcpcb_pd->tcpcb->ts_learned;
    }

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_rtt entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }

    HAL_TRACE_DEBUG("Set RTO to {}", ntohl(data.u.tcp_rtt_d.rto));
    HAL_TRACE_DEBUG("Set rtt_seq_tsoffset to {}", ntohl(data.u.tcp_rtt_d.rtt_seq_tsoffset));
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_rx_tcp_cc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s4_t0_tcp_rx_d      data = {0};
    hal_ret_t           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_CC);

    if(!del) {
        data.u.tcp_cc_d.cc_algo = TCP_CC_ALGO_NEW_RENO;
        data.u.tcp_cc_d.smss = htons(tcpcb_pd->tcpcb->smss);
        data.u.tcp_cc_d.snd_cwnd = htonl(tcpcb_pd->tcpcb->snd_cwnd);
        data.u.tcp_cc_d.snd_ssthresh = htonl(tcpcb_pd->tcpcb->snd_ssthresh);
        data.u.tcp_cc_d.max_win = TCP_MAX_WIN << tcpcb_pd->tcpcb->snd_wscale;
        data.u.tcp_cc_d.snd_wscale = tcpcb_pd->tcpcb->snd_wscale;
        // 'L' value for ABC : Appropriate Byte Counting
        data.u.tcp_cc_d.abc_l_var = tcpcb_pd->tcpcb->abc_l_var;
        if(data.u.tcp_cc_d.cc_algo == TCP_CC_ALGO_NEW_RENO) {
            data.u.tcp_cc_new_reno_d.smss_times_abc_l = htonl(tcpcb_pd->tcpcb->smss *
                data.u.tcp_cc_d.abc_l_var);
            data.u.tcp_cc_new_reno_d.smss_squared = htonl(tcpcb_pd->tcpcb->smss *
                tcpcb_pd->tcpcb->smss);
        }
        if(data.u.tcp_cc_d.cc_algo == TCP_CC_ALGO_CUBIC) {
            data.u.tcp_cc_cubic_d.t_last_cong = 1;  //should be current 10us snapshot
            data.u.tcp_cc_cubic_d.min_rtt_ticks = 0;
            data.u.tcp_cc_cubic_d.mean_rtt_ticks = 1;
        }
    }

    HAL_TRACE_DEBUG("Received snd_cwnd: {}", tcpcb_pd->tcpcb->snd_cwnd);
    HAL_TRACE_DEBUG("Received snd_ssthresh: {}", tcpcb_pd->tcpcb->snd_ssthresh);

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

uint64_t tcpcb_pd_read_notify_addr_get(uint32_t qid)
{
    uint64_t addr;

    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, 0,
            qid) + (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_RETX);
    // offsetof does not work on bitfields
    // addr += offsetof(s3_t0_tcp_tx_retx_d, read_notify_bytes);
    addr += 0;

    return addr;
}

hal_ret_t
p4pd_add_or_del_tcp_rx_tcp_fc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s5_t0_tcp_rx_d      data = {0};
    hal_ret_t           ret = HAL_RET_OK;
    int                 num_slots;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FC);

    if(!del) {
        data.u.tcp_fc_d.rcv_wnd = htonl(tcpcb_pd->tcpcb->rcv_wnd);
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            data.u.tcp_fc_d.read_notify_addr = htonl(tcpcb_pd_read_notify_addr_get(tcpcb_pd->tcpcb->other_qid));
        } else {
            data.u.tcp_fc_d.read_notify_addr = 0;
        }
        data.u.tcp_fc_d.rcv_scale = tcpcb_pd->tcpcb->rcv_wscale;
        data.u.tcp_fc_d.cpu_id = tcpcb_pd->tcpcb->cpu_id;
        data.u.tcp_fc_d.rcv_wup = htonl(tcpcb_pd->tcpcb->rcv_wup);
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            num_slots = ASIC_SESQ_RING_SLOTS;
        } else {
            num_slots = ASIC_SERQ_RING_SLOTS;
        }
        data.u.tcp_fc_d.consumer_ring_slots = htons(num_slots - 1);
        data.u.tcp_fc_d.consumer_ring_slots_mask = htons(num_slots - 1);
        data.u.tcp_fc_d.high_thresh1 = (uint16_t)htons(num_slots * .75);
        data.u.tcp_fc_d.high_thresh2 = (uint16_t)htons(num_slots * .50);
        data.u.tcp_fc_d.high_thresh3 = (uint16_t)htons(num_slots * .25);
        data.u.tcp_fc_d.high_thresh4 = (uint16_t)htons(num_slots * .125);
        data.u.tcp_fc_d.avg_pkt_size_shift = 7;
        data.u.tcp_fc_d.rcv_mss = htons(tcpcb_pd->tcpcb->rcv_mss);
    }

    HAL_TRACE_DEBUG("Received read_notify_addr: {}", htonl(data.u.tcp_fc_d.read_notify_addr));
    HAL_TRACE_DEBUG("Received read_notify_addr: {}", data.u.tcp_fc_d.read_notify_addr);
    HAL_TRACE_DEBUG("Received rcv_wnd: {}", htonl(data.u.tcp_fc_d.rcv_wnd));
    HAL_TRACE_DEBUG("Received rcv_scale {}", data.u.tcp_fc_d.rcv_scale);
    HAL_TRACE_DEBUG("Received rcv_wup {:#x}", data.u.tcp_fc_d.rcv_wup);
    HAL_TRACE_DEBUG("Received rcv_mss {}", tcpcb_pd->tcpcb->rcv_mss);

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_fc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcpcb_rx_dma(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s6_t0_tcp_rx_dma_d rx_dma_d = { 0 };
    hal_ret_t ret = HAL_RET_OK;

    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_DMA);

    if(!del) {
        uint64_t stats_base = asicpd_get_mem_addr(
                ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS);
        SDK_ASSERT(stats_base + tcpcb_pd->tcpcb->cb_id *
                   TCP_PER_FLOW_STATS_SIZE +
                   TCP_PER_FLOW_STATS_SIZE <= stats_base +
                   asicpd_get_mem_size_kb(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) *
                   1024);
        rx_dma_d.rx_stats_base = htonll(stats_base +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_RX_PER_FLOW_STATS_OFFSET);
        HAL_TRACE_DEBUG("tcp qid {}, rx stats base: {:#x}",
                        tcpcb_pd->tcpcb->cb_id, ntohll(rx_dma_d.rx_stats_base));
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_NVME) {
            rx_dma_d.app_type_cfg = TCP_APP_TYPE_NVME;
        }
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            // Get SESQ address of the other TCP flow
            rx_dma_d.app_type_cfg = TCP_APP_TYPE_BYPASS;
            wring_hw_id_t  sesq_base;
            ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                         tcpcb_pd->tcpcb->other_qid,
                                         &sesq_base);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to receive sesq base for tcp cb: {}",
                            tcpcb_pd->tcpcb->other_qid);
            } else {
                HAL_TRACE_DEBUG("TCP2SESQ[qid:{:#x}, oqid:{:#x}]:OSesq base: {:#x}",
                        tcpcb_pd->tcpcb->cb_id, tcpcb_pd->tcpcb->other_qid, sesq_base);
                rx_dma_d.serq_base = htonll(sesq_base);
            }
            rx_dma_d.consumer_qid = htons(tcpcb_pd->tcpcb->other_qid);
            rx_dma_d.nde_shift = ASIC_SESQ_ENTRY_SIZE_SHIFT;
            rx_dma_d.nde_offset = ASIC_SESQ_DESC_OFFSET;
            rx_dma_d.nde_len = ASIC_SESQ_ENTRY_SIZE;
            rx_dma_d.consumer_lif = htons(SERVICE_LIF_TCP_PROXY);
            rx_dma_d.consumer_ring = TCP_SCHED_RING_SESQ;
            rx_dma_d.consumer_num_slots_mask = htons(ASIC_SESQ_RING_SLOTS_MASK);
        } else {
            // Get Serq address
            rx_dma_d.app_type_cfg = TCP_APP_TYPE_TLS;
            wring_hw_id_t  serq_base;
            ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                         tcpcb_pd->tcpcb->cb_id,
                                         &serq_base);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to receive serq base for tcp cb: {}",
                            tcpcb_pd->tcpcb->cb_id);
            } else {
                HAL_TRACE_DEBUG("Serq base: {:#x}", serq_base);
                rx_dma_d.serq_base = htonll(serq_base);
            }
            rx_dma_d.consumer_qid = htons(tcpcb_pd->tcpcb->cb_id);
            rx_dma_d.nde_shift = ASIC_SERQ_ENTRY_SIZE_SHIFT;
            rx_dma_d.nde_offset = ASIC_SERQ_DESC_OFFSET;
            rx_dma_d.nde_len = ASIC_SERQ_ENTRY_SIZE;
            rx_dma_d.consumer_lif = htons(SERVICE_LIF_TLS_PROXY);
            rx_dma_d.consumer_ring = TLS_SCHED_RING_SERQ;
            rx_dma_d.consumer_num_slots_mask = htons(ASIC_SERQ_RING_SLOTS_MASK);
        }
    }

    if(!p4plus_hbm_write(hwid, (uint8_t *)&rx_dma_d, sizeof(rx_dma_d),
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create rx: rx_dma entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
p4pd_add_or_del_tcpcb_rx_ooq(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s5_t2_tcp_rx_ooo_qbase_cb_load_d ooq_cb_load_d = { 0 };
    s2_t2_tcp_rx_ooo_book_keeping_d ooq_book_keeping_d = { 0 };
    hal_ret_t ret = HAL_RET_OK;

    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_OOO_QADDR_OFFSET);

    if(!del) {
        wring_hw_id_t ooo_rx2tx_qbase;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_TCP_OOO_RX2TX,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &ooo_rx2tx_qbase);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive ooo rx2tx base for tcp cb: {}",
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("ooo rx2tx base: {:#x}", ooo_rx2tx_qbase);
            ooq_cb_load_d.ooo_rx2tx_qbase = htonll(ooo_rx2tx_qbase);
        }
        if (tcpcb_pd->tcpcb->sack_perm) {
            ooq_book_keeping_d.tcp_opt_flags |= TCP_OPT_FLAG_SACK_PERM;
        }
        if (tcpcb_pd->tcpcb->timestamps) {
            ooq_book_keeping_d.tcp_opt_flags |= TCP_OPT_FLAG_TIMESTAMPS;
        }
        HAL_TRACE_DEBUG("tcp_opt_flags: {:#x}", ooq_book_keeping_d.tcp_opt_flags);
    }
    if(!p4plus_hbm_write(hwid, (uint8_t *)&ooq_cb_load_d, sizeof(ooq_cb_load_d),
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create rx: ooq cb_load entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_OOO_BOOK_KEEPING_OFFSET);
    if(!p4plus_hbm_write(hwid, (uint8_t *)&ooq_book_keeping_d, sizeof(ooq_book_keeping_d),
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create rx: ooq bookkeeping entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

hal_ret_t
p4pd_add_or_del_tcpcb_rxdma_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tcp_rx_read_tx2rx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_rx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_rtt_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_cc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_rx_tcp_fc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcpcb_rx_dma(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcpcb_rx_ooq(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

uint64_t tcpcb_pd_serq_prod_ci_addr_get(uint32_t qid)
{
    uint64_t addr;

    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, 0,
            qid) + (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);
    // offsetof does not work on bitfields
    //addr += offsetof(common_p4plus_stage0_app_header_table_read_tx2rx_d, serq_cidx);
    addr += 24;

    return addr;
}

uint64_t tcpcb_pd_ooo_rx2tx_prod_ci_addr_get(uint32_t qid)
{
    uint64_t addr;

    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, 0,
            qid) + (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_OOO_QADDR_OFFSET) +
            TCP_TCB_OOO_QADDR_CI_OFFSET;

    return addr;
}


hal_ret_t
p4pd_get_tcp_rx_read_tx2rx_entry(pd_tcpcb_t* tcpcb_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: read_tx2rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->rx_ts = ntohll(data.u.read_tx2rx_d.rx_ts);
    tcpcb_pd->tcpcb->debug_dol = ntohs(data.u.read_tx2rx_d.debug_dol);
    tcpcb_pd->tcpcb->snd_nxt = ntohl(data.u.read_tx2rx_d.snd_nxt);
    tcpcb_pd->tcpcb->l7_proxy_type = types::AppRedirType(data.u.read_tx2rx_d.l7_proxy_type);
    tcpcb_pd->tcpcb->serq_ci = types::AppRedirType(data.u.read_tx2rx_d.serq_cidx);

    HAL_TRACE_DEBUG("Received snd_nxt: {:#x}", tcpcb_pd->tcpcb->snd_nxt);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s1_t0_tcp_rx_d          data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: tcp_rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->rcv_nxt = ntohl(data.u.tcp_rx_d.rcv_nxt);
    tcpcb_pd->tcpcb->snd_una = ntohl(data.u.tcp_rx_d.snd_una);
    tcpcb_pd->tcpcb->snd_wnd = ntohs(data.u.tcp_rx_d.snd_wnd);
    tcpcb_pd->tcpcb->serq_pi = ntohs(data.u.tcp_rx_d.serq_pidx);
    tcpcb_pd->tcpcb->ts_recent = ntohl(data.u.tcp_rx_d.ts_recent);
    tcpcb_pd->tcpcb->state = data.u.tcp_rx_d.state;
    tcpcb_pd->tcpcb->pred_flags = ntohl(data.u.tcp_rx_d.pred_flags);
    tcpcb_pd->tcpcb->snd_recover = ntohl(data.u.tcp_rx_d.snd_recover);
    tcpcb_pd->tcpcb->ooq_not_empty = data.u.tcp_rx_d.ooq_not_empty;
    if (data.u.tcp_rx_d.cfg_flags & TCP_OPER_FLAG_DELACK) {
        tcpcb_pd->tcpcb->delay_ack = true;
    }
    if (data.u.tcp_rx_d.cfg_flags & TCP_OPER_FLAG_OOO_QUEUE) {
        tcpcb_pd->tcpcb->ooo_queue = true;
    }
    tcpcb_pd->tcpcb->ato = ntohs(data.u.tcp_rx_d.ato * TCP_TIMER_TICK);
    tcpcb_pd->tcpcb->cc_flags = data.u.tcp_rx_d.cc_flags;

    HAL_TRACE_DEBUG("Received rcv_nxt: {:#x}", tcpcb_pd->tcpcb->rcv_nxt);
    HAL_TRACE_DEBUG("Received snd_una: {:#x}", tcpcb_pd->tcpcb->snd_una);
    HAL_TRACE_DEBUG("Received snd_wnd: {:#x}", tcpcb_pd->tcpcb->snd_wnd);
    HAL_TRACE_DEBUG("HWID: {} Received ts_recent: {}", hwid, tcpcb_pd->tcpcb->ts_recent);
    HAL_TRACE_DEBUG("Received debug_dol: {:#x}", tcpcb_pd->tcpcb->debug_dol);
    HAL_TRACE_DEBUG("Received state: {:#x}", tcpcb_pd->tcpcb->state);
    HAL_TRACE_DEBUG("Received pred_flags: {:#x}", tcpcb_pd->tcpcb->pred_flags);
    HAL_TRACE_DEBUG("Received snd_recover: {:#x}", tcpcb_pd->tcpcb->snd_recover);
    HAL_TRACE_DEBUG("Received cfg_flags: {:#x}", data.u.tcp_rx_d.cfg_flags);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_tcp_rtt_entry(pd_tcpcb_t* tcpcb_pd)
{
    //s3_t0_tcp_rx_tcp_rtt_d data = {0};
    s3_t0_tcp_rx_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RTT);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: tcp-rtt entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->rto = ntohl(data.u.tcp_rtt_d.rto);
    tcpcb_pd->tcpcb->srtt_us = ntohl(data.u.tcp_rtt_d.srtt_us);
    tcpcb_pd->tcpcb->rtt_seq_tsoffset = ntohl(data.u.tcp_rtt_d.rtt_seq_tsoffset);
    tcpcb_pd->tcpcb->ts_offset = ntohl(data.u.tcp_rtt_d.rtt_seq_tsoffset);
    tcpcb_pd->tcpcb->ts_time = ntohl(data.u.tcp_rtt_d.rtt_time);
    tcpcb_pd->tcpcb->rtt_time = ntohl(data.u.tcp_rtt_d.rtt_time);
    tcpcb_pd->tcpcb->ts_learned = data.u.tcp_rtt_d.ts_learned;

    //tcpcb_pd->tcpcb->curr_ts = ntohl(data.u.tcp_rtt_d.curr_ts);
    //tcpcb_pd->tcpcb->rttvar_us = ntohl(data.u.tcp_rtt_d.rttvar_us);
    //tcpcb_pd->tcpcb->rtt_updated = ntohl(data.u.tcp_rtt_d.rtt_updated);

    HAL_TRACE_DEBUG("Received rto: {:#x}", tcpcb_pd->tcpcb->rto);
    HAL_TRACE_DEBUG("Received srtt_us: {}", tcpcb_pd->tcpcb->srtt_us);
    HAL_TRACE_DEBUG("Received rtt_seq_tsoffset: {}", tcpcb_pd->tcpcb->rtt_seq_tsoffset);
    HAL_TRACE_DEBUG("Received rtt_time: {}", tcpcb_pd->tcpcb->rtt_time);
    HAL_TRACE_DEBUG("Received ts_learned: {}", tcpcb_pd->tcpcb->ts_learned);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_tcp_cc_entry(pd_tcpcb_t* tcpcb_pd)
{
    s4_t0_tcp_rx_d      data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_CC);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->cc_algo = data.u.tcp_cc_d.cc_algo;
    tcpcb_pd->tcpcb->snd_cwnd = ntohl(data.u.tcp_cc_d.snd_cwnd);
    tcpcb_pd->tcpcb->smss = ntohs(data.u.tcp_cc_d.smss);
    tcpcb_pd->tcpcb->snd_ssthresh = ntohl(data.u.tcp_cc_d.snd_ssthresh);
    tcpcb_pd->tcpcb->abc_l_var = data.u.tcp_cc_d.abc_l_var;

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_tcp_fc_entry(pd_tcpcb_t* tcpcb_pd)
{
    s5_t0_tcp_rx_d      data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FC);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_fc entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->cpu_id = data.u.tcp_fc_d.cpu_id;
    tcpcb_pd->tcpcb->rcv_wnd = ntohl(data.u.tcp_fc_d.rcv_wnd);
    tcpcb_pd->tcpcb->rcv_wscale = data.u.tcp_fc_d.rcv_scale;
    tcpcb_pd->tcpcb->rcv_mss = ntohs(data.u.tcp_fc_d.rcv_mss);
    HAL_TRACE_DEBUG("Received rcv_wnd: {}", tcpcb_pd->tcpcb->rcv_wnd);
    HAL_TRACE_DEBUG("Received rcv_wscale: {}", tcpcb_pd->tcpcb->rcv_wscale);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_rx_dma_entry(pd_tcpcb_t* tcpcb_pd)
{
    s6_t0_tcp_rx_d      data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_DMA);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to read rx: rx_dma entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->serq_base = ntohll(data.u.dma_d.serq_base);
    if (data.u.dma_d.app_type_cfg == TCP_APP_TYPE_NVME) {
        tcpcb_pd->tcpcb->proxy_type = types::PROXY_TYPE_NVME;
    } else if (data.u.dma_d.app_type_cfg == TCP_APP_TYPE_TLS) {
        tcpcb_pd->tcpcb->proxy_type = types::PROXY_TYPE_TLS;
    } else {
        tcpcb_pd->tcpcb->proxy_type = types::PROXY_TYPE_TCP;
    }
    HAL_TRACE_DEBUG("Received serq_base: {:#x}", tcpcb_pd->tcpcb->serq_base);
    HAL_TRACE_DEBUG("Received proxy_type: {}", tcpcb_pd->tcpcb->proxy_type);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_ooq_entry(pd_tcpcb_t* tcpcb_pd)
{
    s5_t2_tcp_rx_ooo_qbase_cb_load_d cb_load_d = {0};
    s2_t2_tcp_rx_ooo_book_keeping_d bookkeeping_d = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_OOO_QADDR_OFFSET);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&cb_load_d, sizeof(cb_load_d))){
        HAL_TRACE_ERR("Failed to read rx: ooq qaddr entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_OOO_BOOK_KEEPING_OFFSET);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&bookkeeping_d,
                sizeof(bookkeeping_d))){
        HAL_TRACE_ERR("Failed to read rx: ooq bookkeeping entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    if (bookkeeping_d.tcp_opt_flags & TCP_OPT_FLAG_SACK_PERM) {
        tcpcb_pd->tcpcb->sack_perm = true;
    }
    if (bookkeeping_d.tcp_opt_flags & TCP_OPT_FLAG_TIMESTAMPS) {
        tcpcb_pd->tcpcb->timestamps = true;
    }
    HAL_TRACE_DEBUG("tcp_opt_flags: {:#x}", bookkeeping_d.tcp_opt_flags);

    tcpcb_pd->tcpcb->ooq_entry[0].queue_addr = ntohll(cb_load_d.ooo_qbase_addr0);
    tcpcb_pd->tcpcb->ooq_entry[0].start_seq = ntohl(bookkeeping_d.start_seq0);
    tcpcb_pd->tcpcb->ooq_entry[0].end_seq = ntohl(bookkeeping_d.end_seq0);
    tcpcb_pd->tcpcb->ooq_entry[0].num_entries = ntohs(bookkeeping_d.tail_index0);

    tcpcb_pd->tcpcb->ooq_entry[1].queue_addr = ntohll(cb_load_d.ooo_qbase_addr1);
    tcpcb_pd->tcpcb->ooq_entry[1].start_seq = ntohl(bookkeeping_d.start_seq1);
    tcpcb_pd->tcpcb->ooq_entry[1].end_seq = ntohl(bookkeeping_d.end_seq1);
    tcpcb_pd->tcpcb->ooq_entry[1].num_entries = ntohs(bookkeeping_d.tail_index1);

    tcpcb_pd->tcpcb->ooq_entry[2].queue_addr = ntohll(cb_load_d.ooo_qbase_addr2);
    tcpcb_pd->tcpcb->ooq_entry[2].start_seq = ntohl(bookkeeping_d.start_seq2);
    tcpcb_pd->tcpcb->ooq_entry[2].end_seq = ntohl(bookkeeping_d.end_seq2);
    tcpcb_pd->tcpcb->ooq_entry[2].num_entries = ntohs(bookkeeping_d.tail_index2);

    tcpcb_pd->tcpcb->ooq_entry[3].queue_addr = ntohll(cb_load_d.ooo_qbase_addr3);
    tcpcb_pd->tcpcb->ooq_entry[3].start_seq = ntohl(bookkeeping_d.start_seq3);
    tcpcb_pd->tcpcb->ooq_entry[3].end_seq = ntohl(bookkeeping_d.end_seq3);
    tcpcb_pd->tcpcb->ooq_entry[3].num_entries = ntohs(bookkeeping_d.tail_index3);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcpcb_rxdma_entry(pd_tcpcb_t* tcpcb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_tcp_rx_read_tx2rx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get read_tx2rx entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_tcp_rx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_rx entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_tcp_rtt_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_rtt entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_tcp_cc_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_cc entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_tcp_fc_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_fc entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_rx_dma_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_fc entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_ooq_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ooq entry");
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

static hal_ret_t
p4pd_get_tcpcb_rxdma_stats(pd_tcpcb_t* tcpcb_pd)
{
    s1_t0_tcp_rx_d tcp_rx_d = { 0 };
    s6_t0_tcp_rx_dma_d rx_dma_d = { 0 };
    tcp_rx_stats_t stats;
    tcpcb_hw_id_t hwid;

    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&tcp_rx_d, sizeof(tcp_rx_d))) {
        HAL_TRACE_ERR("Failed to get rx: tcp_rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
     }

    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_DMA);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&rx_dma_d, sizeof(rx_dma_d))) {
        HAL_TRACE_ERR("Failed to get rx: rx_dma entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_RX_PER_FLOW_STATS_OFFSET;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats, sizeof(stats))) {
        HAL_TRACE_ERR("Failed to get rx: stats entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->bytes_rcvd = ntohs(tcp_rx_d.u.tcp_rx_d.bytes_rcvd) +
                                    stats.bytes_rcvd;
    tcpcb_pd->tcpcb->bytes_acked = ntohs(tcp_rx_d.u.tcp_rx_d.bytes_acked) +
                                    stats.bytes_acked;
    tcpcb_pd->tcpcb->pure_acks_rcvd = tcp_rx_d.u.tcp_rx_d.pure_acks_rcvd +
                                    stats.pure_acks_rcvd;
    tcpcb_pd->tcpcb->dup_acks_rcvd = tcp_rx_d.u.tcp_rx_d.dup_acks_rcvd +
                                    stats.dup_acks_rcvd;
    tcpcb_pd->tcpcb->ooo_cnt = tcp_rx_d.u.tcp_rx_d.ooo_cnt + stats.ooo_cnt;
    tcpcb_pd->tcpcb->pkts_rcvd = rx_dma_d.pkts_rcvd + stats.pkts_rcvd;

    HAL_TRACE_DEBUG("bytes_rcvd {} pkts_rcvd {}",
            tcpcb_pd->tcpcb->bytes_rcvd,
            tcpcb_pd->tcpcb->pkts_rcvd);

    // These stats don't have overflow implemented in stats region
    tcpcb_pd->tcpcb->slow_path_cnt = tcp_rx_d.u.tcp_rx_d.slow_path_cnt;
    tcpcb_pd->tcpcb->serq_full_cnt = tcp_rx_d.u.tcp_rx_d.serq_full_cnt;
    tcpcb_pd->tcpcb->rx_drop_cnt = tcp_rx_d.u.tcp_rx_d.rx_drop_cnt;

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_get_tcpcb_txdma_stats(pd_tcpcb_t* tcpcb_pd)
{
    s5_t0_tcp_tx_tso_d tso_d = { 0 };
    tcp_tx_stats_t stats;
    s0_t0_tcp_tx_d rx2tx_d;
    tcpcb_hw_id_t hwid;

    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&rx2tx_d, sizeof(rx2tx_d))) {
        HAL_TRACE_ERR("Failed to get rx: rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
     }

    hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&tso_d, sizeof(tso_d))) {
        HAL_TRACE_ERR("Failed to get rx: tso entry for TCP CB");
        return HAL_RET_HW_FAIL;
     }

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_TX_PER_FLOW_STATS_OFFSET;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats, sizeof(stats))) {
        HAL_TRACE_ERR("Failed to get rx: stats entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->zero_window_sent = ntohs(tso_d.zero_window_sent);
    tcpcb_pd->tcpcb->bytes_sent = ntohs(tso_d.bytes_sent) +
                                    stats.bytes_sent;
    tcpcb_pd->tcpcb->pkts_sent = tso_d.pkts_sent + stats.pkts_sent;
    tcpcb_pd->tcpcb->pure_acks_sent = tso_d.pure_acks_sent + stats.pure_acks_sent;
    tcpcb_pd->tcpcb->send_ack_pi = rx2tx_d.u.read_rx2tx_d.pi_1;
    tcpcb_pd->tcpcb->send_ack_ci = rx2tx_d.u.read_rx2tx_d.ci_1;
    tcpcb_pd->tcpcb->fast_timer_pi = rx2tx_d.u.read_rx2tx_d.pi_2;
    tcpcb_pd->tcpcb->fast_timer_ci = rx2tx_d.u.read_rx2tx_d.ci_2;
    tcpcb_pd->tcpcb->del_ack_pi = rx2tx_d.u.read_rx2tx_d.pi_3;
    tcpcb_pd->tcpcb->del_ack_ci = rx2tx_d.u.read_rx2tx_d.ci_3;
    tcpcb_pd->tcpcb->asesq_pi = rx2tx_d.u.read_rx2tx_d.pi_4;
    tcpcb_pd->tcpcb->asesq_ci = rx2tx_d.u.read_rx2tx_d.ci_4;
    tcpcb_pd->tcpcb->pending_tx_pi = rx2tx_d.u.read_rx2tx_d.pi_5;
    tcpcb_pd->tcpcb->pending_tx_ci = rx2tx_d.u.read_rx2tx_d.ci_5;
    tcpcb_pd->tcpcb->fast_retrans_pi = rx2tx_d.u.read_rx2tx_d.pi_6;
    tcpcb_pd->tcpcb->fast_retrans_ci = rx2tx_d.u.read_rx2tx_d.ci_6;
    tcpcb_pd->tcpcb->clean_retx_pi = rx2tx_d.u.read_rx2tx_d.pi_7;
    tcpcb_pd->tcpcb->clean_retx_ci = rx2tx_d.u.read_rx2tx_d.ci_7;

    HAL_TRACE_DEBUG("bytes_sent {} pkts_sent {}",
            tcpcb_pd->tcpcb->bytes_sent, tcpcb_pd->tcpcb->pkts_sent)

    return HAL_RET_OK;
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_tcp_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "tcp_tx_stage0";

    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    *offset >>= MPU_PC_ADDR_SHIFT;
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_ooq_rx2tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "tcp_ooq_rx2tx_stage0";

    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    *offset >>= MPU_PC_ADDR_SHIFT;
    return HAL_RET_OK;
}

#define     DEBUG_DOL_TEST_TIMER_FULL_SET       1
#define     DEBUG_DOL_TEST_TIMER_FULL_RESET     0
#define     DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES  2

static void
debug_dol_init_timer_full_area(int state)
{
    uint64_t timer_key_hbm_base_addr;
    uint8_t byte;
    uint64_t data[DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES * 2 * 8];

    timer_key_hbm_base_addr = (uint64_t)asicpd_get_mem_addr((char *)JTIMERS);
#ifdef ELBA
    timer_key_hbm_base_addr += (DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES *
                    ELBA_TIMER_NUM_KEY_PER_CACHE_LINE * 64);
#else
    timer_key_hbm_base_addr += (DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES *
                    CAPRI_TIMER_NUM_KEY_PER_CACHE_LINE * 64);
#endif
    if (state == DEBUG_DOL_TEST_TIMER_FULL_SET) {
        byte = 0xff;
    } else {
        byte = 0;
    }

    memset(&data, byte, sizeof(data));

    p4plus_hbm_write(timer_key_hbm_base_addr, (uint8_t *)&data, sizeof(data),
            P4PLUS_CACHE_INVALIDATE_BOTH);
}

static void
debug_dol_test_timer_full(int state)
{
#ifdef ELBA
    if (state == DEBUG_DOL_TEST_TIMER_FULL_SET) {
        /*
         * Debug code to force num key_lines to 2
         */
        HAL_TRACE_DEBUG("setting num key_lines = 2");
        sdk::platform::elba::elba_txs_timer_init_hsh_depth(DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES);
    } else {
        HAL_TRACE_DEBUG("resetting num key_lines back to default");
        sdk::platform::elba::elba_txs_timer_init_hsh_depth(ELBA_TIMER_NUM_KEY_CACHE_LINES);
    }
#else
    if (state == DEBUG_DOL_TEST_TIMER_FULL_SET) {
        /*
         * Debug code to force num key_lines to 2
         */
        HAL_TRACE_DEBUG("setting num key_lines = 2");
        sdk::platform::capri::capri_txs_timer_init_hsh_depth(DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES);
    } else {
        HAL_TRACE_DEBUG("resetting num key_lines back to default");
        sdk::platform::capri::capri_txs_timer_init_hsh_depth(CAPRI_TIMER_NUM_KEY_CACHE_LINES);
    }
#endif

    debug_dol_init_timer_full_area(state);
}

hal_ret_t
p4pd_add_or_del_tcp_ooo_rx2tx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s0_t0_ooq_tcp_tx_d             data = { 0 };
    hal_ret_t                      ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id_qtype1;

    if(!del) {
        uint64_t pc_offset;
        wring_hw_id_t ooo_rx2tx_qbase;
        // get pc address
        ret = p4pd_get_tcp_ooq_rx2tx_stage0_prog_addr(&pc_offset);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
        }
        HAL_TRACE_DEBUG("Received pc address {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.load_stage0_d.total = TCP_PROXY_OOO_RX2TX_TOTAL_RINGS;

        ret = wring_pd_get_base_addr(types::WRING_TYPE_TCP_OOO_RX2TX,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &ooo_rx2tx_qbase);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive ooo rx2tx base for tcp cb: {}",
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            data.u.load_stage0_d.ooo_rx2tx_qbase = htonll(ooo_rx2tx_qbase);
        }

#ifdef ELBA
        uint64_t mem_addr = (uint64_t)asicpd_get_mem_addr(
                ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + ELBA_GC_GLOBAL_OOQ_TX2RX_FP_PI;
#else
        uint64_t mem_addr = (uint64_t)asicpd_get_mem_addr(
                ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + CAPRI_GC_GLOBAL_OOQ_TX2RX_FP_PI;
#endif
        data.u.load_stage0_d.ooo_rx2tx_free_pi_addr = htonll(mem_addr);

        data.u.load_stage0_d.ooo_rx2tx_producer_ci_addr =
                htonll(tcpcb_pd_ooo_rx2tx_prod_ci_addr_get(tcpcb_pd->tcpcb->cb_id));
    }
    if (!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create ooq rx2tx entry");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_get_tcp_ooo_rx2tx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s0_t0_ooq_tcp_tx_d             data = { 0 };
    hal_ret_t                      ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id_qtype1;

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get ooq tx: stage0 entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->ooq_rx2tx_pi = data.u.load_stage0_d.pi_0;
    tcpcb_pd->tcpcb->ooq_rx2tx_ci = data.u.load_stage0_d.ci_0;
    tcpcb_pd->tcpcb->window_update_pi = data.u.load_stage0_d.pi_1;
    tcpcb_pd->tcpcb->window_update_ci = data.u.load_stage0_d.ci_1;

    tcpcb_pd->tcpcb->ooo_rx2tx_qbase = htonll(data.u.load_stage0_d.ooo_rx2tx_qbase);

    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_tx_read_rx2tx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s0_t0_tcp_tx_d                 data = {0};
    hal_ret_t                      ret = HAL_RET_OK;
    static tcpcb_hw_id_t           debug_dol_timer_full_hw_id;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);

    if(!del) {
        uint64_t pc_offset;
        // get pc address
        if(p4pd_get_tcp_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            // ret = HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG("Received pc address {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.read_rx2tx_d.total = TCP_PROXY_TX_TOTAL_RINGS;
        data.u.read_rx2tx_d.eval_last = 1 << TCP_SCHED_RING_FAST_TIMER;
        data.u.read_rx2tx_d.debug_dol_tx = htons(tcpcb_pd->tcpcb->debug_dol_tx);
        if (!debug_dol_timer_full_hw_id &&
                tcpcb_pd->tcpcb->debug_dol_tx & TCP_TX_DDOL_FORCE_TIMER_FULL) {
            debug_dol_test_timer_full(DEBUG_DOL_TEST_TIMER_FULL_SET);
            debug_dol_timer_full_hw_id = hwid;
        } else if (debug_dol_timer_full_hw_id &&
                !(tcpcb_pd->tcpcb->debug_dol_tx & TCP_TX_DDOL_FORCE_TIMER_FULL)) {
            debug_dol_test_timer_full(DEBUG_DOL_TEST_TIMER_FULL_RESET);
            debug_dol_timer_full_hw_id = 0;
        }
        HAL_TRACE_DEBUG("TCPCB rx2tx debug_dol_tx: {:#x}", data.u.read_rx2tx_d.debug_dol_tx);

        // get sesq address
        wring_hw_id_t   sesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &sesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive sesq base for tcp cb: {}",
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Sesq base[{:#x}]: {:#x}",
                    tcpcb_pd->tcpcb->cb_id, sesq_base);
            data.u.read_rx2tx_d.sesq_base = sesq_base;
        }

        // set sesq_tx_ci to invalid
        // This is set to valid ci only in window full conditions
        // otherwise transmission happens from ci_0
        data.u.read_rx2tx_d.sesq_tx_ci = TCP_TX_INVALID_SESQ_TX_CI;
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_tx_read_rx2tx_extra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s1_t0_tcp_tx_d                      data = {0};
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);

    if(!del) {
        data.u.read_rx2tx_extra_d.snd_una = htonl(tcpcb_pd->tcpcb->snd_una);
        data.u.read_rx2tx_extra_d.snd_wnd = htons(tcpcb_pd->tcpcb->snd_wnd);
        data.u.read_rx2tx_extra_d.snd_cwnd = htonl(tcpcb_pd->tcpcb->snd_cwnd);
        data.u.read_rx2tx_extra_d.rcv_wnd = htons(tcpcb_pd->tcpcb->rcv_wnd >>
                                                  tcpcb_pd->tcpcb->rcv_wscale);
        data.u.read_rx2tx_extra_d.rcv_nxt = htonl(tcpcb_pd->tcpcb->rcv_nxt);
        // TODO : fix this hardcoding
        data.u.read_rx2tx_extra_d.rto = htonl(100);
        data.u.read_rx2tx_extra_d.state = (uint8_t)tcpcb_pd->tcpcb->state;
        data.u.read_rx2tx_extra_d.rcv_tsval = htonl(tcpcb_pd->tcpcb->ts_recent);


    }
    HAL_TRACE_DEBUG("rx2tx extra add/del");
    HAL_TRACE_DEBUG("    snd_cwnd: {}", data.u.read_rx2tx_extra_d.snd_cwnd);
    HAL_TRACE_DEBUG("    rcv_nxt: {}", data.u.read_rx2tx_extra_d.rcv_nxt);
    HAL_TRACE_DEBUG("    snd_wnd: {}", data.u.read_rx2tx_extra_d.snd_wnd);
    HAL_TRACE_DEBUG("    rcv_wnd: {}", tcpcb_pd->tcpcb->rcv_wnd);
    HAL_TRACE_DEBUG("    rcv_wscale: {}", tcpcb_pd->tcpcb->rcv_wscale);
    HAL_TRACE_DEBUG("    rcv_tsval: {}", data.u.read_rx2tx_extra_d.rcv_tsval);


    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_tx_tcp_retx_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s3_t0_tcp_tx_retx_d     data = {0};
    hal_ret_t               ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_RETX);

    if(!del) {
        uint64_t gc_base;

        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            data.sesq_ci_addr =
                htonll(tcpcb_pd_serq_prod_ci_addr_get(tcpcb_pd->tcpcb->other_qid));
        } else {
            uint64_t tls_stage0_addr;
            tls_stage0_addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TLS_PROXY, 0,
                        tcpcb_pd->tcpcb->other_qid);
            data.sesq_ci_addr =
                htonll(tls_stage0_addr + pd_tlscb_sesq_ci_offset_get());
        }
        data.retx_snd_una = htonl(tcpcb_pd->tcpcb->snd_una);

        // get gc address
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            gc_base = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_GC,
                    ASIC_HBM_GC_RNMDR_QTYPE,
                    ASIC_RNMDR_GC_TCP_RING_PRODUCER) + TCP_GC_CB_SW_PI_OFFSET;
        } else {
            gc_base = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_GC,
                    ASIC_HBM_GC_TNMDR_QTYPE,
                    ASIC_TNMDR_GC_TCP_RING_PRODUCER) + TCP_GC_CB_SW_PI_OFFSET;
        }
        HAL_TRACE_DEBUG("gc_base: {:#x}", gc_base);
        data.gc_base = htonll(gc_base);
        if (tcpcb_pd->tcpcb->proxy_type == types::PROXY_TYPE_TCP) {
            // Used by window update logic to wakeup proxy peer TCP segment queue
            data.consumer_qid = htons(tcpcb_pd->tcpcb->other_qid);
        } else {
            // Not used
            data.consumer_qid = 0;
        }
	data.read_notify_bytes = 0;
	data.read_notify_bytes_local = 0;

        HAL_TRACE_DEBUG("consumer_qid: {:#x}", data.consumer_qid);
    }

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t
p4pd_add_or_del_tcp_tx_xmit_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s4_t0_tcp_tx_xmit_d             data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_XMIT);

    if(!del) {
        data.snd_nxt = htonl(tcpcb_pd->tcpcb->snd_nxt);
        data.snd_wscale = tcpcb_pd->tcpcb->snd_wscale;
        data.is_cwnd_limited = 0x00;
        data.rto_backoff = (uint8_t)htonl(tcpcb_pd->tcpcb->rto_backoff);
        data.smss = htons(tcpcb_pd->tcpcb->smss);
        data.initial_window = htonl(tcpcb_pd->tcpcb->initial_window);
        data.rtt_seq_req = 1;
    }

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Set RTT seq req in XMIT to  {:#x}", data.rtt_seq_req);
    return ret;
}

hal_ret_t
p4pd_add_or_del_tcp_tx_tso_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s5_t0_tcp_tx_tso_d                  data = {0};
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);

    if(!del) {
        uint64_t stats_base = asicpd_get_mem_addr(
                ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS);
        SDK_ASSERT(stats_base + tcpcb_pd->tcpcb->cb_id *
                   TCP_PER_FLOW_STATS_SIZE +
                   TCP_PER_FLOW_STATS_SIZE <= stats_base +
                   asicpd_get_mem_size_kb(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) *
                   1024);
        data.tx_stats_base = htonll(stats_base +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_TX_PER_FLOW_STATS_OFFSET);
        HAL_TRACE_DEBUG("tcp qid {}, tx stats base: {:#x}",
                        tcpcb_pd->tcpcb->cb_id, ntohll(data.tx_stats_base));

        data.source_lif = htons(tcpcb_pd->tcpcb->source_lif);
        data.source_port = htons(tcpcb_pd->tcpcb->source_port);
        data.dest_port = htons(tcpcb_pd->tcpcb->dest_port);
        if (tcpcb_pd->tcpcb->header_len != INVALID_HEADER_TEMPLATE_LEN) {
            data.header_len = htons(tcpcb_pd->tcpcb->header_len);
        } else {
            HAL_TRACE_DEBUG("Skipping header template len update");
        }
        data.smss = htons(tcpcb_pd->tcpcb->smss);
        data.ts_offset = htonl(tcpcb_pd->tcpcb->ts_offset);
        data.ts_time   = htonl(tcpcb_pd->tcpcb->ts_time);
        HAL_TRACE_DEBUG(" tcp_tx_tso add/del:")
        HAL_TRACE_DEBUG("    TCPCB source lif: {:#x}", data.source_lif);
        HAL_TRACE_DEBUG("    TCPCB source port: {:#x} dest port {:#x}",
            data.source_port, data.dest_port);
        HAL_TRACE_DEBUG("TCPCB header len: {}", data.header_len);

        if (tcpcb_pd->tcpcb->sack_perm) {
            data.tcp_opt_flags |= TCP_OPT_FLAG_SACK_PERM;
        }
        if ( tcpcb_pd->tcpcb->timestamps && tcpcb_pd->tcpcb->ts_recent) {
            data.tcp_opt_flags |= TCP_OPT_FLAG_TIMESTAMPS;
        }
        HAL_TRACE_DEBUG("tcp_opt_flags: {:#x}", data.tcp_opt_flags);
        HAL_TRACE_DEBUG("    TCPCB tsoffset: {}", data.ts_offset);
    }

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

#define __MAX(a, b) ((a) > (b) ? (a) : (b))

hal_ret_t
p4pd_add_or_del_tcp_tx_header_template_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    uint8_t                             data[64];
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_HEADER_TEMPLATE);

    if(!del) {
        memcpy(data, tcpcb_pd->tcpcb->header_template, sizeof(data));
        HAL_TRACE_DEBUG("TCPCB header template addr {:#x}", hwid);
        HAL_TRACE_DEBUG("TCPCB header template={}", hex_dump((uint8_t*)data, sizeof(data)));
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}
hal_ret_t
p4pd_add_or_del_tcpcb_txdma_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    // qtype 1
    ret = p4pd_add_or_del_tcp_ooo_rx2tx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    // qtype 0
    ret = p4pd_add_or_del_tcp_tx_read_rx2tx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tcp_tx_read_rx2tx_extra_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_tcp_tx_tcp_retx_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_tcp_tx_xmit_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_tcp_tx_tso_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    if (tcpcb_pd->tcpcb->header_len != INVALID_HEADER_TEMPLATE_LEN) {
        ret = p4pd_add_or_del_tcp_tx_header_template_entry(tcpcb_pd, del);
        if(ret != HAL_RET_OK) {
            goto cleanup;
        }
    }

    return HAL_RET_OK;

cleanup:

    /* TODO: Cleanup */
    return ret;
}


hal_ret_t
p4pd_get_tcp_tx_read_rx2tx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s0_t0_tcp_tx_d          data = {0};
    uint16_t                debug_dol_tx;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    debug_dol_tx = ntohs(data.u.read_rx2tx_d.debug_dol_tx);
    if (debug_dol_tx & TCP_TX_DDOL_FORCE_TBL_SETADDR) {
        /*
         * DEBUG ONLY : read at a shifted offset
         */
        HAL_TRACE_DEBUG("reading rx2tx at an offset {}",
                TCP_DDOL_TBLADDR_SHIFT_OFFSET);
        hwid += TCP_DDOL_TBLADDR_SHIFT_OFFSET;
        if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
            HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
            return HAL_RET_HW_FAIL;
        }
    }
    tcpcb_pd->tcpcb->debug_dol_tx = debug_dol_tx;
    tcpcb_pd->tcpcb->sesq_base = data.u.read_rx2tx_d.sesq_base;
    tcpcb_pd->tcpcb->sesq_pi = data.u.read_rx2tx_d.pi_0;
    tcpcb_pd->tcpcb->sesq_ci = data.u.read_rx2tx_d.ci_0;
    tcpcb_pd->tcpcb->sesq_retx_ci = ntohs(data.u.read_rx2tx_d.sesq_retx_ci);
    tcpcb_pd->tcpcb->asesq_retx_ci = ntohs(data.u.read_rx2tx_d.asesq_retx_ci);
    tcpcb_pd->tcpcb->sesq_tx_ci = ntohs(data.u.read_rx2tx_d.sesq_tx_ci);

    tcpcb_pd->tcpcb->asesq_pi = data.u.read_rx2tx_d.pi_4;
    tcpcb_pd->tcpcb->asesq_ci = data.u.read_rx2tx_d.ci_4;

    tcpcb_pd->tcpcb->debug_dol_tblsetaddr = data.u.read_rx2tx_d.debug_dol_tblsetaddr;

    tcpcb_pd->tcpcb->debug_dol_tblsetaddr = data.u.read_rx2tx_d.debug_dol_tblsetaddr;
    tcpcb_pd->tcpcb->fast_timer_ci = data.u.read_rx2tx_d.ci_2;
    tcpcb_pd->tcpcb->rto_deadline = ntohs(data.u.read_rx2tx_d.rto_deadline);
    tcpcb_pd->tcpcb->ato_deadline = ntohs(data.u.read_rx2tx_d.ato_deadline);
    tcpcb_pd->tcpcb->idle_deadline = ntohs(data.u.read_rx2tx_d.idle_deadline);

    HAL_TRACE_DEBUG("Received sesq_base: {:#x}", tcpcb_pd->tcpcb->sesq_base);
    HAL_TRACE_DEBUG("Received sesq_pi: {:#x}", tcpcb_pd->tcpcb->sesq_pi);
    HAL_TRACE_DEBUG("Received sesq_ci: {:#x}", tcpcb_pd->tcpcb->sesq_ci);
    HAL_TRACE_DEBUG("Received asesq_pi: {:#x}", tcpcb_pd->tcpcb->asesq_pi);
    HAL_TRACE_DEBUG("Received asesq_ci: {:#x}", tcpcb_pd->tcpcb->asesq_ci);
    HAL_TRACE_DEBUG("Received tblsetaddr: {:#x}",
                    tcpcb_pd->tcpcb->debug_dol_tblsetaddr);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_tx_read_rx2tx_extra_entry(pd_tcpcb_t* tcpcb_pd)
{
    s1_t0_tcp_tx_d              data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->rcv_tsval = ntohs(data.u.read_rx2tx_extra_d.rcv_tsval);

    return HAL_RET_OK;

}

hal_ret_t
p4pd_get_tcp_tx_tcp_retx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s3_t0_tcp_tx_retx_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_RETX);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    // The following are used for DOL tests only
    tcpcb_pd->tcpcb->retx_snd_una = ntohl(data.retx_snd_una);
    tcpcb_pd->tcpcb->tx_ring_pi = ntohs(data.tx_ring_pi);
    tcpcb_pd->tcpcb->partial_pkt_ack_cnt = ntohl(data.partial_pkt_ack_cnt);
    tcpcb_pd->tcpcb->tx_window_update_pi = ntohl(data.tx_window_update_pi);
    tcpcb_pd->tcpcb->read_notify_bytes = ntohl(data.read_notify_bytes);
    tcpcb_pd->tcpcb->read_notify_bytes_local = ntohl(data.read_notify_bytes_local);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_tx_xmit_entry(pd_tcpcb_t* tcpcb_pd)
{
    s4_t0_tcp_tx_xmit_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_XMIT);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->packets_out = ntohs(data.packets_out);
    tcpcb_pd->tcpcb->rto_backoff = data.rto_backoff;
    tcpcb_pd->tcpcb->snd_wscale = data.snd_wscale;
    tcpcb_pd->tcpcb->initial_window = ntohl(data.initial_window);
    tcpcb_pd->tcpcb->window_full_cnt = ntohl(data.window_full_cnt);
    tcpcb_pd->tcpcb->retx_cnt = ntohl(data.retx_cnt);

    HAL_TRACE_DEBUG("TCPCB packets_out: {}", tcpcb_pd->tcpcb->packets_out);

    return HAL_RET_OK;

}

hal_ret_t
p4pd_get_tcp_tx_tso_entry(pd_tcpcb_t* tcpcb_pd)
{
    s5_t0_tcp_tx_tso_d              data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->source_lif  =  ntohs(data.source_lif);
    tcpcb_pd->tcpcb->source_port = ntohs(data.source_port);
    tcpcb_pd->tcpcb->dest_port = ntohs(data.dest_port);
    tcpcb_pd->tcpcb->header_len = ntohs(data.header_len);
    HAL_TRACE_DEBUG("TCPCB source lif: {:#x}", tcpcb_pd->tcpcb->source_lif);
    HAL_TRACE_DEBUG("TCPCB source port: {:#x} dest port {:#x}",
                    tcpcb_pd->tcpcb->source_port, tcpcb_pd->tcpcb->dest_port);
    HAL_TRACE_DEBUG("TCPCB header len: {}", tcpcb_pd->tcpcb->header_len);

    return HAL_RET_OK;

}

hal_ret_t
p4pd_get_tcp_tx_header_template_entry(pd_tcpcb_t* tcpcb_pd)
{
    uint8_t                             data[64];

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_HEADER_TEMPLATE);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    memcpy(tcpcb_pd->tcpcb->header_template, data, sizeof(data));

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcpcb_txdma_entry(pd_tcpcb_t* tcpcb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    // qtype 1
    ret = p4pd_get_tcp_ooo_rx2tx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    // qtype 0
    ret = p4pd_get_tcp_tx_read_rx2tx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_read_rx2tx_extra_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_xmit_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_tso_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_tcp_retx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_header_template_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

cleanup:
    return ret;
}

/**************************/

tcpcb_hw_id_t
pd_tcpcb_get_base_hw_index(pd_tcpcb_t* tcpcb_pd, uint32_t qtype)
{
    SDK_ASSERT(NULL != tcpcb_pd);
    SDK_ASSERT(NULL != tcpcb_pd->tcpcb);

    // Get the base address of TCP CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, qtype,
                    tcpcb_pd->tcpcb->cb_id);
    HAL_TRACE_DEBUG("received offset {:#x}", offset);
    return offset;
}

hal_ret_t
p4pd_add_or_del_tcpcb_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tcpcb_rxdma_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;
    }

    ret = p4pd_add_or_del_tcpcb_txdma_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_tcpcb_entry(pd_tcpcb_t* tcpcb_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_tcpcb_rxdma_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rxdma entry for tcpcb");
        goto err;
    }

    ret = p4pd_get_tcpcb_rxdma_stats(tcpcb_pd);
    ret = p4pd_get_tcpcb_txdma_stats(tcpcb_pd);

    ret = p4pd_get_tcpcb_txdma_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for tcpcb");
        goto err;
    }

err:
    /*TODO: cleanup */
    return ret;
}

static hal_ret_t
p4pd_init_tcpcb_stats(pd_tcpcb_t* tcpcb_pd)
{
    tcpcb_hw_id_t hwid;
    tcp_rx_stats_t rx_stats = { 0 };
    tcp_tx_stats_t tx_stats = { 0 };

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_RX_PER_FLOW_STATS_OFFSET;
    if (!p4plus_hbm_write(hwid, (uint8_t *)&rx_stats, sizeof(rx_stats),
                P4PLUS_CACHE_ACTION_NONE)) {
        HAL_TRACE_ERR("Failed to init rx_stats");
        return HAL_RET_HW_FAIL;
    }

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TCP_PROXY_PER_FLOW_STATS) +
                tcpcb_pd->tcpcb->cb_id * TCP_PER_FLOW_STATS_SIZE +
                TCP_TX_PER_FLOW_STATS_OFFSET;
    if (!p4plus_hbm_write(hwid, (uint8_t *)&tx_stats, sizeof(tx_stats),
                P4PLUS_CACHE_ACTION_NONE)) {
        HAL_TRACE_ERR("Failed to init tx_stats");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_tcpcb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tcpcb_create_args_t *args = pd_func_args->pd_tcpcb_create;
    pd_tcpcb_s              *tcpcb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TCP CB.");

    // allocate PD tcpcb state
    tcpcb_pd = tcpcb_pd_alloc_init();
    if (tcpcb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    tcpcb_pd->tcpcb = args->tcpcb;
    // get hw-id for this TCPCB
    tcpcb_pd->hw_id = pd_tcpcb_get_base_hw_index(tcpcb_pd, TCP_TX_QTYPE);
    tcpcb_pd->tcpcb->cb_base = tcpcb_pd->hw_id;
    tcpcb_pd->hw_id_qtype1 = pd_tcpcb_get_base_hw_index(tcpcb_pd, TCP_OOO_RX2TX_QTYPE);
    tcpcb_pd->tcpcb->cb_base_qtype1 = tcpcb_pd->hw_id_qtype1;

    HAL_TRACE_DEBUG("Creating TCP CB at addr: 0x{:x} qid: {}",
            tcpcb_pd->hw_id, tcpcb_pd->tcpcb->cb_id);

    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_init_tcpcb_stats(tcpcb_pd);

    HAL_TRACE_DEBUG("Programming done");
    // add to db
    ret = add_tcpcb_pd_to_db(tcpcb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->tcpcb->pd = tcpcb_pd;

    return HAL_RET_OK;

cleanup:

    if (tcpcb_pd) {
        tcpcb_pd_free(tcpcb_pd);
    }
    return ret;
}

hal_ret_t
pd_tcpcb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tcpcb_update_args_t *args = pd_func_args->pd_tcpcb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tcpcb_t*                tcpcb = args->tcpcb;
    pd_tcpcb_t*             tcpcb_pd = (pd_tcpcb_t*)tcpcb->pd;

    HAL_TRACE_DEBUG("TCPCB pd update");

    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update tcpcb");
    }
    return ret;
}

hal_ret_t
pd_tcpcb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tcpcb_delete_args_t *args = pd_func_args->pd_tcpcb_delete;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tcpcb_t*                tcpcb = args->tcpcb;
    pd_tcpcb_t*             tcpcb_pd = (pd_tcpcb_t*)tcpcb->pd;

    HAL_TRACE_DEBUG("TCPCB pd delete");

    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete tcpcb entry");
    }

    del_tcpcb_pd_from_db(tcpcb_pd);

    tcpcb_pd_free(tcpcb_pd);

    return ret;
}

hal_ret_t
pd_tcpcb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tcpcb_get_args_t *args = pd_func_args->pd_tcpcb_get;
    pd_tcpcb_t              tcpcb_pd;

    HAL_TRACE_DEBUG("TCPCB pd get for id: {}", args->tcpcb->cb_id);

    // allocate PD tcpcb state
    tcpcb_pd_init(&tcpcb_pd);
    tcpcb_pd.tcpcb = args->tcpcb;

    // get hw-id for this TCPCB
    tcpcb_pd.hw_id = pd_tcpcb_get_base_hw_index(&tcpcb_pd, TCP_TX_QTYPE);
    tcpcb_pd.tcpcb->cb_base = tcpcb_pd.hw_id;
    tcpcb_pd.hw_id_qtype1 = pd_tcpcb_get_base_hw_index(&tcpcb_pd, TCP_OOO_RX2TX_QTYPE);
    tcpcb_pd.tcpcb->cb_base_qtype1 = tcpcb_pd.hw_id_qtype1;
    HAL_TRACE_DEBUG("Received hw-id {:#x}", tcpcb_pd.hw_id);

    // get hw tcpcb entry
    ret = p4pd_get_tcpcb_entry(&tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: {:#x}", tcpcb_pd.tcpcb->cb_id);
    }
    return ret;
}

hal_ret_t
pd_tcp_global_stats_get (pd_func_args_t *pd_func_args)
{
    sdk_ret_t ret;

    sdk::types::mem_addr_t stats_mem_addr =
        asicpd_get_mem_addr(TCP_PROXY_STATS);
    HAL_TRACE_DEBUG("TCP global stats mem_addr: {:x}", stats_mem_addr);

    pd_tcp_global_stats_get_args_t *args = pd_func_args->pd_tcp_global_stats_get;

    ret = sdk::asic::asic_mem_read(stats_mem_addr, (uint8_t *)args,
                                   sizeof(pd_tcp_global_stats_get_args_t));
    return hal_sdk_ret_to_hal_ret(ret);
}

//Short term fix. Bypasses state
hal_ret_t
p4pd_update_sesq_ci_addr (uint32_t qid, uint64_t ci_addr)
{
    hal_ret_t               ret = HAL_RET_OK;
    s3_t0_tcp_tx_retx_d     data = {0};
    uint64_t                addr = 0;

    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, 0,
            qid) + (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_RETX);

    if(sdk::asic::asic_mem_read(addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: read_tx2rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    data.sesq_ci_addr = htonll(ci_addr);

    HAL_TRACE_DEBUG("Updating sesq_ci_addr for for TCP CB qid: {:#x} "
                    "sesq_ci_addr: {:#x} TCPCB write addr: {:#x}",
                    qid, ci_addr, addr);

    if(!p4plus_hbm_write(addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to update sesq_ci_addr for for TCP CB qid: {:#x} "
                      "sesq_ci_addr: {:#x} TCPCB write addr: {:#x}",
                      qid, ci_addr, addr);
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
tcpcb_pd_serq_lif_qtype_qstate_ring_set (uint32_t tcp_qid, uint32_t lif,
                                         uint32_t qtype, uint32_t qid,
                                         uint32_t ring)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint64_t                addr = 0;
    s6_t0_tcp_rx_dma_d      rx_dma_d = { 0 };
    wring_hw_id_t           serq_base;

    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TCP_PROXY, 0,
            tcp_qid) + (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_DMA);

    if(sdk::asic::asic_mem_read(addr,  (uint8_t *)&rx_dma_d, sizeof(rx_dma_d))) {
        HAL_TRACE_ERR("Failed to get rx: read_tx2rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                 tcp_qid, &serq_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive serq base for qid: {}", qid);
    } else {
        HAL_TRACE_DEBUG("Serq base: {:#x}", serq_base);
        rx_dma_d.serq_base = htonll(serq_base);
    }

    rx_dma_d.consumer_lif = htons(lif);
    rx_dma_d.consumer_qtype = qtype;
    rx_dma_d.consumer_qid = htons(qid);
    rx_dma_d.consumer_ring = ring;

    HAL_TRACE_DEBUG("Updating consumer lif: {:#x}, qtype: {:#x}, qid: {:#x}, "
                    "ring: {:#x} for TCP CB qid: {:#x} TCPCB write addr: {:#x}",
                    lif, qtype, qid, ring, tcp_qid, addr);

    if(!p4plus_hbm_write(addr,  (uint8_t *)&rx_dma_d, sizeof(rx_dma_d),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to update consumer lif,qtype,qid for for TCP CB qid: {:#x} "
                      "TCPCB write addr: {:#x}",
                      tcp_qid, addr);
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
