#define CPU_CB_TABLE_ENTRY_SIZE        512 /* 512B */
#define CPU_CB_TABLE_ENTRY_SIZE_SHFT    9  /* 512B */
#define CPU_CB_WRITE_ARQRX_OFFSET       64

#define CPUPR_ALLOC_IDX                0xa55b2000 
#define CPUPR_FREE_IDX                 0xba00ba28
#define CPUDR_ALLOC_IDX                0xa5532000 
#define CPUDR_FREE_IDX                 0xba00ba38

#define TABLE_TYPE_RAW                 0
#define TABLE_LOCK_DIS                 0
#define TABLE_LOCK_EN                  1

#define TABLE_SIZE_8_BITS              0
#define TABLE_SIZE_16_BITS             1
#define TABLE_SIZE_32_BITS             2
#define TABLE_SIZE_64_BITS             3
#define TABLE_SIZE_128_BITS            4
#define TABLE_SIZE_256_BITS            5
#define TABLE_SIZE_512_BITS            6

#define NIC_DESC_ENTRY_0_OFFSET              64         /* &((nic_desc_t *)0)->entry[0]*/
#define NIC_PAGE_HDR_SIZE              52         /* sizeof(nic_page_hdr_t) */
#define CACHE_LINE_SIZE                64
#define NIC_PAGE_HEADROOM             (17 * CACHE_LINE_SIZE)


#define CPU_PHV_AOL_DESC_START      CAPRI_PHV_START_OFFSET(aol_next_pkt)
#define CPU_PHV_AOL_DESC_END        CAPRI_PHV_END_OFFSET(aol_A0)

#define NIC_ARQRX_ENTRY_SIZE                  8
#define NIC_ARQRX_ENTRY_SIZE_SHIFT            3          /* for 8B */

#define CPU_PHV_RING_ENTRY_DESC_ADDR_START CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
#define CPU_PHV_RING_ENTRY_DESC_ADDR_END CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)


