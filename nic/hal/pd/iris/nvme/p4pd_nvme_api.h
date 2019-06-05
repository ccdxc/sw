//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __P4PD_NVME_API_H__
#define __P4PD_NVME_API_H__

#include <stdint.h>
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

#include "gen/p4gen/nvme_sess_pre_dgst_tx/include/nvme_sess_pre_dgst_tx_p4pd_internal.h"
#include "gen/p4gen/nvme_sess_pre_xts_tx/include/nvme_sess_pre_xts_tx_p4pd_internal.h"
#include "gen/p4gen/nvme_req_tx/include/nvme_req_tx_p4pd_internal.h"
#include "gen/p4gen/nvme_req_rx/include/nvme_req_rx_p4pd_internal.h"

#define nvme_nscb_t s2_t0_nvme_req_tx_nscb_process_bitfield_t
#define nvme_txsessprodcb_t s5_t0_nvme_req_tx_sessprodcb_process_bitfield_t
#define nvme_rxsessprodcb_t s5_t0_nvme_req_rx_sessprodcb_process_bitfield_t
#define nvme_resourcecb_t s4_t1_nvme_req_tx_resourcecb_process_bitfield_t
#define nvme_cmd_context_ring_entry_t s5_t1_nvme_req_tx_cmdid_fetch_process_bitfield_t
#define nvme_tx_pdu_context_ring_entry_t s5_t2_nvme_req_tx_pduid_fetch_process_bitfield_t
#define nvme_rx_pdu_context_ring_entry_t s5_t1_nvme_req_rx_pduid_fetch_process_bitfield_t
#define nvme_sessxtstxcb_t s3_t0_nvme_sessprexts_tx_cb_writeback_process_bitfield_t
#define nvme_sessdgsttxcb_t s4_t0_nvme_sesspredgst_tx_cb_writeback_process_bitfield_t
#define nvme_txhwxtscb_t s2_t1_nvme_sessprexts_tx_xtscb_process_bitfield_t
#define nvme_txhwdgstcb_t s3_t0_nvme_sesspredgst_tx_dgstcb_process_bitfield_t
#define nvme_sqcb_t s6_t0_nvme_req_tx_sqcb_writeback_process_bitfield_t
#define nvme_rqcb_t s6_t0_nvme_req_rx_rqcb_writeback_process_bitfield_t

#endif //__P4PD_NVME_API_H__
