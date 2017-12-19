icmp_normalization:
lb_icmp_deprecated_msgs:
  add         r1, k.icmp_typeCode[15:8], r0
  seq         c2, k.l4_metadata_icmp_deprecated_msgs_drop, ACT_ALLOW
  b.c2        lb_icmp_redirect_msg
  seq         c2, k.l4_metadata_icmp_redirect_msg_drop, ACT_ALLOW
  // Tried using indexb here but it didn't reduce the instruction count
  // for the current values we are checking (4, 6, 15-39)
  seq         c3, r1, 4
  seq         c4, r1, 6
  sle         c5, 15, r1
  sle         c6, r1, 39
  setcf       c5, [c5 & c6]
  bcf         ![c3 | c4 | c5], lb_icmp_redirect_msg
  nop
  phvwr.e     p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr       p.capri_intrinsic_drop, 1

lb_icmp_redirect_msg:
  b.c2        lb_icmp_code_removal
  seq         c2, k.l4_metadata_icmp_invalid_code_action, \
                     NORMALIZATION_ACTION_ALLOW
  seq         c3, r1, 5
  b.!c3       lb_icmp_code_removal
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1


lb_icmp_code_removal:
  nop.c2.e
  seq         c3, r1, 0 // Echo Reply
  seq         c4, r1, 8 // Echo Req
  sne         c5, k.icmp_typeCode[7:0], 0
  andcf       c5, [c3 | c4]
  nop.!c5.e
  seq         c3, k.l4_metadata_icmp_invalid_code_action, \
                     NORMALIZATION_ACTION_DROP
  phvwr.c3.e  p.control_metadata_drop_reason[DROP_ICMP_NORMALIZATION], 1
  phvwr.c3    p.capri_intrinsic_drop, 1
  // Edit case is the default option, Make code as zero.
  phvwrmi.e   p.icmp_typeCode, 0x0, 0xff
  nop
