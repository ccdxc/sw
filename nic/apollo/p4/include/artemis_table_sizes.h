#define KEY_MAPPING_TABLE_SIZE                              64
#define VNIC_MAPPING_TABLE_SIZE                             256
#define VNIC_INFO_TABLE_SIZE                                256
#define TEP1_RX_TABLE_SIZE                                  64
#define TEP2_RX_TABLE_SIZE                                  64
#define LOCAL_IP_MAPPING_HASH_TABLE_SIZE                    4096        // 4K
#define SERVICE_MAPPING_HASH_TABLE_SIZE                     4096        // 4K
#define LOCAL_IP_MAPPING_OTCAM_TABLE_SIZE                   1024        // 1K
#define SERVICE_MAPPING_OTCAM_TABLE_SIZE                    1024        // 1K
#define FLOW_TABLE_SIZE                                     8388608     // 8M
#define FLOW_OHASH_TABLE_SIZE                               4194304     // 4M
#define INTER_PIPE_TABLE_SIZE                               16
#define NACL_TABLE_SIZE                                     512

#define MIRROR_SESSION_TABLE_SIZE                           256
#define SESSION_TABLE_SIZE                                  8388608     // 8M
#define NAT_TABLE_SIZE                                      65536       // 64K
#define LOCAL_46_MAPPING_TABLE_SIZE                         256
#define NEXTHOP_TABLE_SIZE                                  1048576     // 1M
#define VNIC_STATS_TABLE_SIZE                               256
#define DROP_STATS_TABLE_SIZE                               64
#define METER_STATS_TABLE_SIZE                              65536       // 64K

//RXDMA Tables
// Single table with dir bit part of key along with VNIC id, so twice the size of 256
#define VNIC_INFO_RXDMA_TABLE_SIZE                          512

//TXDMA Tables
#define MAPPING_TABLE_SIZE                                  2097152     // 2M
#define MAPPING_OHASH_TABLE_SIZE                            262144      // 256K
#define REMOTE_46_MAPPING_TABLE_SIZE                        1024

