#define SACL_TAG_CLASSID_WIDTH            9         // width in bits
#define SACL_SPORT_CLASSID_WIDTH          7         // width in bits
#define SACL_PROTO_DPORT_CLASSID_WIDTH    8         // width in bits
#define SACL_DIP_CLASSID_WIDTH            8         // width in bits
#define SACL_SIP_CLASSID_WIDTH            8         // width in bits
#define SACL_P1_CLASSID_WIDTH             8         // width in bits
#define SACL_P2_CLASSID_WIDTH             8         // width in bits
#define SACL_P3_ENTRY_WIDTH               11        // width in bits

#define SACL_TAG_TREE_MAX_CLASSES         (1<<SACL_TAG_CLASSID_WIDTH)
#define SACL_SPORT_TREE_MAX_CLASSES       (1<<SACL_SPORT_CLASSID_WIDTH)
#define SACL_PROTO_DPORT_TREE_MAX_CLASSES (1<<SACL_PROTO_DPORT_CLASSID_WIDTH)
#define SACL_IPV4_DIP_TREE_MAX_CLASSES    (1<<SACL_DIP_CLASSID_WIDTH)
#define SACL_IPV6_DIP_TREE_MAX_CLASSES    (1<<SACL_DIP_CLASSID_WIDTH)
#define SACL_IPV4_SIP_TREE_MAX_CLASSES    (1<<SACL_SIP_CLASSID_WIDTH)
#define SACL_IPV6_SIP_TREE_MAX_CLASSES    (1<<SACL_SIP_CLASSID_WIDTH)
#define SACL_P1_MAX_CLASSES               (1<<SACL_P1_CLASSID_WIDTH)
#define SACL_P2_MAX_CLASSES               (1<<SACL_P2_CLASSID_WIDTH)

#define SACL_SPORT_TABLE_SIZE             2112      // 64+(32*64)
#define SACL_PROTO_DPORT_TABLE_SIZE       17472     // 64+(16*64)+(16*16*64)
#define SACL_IPV4_DIP_TABLE_SIZE          17472     // 64+(16*64)+(16*16*64)
#define SACL_IPV4_SIP_TABLE_SIZE          17472     // 64+(16*64)+(16*16*64)
#define SACL_IPV6_DIP_TABLE_SIZE          87360     // 64+(4*64)+(16*64)+(64*64)
                                                    // +(256*64)+(1024*64)
#define SACL_IPV6_SIP_TABLE_SIZE          87360     // 64+(4*64)+(16*64)+(64*64)
                                                    // +(256*64)+(1024*64)

#define SACL_SPORT_TREE_MAX_NODES         511       // (31 + 32 * 15)
#define SACL_PROTO_DPORT_TREE_MAX_NODES   2047      // 15 + (16 * 15) + (256 * 7)
#define SACL_IPV4_DIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 15+(16*15)+(256*7)
#define SACL_IPV6_DIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 3+(4*3)+(16*3)+(64*3)
                                                    // +(256*3)+(1024*1)
#define SACL_IPV4_SIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 15+(16*15)+(256*7)
#define SACL_IPV6_SIP_TREE_MAX_NODES      2047      // for 1023 prefixes.
                                                    // 3+(4*3)+(16*3)+(64*3)
                                                    // +(256*3)+(1024*1)

#define SACL_CACHE_LINE_SIZE              64
#define SACL_P1_ENTRIES_PER_CACHE_LINE    64        // 64 entries of 8 bits
#define SACL_P2_ENTRIES_PER_CACHE_LINE    64        // 64 entries of 8 bits
#define SACL_P3_ENTRIES_PER_CACHE_LINE    46        // 46 entries of 11 bits

#define SACL_P1_1_TABLE_NUM_ENTRIES       (1 << 15) // SIP:SPORT (2^8 * 2^7)
#define SACL_P1_1_TABLE_SIZE              32768     // round64((1<<15)*(64/64))

#define SACL_P2_1_TABLE_NUM_ENTRIES       (1 << 16) // DIP:PROTO_DPORT (2^8 * 2^8)
#define SACL_P2_1_TABLE_SIZE              65536     // round64((1<<16)*(64/64))

#define SACL_P3_1_TABLE_NUM_ENTRIES       (1 << 16) // P1:P2 (2^8 * 2^8)
#define SACL_P3_1_TABLE_SIZE              91200     // round64((1<<16)*(64/46))

#define SACL_P1_2_TABLE_NUM_ENTRIES       (1 << 17) // STAG:DIP (2^9 * 2^8)
#define SACL_P1_2_TABLE_SIZE              131072    // round64((1<<17)*(64/64))

#define SACL_P2_2_TABLE_NUM_ENTRIES       (1 << 15) // SPORT:DPORT (2^7 * 2^8)
#define SACL_P2_2_TABLE_SIZE              32768     // round64((1<<15)*(64/64))

#define SACL_P3_2_TABLE_NUM_ENTRIES       (1 << 16) // P1:P2 (2^8 * 2^8)
#define SACL_P3_2_TABLE_SIZE              91200     // round64((1<<16)*(64/46))

#define SACL_P1_3_TABLE_NUM_ENTRIES       (1 << 17) // SIP:DTAG (2^8 * 2^9)
#define SACL_P1_3_TABLE_SIZE              131072    // round64((1<<17)*(64/64))

#define SACL_P2_3_TABLE_NUM_ENTRIES       (1 << 15) // SPORT:DPORT (2^7 * 2^8)
#define SACL_P2_3_TABLE_SIZE              32768     // round64((1<<15)*(64/64))

#define SACL_P3_3_TABLE_NUM_ENTRIES       (1 << 16) // P1:P2 (2^8 * 2^8)
#define SACL_P3_3_TABLE_SIZE              91200     // round64((1<<16)*(64/46))

#define SACL_P1_4_TABLE_NUM_ENTRIES       (1 << 16) // STAG:SPORT (2^9 * 2^7)
#define SACL_P1_4_TABLE_SIZE              65536     // round64((1<<16)*(64/64))

#define SACL_P2_4_TABLE_NUM_ENTRIES       (1 << 17) // DTAG:DPORT (2^9 * 2^8)
#define SACL_P2_4_TABLE_SIZE              131072    // round64((1<<17)*(64/64))

#define SACL_P3_4_TABLE_NUM_ENTRIES       (1 << 16) // P1:P2 (2^8 * 2^8)
#define SACL_P3_4_TABLE_SIZE              91200     // round64((1<<16)*(64/46))

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

#define SACL_PRIORITY_HIGHEST           0
#define SACL_PRIORITY_LOWEST            0x3FF
#define SACL_PRIORITY_INVALID           (SACL_PRIORITY_LOWEST+1)
#define SACL_P3_ENTRY_PRIORITY_SHIFT    1
#define SACL_P3_ENTRY_ACTION_MASK       0x1
