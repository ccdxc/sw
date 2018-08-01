#define LOCAL_VNIC_BY_VLAN_TABLE_SIZE                       4096
#define LOCAL_VNIC_BY_SLOT_TABLE_SIZE                       4096
#define LOCAL_VNIC_BY_SLOT_OTCAM_TABLE_SIZE                 512 
#define KEY_MAPPING_TABLE_SIZE                              64
#define VNIC_IP_MAPPING_TABLE_SIZE                          131072      // 128K
#define VNIC_IP_MAPPING_OHASH_TABLE_SIZE                    16384       // 16K
#define REMOTE_VNIC_MAPPINGS_RX_TABLE_SIZE                  1048576     // 1M
#define TEP_RX_TABLE_SIZE                                   1024
#define POLICY_TABLE_SIZE                                   16777216    // 16M
#define POLICY_OHASH_TABLE_SIZE                             2097152     // 2M
#define POLICY_INFO_TABLE_SIZE                              8388608     // 8M
#define VNIC_STATS_TABLE_SIZE                               1024
#define NACL_TABLE_SIZE                                     512

#define EGRESS_LOCAL_VNIC_INFO_TABLE_SIZE                   1024
#define REMOTE_VNIC_MAPPING_TX_TABLE_SIZE                   1048576     // 1M
#define REMOTE_VNIC_MAPPING_TX_OHASH_TABLE_SIZE             262144      // 256K
#define MIRROR_SESSION_TABLE_SIZE                           256
#define REWRITE_TABLE_SIZE                                  1024
#define TUNNEL_REWRITE_TABLE_SIZE                           1024

// Resource pool tables
#define RESOURCE_POOL_AGG_TABLE_SIZE                        1024
#define RESOURCE_POOL_CLASSIFIED_TABLE_SIZE                 1024
#define RESOURCE_POOL_CLASSIFIED_OTCAM_TABLE_SIZE           128
#define AGG_POLICER_TABLE_SIZE                              1024
#define CLASSIFIED_POLICER_TABLE_SIZE                       1024
