#include "egress.h"
#include "EGRESS_p.h"
#include "athena.h"
#include "EGRESS_p4i_to_p4e_state_k.h"

struct p4i_to_p4e_state_k_  k;
struct phv_                 p;

%%

p4i_to_p4e_state:
    phvwr           p.control_metadata_direction, k.p4i_to_p4e_header_direction
    phvwr           p.control_metadata_flow_miss, k.p4i_to_p4e_header_flow_miss

    seq             c1, k.p4i_to_p4e_header_valid, TRUE
    seq.c1          c1, k.p4i_to_p4e_header_flow_miss, FALSE
    
    seq.c1          c2, k.p4i_to_p4e_header_index_type, FLOW_CACHE_INDEX_TYPE_SESSION_INFO
    phvwr.c2        p.control_metadata_session_index_valid, TRUE
    phvwr.c2        p.control_metadata_session_index, k.p4i_to_p4e_header_index

    seq.c1          c2, k.p4i_to_p4e_header_index_type, FLOW_CACHE_INDEX_TYPE_CONNTRACK_INFO
    phvwr.c2        p.control_metadata_conntrack_index_valid, TRUE
    phvwr.c2        p.control_metadata_conntrack_index, k.p4i_to_p4e_header_index

    nop.e
    nop
