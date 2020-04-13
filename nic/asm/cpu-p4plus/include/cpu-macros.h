/*********************************************************
 * cpu-macros.h
 ********************************************************/
#ifndef __CPU_MACROS_H__
#define __CPU_MACROS_H__

#include "capri-macros.h"
#include "asic/cmn/asic_common.hpp"

#define CPU_PIDX_SIZE                       16
#define CPU_PIDX_SHIFT                      4

#define CPU_ARQ_TABLE_SIZE                  4096
#define CPU_ARQ_TABLE_SHIFT                 12

#define NIC_ARQ_ENTRY_SIZE                  8
#define NIC_ARQ_ENTRY_SIZE_SHIFT            3          /* for 8B */

#define ARQ_SEM_ENTRY_SHIFT                 3          /* for 8B */

#define CPU_VALID_BIT_SHIFT                 63

#define CPU_ASQ_TABLE_SIZE                  4096
#define CPU_ASQ_TABLE_SHIFT                 12
#define CPU_ASQ_ENTRY_SIZE                  8
#define CPU_ASQ_ENTRY_SIZE_SHIFT            3          /* for 8B */

#define CPU_ASCQ_TABLE_SIZE                 4096
#define CPU_ASCQ_TABLE_SHIFT                12
#define CPU_ASCQ_ENTRY_SIZE                 8
#define CPU_ASCQ_ENTRY_SIZE_SHIFT           3

#define CPU_CB_RX_TOTAL_OFFSET              16
#define CPU_CB_RX_ARQ_FULL_OFFSET           24
#define CPU_RX_CB_SEM_FULL_OFFSET           32
#define CPU_CB_RX_QUEUE0_OFFSET             40
#define CPU_CB_RX_QUEUE1_OFFSET             48
#define CPU_CB_RX_QUEUE2_OFFSET             56


#define CPU_PHV_RING_ENTRY_DESC_ADDR_START CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
#define CPU_PHV_RING_ENTRY_DESC_ADDR_END CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)

# define CPU_RX_ARQ_BASE_FOR_ID(_dest_r, \
                                _arqrx_base, \
                                _k_cpu_id) \
    add    _dest_r, r0, _k_cpu_id; \
    sll    _dest_r, _dest_r, CPU_ARQ_TABLE_SHIFT; \
    sll    _dest_r, _dest_r, NIC_ARQ_ENTRY_SIZE_SHIFT; \
    add    _dest_r, _dest_r, _arqrx_base

#define CPU_ARQ_SEM_INF_ADDR(_k_cpu_id, _dest_r) \
    addi   _dest_r, r0, ASIC_SEM_ARQ_0_ADDR;                \
    add    _dest_r, _dest_r, _k_cpu_id, ARQ_SEM_ENTRY_SHIFT; \
    addi   _dest_r, _dest_r, ASIC_SEM_INF_OFFSET;

#define CPU_RX_ENQUEUE(_dest_r, \
                       _descr_addr, \
                       _arq_pindex, \
                       _arq_base, \
                       _phv_desc_sfield_name, \
                       _phv_desc_efield_name, \
                       _dma_cmd_prefix, \
                       _eop_, \
                       _wr_fence_, \
                       _debug_dol_cr) \
    andi    _dest_r, _arq_pindex, ((1 << CPU_ARQ_TABLE_SHIFT) - 1);     \
    add     _dest_r, _arq_base, _dest_r, NIC_ARQ_ENTRY_SIZE_SHIFT;      \
    phvwri  p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;   \
    phvwr   p.##_dma_cmd_prefix##_addr, _dest_r;                          \
    andi    _dest_r, _arq_pindex, (1 << CPU_ARQ_TABLE_SHIFT);             \
    add     _dest_r, _descr_addr, _dest_r, (CPU_VALID_BIT_SHIFT - CPU_ARQ_TABLE_SHIFT); \
    phvwr   p._phv_desc_efield_name, _dest_r;                            \
    phvwri  p.##_dma_cmd_prefix##_phv_start_addr, CAPRI_PHV_START_OFFSET(_phv_desc_sfield_name); \
    phvwri  p.##_dma_cmd_prefix##_phv_end_addr, CAPRI_PHV_END_OFFSET(_phv_desc_efield_name); \
    phvwri  p.##_dma_cmd_prefix##_eop, _eop_;                           \
    phvwri  p.##_dma_cmd_prefix##_wr_fence, _wr_fence_

#define CPU_TX_ASCQ_ENQUEUE(_dest_r,                \
                            _descr_addr,            \
                            _ascq_pindex,           \
                            _ascq_base,             \
                            _phv_desc_field_name,   \
                            _dma_cmd_prefix,        \
                            _eop_,                  \
                            _wr_fence_)             \
    and     _dest_r, _ascq_pindex, ((1 << CPU_ASCQ_TABLE_SHIFT) - 1);   \
    add     _dest_r, _ascq_base, _dest_r, CPU_ASCQ_ENTRY_SIZE_SHIFT;    \
    phvwri  p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PHV_TO_MEM;   \
    phvwr   p.##_dma_cmd_prefix##_addr, _dest_r;                        \
    and     _dest_r, _ascq_pindex, (1 << CPU_ASCQ_TABLE_SHIFT);         \
    add     _dest_r, _descr_addr, _dest_r, (CPU_VALID_BIT_SHIFT - CPU_ASCQ_TABLE_SHIFT); \
    phvwr   p._phv_desc_field_name, _dest_r;                            \
    phvwri  p.##_dma_cmd_prefix##_phv_start_addr, CAPRI_PHV_START_OFFSET(_phv_desc_field_name); \
    phvwri  p.##_dma_cmd_prefix##_phv_end_addr, CAPRI_PHV_END_OFFSET(_phv_desc_field_name);      \
    phvwri  p.##_dma_cmd_prefix##_cache, 1;                             \
    phvwrpair p.##_dma_cmd_prefix##_wr_fence, _wr_fence_, p.##_dma_cmd_prefix##_eop, _eop_;


/*
 * Set up HW toeplitz hash key and data
 */
#define TOEPLITZ_KEY_DATA_SETUP(_hash_key_prefix, _app_type, _app_header, \
                                _r_pkt_type, _r_hash_key, _brend_label) \
    addi        _r_hash_key, r0, _hash_key_prefix; \
    addui       _r_hash_key, _r_hash_key, _hash_key_prefix; \
    phvwrpair   p.toeplitz_key0_data[127:64], _r_hash_key, \
                p.toeplitz_key0_data[63:0],   _r_hash_key; \
    phvwrpair   p.toeplitz_key1_data[127:64], _r_hash_key, \
                p.toeplitz_key1_data[63:0],   _r_hash_key; \
    phvwrpair   p.toeplitz_key2_data[127:64], _r_hash_key, \
                p.toeplitz_key2_data[3:0], _app_type; \
    add         _r_pkt_type, r0, k.##_app_header##_packet_type; \
    .brbegin; \
    br          _r_pkt_type[1:0]; \
    nop; \
    .brcase CPU_PACKET_TYPE_NONE; \
        b           _brend_label; \
        nop; \
    .brcase CPU_PACKET_TYPE_IPV4; \
        b           _brend_label; \
        phvwrpair   p.toeplitz_input0_data[127:64], k.##_app_header##_ip_sa_sbit32_ebit127[79:16], \
                    p.toeplitz_input0_data[63:24],  k.{##_app_header##_ip_proto...##_app_header##_l4_dport_sbit8_ebit15}; \
    .brcase CPU_PACKET_TYPE_IPV6; \
        phvwr       p.toeplitz_input0_data, k.{##_app_header##_ip_sa_sbit0_ebit15...##_app_header##_ip_sa_sbit32_ebit127}; \
        phvwrpair   p.toeplitz_input1_data[127:120], k.##_app_header##_ip_da1_sbit0_ebit7, \
                    p.toeplitz_input1_data[119:64],  k.##_app_header##_ip_da1_sbit8_ebit63; \
        phvwr       p.toeplitz_input1_data[63:0], k.##_app_header##_ip_da2; \
        b           _brend_label; \
        phvwrpair   p.toeplitz_input2_data, k.{##_app_header##_l4_sport...##_app_header##_l4_dport_sbit8_ebit15}, \
                    p.toeplitz_input2_data2, k.##_app_header##_ip_proto; \
    .brcase 3; \
        illegal; \
        nop; \
    .brend;


/*
 * Evaluate HW toeplitz hash result to a mask and max value
 * (without using mod)
 */
#define TOEPLITZ_HASH_CALC_ID(_r_hash_result, _hash_mask, _hash_max_val, _cf) \
    and         _r_hash_result, _r_hash_result.wx, _hash_mask; \
    sle         _cf, _r_hash_result, _hash_max_val; \
    add.!_cf    _r_hash_result, r0, r0;

#endif
