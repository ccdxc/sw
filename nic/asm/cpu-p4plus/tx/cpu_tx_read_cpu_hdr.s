#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_cpu_hdr_k k;
struct cpu_tx_read_cpu_hdr_d d;

#define c_upd_vlan          c1
#define c_rem_vlan          c2
#define c_free_buf          c3
#define c_add_qs_trlr       c4
#define r_hdr_flags         r1
        
%%
    .param cpu_tx_write_pkt_start
    .align
cpu_tx_read_cpu_hdr_start:
    CAPRI_CLEAR_TABLE_VALID(0)

    phvwr   p.to_s4_src_lif, d.{u.read_cpu_hdr_d.src_lif}.hx
    phvwr   p.to_s4_tm_oq, d.u.read_cpu_hdr_d.tm_oq 

    // Check if vlan tag needs to be added
    add                 r_hdr_flags, r0, d.{u.read_cpu_hdr_d.flags}.hx
    smeqh               c_upd_vlan, r_hdr_flags, CPU_TO_P4PLUS_FLAGS_UPD_VLAN, CPU_TO_P4PLUS_FLAGS_UPD_VLAN
    smeqh.!c_upd_vlan   c_rem_vlan, r_hdr_flags, CPU_TO_P4PLUS_FLAGS_REM_VLAN, CPU_TO_P4PLUS_FLAGS_REM_VLAN
    phvwr.c_rem_vlan    p.common_phv_rem_vlan_tag, 1

    phvwr.c_upd_vlan    p.common_phv_write_vlan_tag, 1
    phvwri.c_upd_vlan   p.vlan_hdr_entry_etherType, 0x8100 
    phvwr.c_upd_vlan    p.vlan_hdr_entry_vid, d.{u.read_cpu_hdr_d.hw_vlan_id}.hx

    smeqh               c_add_qs_trlr, r_hdr_flags, CPU_TO_P4PLUS_FLAGS_ADD_TX_QS_TRLR, CPU_TO_P4PLUS_FLAGS_ADD_TX_QS_TRLR
    phvwr.c_add_qs_trlr p.common_phv_add_qs_trlr, 1

    smeqh               c_free_buf, r_hdr_flags, CPU_TO_P4PLUS_FLAGS_FREE_BUFFER, CPU_TO_P4PLUS_FLAGS_FREE_BUFFER
    phvwr.c_free_buf    p.common_phv_free_buffer, 1

    bcf                 [c_free_buf], cpu_tx_read_ascq_index
    nop
  
cpu_tx_read_cpu_hdr_done:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, cpu_tx_write_pkt_start)
    nop.e
    nop

cpu_tx_read_ascq_index:
    CAPRI_NEXT_TABLE_READ_e(0, TABLE_LOCK_DIS, cpu_tx_write_pkt_start, k.to_s3_ascq_sem_inf_addr, TABLE_SIZE_64_BITS)
    nop
