
#define ADDRESS_WIDTH 64 

#define RING_WIDTH    8

// 3 Intrinsics is 251 bits and IPSec SW is 88 bits
#define RXDMA_IPSEC_HW_SW_INTRINSIC 43 

#define PINDEX_SIZE_SHIFT 8

#define RNMDR_BASE_ADDR 0xa0a0a0a0
#define RNMPR_BASE_ADDR 0xb0b0b0b0
//Let me check the below and correct it
#define NIC_PAGE_ENTRY_SIZE_SHIFT 14 
//Let me check the below and correct it - for now 64 bytes 
#define NIC_DESC_ENTRY_SIZE_SHIFT 6 
// internal-structure+2 AOLs (need to put the right number here - can keep changing)
#define NIC_DESC_SIZE 96

#define OUT_PAGE_PINDEX_BASE_ADDR 0xabababab
#define IN_PAGE_PINDEX_BASE_ADDR 0xabababab
#define IN_DESC_PINDEX_BASE_ADDR 0xcccccccc
#define OUT_DESC_PINDEX_BASE_ADDR 0xacacacac

#define TM_P4_DEF_OQUEUE 0

#define TM_P4_INGRESS  1
#define TM_P4_EGRESS   2
#define TM_P4P_RXDMA   3
#define TM_P4P_TXDMA   4

#define IPSEC_ESP_SVC_LIF 1001
#define IPSEC_AH_SVC_LIF  1002

#define ESP_BASE 8
#define IPSEC_WIN_REPLAY_MAX_DIFF 63

// this is equal to size of intrinsic+appheader - need to put the correct value here
#define IPSEC_PHV_HEADROOM_RX 64

#define BRQ_REQ_ENTRY_SIZE 64
#define BRQ_REQ_BASE_ADDR 0xbabababa
 
