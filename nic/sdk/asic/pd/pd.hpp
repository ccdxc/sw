// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_PD_HPP__
#define __SDK_ASIC_PD_HPP__

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "asic/asic.hpp"
#include "include/sdk/qos.hpp"

namespace sdk {
namespace asic {
namespace pd {

// set this macro with max coses supported
#define ASIC_NUM_MAX_COSES   16
typedef enum pd_adjust_perf_index {
    PD_PERF_ID0 = 0,
    PD_PERF_ID1 = 1,
    PD_PERF_ID2 = 2,
    PD_PERF_ID3 = 3,
    PD_PERF_ID4 = 4
} pd_adjust_perf_index_t;

typedef enum pd_adjust_perf_type {
    PD_PERF_UP = 0,
    PD_PERF_DOWN = 1,
    PD_PERF_SET = 2
} pd_adjust_perf_type_t;

typedef enum pd_adjust_perf_status {
    PD_PERF_SUCCESS = 0,
    PD_PERF_FAILED = 1,
    PD_PERF_INVALID = 2
} pd_adjust_perf_status_t;

typedef struct llc_counters_s {
    uint32_t mask;
    uint32_t data[16];
} llc_counters_t;

typedef struct scheduler_stats_s {
    uint64_t doorbell_set_count;
    uint64_t doorbell_clear_count;
    uint32_t ratelimit_start_count;
    uint32_t ratelimit_stop_count;

    typedef struct txs_scheduler_cos_stats_s {
        uint32_t cos;
        bool xon_status;
        uint64_t doorbell_count;
    } txs_scheduler_cos_stats_t;

    // cos_count will be updated by the asic
    uint32_t num_coses;
    txs_scheduler_cos_stats_t cos_stats[ASIC_NUM_MAX_COSES];
} scheduler_stats_t;

typedef struct hbm_bw_samples_s {
    uint32_t      num_samples;
    uint32_t      sleep_interval;
    asic_hbm_bw_t *hbm_bw;
} __PACK__ hbm_bw_samples_t;

sdk_ret_t asicpd_read_table_constant(uint32_t tableid, uint64_t *value);
sdk_ret_t asicpd_program_table_thread_constant(uint32_t tableid,
                                               uint8_t table_thread_id,
                                               uint64_t const_value);

// this is mainly used during upgrade to modify the
// table engine configuration to map to new hbm
// and p4 program layouts.
typedef struct p4_tbl_eng_cfg_s {
    int tableid;
    int stage;
    int stage_tableid;
    uint64_t asm_err_offset;
    uint64_t asm_base;
    uint64_t mem_offset; // only for hbm tables
    uint32_t pc_offset;
    bool     pc_dyn;
} p4_tbl_eng_cfg_t;

typedef struct hw_fifo_prof_s {
    uint64_t        opaque;
    uint32_t        lg2size;
    uint32_t        lg2occ1;
    uint32_t        lg2occ2;
    uint32_t        lg2occ3;
    uint32_t        lg2occ4;
} hw_fifo_prof_t;

typedef struct hw_fifo_stats_s {
    uint32_t        head;
    uint32_t        tail;
    bool            empty;
    bool            full;
} hw_fifo_stats_t;

sdk_ret_t asicpd_program_table_constant(uint32_t tableid, uint64_t const_value);
sdk_ret_t asicpd_p4plus_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);
sdk_ret_t asicpd_table_mpu_base_init(p4pd_cfg_t *p4pd_cfg);
sdk_ret_t asicpd_program_table_mpu_pc(void);
sdk_ret_t asicpd_deparser_init(void);
sdk_ret_t asicpd_program_hbm_table_base_addr(bool hw_init = true);
uint8_t asicpd_get_action_pc(uint32_t tableid, uint8_t actionid);
uint8_t asicpd_get_action_id(uint32_t tableid, uint8_t actionpc);
int asicpd_table_entry_write(uint32_t tableid, uint32_t index,
                             uint8_t  *hwentry, uint16_t hwentry_bit_len,
                             uint8_t  *hwentry_mask);
int asicpd_table_entry_read(uint32_t tableid, uint32_t index,
                            uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_tcam_table_entry_write(uint32_t tableid, uint32_t index,
                                  uint8_t  *trit_x, uint8_t  *trit_y,
                                  uint16_t hwentry_bit_len);
int asicpd_tcam_table_entry_read(uint32_t tableid, uint32_t index,
                                 uint8_t  *trit_x, uint8_t  *trit_y,
                                 uint16_t *hwentry_bit_len);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t entry_size);
int asicpd_hbm_table_entry_write (uint32_t tableid, uint32_t index,
                                  uint8_t *hwentry, uint16_t *entry_size);
int asicpd_table_hw_entry_read(uint32_t tableid, uint32_t index,
                               uint8_t  *hwentry, uint16_t *hwentry_bit_len);
int asicpd_tcam_table_hw_entry_read(uint32_t tableid, uint32_t index,
                                    uint8_t  *trit_x, uint8_t  *trit_y,
                                    uint16_t *hwentry_bit_len);
void asicpd_copy_table_info(p4_table_mem_layout_t *out,
                            p4pd_table_mem_layout_t *in,
                            p4pd_table_properties_t *tbl_ctx);
int asicpd_hbm_table_entry_read(uint32_t tableid, uint32_t index,
                                uint8_t *hwentry, uint16_t *entry_size);
int asicpd_hbm_table_entry_write(uint32_t tableid, uint32_t index,
                                 uint8_t *hwentry, uint16_t entry_size);
sdk_ret_t asic_pd_scheduler_stats_get(scheduler_stats_t *sch_stats);
sdk_ret_t asic_pd_hbm_bw_get(hbm_bw_samples_t *hbm_bw_samples);
sdk_ret_t asic_pd_llc_setup(llc_counters_t *llc);
sdk_ret_t asic_pd_llc_get(llc_counters_t *llc);
sdk_ret_t asicpd_p4plus_recirc_init(void);
sdk_ret_t asicpd_qstate_map_clear(uint32_t lif_id);
sdk_ret_t asicpd_qstate_map_write(lif_qstate_t *qstate, uint8_t enable);
sdk_ret_t asicpd_qstate_map_read (lif_qstate_t *qstate);
sdk_ret_t asicpd_qstate_write(uint64_t addr, const uint8_t *buf,
                              uint32_t size);
sdk_ret_t asicpd_qstate_read(uint64_t addr, uint8_t *buf, uint32_t size);
sdk_ret_t asicpd_qstate_clear(lif_qstate_t *qstate);
void asicpd_reset_qstate_map(uint32_t lif_id);
sdk_ret_t asicpd_p4plus_invalidate_cache(mpartition_region_t *reg,
                                         uint64_t q_addr, uint32_t size);
uint32_t asic_pd_clock_freq_get(void);
pd_adjust_perf_status_t asic_pd_adjust_perf(int chip_id, int inst_id,
                                            pd_adjust_perf_index_t &idx,
                                            pd_adjust_perf_type_t perf_type);
void asic_pd_set_half_clock(int chip_id, int inst_id);
sdk_ret_t asic_pd_unravel_hbm_intrs(bool *iscattrip, bool *iseccerr,
                                    bool logging=false);
sdk_ret_t asicpd_toeplitz_init(const char *handle, uint32_t table_id,
                               uint32_t rss_indir_tbl_entry_size);
void asicpd_p4_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                                p4pd_table_cache_t cache);
bool asicpd_p4plus_invalidate_cache(uint64_t addr, uint32_t size_in_bytes,
                                    p4plus_cache_action_t action);

int asicpd_p4plus_table_init(p4plus_table_params_t *table_params);
int asicpd_p4plus_table_init(platform_type_t platform_type,
                             int stage_apphdr, int stage_tableid_apphdr,
                             int stage_apphdr_ext, int stage_tableid_apphdr_ext,
                             int stage_apphdr_off, int stage_tableid_apphdr_off,
                             int stage_apphdr_ext_off,
                             int stage_tableid_apphdr_ext_off,
                             int stage_txdma_act, int stage_tableid_txdma_act,
                             int stage_txdma_act_ext,
                             int stage_tableid_txdma_act_ext,
                             int stage_sxdma_act, int stage_tableid_sxdma_act);
int asicpd_p4plus_table_init(p4plus_prog_t *prog,
                             platform_type_t platform_type);

sdk_ret_t asicpd_tm_get_clock_tick(uint64_t *tick);
sdk_ret_t asicpd_tm_debug_stats_get(tm_port_t port,
                                    tm_debug_stats_t *debug_stats, bool reset);
sdk_ret_t asicpd_sw_phv_init(void);
sdk_ret_t asicpd_sw_phv_get(asic_swphv_type_t type, uint8_t prof_num,
                            asic_sw_phv_state_t *state);
sdk_ret_t asicpd_sw_phv_inject(asic_swphv_type_t type, uint8_t prof_num,
                               uint8_t start_idx, uint8_t num_flits,
                               void *data);

uint32_t asicpd_get_coreclk_freq(platform_type_t platform_type);
void asicpd_txs_timer_init_hsh_depth(uint32_t key_lines);

sdk_ret_t asicpd_init_hw_fifo(int fifo_num, uint64_t addr, int n,
                              hw_fifo_prof_t *prof);
sdk_ret_t asicpd_get_hw_fifo_info(int fifo_num, hw_fifo_stats_t *stats);
sdk_ret_t asicpd_set_hw_fifo_info(int fifo_num, hw_fifo_stats_t *stats);
inline bool asicpd_tm_q_valid(int32_t tm_q);

typedef struct queue_credit_s {
    uint32_t oq;
    uint32_t credit;
} queue_credit_t;

typedef struct port_queue_credit_s {
    uint32_t num_queues;
    queue_credit_t *queues;
} port_queue_credit_t;

typedef void (*queue_credits_get_cb_t)(uint32_t port_num,
                                       port_queue_credit_t *credit,
                                       void *ctxt);

sdk_ret_t queue_credits_get(queue_credits_get_cb_t cb, void *ctxt);

uint64_t asicpd_table_asm_base_addr_get(uint32_t tableid);
uint64_t asicpd_table_asm_err_offset_get(uint32_t tableid);

uint32_t asicpd_tbl_eng_cfg_get(p4pd_pipeline_t pipeline,
                                p4_tbl_eng_cfg_t *info,
                                uint32_t ninfos);
sdk_ret_t asicpd_tbl_eng_cfg_modify(p4pd_pipeline_t pipeline,
                                    p4_tbl_eng_cfg_t *cfg,
                                    uint32_t ncfgs);
sdk_ret_t asicpd_rss_tbl_eng_cfg_get(const char *handle, uint32_t tableid,
                                     p4_tbl_eng_cfg_t *rss);
void asicpd_rss_tbl_eng_cfg_modify(p4_tbl_eng_cfg_t *rss);

uint64_t asicpd_get_mem_base(void);
uint64_t asicpd_get_mem_offset(const char *reg_name);
uint64_t asicpd_get_mem_addr(const char *reg_name);
uint32_t asicpd_get_mem_size_kb(const char *reg_name);
mpartition_region_t *asicpd_get_mem_region(char *name);
mpartition_region_t *asicpd_get_hbm_region_by_address(uint64_t addr);

uint64_t asicpd_get_p4plus_table_mpu_pc(int tableid);
void asicpd_program_p4plus_table_mpu_pc(int tableid, int stage_tbl_id,
                                        int stage);
void asicpd_program_tbl_mpu_pc(int tableid, bool gress, int stage,
                               int stage_tableid, uint64_t table_asm_err_offset,
                               uint64_t table_asm_base);

void asicpd_set_action_asm_base(int tableid, int actionid, uint64_t asm_base);

void asicpd_set_action_rxdma_asm_base(int tableid, int actionid,
                                      uint64_t asm_base);
void asicpd_set_action_txdma_asm_base(int tableid, int actionid,
                                      uint64_t asm_base);
void asicpd_set_table_rxdma_asm_base(int tableid, uint64_t asm_base);
void asicpd_set_table_txdma_asm_base(int tableid, uint64_t asm_base);
sdk_ret_t asicpd_init(asic_cfg_t *cfg);
void asicpd_cleanup(void);
sdk_ret_t asicpd_pgm_init(void);
sdk_ret_t asicpd_p4plus_table_rw_init(void);
sdk_ret_t asicpd_state_pd_init(asic_cfg_t *cfg);
void asicpd_csr_init(void);


// Asic Doorbell address
uint64_t asicpd_local_dbaddr_get(void);
uint64_t asicpd_local_db32_addr_get(void);
uint64_t asicpd_host_dbaddr_get(void);

sdk_ret_t asicpd_tm_uplink_lif_set(tm_port_t port, uint32_t lif);
sdk_ret_t asicpd_tm_enable_disable_uplink_port(tm_port_t port, bool enable);
sdk_ret_t asicpd_tm_flush_uplink_port(tm_port_t port, bool enable);
sdk_ret_t asicpd_tm_write_control_uplink_port(tm_port_t port, bool enable);
sdk_ret_t asicpd_tm_drain_uplink_port(tm_port_t port);

sdk_ret_t asicpd_p4p_rxdma_mpu_trace_enable(uint32_t stage_id,
                                            uint32_t mpu,
                                            uint8_t enable,
                                            uint8_t trace_enable,
                                            uint8_t phv_debug,
                                            uint8_t phv_error,
                                            uint64_t watch_pc,
                                            uint64_t base_addr,
                                            uint8_t table_key,
                                            uint8_t instructions,
                                            uint8_t wrap,
                                            uint8_t reset,
                                            uint32_t buf_size);
sdk_ret_t asicpd_p4p_txdma_mpu_trace_enable(uint32_t stage_id,
                                            uint32_t mpu,
                                            uint8_t enable,
                                            uint8_t trace_enable,
                                            uint8_t phv_debug,
                                            uint8_t phv_error,
                                            uint64_t watch_pc,
                                            uint64_t base_addr,
                                            uint8_t table_key,
                                            uint8_t instructions,
                                            uint8_t wrap,
                                            uint8_t reset,
                                            uint32_t buf_size);
sdk_ret_t asicpd_p4_ingress_mpu_trace_enable(uint32_t stage_id,
                                             uint32_t mpu,
                                             uint8_t enable,
                                             uint8_t trace_enable,
                                             uint8_t phv_debug,
                                             uint8_t phv_error,
                                             uint64_t watch_pc,
                                             uint64_t base_addr,
                                             uint8_t table_key,
                                             uint8_t instructions,
                                             uint8_t wrap,
                                             uint8_t reset,
                                             uint32_t buf_size);
sdk_ret_t asicpd_p4_egress_mpu_trace_enable(uint32_t stage_id,
                                            uint32_t mpu,
                                            uint8_t enable,
                                            uint8_t trace_enable,
                                            uint8_t phv_debug,
                                            uint8_t phv_error,
                                            uint64_t watch_pc,
                                            uint64_t base_addr,
                                            uint8_t table_key,
                                            uint8_t instructions,
                                            uint8_t wrap, uint8_t reset,
                                            uint32_t buf_size);

sdk_ret_t asicpd_quiesce_start(void);
sdk_ret_t asicpd_quiesce_stop(void);
sdk_ret_t asicpd_pxb_cfg_lif_bdf(uint32_t lif, uint16_t bdf);

/* barco interfaces */
sdk_ret_t asicpd_barco_asym_ecc_point_mul(uint16_t key_size, uint8_t *p,
                                          uint8_t *n, uint8_t *xg, uint8_t *yg,
                                          uint8_t *a, uint8_t *b, uint8_t *x1,
                                          uint8_t *y1, uint8_t *k, uint8_t *x3,
                                          uint8_t *y3);
sdk_ret_t asicpd_barco_asym_ecdsa_p256_setup_priv_key(uint8_t *p, uint8_t *n,
                                                      uint8_t *xg, uint8_t *yg,
                                                      uint8_t *a, uint8_t *b,
                                                      uint8_t *da,
                                                      int32_t *key_idx);
sdk_ret_t asicpd_barco_asym_ecdsa_p256_sig_gen(int32_t key_idx, uint8_t *p,
                                               uint8_t *n, uint8_t *xg,
                                               uint8_t *yg, uint8_t *a,
                                               uint8_t *b, uint8_t *da,
                                               uint8_t *k, uint8_t *h,
                                               uint8_t *r, uint8_t *s,
                                               bool async_en,
                                               const uint8_t *unique_key);
sdk_ret_t asicpd_barco_asym_ecdsa_p256_sig_verify(uint8_t *p, uint8_t *n,
                                                  uint8_t *xg, uint8_t *yg,
                                                  uint8_t *a, uint8_t *b,
                                                  uint8_t *xq, uint8_t *yq,
                                                  uint8_t *r, uint8_t *s,
                                                  uint8_t *h, bool async_en,
                                                  const uint8_t *unique_key);

sdk_ret_t asicpd_barco_asym_rsa2k_encrypt(uint8_t *n, uint8_t *e,
                                          uint8_t *m,  uint8_t *c,
                                          bool async_en,
                                          const uint8_t *unique_key);
sdk_ret_t asicpd_barco_asym_rsa_encrypt(uint16_t key_size, uint8_t *n,
                                        uint8_t *e, uint8_t *m,  uint8_t *c,
                                        bool async_en,
                                        const uint8_t *unique_key);

sdk_ret_t asicpd_barco_asym_rsa2k_decrypt(uint8_t *n, uint8_t *d,
                                          uint8_t *c,  uint8_t *m);
sdk_ret_t asicpd_barco_asym_rsa2k_crt_decrypt(int32_t key_idx, uint8_t *p,
                                              uint8_t *q, uint8_t *dp,
                                              uint8_t *dq, uint8_t *qinv,
                                              uint8_t *c, uint8_t *m,
                                              bool async_en,
                                              const uint8_t *unique_key);

sdk_ret_t asicpd_barco_asym_rsa2k_setup_sig_gen_priv_key(uint8_t *n, uint8_t *d,
                                                         int32_t *key_idx);
sdk_ret_t asicpd_barco_asym_rsa2k_crt_setup_decrypt_priv_key(uint8_t *p,
                                                             uint8_t *q,
                                                             uint8_t *dp,
                                                             uint8_t *dq,
                                                             uint8_t *qinv,
                                                             int32_t* key_idx);
sdk_ret_t asicpd_barco_asym_rsa_setup_priv_key(uint16_t key_size, uint8_t *n,
                                               uint8_t *d, int32_t* key_idx);

sdk_ret_t asicpd_barco_asym_rsa2k_sig_gen(int32_t key_idx, uint8_t *n,
                                          uint8_t *d, uint8_t *h, uint8_t *s,
                                          bool async_en,
                                          const uint8_t *unique_key);
sdk_ret_t asicpd_barco_asym_rsa_sig_gen(uint16_t key_size, int32_t key_idx,
                                        uint8_t *n, uint8_t *d,
                                        uint8_t *h, uint8_t *s,
                                        bool async_en,
                                        const uint8_t *unique_key);
sdk_ret_t asicpd_barco_asym_fips_rsa_sig_gen(uint16_t key_size, int32_t key_idx,
                                             uint8_t *n, uint8_t *e,
                                             uint8_t *msg, uint16_t msg_len,
                                             uint8_t *s, uint8_t hash_type,
                                             uint8_t sig_scheme, bool async_en,
                                             const uint8_t *unique_key);

sdk_ret_t asicpd_barco_asym_fips_rsa_sig_verify(uint16_t key_size, uint8_t *n,
                                                uint8_t *e, uint8_t *msg,
                                                uint16_t msg_len, uint8_t *s,
                                                uint8_t hash_type,
                                                uint8_t sig_scheme,
                                                bool async_en,
                                                const uint8_t *unique_key);
sdk_ret_t asicpd_barco_asym_rsa2k_sig_verify(uint8_t *n, uint8_t *e,
                                             uint8_t *h, uint8_t *s);

sdk_ret_t asicpd_dpp_int_credit(uint32_t instance, uint32_t value);

sdk_ret_t asicpd_barco_asym_req_descr_get(uint32_t slot_index,
                                          void *asym_req_descr);
sdk_ret_t asicpd_barco_symm_req_descr_get(uint8_t ring_type,
                                          uint32_t slot_index,
                                          void *symm_req_descr);

sdk_ret_t asicpd_barco_ring_meta_get(uint8_t ring_type, uint32_t *pi,
                                      uint32_t *ci);
sdk_ret_t asicpd_barco_get_meta_config_info(uint8_t ring_type,
                                            void *meta);

sdk_ret_t asicpd_barco_asym_add_pend_req(uint32_t hw_id, uint32_t sw_id);
sdk_ret_t asicpd_barco_asym_poll_pend_req(uint32_t batch_size,
                                          uint32_t* id_count, uint32_t *ids);
sdk_ret_t asicpd_barco_sym_hash_process_request(uint8_t hash_type,
                                                bool generate,
                                                unsigned char *key,
                                                int key_len,
                                                unsigned char *data,
                                                int data_len,
                                                uint8_t *output_digest,
                                                int digest_len);

sdk_ret_t asicpd_barco_get_capabilities(uint8_t ring_type,
                                        bool *sw_reset_capable,
                                        bool *sw_enable_capable);

sdk_ret_t asicpd_table_rw_init(asic_cfg_t *cfg);

void asicpd_table_constant_write(uint64_t val, uint32_t stage,
                                 uint32_t stage_tableid, bool ingress);
sdk_ret_t asicpd_tm_set_span_threshold(uint32_t span_threshold);

}    // namespace pd
}    // namespace asic
}    // namespace sdk

using sdk::asic::pd::llc_counters_t;
using sdk::asic::pd::scheduler_stats_t;
using sdk::asic::pd::hbm_bw_samples_t;
using sdk::asic::pd::p4_tbl_eng_cfg_t;
using sdk::asic::pd::asicpd_qstate_map_read;
using sdk::asic::pd::asicpd_get_mem_addr;
using sdk::asic::pd::asicpd_get_mem_size_kb;
using sdk::asic::pd::asicpd_get_hbm_region_by_address;

#endif    // __SDK_ASIC_PD_HPP__
