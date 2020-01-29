#define KEY_MAPPING_TABLE_SIZE                              64

#define FLOW_TABLE_SIZE                                     4194304     // 4M
#define FLOW_OHASH_TABLE_SIZE                               1048576     // 4M/4 

#define DNAT_TABLE_SIZE                                     16384       // 16K
#define DNAT_OHASH_TABLE_SIZE                               4096        // 16K/4

//Dummy IPv4 table for compilation
#define IPV4_FLOW_TABLE_SIZE                                64
#define IPV4_FLOW_OHASH_TABLE_SIZE                          64

#define NACL_TABLE_SIZE                                     512

#define CHECKSUM_TABLE_SIZE                                 32

#define VNIC_TABLE_SIZE                                     1024
#define VLAN_VNIC_MAP_TABLE_SIZE                            4096
#define MPLS_LABEL_VNIC_MAP_TABLE_SIZE                      1048576

#define SESSION_TABLE_SIZE                                  4194304 // 4M

#define POLICER_PPS_SIZE                                    2048
#define POLICER_BW_SIZE                                     2048
