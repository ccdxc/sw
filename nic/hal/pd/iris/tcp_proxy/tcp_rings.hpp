#define MAX_TCP_RINGS_CTXT  3 // Must be defined to number of FTE inst or equivalent
#define MAX_TCP_RING_TYPES  1 // ACTL_Q
#define TCP_RING_SEM_CI_BATCH_SIZE  8
#define BITS_IN_BYTE 8
namespace hal {
namespace pd {

#define TCP_ACTL_MSG_TYPE_BITS  8
#define TCP_ACTL_MSG_TYPE_SHIFT 56
#define TCP_ACTL_MSG_TYPE_MASK  \
  (((1ULL << TCP_ACTL_MSG_TYPE_BITS) - 1) << TCP_ACTL_MSG_TYPE_SHIFT)

#define TCP_ACTL_PKT_DESC_BITS  56
#define TCP_ACTL_PKT_DESC_SHIFT 0
#define TCP_ACTL_PKT_DESC_MASK  \
  (((1ULL << TCP_ACTL_PKT_DESC_BITS) - 1) << TCP_ACTL_PKT_DESC_SHIFT)

/*
 * Struct for PKT type msg on TCP ACTL Q
 * To access msg fields on CPU using this struct,
 * each field (gt a byte) needs to be swapped for endinanes
 */
typedef struct tcp_msg_pkt_s {
        uint64_t    msg_type : 8; // MUST be the first field
        uint64_t    descr_addr : 56; // 34 bits used
} __PACK__ tcp_msg_pkt_t;

// Extracts msg type from msg on TCP ACTL Q
static inline uint8_t tcp_actl_get_msg_type(uint64_t msg)
{
    return (uint8_t)((msg & TCP_ACTL_MSG_TYPE_MASK) >>
          TCP_ACTL_MSG_TYPE_SHIFT);
}

// Extracts Pkt Desc addr from Pkt type msg on TCP ACTL Q
static inline uint64_t tcp_actl_get_pkt_desc(uint64_t msg)
{
    return (uint64_t)((msg & TCP_ACTL_PKT_DESC_MASK) >>
          TCP_ACTL_PKT_DESC_SHIFT);
}

/*
 * Struct for CB cleanup type msg on TCP ACTL Q
 * To access msg fields on CPU using this struct,
 * each field (gt a byte) needs to be swapped for endinaness
 */
typedef struct tcp_msg_ctrl_s {
    uint64_t msg_type : 8; // MUST be the first field
    uint64_t qtype: 8; // 3 bits used
    uint64_t lif: 16; // 11 bits used
    uint64_t qid : 32; // 24 bits used
} __PACK__ tcp_msg_ctrl_t;

/*
 * Struct for unionized msg formats for all msg types on TCP ACTL Q
 * To access msg fields on CPU using this struct,
 * each field (gt a byte) needs to be swapped for endinaness
 */
typedef struct tcp_actl_q_elem_s {
        union {
            uint64_t long_word;
            struct {
                uint8_t msg_type;
                uint8_t bytes[7];
            };
            tcp_msg_pkt_t msg_pkt;
            tcp_msg_ctrl_t  msg_ctrl;
        } u;
} __PACK__ tcp_actl_q_elem_t;

// MSBit of msg element is the valid bit
#define TCP_ACTL_Q_MSG_VALID_BIT_MASK   \
    (1ULL << ((sizeof(tcp_actl_q_elem_t) * BITS_IN_BYTE) - 1))

typedef struct tcp_ring_info_s {
    // Only one ring, the ACTL ring right now.
    uint64_t                ring_base;
    uint8_t                 *virt_ring_base; //mmap'ed virtual base address of the ring region
    uint64_t                pc_idx_addr;
    uint8_t                 *virt_pc_idx_addr; //mmap'ed virtual address of the ring slot
    uint64_t                valid_bit_val;
    uint32_t                pc_idx; // Consumer index in case of RXQ
    uint16_t                ring_size_shift;
    uint8_t                 queue_id;
    uint8_t                 type;
    pd_wring_meta_t         *wring_meta;
} tcp_ring_info_t;

typedef struct tcp_rings_ctxt_s {
    tcp_ring_info_t ring_info[MAX_TCP_RING_TYPES];
    uint8_t num_queues;
} tcp_rings_ctxt_t;

}
}
