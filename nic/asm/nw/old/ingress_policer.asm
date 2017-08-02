/******************************************************************************/
TABLE : ingress_policer
/******************************************************************************/
struct k {
  phv.ingress_policer_index : 10;
}

ACTION ingress_policer.execute_ingress_policer:
struct d {
  policer_color : 2;
}
phvwr.e phv.ingress_policer_color, d.policer_color
nop
