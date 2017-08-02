/******************************************************************************/
TABLE : decode_roce_opcode
/******************************************************************************/
struct k {
  phv.roce_bth_opCode : 8;
}

ACTION : decode_roce_opcode.decode_roce_opcode:
struct d {
  parse_order : 5;
}
phvwr.e phv.roce_parse_order, d.parse_order
nop
