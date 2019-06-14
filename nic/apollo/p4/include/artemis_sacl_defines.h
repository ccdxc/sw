#define SACL_SPORT_TABLE_SIZE             2112      // 64+(32*64)
#define SACL_SPORT_TREE_MAX_NODES         511       // (31 + 32 * 15)
#define SACL_SPORT_TREE_MAX_CLASSES       128

#define SACL_PROTO_DPORT_TABLE_SIZE       17472     // 64+(16*64)+(16*16*64)
#define SACL_PROTO_DPORT_TREE_MAX_NODES   2047      // 15 + (16 * 15) + (256 * 7)
#define SACL_PROTO_DPORT_TREE_MAX_CLASSES 256

#define SACL_P1_MAX_CLASSES               1024
#define SACL_P2_MAX_CLASSES               1024
#define SACL_P3_MAX_CLASSES               1024

#define SACL_P1_ENTRIES_PER_CACHE_LINE    51        // 51 entries of 10 bits
#define SACL_P2_ENTRIES_PER_CACHE_LINE    51        // 51 entries of 10 bits
#define SACL_P3_ENTRIES_PER_CACHE_LINE    46        // 46 entries of 11 bits

#define SACL_P1_ENTRY_SIZE                10        // size in bits
#define SACL_P2_ENTRY_SIZE                10        // size in bits
#define SACL_P3_ENTRY_SIZE                11        // size in bits

#define SACL_P1_1_TABLE_NUM_ENTRIES       (1 << 17) // (2^10 * 2^7)
#define SACL_P1_1_TABLE_SIZE              164544    // round64((1<<17)*(64/51))

#define SACL_P2_1_TABLE_NUM_ENTRIES       (1 << 18) // (2^10 * 2^8)
#define SACL_P2_1_TABLE_SIZE              329024    // round64((1<<18)*(64/51))

#define SACL_P3_1_TABLE_NUM_ENTRIES       (1 << 20) // (2^10 * 2^10)
#define SACL_P3_1_TABLE_SIZE              1458944   // round64((1<<20)*(64/46))

#define SACL_P1_2_TABLE_NUM_ENTRIES       (1 << 17) // (2^10 * 2^7)
#define SACL_P1_2_TABLE_SIZE              164544    // round64((1<<17)*(64/51))

#define SACL_P2_2_TABLE_NUM_ENTRIES       (1 << 15) // (2^7 * 2^8)
#define SACL_P2_2_TABLE_SIZE              41152     // round64((1<<15)*(64/51))

#define SACL_P3_2_TABLE_NUM_ENTRIES       (1 << 20) // (2^10 * 2^10)
#define SACL_P3_2_TABLE_SIZE              1458944   // round64((1<<20)*(64/46))

#define SACL_P1_3_TABLE_NUM_ENTRIES       (1 << 17) // (2^10 * 2^7)
#define SACL_P1_3_TABLE_SIZE              164544    // round64((1<<17)*(64/51))

#define SACL_P2_3_TABLE_NUM_ENTRIES       (1 << 15) // (2^7 * 2^8)
#define SACL_P2_3_TABLE_SIZE              41152     // round64((1<<15)*(64/51))

#define SACL_P3_3_TABLE_NUM_ENTRIES       (1 << 20) // (2^10 * 2^10)
#define SACL_P3_3_TABLE_SIZE              1458944   // round64((1<<20)*(64/46))

#define SACL_P1_4_TABLE_NUM_ENTRIES       (1 << 14) // (2^7 * 2^7)
#define SACL_P1_4_TABLE_SIZE              20608     // round64((1<<14)*(64/51))

#define SACL_P2_4_TABLE_NUM_ENTRIES       (1 << 15) // (2^7 * 2^8)
#define SACL_P2_4_TABLE_SIZE              41152     // round64((1<<15)*(64/51))

#define SACL_P3_4_TABLE_NUM_ENTRIES       (1 << 20) // (2^10 * 2^10)
#define SACL_P3_4_TABLE_SIZE              1458944   // round64((1<<20)*(64/46))

#define SACL_IPV4_DIP_TABLE_SIZE          17472     // 64+(16*64)+(16*16*64)
#define SACL_IPV6_DIP_TABLE_SIZE          87360     // 64+(4*64)+(16*64)+(64*64)
                                                    // +(256*64)+(1024*64)
#define SACL_IPV4_DIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 15+(16*15)+(256*7)
#define SACL_IPV6_DIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 3+(4*3)+(16*3)+(64*3)
                                                    // +(256*3)+(1024*1)
#define SACL_IPV4_DIP_TREE_MAX_CLASSES    1024
#define SACL_IPV6_DIP_TREE_MAX_CLASSES    1024

#define SACL_IPV4_SIP_TABLE_SIZE          17472     // 64+(16*64)+(16*16*64)
#define SACL_IPV6_SIP_TABLE_SIZE          87360     // 64+(4*64)+(16*64)+(64*64)
                                                    // +(256*64)+(1024*64)
#define SACL_IPV4_SIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 15+(16*15)+(256*7)
#define SACL_IPV6_SIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 3+(4*3)+(16*3)+(64*3)
                                                    // +(256*3)+(1024*1)
#define SACL_IPV4_SIP_TREE_MAX_CLASSES    1024
#define SACL_IPV6_SIP_TREE_MAX_CLASSES    1024
#define SACL_TAG_TREE_MAX_CLASSES         128

#define SACL_SPORT_TABLE_OFFSET         0
#define SACL_PROTO_DPORT_TABLE_OFFSET   (SACL_SPORT_TABLE_SIZE)
#define SACL_P1_1_TABLE_OFFSET          (SACL_PROTO_DPORT_TABLE_OFFSET +\
                                         SACL_PROTO_DPORT_TABLE_SIZE)
#define SACL_P1_2_TABLE_OFFSET          (SACL_P1_1_TABLE_OFFSET +\
                                         SACL_P1_1_TABLE_SIZE)
#define SACL_P1_3_TABLE_OFFSET          (SACL_P1_2_TABLE_OFFSET +\
                                         SACL_P1_2_TABLE_SIZE)
#define SACL_P1_4_TABLE_OFFSET          (SACL_P1_3_TABLE_OFFSET +\
                                         SACL_P1_3_TABLE_SIZE)
#define SACL_P2_1_TABLE_OFFSET          (SACL_P1_4_TABLE_OFFSET +\
                                         SACL_P1_4_TABLE_SIZE)
#define SACL_P2_2_TABLE_OFFSET          (SACL_P2_1_TABLE_OFFSET +\
                                         SACL_P2_1_TABLE_SIZE)
#define SACL_P2_3_TABLE_OFFSET          (SACL_P2_2_TABLE_OFFSET +\
                                         SACL_P2_2_TABLE_SIZE)
#define SACL_P2_4_TABLE_OFFSET          (SACL_P2_3_TABLE_OFFSET +\
                                         SACL_P2_3_TABLE_SIZE)
#define SACL_P3_1_TABLE_OFFSET          (SACL_P2_4_TABLE_OFFSET +\
                                         SACL_P2_4_TABLE_SIZE)
#define SACL_P3_2_TABLE_OFFSET          (SACL_P3_1_TABLE_OFFSET +\
                                         SACL_P3_1_TABLE_SIZE)
#define SACL_P3_3_TABLE_OFFSET          (SACL_P3_2_TABLE_OFFSET +\
                                         SACL_P3_2_TABLE_SIZE)
#define SACL_P3_4_TABLE_OFFSET          (SACL_P3_3_TABLE_OFFSET +\
                                         SACL_P3_3_TABLE_SIZE)
#define SACL_DIP_TABLE_OFFSET           (SACL_P3_4_TABLE_OFFSET +\
                                         SACL_P3_4_TABLE_SIZE)
#define SACL_IPV4_SIP_TABLE_OFFSET      (SACL_DIP_TABLE_OFFSET +\
                                         SACL_IPV4_DIP_TABLE_SIZE)
#define SACL_IPV6_SIP_TABLE_OFFSET      (SACL_DIP_TABLE_OFFSET +\
                                         SACL_IPV6_DIP_TABLE_SIZE)
