header_type apulu_p4i_to_rxdma_header_t {
    fields {
        p4plus_app_id : 4;
        pad : 4;

        vnic_id : 8;
        vpc_id : 8;

        flow_src : 128;
        flow_dst : 128;
        flow_sport : 16;
        flow_dport : 16;
        flow_proto : 8;
    }
}

header_type apulu_txdma_to_p4e_header_t {
    fields {
        p4plus_app_id : 4;
        pad0 : 4;
    }
}

header_type apulu_ingress_recirc_header_t {
    fields {
        flow_ohash : 32;
        local_mapping_ohash : 32;
        pad1 : 6;
        flow_done : 1;
        local_mapping_done : 1;
    }
}
