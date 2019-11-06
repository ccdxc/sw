control rxlpm1 {}
control rxlpm2 {}
control vnic_info_rxdma {}
control nat_rxdma {}
control sacl {}
control pkt_enqueue {}
control session_aging {}
control session2flow {}

#define p4_to_rxdma             app_header
#define lpm1_enable             table0_valid
#define lpm2_enable             table1_valid
#define vnic_info_en            table2_valid
#define apulu_p4plus            table3_valid
