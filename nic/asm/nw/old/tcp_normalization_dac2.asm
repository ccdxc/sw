
struct k_struct {
    ipv4_flags : 3;
    ipv4_hlen : 4;
    qos_metadata_dscp : 8;
    valid_vlan_tag : 1;
    ipv4_totalLen : 16;
    packet_length : 14;
    valid_tcp : 1;
    tcp_flags : 8;
    valid_tcp_option_mss : 1;
    valid_tcp_option_ws : 1;
    valid_tcp_option_timestamp : 1;
    tcp_timestamp_negotiated : 1;
    tcp_option_timestamp_prev_echo_ts : 32;
    tcp_urgent_ptr : 16;
    tcp_data_len : 16;
    tcp_mss : 16;
    tcp_ws : 16;
    tcp_res : 4;
    valid_icmp : 1;
    icmp_type : 8;
    control_metadata_flow_miss : 1;
};

#define IP_FLAGS_RSVD_MASK  0x4
#define IP_FLAGS_DF_MASK    0x2

#define TCP_FLAGS_MASK_URG  0x20
#define TCP_FLAGS_MASK_ACK  0x10
#define TCP_FLAGS_MASK_RST  0x04
#define TCP_FLAGS_MASK_SYN  0x02
#define TCP_FLAGS_MASK_FIN  0x01

struct k_struct k;

%%

start:
    // Reserved bit in ip header non-zero
    // (k.ipv4_flags & IP_FLAGS_RSVD_MASK != 0)
    //
    // DF bit in ip header non-zero
    // (k.ipv4_flags & IP_FLAGS_DF_MASK != 0)
    smneb   c1, k.ipv4_flags, IP_FLAGS_RSVD_MASK | IP_FLAGS_DF_MASK, 0

    // IPV4 Options packets
    // (k.ipv4_hlen > 20)
    sle     c2, 5, k.ipv4_hlen

    // Rewrite IP TOS based on config
    // (k.qos_metadata_dscp != 0)
    sne     c3, k.qos_metadata_dscp, 0

    // IP Header length to frame length validation
    // If there are options - need to check for validity of each of them and add - becomes a huge nested if - so cannot be supported with options.
    // doing it for outer only.
    // ((k.valid_vlan_tag) and (k.packet_length > (k.ipv4_totalLen + 18))) or (not(k.valid_vlan_tag)) and (k.packet_length > (k.ipv4_totalLen + 14))
    // i.e.
    // k.packet_length > (k.ipv4_totalLen + (k.valid_vlan_tag ? 18 : 14))
    sne     c4, k.valid_vlan_tag, 0
    add.c4  r1, k.ipv4_totalLen, 18
    add.!c4 r1, k.ipv4_totalLen, 14
    slt     c4, r1, k.packet_length
    bcf     [c1 | c2 | c3 | c4], examine_closer
    sne     c1, k.valid_tcp, 0                  // delay slot
    bcf     [!c1], not_tcp

    // MSS is present but SYN is not present
    // (k.tcp_flags & TCP_FLAGS_MASK_SYN == 0) and k.valid_tcp_option_mss))
    smneb   c1, k.tcp_flags, TCP_FLAGS_MASK_SYN, 0      // delay slot
    sne     c2, k.valid_tcp_option_mss, 0
    setcf   c7, [!c1 & c2]

    // WindowScale is present but SYN is not present
    // (k.tcp_flags & TCP_FLAGS_MASK_SYN == 0) and k.valid_tcp_option_ws))
    sne     c2, k.valid_tcp_option_ws, 0
    setcf   c6, [!c1 & c2]

    //timestamp option present, but not negotiated and this is not a SYN packet
    // Need to move this after flow table lookup
    // (k.tcp_timestamp_negotiated == FALSE) and k.valid_tcp_option_timestamp)) and (k.tcp_flags & TCP_FLAGS_MASK_SYN == 0)
    sne     c2, k.valid_tcp_option_timestamp, 0
    sne     c3, k.tcp_timestamp_negotiated, 0
    setcf   c5, [!c3 & c2 & !c1]

    // TCP timestamp negotiated but not present - no EDIT action
    // (k.tcp_timestamp_negotiated == TRUE) and (not(k.valid_tcp_option_timestamp)) )
    setcf   c4, [c3 & !c2]
    bcf     [c4 | c5 | c6 | c7], examine_closer

    // Timestamp option present, prev_echo_timestamp hash non-zero value, but ACK is not set
    // (k.tcp_flags & TCP_FLAGS_MASK_ACK == 0) and k.valid_tcp_option_timestamp) and (tcp_option_timestamp.prev_echo_ts != 0))
    smneb   c1, k.tcp_flags, TCP_FLAGS_MASK_ACK, 0              // delay slot
    sne     c3, k.tcp_option_timestamp_prev_echo_ts, 0
    setcf   c7, [!c1 & c2 & c3]

    // URG flag not set but UrgentPtr has non-zero value
    // (k.tcp_flags & TCP_FLAGS_MASK_URG  == 0) and (k.tcp_urgent_ptr != 0)
    smneb   c1, k.tcp_flags, TCP_FLAGS_MASK_URG, 0
    sne     c2, k.tcp_urgent_ptr, 0
    setcf   c6, [!c1 & c2]

    // URG flag set but UrgentPtr has zero value
    // ((k.tcp_flags & TCP_FLAGS_MASK_URG  != 0) and (k.tcp_urgent_ptr == 0)
    setcf   c5, [c1 & !c2]

    // URG flag set, UrgentPtr has non-zero value but no payload.
    // ((k.tcp_flags & TCP_FLAGS_MASK_URG  != 0) and (k.tcp_urgent_ptr != 0)) and (k.tcp_data_len == 0)
    sne     c3, k.tcp_data_len, 0
    setcf   c4, [c1 & c2 & !c3]
    bcf     [c4 | c5 | c6 | c7], examine_closer

    // RST set but there is a non-zero payload
    // This has to be handled by P4+
    // (k.tcp_flags & TCP_FLAGS_MASK_RST != 0) and (k.tcp_data_len != 0)
    smneb   c1, k.tcp_flags, TCP_FLAGS_MASK_RST, 0      // delay slot
    setcf   c7, [c1 & c3]

    // tcp segment length more than negotiated MSS
    // Need to move this after flow table lookup
    // (k.tcp_data_len > k.tcp_mss)
    slt     c6, k.tcp_mss, k.tcp_data_len

    //tcp segment length more than WS
    // Need to move this after flow table lookup
    // (k.tcp_data_len > k.tcp_ws)
    slt     c5, k.tcp_ws, k.tcp_data_len

    // Reserved bits in the TCP header are non-zero
    // (k.tcp_res != 0)
    sne     c4, k.tcp_res, 0
    bcf     [c7 | c6 | c5 | c4], examine_closer

    // Bad flags combination
    // (((k.tcp_flags & TCP_FLAGS_MASK_SYN != 0) and (k.tcp_flags & TCP_FLAGS_MASK_RST != 0))
    // ((k.tcp_flags & TCP_FLAGS_MASK_SYN != 0) and (k.tcp_flags & TCP_FLAGS_MASK_FIN != 0))
    // (k.tcp_flags == 0)
    // (k.tcp_flags == 0xff))
    // ((k.tcp_flags & TCP_FLAGS_MASK_SYN == 0) and (k.tcp_flags & TCP_FLAGS_MASK_ACK != 0)))
    smeqb   c7, k.tcp_flags, TCP_FLAGS_MASK_SYN | TCP_FLAGS_MASK_RST, TCP_FLAGS_MASK_SYN | TCP_FLAGS_MASK_RST
    smeqb   c6, k.tcp_flags, TCP_FLAGS_MASK_SYN | TCP_FLAGS_MASK_FIN, TCP_FLAGS_MASK_SYN | TCP_FLAGS_MASK_FIN
    smeqb   c5, k.tcp_flags, TCP_FLAGS_MASK_SYN | TCP_FLAGS_MASK_ACK, TCP_FLAGS_MASK_ACK
    seq     c4, k.tcp_flags, 0
    seq     c3, k.tcp_flags, 0xff
    bcf     [c7 | c6 | c5 | c4 | c3], examine_closer
    nop
    nop.e
    nop

not_tcp:
    sne     c1, k.valid_icmp, 0
    bcf     [!c1], not_icmp

    // ICMP Responses not allowed for flow miss packets as Request would have created flow entry.
    //   ((icmp.icmp_type == 0) or    //echo 
    //    (icmp.icmp_type == 14) or   //timestamp 
    //    (icmp.icmp_type == 16) or   // information(deprecated) 
    //    (icmp.icmp_type == 18) or   //address-mask(deprecated) 
    //    (icmp.icmp_type == 34) or   //ipv6 (deprecated) 
    //    (icmp.icmp_type == 36) or   //Mobile regn (deprecated) 
    //    (icmp.icmp_type == 4) or   // information(deprecated) 
    //    (icmp.icmp_type == 6) or   //address-mask(deprecated) 
    //    (icmp.icmp_type == 8) or   // ICMP code removal
    //    (icmp.icmp_type == 13) or   // ICMP code removal
    //    (icmp.icmp_type == 38)) and    //Domain name (deprecated) 
    //  (control_metadata.flow_miss == TRUE)))
    add     r1, r0, k.icmp_type         // branch delay
    indexb  r2, r1, [0, 14, 16, 18], 0
    indexb  r2, r1, [34, 36, 38, 4], 1
    indexb  r2, r1, [6, 8, 13], 2
    sne     c1, r2, -1
    seq     c2, k.control_metadata_flow_miss, 1
    setcf   c1, [c1 & c2]

    // ICMP bad request types to be dropped
    //    ( (icmp.icmp_type >= 15) and  // all types between 15 and 39 are deprecated.
    //      (icmp.icmp_type <= 39))))
    sle     c2, 15, k.icmp_type
    sle     c3, k.icmp_type, 39
    setcf   c2, [c2 & c3]
    bcf     [c1 | c2], examine_closer
    nop

not_icmp:
    nop.e
    nop

examine_closer:
    stop    0xdead
    nop
