sne     c1, k.valid_tcp, 0
bcf     [!c1], not_tcp  // bail out
//above 2 instructions not needed if done above.
sne     c2, k.valid_tcp_option_timestamp, 0 //delay
sne     c3, k.tcp_timestamp_negotiated, 0
setcf   c5, [!c3 & c2]
setcf   c4, [c3 & !c2]
slt     c6, k.tcp_ws, k.tcp_data_len
slt     c7, k.tcp_mss, k.tcp_data_len
bcf     [c4 | c5 | c6 | c7], bad_packet
nop

seq c1,d.flow_rtt_seq_check_enabled, 1
smneb   c2, k.tcp_flags, TCP_FLAGS_MASK_ACK, 0
slt c3,d.flow_rtt_seq_no, k.tcp_seqNo     // one from flow, one from packet
bcf [!c1 | !c2 | !c3] skip_rtt 
rsubi r2, k.intrinsic_timestamp, d.flow_rtt_timestamp
add r3,k.flow_rtt,r0, 1
add r4,r3,r2, 0
//write back rtt, new seqno, rtt timestamp to the table entry.
tblwr d.flow_rtt, r4
tblwr d.flow_rtt_seq_no, k.tcp_seqNo
tblwr d.flow_rtt_timestamp,k.intrinsic_timestamp
// if this is the end -put a .e to the above instruction

