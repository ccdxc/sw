#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_cpu_hdr_k k;
struct cpu_tx_read_cpu_hdr_d d;

%%
    .param cpu_tx_read_l2_vlan_hdr_start
    .align
cpu_tx_read_cpu_hdr_start:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_OPERAND_DEBUG(d.{u.read_cpu_hdr_d.flags}.hx)
    CAPRI_OPERAND_DEBUG(d.{u.read_cpu_hdr_d.src_lif}.hx)
    CAPRI_OPERAND_DEBUG(d.{u.read_cpu_hdr_d.hw_vlan_id}.hx)
    CAPRI_OPERAND_DEBUG(d.{u.read_cpu_hdr_d.l2_offset}.hx)
    phvwr   p.to_s5_src_lif, d.{u.read_cpu_hdr_d.src_lif}.hx
    
    // Check if vlan tag needs to be added
    seq     c1, d.{u.read_cpu_hdr_d.flags}.hx, CPU_TO_P4PLUS_FLAGS_UPD_VLAN
    bcf     [!c1], cpu_tx_read_cpu_hdr_done
    phvwr.c1  p.common_phv_write_vlan_tag, 1

    phvwri.c1 p.vlan_hdr_entry_etherType, 0x8100 
    phvwr.c1  p.vlan_hdr_entry_vid, d.{u.read_cpu_hdr_d.hw_vlan_id}.hx

cpu_tx_read_cpu_hdr_done:
    add     r3, k.common_te0_phv_table_addr, d.{u.read_cpu_hdr_d.l2_offset}.hx
    CAPRI_NEXT_TABLE_READ(0,
                          TABLE_LOCK_DIS,
                          cpu_tx_read_l2_vlan_hdr_start,
                          r3,
                          TABLE_SIZE_32_BITS)
   
    nop.e
    nop
