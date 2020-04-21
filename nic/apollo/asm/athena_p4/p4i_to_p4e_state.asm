#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_p4i_to_p4e_state_k.h"

struct p4i_to_p4e_state_k_  k;
struct phv_                 p;

%%

p4i_to_p4e_state:
    phvwr           p.control_metadata_update_checksum, k.p4i_to_p4e_header_update_checksum
    phvwr           p.control_metadata_direction, k.p4i_to_p4e_header_direction
    phvwr           p.control_metadata_flow_miss, k.p4i_to_p4e_header_flow_miss

    seq             c1, k.p4i_to_p4e_header_valid, TRUE
    seq.c1          c1, k.p4i_to_p4e_header_flow_miss, FALSE
    b.!c1           p4i_to_p4e_state_flow_miss
    
    seq.c1          c2, k.p4i_to_p4e_header_index_type, FLOW_CACHE_INDEX_TYPE_SESSION_INFO
    phvwr.c2        p.control_metadata_session_index_valid, TRUE
    phvwr.c2        p.control_metadata_session_index, k.p4i_to_p4e_header_index

    seq.c1          c2, k.p4i_to_p4e_header_index_type, FLOW_CACHE_INDEX_TYPE_CONNTRACK_INFO
    phvwr.c2        p.control_metadata_conntrack_index_valid, TRUE
    phvwr.c2        p.control_metadata_conntrack_index, k.p4i_to_p4e_header_index

    nop.e
    nop

p4i_to_p4e_state_flow_miss:
    /* Setup parts of P4+ redirect info on a flow miss due to key-maker constraints in stage-5 */
    seq             c1, k.udp_1_valid, TRUE
    phvwr.c1.e      p.p4e_to_p4plus_classic_nic_l4_sport, k.udp_1_srcPort
    phvwr.c1        p.p4e_to_p4plus_classic_nic_l4_dport, k.udp_1_dstPort

    seq             c1, k.tcp_valid, TRUE
    phvwr.c1.e      p.p4e_to_p4plus_classic_nic_l4_sport, k.tcp_srcPort
    phvwr.c1        p.p4e_to_p4plus_classic_nic_l4_dport, k.tcp_dstPort

    nop.e
    nop
