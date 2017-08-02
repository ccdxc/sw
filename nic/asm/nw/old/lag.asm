/******************************************************************************/
TABLE : lag
/******************************************************************************/
struct k {
  phv.standard_egress_spec : 10;
  phv.computed_hash : 16;
}

ACTION lag.lag_hit:
struct d {
  num_members : 4;
  mbr0 : 10;
  mbr1 : 10;
  mbr2 : 10;
  mbr3 : 10;
}
mod r1, k.computed_hash, d.num_members
br r1, 4, member0
nop

member0:
  phvwr.e phv.standard_egress_spec, d.mbr0
  nop
member1:
  phvwr.e phv.standard_egress_spec, d.mbr1
  nop
member2:
  phvwr.e phv.standard_egress_spec, d.mbr2
  nop
member3:
  phvwr.e phv.standard_egress_spec, d.mbr3
  nop
