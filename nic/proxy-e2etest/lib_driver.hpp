
#ifndef __LIB_DRIVER__
#define __LIB_DRIVER__

enum queue_type {
    RX = 0,
    TX = 1,
};

struct tx_desc {    // 16 B

    uint64_t    addr;
    uint16_t    len;
    uint16_t    vlan_tag;
    uint16_t    mss : 14;
    uint8_t     encap : 2;
    uint16_t    hdr_len : 10;
    uint8_t     offload_mode : 2;
    uint8_t     eop : 1;
    uint8_t     cq_entry : 1;
    uint8_t     vlan_insert : 1;
    uint8_t     rsvd0 : 1;

} __attribute__((packed));

struct rx_desc {    // 16 B

    uint64_t    addr;
    uint16_t    len;
    uint16_t    packet_len;
    uint32_t    __pad0;

} __attribute__((packed));

struct qstate {

    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;

    uint16_t    p_index0;
    uint16_t    c_index0;
    uint16_t    p_index1;
    uint16_t    c_index1;
    uint16_t    p_index2;
    uint16_t    c_index2;
    uint16_t    p_index3;
    uint16_t    c_index3;
    uint16_t    p_index4;
    uint16_t    c_index4;
    uint16_t    p_index5;
    uint16_t    c_index5;
    uint16_t    p_index6;
    uint16_t    c_index6;
    uint16_t    p_index7;
    uint16_t    c_index7;

    uint8_t     enable;
    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;

} __attribute__((packed));

uint64_t get_qstate_addr(uint64_t lif, uint32_t qtype, uint32_t qid);

std::pair<uint32_t,uint64_t>
make_doorbell(int upd, int lif, int type, int pid, int qid, int ring, int p_index);

void alloc_queue(uint64_t lif, queue_type qtype, uint32_t qid, uint16_t size);
void poll_queue(uint64_t lif, queue_type qtype, uint32_t qid);
void write_queue(uint64_t lif, queue_type qtype, uint32_t qid);
void read_queue(uint64_t lif, queue_type qtype, uint32_t qid);
void print_queue(uint64_t lif, queue_type qtype, uint32_t qid);

uint8_t *alloc_buffer(uint16_t size);
void post_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t size);
void consume_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t *size);

#endif
