/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type llc_header_t {
    fields {
        dsap : 8;
        ssap : 8;
        control_ : 8;
    }
}

header_type snap_header_t {
    fields {
        oui : 24;
        type_ : 16;
    }
}

header_type vlan_tag_t {
    fields {
        pcp : 3;
        dei : 1;
        vid : 12;
        etherType : 16;
    }
}

header_type mpls_t {
    fields {
        label : 20;
        exp : 3;
        bos : 1;
        ttl : 8;
    }
}

header_type ipv4_option_eol_t {
    fields {
        eol_value : 8;
    }
}

header_type ipv4_option_nop_t {
    fields {
        nop_value : 8;
    }
}

header_type ipv4_option_generic_t {
    fields {
        opt_val : 8;
        len  : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

// Enhancement to P4 to support variable len header whose len is specified
// using another header field.
// This is achieved using @pragma hdr_len on an instance of variable len header
// The following rules MUST be followed -
// 1. var_len header can only have 2 fields - hdr_len and data
// 2. First field name must be 'hdr_len'
// 3. length must be specified as an expression using hdr_len as variable
// 4. hdr_len pragma should be used on the header instance to specify another
//      another hdr field to replace 'hdr_len' in the length expression
// 5. Header that specifies the the len and the var len header MUST be extracted
//      in the SAME parse state (this req.t may be removed in future)

header_type ipv4_options_blob_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : hdr_len;
    max_length : 40;
}

header_type ipv6_t {
    fields {
        version : 4;
        trafficClass : 8;
        flowLabel : 20;
        payloadLen : 16;
        nextHdr : 8;
        hopLimit : 8;
        srcAddr : 128;
        dstAddr : 128;
    }
}

header_type ipv6_extn_generic_t {
    fields {
        nextHdr : 8;
        len  : 8;
        data : *;
    }
    length : (len << 3) + 8;
    max_length : 72;
}

// special header where hdr_len must be the first field
header_type ipv6_options_blob_t {
    fields {
        hdr_len : 16;
        data    : *;
    }
    length : hdr_len;
    max_length : 256;   // not used
}

header_type ipv6_extn_frag_t {
    fields {
        nextHdr : 8;
        rsvd  : 8;
        fragOffset : 12;
        res2 : 3;
        mf : 1;
        identification : 32;
    }
}

header_type icmp_t {
    fields {
        typeCode : 16;
        hdrChecksum : 16;
    }
}

header_type ah_t {
    fields {
        next_header : 8;
        payload_length : 8;
        reserved : 16;
        spi_hi : 16;
        spi_lo : 16;
        seqNo : 32;
    }
}

header_type esp_t {
    fields {
        spi_hi : 16;
        spi_lo : 16;
        seqNo  : 32;
    }
}

header_type icmp_echo_req_reply_t {
    fields {
        identifier : 16;
        seqNum : 16;
    }
}

header_type tcp_option_eol_t {
    fields {
        optType : 8;
    }
}

header_type tcp_option_nop_t {
    fields {
        optType : 8;
    }
}

header_type tcp_option_mss_t {
    fields {
        optType : 8;
        optLength : 8;
        value : 16;
    }
}

header_type tcp_option_ws_t {
    fields {
        optType : 8;
        optLength : 8;
        value : 8;
    }
}

header_type tcp_option_sack_perm_t {
    fields {
        optType: 8;
        optLength : 8;
    }
}

header_type tcp_option_timestamp_t {
    fields {
        optType : 8;
        optLength: 8;
        ts : 32;
        prev_echo_ts : 32;
    }
}

header_type tcp_option_one_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
    }
}

header_type tcp_option_two_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
    }
}

header_type tcp_option_three_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
        third_le : 32;
        third_re : 32;
    }
}

header_type tcp_option_four_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
        third_le : 32;
        third_re : 32;
        fourth_le : 32;
        fourth_re : 32;
    }
}

header_type tcp_option_unknown_t {
    fields {
        optType : 8;
        optLength : 8;
        data : *;
    }
    length : optLength;
    max_length : 40;
}

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 4;
        flags : 8;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}

header_type tcp_options_blob_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : hdr_len;
    max_length : 40;
}

header_type udp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        len : 16;
        checksum : 16;
    }
}

header_type udp_payload_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : (hdr_len);
    max_length : 65535;
}

header_type udp_opt_ocs_t {
    fields {
        kind :      8;
        chksum :    8;
    }
}
header_type udp_opt_timestamp_t {
    fields {
        kind : 8;
        len  : 8;
        ts_value : 32;
        ts_echo : 32;
    }
}

header_type udp_opt_mss_t {
    fields {
        kind :  8;
        len :  8;
        mss :  16;
    }
}
header_type udp_opt_nop_t {
    fields {
        kind : 8;
    }
}
header_type udp_opt_eol_t {
    fields {
        kind : 8;
    }
}

header_type udp_opt_unknown_t {
    fields {
        kind : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

header_type gre_t {
    fields {
        C : 1;
        R : 1;
        K : 1;
        S : 1;
        s : 1;
        recurse : 3;
        flags : 5;
        ver : 3;
        proto : 16;
    }
}

header_type nvgre_t {
    fields {
        tni : 24;
        flow_id : 8;
    }
}

header_type erspan_t3_t {
    fields {
        version : 4;
        vlan : 12;
        cos : 3;
        bso : 2;
        truncated : 1;
        span_id : 10;
        timestamp : 32;
        sgt : 16;
        pdu : 1;
        frame_type : 5;
        hw_id : 6;
        direction : 1;
        granularity : 2;
        options : 1;
    }
}

header_type erspan_t3_opt_t {
    fields {
        platf_id : 6;
        reserved : 12;
        port_id : 14;
        timestamp : 32;
    }
}

header_type arp_rarp_t {
    fields {
        hwType : 16;
        protoType : 16;
        hwAddrLen : 8;
        protoAddrLen : 8;
        opcode : 16;
    }
}

header_type arp_rarp_ipv4_t {
    fields {
        srcHwAddr : 48;
        srcProtoAddr : 32;
        dstHwAddr : 48;
        dstProtoAddr : 32;
    }
}

header_type vxlan_t {
    fields {
        flags : 8;
        reserved : 24;
        vni : 24;
        reserved2 : 8;
    }
}

header_type vxlan_gpe_t {
    fields {
        flags : 8;
        reserved : 16;
        next_proto : 8;
        vni : 24;
        reserved2 : 8;
    }
}

header_type genv_t {
    fields {
        ver : 2;
        optLen : 6;
        oam : 1;
        critical : 1;
        reserved : 6;
        protoType : 16;
        vni : 24;
        reserved2 : 8;
    }
}

header_type roce_bth_t {
    fields {
        opCode : 8;
        se : 1;
        m : 1;
        padCnt : 2;
        tVer : 4;
        pKey : 16;
        fr : 1;
        br : 1;
        reserved1 : 6;
        destQP : 24;
        ack : 1;
        reserved2 : 7;
        psn : 24;
    }
}

header_type roce_deth_t {
    fields {
        queueKey : 32;
        reserved : 8;
        srcQP : 24;
    }
}

header_type roce_deth_immdt_t {
    fields {
        queueKey : 32;
        reserved : 8;
        srcQP : 24;
        immDt : 32;
    }
}

header_type icrc_t {
    fields {
        value : 32;
    }
}

header_type roce_ipv4_t {
    fields {
        pad0 : 32;
        pad1 : 32;
        pad2 : 32;
        pad3 : 32;
        pad4 : 32;
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

header_type recirc_header_t {
    fields {
        src_tm_iport : 4;
        reason : 4;
        overflow_entry_index : 32;  // must be 32 bits, msbit=1 to perform recirc lookup
    }
}

/*
 * repl_type values defined in  p4/iris/include/defines.h
 */
header_type tm_replication_data_t {
    fields {
        qtype                  : 3;
        lport                  : 11;
        tunnel_rewrite_index   : 10;
        qid_or_vnid            : 24;
        repl_type              : 2;
        is_qid                 : 1;
        is_tunnel              : 1;
        rewrite_index          : 12;
    }
}
