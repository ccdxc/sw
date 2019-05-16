#define LOCAL_VNIC_BY_VLAN_TABLE_SIZE                       4096        // 4K
#define LOCAL_VNIC_BY_SLOT_TABLE_SIZE                       2048        // 2K
#define LOCAL_VNIC_BY_SLOT_OTCAM_TABLE_SIZE                 512
#define KEY_MAPPING_TABLE_SIZE                              64
#define VNIC_IP_MAPPING_TABLE_SIZE                          262144      // 256K
#define VNIC_IP_MAPPING_OHASH_TABLE_SIZE                    32768       // 32K
#define REMOTE_VNIC_MAPPINGS_RX_TABLE_SIZE                  2097152     // 2M
#define REMOTE_VNIC_MAPPINGS_RX_OHASH_TABLE_SIZE            262144      // 256K
#define TEP_TABLE_SIZE                                      1024        // 1K
#define FLOW_TABLE_SIZE                                     8388608     // 8M
#define FLOW_OHASH_TABLE_SIZE                               4194304     // 4M
#define SESSION_TABLE_SIZE                                  8388608     // 8M
#define VNIC_STATS_TABLE_SIZE                               1024        // 1K
#define NACL_TABLE_SIZE                                     512
#define DROP_STATS_TABLE_SIZE                               64
#define APP_TABLE_SIZE                                      16

#define EGRESS_LOCAL_VNIC_INFO_TABLE_SIZE                   1024        // 1K
#define REMOTE_VNIC_MAPPING_TX_TABLE_SIZE                   2097152     // 2M
#define REMOTE_VNIC_MAPPING_TX_OHASH_TABLE_SIZE             262144      // 256K
#define MIRROR_SESSION_TABLE_SIZE                           256

// Resource pool tables
#define RESOURCE_POOL_CLASSIFIED_TABLE_SIZE                 1024        // 1K
#define RESOURCE_POOL_CLASSIFIED_OTCAM_TABLE_SIZE           128
#define AGG_POLICER_TABLE_SIZE                              1024        // 1K
#define CLASSIFIED_POLICER_TABLE_SIZE                       1024        // 1K

// Rewrite tables
#define NEXTHOP_GROUP_TABLE_SIZE                            1024        // 1K
#define NEXTHOP_TABLE_SIZE                                  4096        // 4K
#define NAT_TABLE_SIZE                                      131072      // 128K
