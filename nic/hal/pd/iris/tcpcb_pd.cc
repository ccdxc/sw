#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/tcpcb_pd.hpp"
#include "nic/hal/pd/iris/p4pd_tcp_proxy_api.h"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_tbl_rw.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/wring_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/app_redir_shared.h"

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
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tcpcb_t *)entry)->hw_id);
}

uint32_t
tcpcb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(tcpcb_hw_id_t)) % ht_size;
}

bool
tcpcb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tcpcb_hw_id_t *)key1 == *(tcpcb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_tcp_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "tcp_rx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret < 0) {
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
        HAL_TRACE_DEBUG("Received pc address 0x{0:x}", pc_offset);
        data.action_id = pc_offset;
        data.u.read_tx2rx_d.debug_dol = (uint8_t)tcpcb_pd->tcpcb->debug_dol;
        data.u.read_tx2rx_d.snd_nxt = htonl(tcpcb_pd->tcpcb->snd_nxt);
        data.u.read_tx2rx_d.prr_out = 0xFEEDBABA;
        data.u.read_tx2rx_d.rcv_wup = htonl(tcpcb_pd->tcpcb->rcv_nxt);
        data.u.read_tx2rx_d.l7_proxy_type = tcpcb_pd->tcpcb->l7_proxy_type;
        HAL_TRACE_DEBUG("TCPCB snd_nxt: 0x{0:x}", data.u.read_tx2rx_d.snd_nxt);
        HAL_TRACE_DEBUG("TCPCB rcv_wup: 0x{0:x}", data.u.read_tx2rx_d.rcv_wup);
        HAL_TRACE_DEBUG("TCPCB l7_proxy_type: {:#x}", data.u.read_tx2rx_d.l7_proxy_type);
        HAL_TRACE_DEBUG("TCPCB _debug_dol: 0x{0:x}", data.u.read_tx2rx_d.debug_dol);
    }
    HAL_TRACE_DEBUG("Programming tx2rx at hw-id: 0x{0:x}", hwid); 
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
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
        data.u.tcp_rx_d.ts_recent = htonl(tcpcb_pd->tcpcb->ts_recent);
        data.u.tcp_rx_d.snd_wnd = htons((uint16_t)tcpcb_pd->tcpcb->snd_wnd);
        data.u.tcp_rx_d.rcv_mss = htons((uint16_t)tcpcb_pd->tcpcb->rcv_mss);
        data.u.tcp_rx_d.state = (uint8_t)tcpcb_pd->tcpcb->state;
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

        HAL_TRACE_DEBUG("TCPCB rcv_nxt: 0x{0:x}", data.u.tcp_rx_d.rcv_nxt);
        HAL_TRACE_DEBUG("TCPCB snd_una: 0x{0:x}", data.u.tcp_rx_d.snd_una);
        HAL_TRACE_DEBUG("TCPCB ts_recent: 0x{0:x}", data.u.tcp_rx_d.ts_recent);
        HAL_TRACE_DEBUG("TCPCB snd_wnd: 0x{0:x}", data.u.tcp_rx_d.snd_wnd);
        HAL_TRACE_DEBUG("TCPCB rcv_mss: 0x{0:x}", data.u.tcp_rx_d.rcv_mss);
        HAL_TRACE_DEBUG("TCPCB state: 0x{0:x}", data.u.tcp_rx_d.state);
    }
    int size = sizeof(s1_t0_tcp_rx_d);
    HAL_TRACE_DEBUG("Programming tcp_rx at hw-id: 0x{0:x}", hwid);
    HAL_TRACE_DEBUG("Programming tcp_rx at size: 0x{0:x}", size);
    
    if(!p4plus_hbm_write(hwid, (uint8_t *)&data, size)) {
        HAL_TRACE_ERR("Failed to create rx: tcp_rx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_rtt_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s2_t0_tcp_rx_d    data = {0};
    hal_ret_t          ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_RTT);

    if(!del) {
        data.u.tcp_rtt_d.rto = 100;
        data.u.tcp_rtt_d.srtt_us = 0x80;
        data.u.tcp_rtt_d.seq_rtt_us = 0x10;
        data.u.tcp_rtt_d.ca_rtt_us = 0x10;
        data.u.tcp_rtt_d.curr_ts = 0xf0;
        data.u.tcp_rtt_d.rtt_min = 0x1;
        data.u.tcp_rtt_d.rttvar_us = 0x20;
        data.u.tcp_rtt_d.mdev_us = 0x20;
        data.u.tcp_rtt_d.mdev_max_us = 0;
        data.u.tcp_rtt_d.rtt_seq = 0x20;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_rtt entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tcp_rx_tcp_fc_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s4_t0_tcp_rx_d      data = {0};
    hal_ret_t           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FC);

    if(!del) {
        data.u.tcp_fc_d.page_cnt = 0x1000;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcpcb_write_serq(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s5_t0_tcp_rx_write_serq_d write_serq_d = { 0 };
    hal_ret_t ret = HAL_RET_OK;

    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_SERQ);

    if(!del) {
        // Get Serq address
        wring_hw_id_t  serq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &serq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive serq base for tcp cb: {}", 
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Serq base: 0x{0:x}", serq_base);
            write_serq_d.serq_base = htonl(serq_base);    
        }
    }

    if(!p4plus_hbm_write(hwid, (uint8_t *)&write_serq_d, sizeof(write_serq_d))) {
        HAL_TRACE_ERR("Failed to create rx: write_serq entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    return HAL_RET_OK;
}

hal_ret_t
p4pd_add_or_del_tcpcb_write_l7q_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;
    s5_t2_tcp_rx_d data = { 0 };

    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_L7Q);

    if(!del) {
        // Get L7Q address
        wring_hw_id_t  q_base;
        uint32_t proxyrcb_id = tcpcb_pd->tcpcb->cb_id;

        ret = wring_pd_get_base_addr(types::WRING_TYPE_APP_REDIR_PROXYR,
                                     proxyrcb_id, &q_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive l7q base for proxyrcb_id: {}",
                    proxyrcb_id);
        } else {
            HAL_TRACE_DEBUG("l7q id: {:#x}, base: {:#x}", proxyrcb_id, q_base);
            data.u.write_l7q_d.l7q_base = q_base;
        }
    }
    if(!p4plus_hbm_write(hwid, (uint8_t *)&data, sizeof(s5_t2_tcp_rx_d))) {
        HAL_TRACE_ERR("Failed to create rx: write_serq entry for TCP CB");
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
    
    ret = p4pd_add_or_del_tcp_rx_tcp_fc_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    
    ret = p4pd_add_or_del_tcpcb_write_serq(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
 
    ret = p4pd_add_or_del_tcpcb_write_l7q_entry(tcpcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}
hal_ret_t 
p4pd_get_tcp_rx_read_tx2rx_entry(pd_tcpcb_t* tcpcb_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_READ_TX2RX);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: read_tx2rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->rx_ts = ntohll(data.u.read_tx2rx_d.rx_ts);
    tcpcb_pd->tcpcb->debug_dol = data.u.read_tx2rx_d.debug_dol;
    tcpcb_pd->tcpcb->snd_nxt = ntohl(data.u.read_tx2rx_d.snd_nxt);
    tcpcb_pd->tcpcb->l7_proxy_type = types::AppRedirType(data.u.read_tx2rx_d.l7_proxy_type);

    HAL_TRACE_DEBUG("Received snd_nxt: 0x{0:x}", tcpcb_pd->tcpcb->snd_nxt);

    return HAL_RET_OK;
}


hal_ret_t 
p4pd_get_tcp_rx_tcp_rx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s1_t0_tcp_rx_d          data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: tcp_rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->rcv_nxt = ntohl(data.u.tcp_rx_d.rcv_nxt);
    tcpcb_pd->tcpcb->snd_una = ntohl(data.u.tcp_rx_d.snd_una);
    tcpcb_pd->tcpcb->snd_wnd = ntohs(data.u.tcp_rx_d.snd_wnd);
    tcpcb_pd->tcpcb->ts_recent = ntohl(data.u.tcp_rx_d.ts_recent);
    tcpcb_pd->tcpcb->state = data.u.tcp_rx_d.state;

    HAL_TRACE_DEBUG("Received rcv_nxt: 0x{0:x}", tcpcb_pd->tcpcb->rcv_nxt);
    HAL_TRACE_DEBUG("Received snd_una: 0x{0:x}", tcpcb_pd->tcpcb->snd_una);
    HAL_TRACE_DEBUG("Received snd_wnd: 0x{0:x}", tcpcb_pd->tcpcb->snd_wnd);
    HAL_TRACE_DEBUG("Received ts_recent: 0x{0:x}", tcpcb_pd->tcpcb->ts_recent);
    HAL_TRACE_DEBUG("Received debug_dol: 0x{0:x}", tcpcb_pd->tcpcb->debug_dol);
    HAL_TRACE_DEBUG("Received state: 0x{0:x}", tcpcb_pd->tcpcb->state);

    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_tcp_rx_tcp_fc_entry(pd_tcpcb_t* tcpcb_pd)
{
    s4_t0_tcp_rx_d      data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_FC);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_fc entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_rx_write_serq_entry(pd_tcpcb_t* tcpcb_pd)
{
    s5_t0_tcp_rx_d      data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_SERQ);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to read rx: write_serq entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->serq_base = ntohl(data.u.write_serq_d.serq_base);
    HAL_TRACE_DEBUG("Received serq_base: 0x{0:x}", tcpcb_pd->tcpcb->serq_base);

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

    ret = p4pd_get_tcp_rx_tcp_fc_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_fc entry");
        goto cleanup;
    }

    ret = p4pd_get_tcp_rx_write_serq_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get tcp_fc entry");
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
    s5_t0_tcp_rx_write_serq_d write_serq_d = { 0 };
    tcp_rx_stats_t stats;
    tcpcb_hw_id_t hwid;

    hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_TCP_RX);
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&tcp_rx_d, sizeof(tcp_rx_d))) {
        HAL_TRACE_ERR("Failed to get rx: tcp_rx entry for TCP CB");
        return HAL_RET_HW_FAIL;
     }

    hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_WRITE_SERQ);
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&write_serq_d, sizeof(write_serq_d))) {
        HAL_TRACE_ERR("Failed to get rx: write_serq entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_RX_STATS);
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&stats, sizeof(stats))) {
        HAL_TRACE_ERR("Failed to get rx: stats entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->bytes_rcvd = ntohs(tcp_rx_d.u.tcp_rx_d.bytes_rcvd) +
                                    stats.bytes_rcvd;
    tcpcb_pd->tcpcb->pkts_rcvd = write_serq_d.pkts_rcvd + stats.pkts_rcvd;
    tcpcb_pd->tcpcb->pages_alloced = write_serq_d.pages_alloced +
                                    stats.pages_alloced;
    tcpcb_pd->tcpcb->desc_alloced = write_serq_d.desc_alloced +
                                    stats.desc_alloced;
    tcpcb_pd->tcpcb->debug_num_pkt_to_mem = write_serq_d.debug_num_pkt_to_mem +
                                    stats.debug_num_pkt_to_mem;
    tcpcb_pd->tcpcb->debug_num_phv_to_mem = write_serq_d.debug_num_phv_to_mem +
                                    stats.debug_num_phv_to_mem;

    HAL_TRACE_DEBUG("bytes_rcvd {} pkts_rcvd {} pages_alloced {} desc_alloced {}"
            " pkt2mem {} phv2mem {}", tcpcb_pd->tcpcb->bytes_rcvd,
            tcpcb_pd->tcpcb->pkts_rcvd, tcpcb_pd->tcpcb->pages_alloced,
            tcpcb_pd->tcpcb->desc_alloced, tcpcb_pd->tcpcb->debug_num_pkt_to_mem,
            tcpcb_pd->tcpcb->debug_num_phv_to_mem);

    tcpcb_pd->tcpcb->debug_atomic_delta = stats.debug_atomic_delta;
    tcpcb_pd->tcpcb->debug_atomic0_incr1247 = stats.debug_atomic0_incr1247;
    tcpcb_pd->tcpcb->debug_atomic1_incr247 = stats.debug_atomic1_incr247;
    tcpcb_pd->tcpcb->debug_atomic2_incr47 = stats.debug_atomic2_incr47;
    tcpcb_pd->tcpcb->debug_atomic3_incr47 = stats.debug_atomic3_incr47;
    tcpcb_pd->tcpcb->debug_atomic4_incr7 = stats.debug_atomic4_incr7;
    tcpcb_pd->tcpcb->debug_atomic5_incr7 = stats.debug_atomic5_incr7;
    tcpcb_pd->tcpcb->debug_atomic6_incr7 = stats.debug_atomic6_incr7;

    HAL_TRACE_DEBUG("atomic_delta {} atomic0 {} atomic1 {}",
            tcpcb_pd->tcpcb->debug_atomic_delta,
            tcpcb_pd->tcpcb->debug_atomic0_incr1247,
            tcpcb_pd->tcpcb->debug_atomic1_incr247);

    tcpcb_pd->tcpcb->debug_stage0_7_thread = (write_serq_d.debug_stage4_7_thread << 16) |
            write_serq_d.debug_stage0_3_thread;
    HAL_TRACE_DEBUG("debug_stage0_7_thread 0x{0:x}",
            tcpcb_pd->tcpcb->debug_stage0_7_thread);

    return HAL_RET_OK;
}

static hal_ret_t 
p4pd_get_tcpcb_txdma_stats(pd_tcpcb_t* tcpcb_pd)
{
    s6_t0_tcp_tx_tso_d tso_d = { 0 };
    tcp_tx_stats_t stats;
    tcpcb_hw_id_t hwid;

    hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&tso_d, sizeof(tso_d))) {
        HAL_TRACE_ERR("Failed to get rx: tso entry for TCP CB");
        return HAL_RET_HW_FAIL;
     }

    hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_STATS);
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&stats, sizeof(stats))) {
        HAL_TRACE_ERR("Failed to get rx: stats entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->bytes_sent = ntohs(tso_d.bytes_sent) +
                                    stats.bytes_sent;
    tcpcb_pd->tcpcb->pkts_sent = tso_d.pkts_sent + stats.pkts_sent;
    tcpcb_pd->tcpcb->debug_num_phv_to_pkt = tso_d.debug_num_phv_to_pkt +
                                    stats.debug_num_phv_to_pkt;
    tcpcb_pd->tcpcb->debug_num_mem_to_pkt = tso_d.debug_num_mem_to_pkt +
                                    stats.debug_num_mem_to_pkt;

    HAL_TRACE_DEBUG("bytes_sent {} pkts_sent {} debug_num_phv_to_pkt {} debug_num_mem_to_pkt {}",
            tcpcb_pd->tcpcb->bytes_sent, tcpcb_pd->tcpcb->pkts_sent,
            tcpcb_pd->tcpcb->debug_num_phv_to_pkt, tcpcb_pd->tcpcb->debug_num_mem_to_pkt);

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

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret < 0) {
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

    timer_key_hbm_base_addr = (uint64_t)get_start_offset((char *)JTIMERS);
    timer_key_hbm_base_addr += (DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES *
                    CAPRI_TIMER_NUM_KEY_PER_CACHE_LINE * 64);

    if (state == DEBUG_DOL_TEST_TIMER_FULL_SET) {
        byte = 0xff;
    } else {
        byte = 0;
    }

    memset(&data, byte, sizeof(data));

    p4plus_hbm_write(timer_key_hbm_base_addr, (uint8_t *)&data, sizeof(data));
}

static void
debug_dol_test_timer_full(int state)
{
    if (state == DEBUG_DOL_TEST_TIMER_FULL_SET) {
        /*
         * Debug code to force num key_lines to 2
         */
        HAL_TRACE_DEBUG("setting num key_lines = 2");
        capri_timer_init_helper(DEBUG_DOL_TEST_TIMER_NUM_KEY_LINES);
    } else {
        HAL_TRACE_DEBUG("resetting num key_lines back to default");
        capri_timer_init_helper(CAPRI_TIMER_NUM_KEY_CACHE_LINES);
    }

    debug_dol_init_timer_full_area(state);
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
        HAL_TRACE_DEBUG("Received pc address 0x{0:x}", pc_offset);
        data.action_id = pc_offset;
        data.u.read_rx2tx_d.total = TCP_PROXY_TX_TOTAL_RINGS;
        data.u.read_rx2tx_d.eval_last = 1 << TCP_SCHED_RING_FT;
        data.u.read_rx2tx_d.eval_last |= 1 << TCP_SCHED_RING_ST;
        data.u.read_rx2tx_d.snd_wnd = htonl(tcpcb_pd->tcpcb->snd_wnd);
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
        data.u.read_rx2tx_d.rcv_nxt = htonl(tcpcb_pd->tcpcb->rcv_nxt);
        data.u.read_rx2tx_d.snd_una = htonl(tcpcb_pd->tcpcb->snd_una);
        data.u.read_rx2tx_d.pending_ack_send = tcpcb_pd->tcpcb->pending_ack_send;
        data.u.read_rx2tx_d.state = (uint8_t)tcpcb_pd->tcpcb->state;
        // TODO : fix this hardcoding
        data.u.read_rx2tx_d.rto = htons(100);
        HAL_TRACE_DEBUG("TCPCB rx2tx snd_wnd: 0x{0:x}", data.u.read_rx2tx_d.snd_wnd);
        HAL_TRACE_DEBUG("TCPCB rx2tx debug_dol_tx: 0x{0:x}", data.u.read_rx2tx_d.debug_dol_tx);
        HAL_TRACE_DEBUG("TCPCB rx2tx shared pending_ack_send: 0x{0:x}", 
                    data.u.read_rx2tx_d.pending_ack_send);

        // get sesq address
        wring_hw_id_t   sesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &sesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive sesq base for tcp cb: {}", 
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Sesq id: 0x{0:x}", tcpcb_pd->tcpcb->cb_id);
            HAL_TRACE_DEBUG("Sesq base: 0x{0:x}", sesq_base);
            data.u.read_rx2tx_d.sesq_base = sesq_base;
        }
        // get asesq address
        wring_hw_id_t   asesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_ASESQ,
                                     tcpcb_pd->tcpcb->cb_id,
                                     &asesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive asesq base for tcp cb: {}", 
                        tcpcb_pd->tcpcb->cb_id);
        } else {
            HAL_TRACE_DEBUG("ASesq id: 0x{0:x}", tcpcb_pd->tcpcb->cb_id);
            HAL_TRACE_DEBUG("ASesq base: 0x{0:x}", asesq_base);
            data.u.read_rx2tx_d.asesq_base = asesq_base;
        }

    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
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
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);
    
    if(!del) {
        data.u.read_rx2tx_extra_d.rcv_mss = htons((uint16_t)tcpcb_pd->tcpcb->rcv_mss);

    }
    HAL_TRACE_DEBUG("TCPCB rx2tx shared rcv_mss: 0x{0:x}", data.u.read_rx2tx_extra_d.rcv_mss);

    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
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
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_tx_cc_and_fra_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s4_t0_tcp_tx_cc_and_fra_d   data = {0};
    hal_ret_t                   ret = HAL_RET_OK;
    
    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_CC_AND_FRA);

    if(!del) {
        //data.u.tcp_fra_d.ca_state = 0x2;
        //data.u.tcp_fra_d.high_seq = 0xEFEFEFEF;
        data.snd_cwnd = htons((uint16_t)tcpcb_pd->tcpcb->snd_cwnd);
        data.snd_cwnd_cnt = htons((uint16_t)tcpcb_pd->tcpcb->snd_cwnd_cnt);
        data.snd_cwnd_clamp = htons(0x7fff);
        data.max_packets_out = 0x07;
        data.last_max_cwnd = 0x16;
        HAL_TRACE_DEBUG("TCPCB snd_cwnd: 0x{0:x}", data.snd_cwnd);
        HAL_TRACE_DEBUG("TCPCB snd_cwnd_cnt: 0x{0:x}", data.snd_cwnd_cnt);
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_fra entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}



hal_ret_t 
p4pd_add_or_del_tcp_tx_xmit_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s5_t0_tcp_tx_xmit_d             data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_XMIT);
    
    if(!del) {
        data.snd_nxt = htonl(tcpcb_pd->tcpcb->snd_nxt);
        data.is_cwnd_limited = 0x00;
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_tcp_tx_tso_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    s6_t0_tcp_tx_tso_d                  data = {0};
    hal_ret_t                           ret = HAL_RET_OK;

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);
    
    if(!del) {
        data.source_lif = htons(tcpcb_pd->tcpcb->source_lif);
        data.source_port = htons(tcpcb_pd->tcpcb->source_port);
        data.dest_port = htons(tcpcb_pd->tcpcb->dest_port);
        if (tcpcb_pd->tcpcb->header_len != INVALID_HEADER_TEMPLATE_LEN) {
            data.header_len = (uint8_t)tcpcb_pd->tcpcb->header_len;
        } else {
            HAL_TRACE_DEBUG("Skipping header template len update");
        }
        HAL_TRACE_DEBUG("TCPCB source lif: 0x{0:x}", data.source_lif);
        HAL_TRACE_DEBUG("TCPCB source port: 0x{0:x} dest port 0x{1:x}",
            data.source_port, data.dest_port);
        HAL_TRACE_DEBUG("TCPCB header len: {}", data.header_len);
    }
    
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
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
        HAL_TRACE_DEBUG("TCPCB header template addr 0x{0:x}", hwid);
        HAL_TRACE_DEBUG("TCPCB header template={}", hex_dump((uint8_t*)data, sizeof(data)));
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
        HAL_TRACE_ERR("Failed to create tx: read_rx2tx entry for TCP CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}
hal_ret_t 
p4pd_add_or_del_tcpcb_txdma_entry(pd_tcpcb_t* tcpcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

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
    ret = p4pd_add_or_del_tcp_tx_cc_and_fra_entry(tcpcb_pd, del);
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

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    tcpcb_pd->tcpcb->sesq_base = data.u.read_rx2tx_d.sesq_base;
    tcpcb_pd->tcpcb->sesq_pi = data.u.read_rx2tx_d.pi_0;
    tcpcb_pd->tcpcb->sesq_ci = data.u.read_rx2tx_d.ci_0;

    tcpcb_pd->tcpcb->asesq_base = data.u.read_rx2tx_d.asesq_base;
    tcpcb_pd->tcpcb->asesq_pi = data.u.read_rx2tx_d.pi_4;
    tcpcb_pd->tcpcb->asesq_ci = data.u.read_rx2tx_d.ci_4;

    tcpcb_pd->tcpcb->pending_ack_send = data.u.read_rx2tx_d.pending_ack_send;

    HAL_TRACE_DEBUG("Received sesq_base: 0x{0:x}", tcpcb_pd->tcpcb->sesq_base);
    HAL_TRACE_DEBUG("Received sesq_pi: 0x{0:x}", tcpcb_pd->tcpcb->sesq_pi);
    HAL_TRACE_DEBUG("Received sesq_ci: 0x{0:x}", tcpcb_pd->tcpcb->sesq_ci);
    HAL_TRACE_DEBUG("Received asesq_base: 0x{0:x}", tcpcb_pd->tcpcb->asesq_base);
    HAL_TRACE_DEBUG("Received asesq_pi: 0x{0:x}", tcpcb_pd->tcpcb->asesq_pi);
    HAL_TRACE_DEBUG("Received asesq_ci: 0x{0:x}", tcpcb_pd->tcpcb->asesq_ci);
    HAL_TRACE_DEBUG("TCPCB rx2tx shared pending_ack_send: 0x{0:x}", 
                    tcpcb_pd->tcpcb->pending_ack_send);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tcp_tx_read_rx2tx_extra_entry(pd_tcpcb_t* tcpcb_pd)
{
    s1_t0_tcp_tx_d              data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id +
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_READ_RX2TX_EXTRA);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->rcv_mss = ntohs(data.u.read_rx2tx_extra_d.rcv_mss);

    HAL_TRACE_DEBUG("TCPCB rx2tx shared rcv_mss: 0x{0:x}", tcpcb_pd->tcpcb->rcv_mss);
    return HAL_RET_OK;

}

hal_ret_t
p4pd_get_tcp_tx_tcp_retx_entry(pd_tcpcb_t* tcpcb_pd)
{
    s3_t0_tcp_tx_retx_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_RETX);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: tcp_cc entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }
    // The following are used for DOL tests only
    tcpcb_pd->tcpcb->retx_xmit_cursor = ntohll(data.retx_xmit_cursor);
    tcpcb_pd->tcpcb->retx_snd_una = ntohl(data.retx_snd_una);
    
    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_tcp_tx_cc_and_fra_entry(pd_tcpcb_t* tcpcb_pd)
{
    s4_t0_tcp_tx_cc_and_fra_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_CC_AND_FRA);
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->snd_cwnd = ntohs(data.snd_cwnd);
    tcpcb_pd->tcpcb->snd_cwnd_cnt = ntohs(data.snd_cwnd_cnt);
    HAL_TRACE_DEBUG("TCPCB snd_cwnd: 0x{0:x}", tcpcb_pd->tcpcb->snd_cwnd);
    HAL_TRACE_DEBUG("TCPCB snd_cwnd_cnt: 0x{0:x}", tcpcb_pd->tcpcb->snd_cwnd_cnt);

    return HAL_RET_OK;

}

hal_ret_t 
p4pd_get_tcp_tx_xmit_entry(pd_tcpcb_t* tcpcb_pd)
{
    s5_t0_tcp_tx_xmit_d data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_XMIT);
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;

}

hal_ret_t 
p4pd_get_tcp_tx_tso_entry(pd_tcpcb_t* tcpcb_pd)
{
    s6_t0_tcp_tx_tso_d              data = {0};

    // hardware index for this entry
    tcpcb_hw_id_t hwid = tcpcb_pd->hw_id + 
        (P4PD_TCPCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TCP_TX_TCP_TSO);
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: read_rx2tx entry for TCP CB");
        return HAL_RET_HW_FAIL;
    }

    tcpcb_pd->tcpcb->source_lif  =  ntohs(data.source_lif);
    tcpcb_pd->tcpcb->source_port = ntohs(data.source_port);
    tcpcb_pd->tcpcb->dest_port = ntohs(data.dest_port);
    tcpcb_pd->tcpcb->header_len = data.header_len;
    HAL_TRACE_DEBUG("TCPCB source lif: 0x{0:x}", tcpcb_pd->tcpcb->source_lif);
    HAL_TRACE_DEBUG("TCPCB source port: 0x{0:x} dest port 0x{1:x}",
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

    if(!p4plus_hbm_read(hwid,  (uint8_t *)data, P4PD_TCPCB_STAGE_ENTRY_OFFSET)){
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

    ret = p4pd_get_tcp_tx_read_rx2tx_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_read_rx2tx_extra_entry(tcpcb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_get_tcp_tx_cc_and_fra_entry(tcpcb_pd);
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
pd_tcpcb_get_base_hw_index(pd_tcpcb_t* tcpcb_pd)
{
    HAL_ASSERT(NULL != tcpcb_pd);
    HAL_ASSERT(NULL != tcpcb_pd->tcpcb);
    
    // Get the base address of TCP CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_TCP_PROXY, 0,
                    tcpcb_pd->tcpcb->cb_id);
    HAL_TRACE_DEBUG("received offset 0x{0:x}", offset);
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

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_tcpcb_create (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
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
    tcpcb_pd->hw_id = pd_tcpcb_get_base_hw_index(tcpcb_pd);
    printf("Received hw-id: 0x%lx ", tcpcb_pd->hw_id);
    
    // program tcpcb
    ret = p4pd_add_or_del_tcpcb_entry(tcpcb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
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
pd_tcpcb_update (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    
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
pd_tcpcb_delete (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    
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
pd_tcpcb_get (pd_tcpcb_args_t *args)
{
    hal_ret_t               ret;
    pd_tcpcb_t              tcpcb_pd;

    HAL_TRACE_DEBUG("TCPCB pd get for id: {}", args->tcpcb->cb_id);

    // allocate PD tcpcb state
    tcpcb_pd_init(&tcpcb_pd);
    tcpcb_pd.tcpcb = args->tcpcb;
    
    // get hw-id for this TCPCB
    tcpcb_pd.hw_id = pd_tcpcb_get_base_hw_index(&tcpcb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", tcpcb_pd.hw_id);

    // get hw tcpcb entry
    ret = p4pd_get_tcpcb_entry(&tcpcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", tcpcb_pd.tcpcb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
