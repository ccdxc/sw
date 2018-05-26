/*********************************************************
 * cpu-macros.h
 ********************************************************/
#ifndef __CPU_MACROS_H__
#define __CPU_MACROS_H__

#include "capri-macros.h"
#include "capri_common.h"

#define CPU_PIDX_SIZE                       16
#define CPU_PIDX_SHIFT                      4

#define CPU_ARQRX_TABLE_SIZE                1024
#define CPU_ARQRX_TABLE_SHIFT               10

#define NIC_ARQRX_ENTRY_SIZE                8
#define NIC_ARQRX_ENTRY_SIZE_SHIFT          3          /* for 8B */

#define CPU_ARQRX_QIDXR_OFFSET              64
#define ARQRX_QIDXR_DIR_ENTRY_SIZE_SHIFT    9          /* for 512B */
#define ARQ_SEM_IDX_ENTRY_SHIFT             3          /* for 8B */

#define CPU_VALID_BIT_SHIFT                 63

#define CPU_ASQ_TABLE_SIZE                  1024
#define CPU_ASQ_TABLE_SHIFT                 10
#define CPU_ASQ_ENTRY_SIZE                  8
#define CPU_ASQ_ENTRY_SIZE_SHIFT            3          /* for 8B */

#define CPU_PHV_RING_ENTRY_DESC_ADDR_START CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
#define CPU_PHV_RING_ENTRY_DESC_ADDR_END CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)

# define CPU_RX_ARQ_BASE_FOR_ID(_dest_r, \
                                _arqrx_base, \
                                _k_cpu_id) \
    add    _dest_r, r0, _k_cpu_id; \
    sll    _dest_r, _dest_r, CPU_ARQRX_TABLE_SHIFT; \
    add    _dest_r, _dest_r, _arqrx_base


#define CPU_RX_ENQUEUE(_dest_r, \
                       _descr_addr, \
                       _arqrx_pindex, \
                       _arqrx_base, \
                       _phv_desc_field_name, \
                       _dma_cmd_prefix, \
                       _eop_, \
                       _wr_fence_, \
                       _debug_dol_cr) \
 	add     _dest_r, r0, _arqrx_pindex; \
    andi    _dest_r, _dest_r, ((1 << CPU_ARQRX_TABLE_SHIFT) - 1); \
	sll     _dest_r, _dest_r, NIC_ARQRX_ENTRY_SIZE_SHIFT; \
	add     _dest_r, _dest_r, _arqrx_base; \
	phvwri  p.##_dma_cmd_prefix##_type, CAPRI_DMA_COMMAND_PHV_TO_MEM; \
	phvwr   p.##_dma_cmd_prefix##_addr, _dest_r; \
    add     _dest_r, r0, r0; \
    add.!_debug_dol_cr  _dest_r, r0, 0x1; \
    sll.!_debug_dol_cr  _dest_r, _dest_r, CPU_VALID_BIT_SHIFT; \
    add     _dest_r, _dest_r, _descr_addr; \
	phvwr   p._phv_desc_field_name, _dest_r; \
	phvwri  p.##_dma_cmd_prefix##_phv_start_addr, CAPRI_PHV_START_OFFSET(_phv_desc_field_name); \
	phvwri  p.##_dma_cmd_prefix##_phv_end_addr, CAPRI_PHV_END_OFFSET(_phv_desc_field_name); \
    phvwri  p.##_dma_cmd_prefix##_eop, _eop_; \
    phvwri  p.##_dma_cmd_prefix##_wr_fence, _wr_fence_ 
   

#define CPU_ARQ_PIDX_READ_INC(_dest_r, _k_cpu_id, _d_struct, _pi0_field_name, _temp1_r, _temp2_r) \
    add     _temp1_r, r0, _k_cpu_id; \
    sll     _temp1_r, _temp1_r, CPU_PIDX_SHIFT; \
    tblrdp  _dest_r, _temp1_r, offsetof(_d_struct, _pi0_field_name), CPU_PIDX_SIZE; \
    addi    _temp2_r, _dest_r, 1; \
    tblwrp.f _temp1_r, offsetof(_d_struct, _pi0_field_name), CPU_PIDX_SIZE, _temp2_r 

#define CPU_ARQRX_QIDX_ADDR(_dir, _dest_r, _arqrx_qidxr_base_reg) \
    addi   _dest_r, r0, _dir; \
    sll    _dest_r, _dest_r, ARQRX_QIDXR_DIR_ENTRY_SIZE_SHIFT; \
    add    _dest_r, _dest_r, _arqrx_qidxr_base_reg

#define CPU_ARQ_SEM_IDX_INC_ADDR(_dir_s, _k_cpu_id, _dest_r) \
    addi   _dest_r, r0, CAPRI_SEM_ARQ_##_dir_s##_0_IDX_ADDR; \
    add    _dest_r, _dest_r, _k_cpu_id, ARQ_SEM_IDX_ENTRY_SHIFT; \
    addi   _dest_r, _dest_r, CAPRI_SEM_INC_OFFSET;

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
