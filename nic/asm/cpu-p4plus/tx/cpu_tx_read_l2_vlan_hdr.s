#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_tx_read_l2_vlan_hdr_k k;
struct cpu_tx_read_l2_vlan_hdr_d d;

%%
    .align
cpu_tx_read_l2_vlan_hdr_start:
    CAPRI_CLEAR_TABLE_VALID(1)
    CAPRI_OPERAND_DEBUG(d.u.read_l2_vlan_hdr_d.etherType)
    CAPRI_OPERAND_DEBUG(d.u.read_l2_vlan_hdr_d.pcp)
    CAPRI_OPERAND_DEBUG(d.u.read_l2_vlan_hdr_d.dei)
    CAPRI_OPERAND_DEBUG(d.u.read_l2_vlan_hdr_d.vid)

    seq     c2, d.u.read_l2_vlan_hdr_d.etherType, 0x8100
    bcf     [!c2], cpu_tx_read_l2_vlan_hdr_done
    nop
    
    // copy values from existing .1Q header
    phvwr   p.vlan_hdr_entry_pcp, d.u.read_l2_vlan_hdr_d.pcp
    phvwr   p.vlan_hdr_entry_dei, d.u.read_l2_vlan_hdr_d.dei
    phvwr   p.to_s4_vlan_tag_exists, 1

cpu_tx_read_l2_vlan_hdr_done:
    nop.e
    nop
