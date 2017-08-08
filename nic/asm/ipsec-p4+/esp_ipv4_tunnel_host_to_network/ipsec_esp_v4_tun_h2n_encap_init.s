#include "../../assembly.h"
#include <../../rxdma-ipsec-phv.h>

// ipsec_tx_cb_t (table0's data)
struct d_struct {
    key_index : 32;
    iv_size   : 8;
    icv_size  : 8;
    spi       : 32;
    esn_lo    : 32;
    iv        : 64;
    esn_hi    : 32;
    barco_enc_cmd  : 32;
    ipsec_cb_index : 16;
    block_size     : 8;
};

struct k_struct {
    p42p4plus_hdr_payload_start : 16;
    p42p4plus_hdr_payload_end   : 16;
    intrinsic_frame_size        : 16;
    p42p4plus_hdr_l4_proto      : 8;
}; 

struct p_struct p;
struct k_struct k;
struct d_struct d;

%%

ipsec_esp_v4_tun_h2n_encap_init:
    addi r1, r0, ESP_FIXED_HDR_SIZE
    add  r1, r1, d.iv_size
    phvwr p.ipsec_int_headroom, r1
    phvwr p.ipsec_int_headroom_offset, k.p42p4plus_hdr_payload_start
    add  r2, k.p42p4plus_hdr_payload_end, r1
    phvwr p.ipsec_int_tailroom_offset, r2
    //payload_size = (phv->p42p4plus_hdr.payload_start - phv->p42p4plus_hdr.payload_end);
    sub r3,k.p42p4plus_hdr_payload_end, k.p42p4plus_hdr_payload_end, 0
    // payload_size += 2; 
    addi r3, r3, 2
    // phv->ipsec_int.pad_size = ((ipsec_cb->block_size - (payload_size % ipsec_cb->block_size))%ipsec_cb->block_size);
    // load block_size 
    add r4, r0, d.ipsec_int_block_size
    // payload_size % ipsec_cb->block_size
    and r5, r4, r3, 0
    // (ipsec_cb->block_size - (payload_size % ipsec_cb->block_size)
    sub r6, r4, r5
    phvwr p.ipsec_int_pad_size, r6
    
    phvwr p.ipsec_int_ipsec_cb_index, d.ipsec_cb_index
    
ipsec_esp_v4_tun_h2n_encap_init_prepare_stage1_tables:
    phvwri p.table0_valid, 1
    phvwri p.table0_addr, INDESC_SEMAPHORE_ADDR
    phvwri p.table0_pc, allocate_input_desc_semaphore
    phvwri p.table0_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table0_rsvd, 0

    phvwri p.table1_valid, 1
    phvwri p.table1_addr, OUTDESC_SEMAPHORE_ADDR
    phvwri p.table1_pc, allocate_output_desc_semaphore
    phvwri p.table1_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table1_rsvd, 0

    phvwri p.table2_valid, 1
    phvwri p.table2_addr, IN_PAGE_SEMAPHORE_ADDR
    phvwri p.table2_pc, allocate_input_page_semaphore
    phvwri p.table2_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table2_rsvd, 0

    phvwri p.table3_valid, 1
    phvwri p.table3_addr, OUT_PAGE_SEMAPHORE_ADDR
    phvwri p.table3_pc, allocate_output_page_semaphore
    phvwri p.table3_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table3_rsvd, 0
    

     
        
