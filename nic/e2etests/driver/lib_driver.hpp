
#ifndef __LIB_DRIVER__
#define __LIB_DRIVER__

enum queue_type {
    RX = 0,
    TX = 1,
};

struct tx_desc {    // 16 B

    uint64_t    addr:52;
    uint64_t    rsvd:4;
    uint64_t    num_sg_elems:5;
    uint64_t    opcode:3;
    uint16_t    len;
    uint16_t    vlan_tci;
    uint16_t    hdr_len:10;
    uint16_t    rsvd2:3;
    uint16_t    V:1;
    uint16_t    C:1;
    uint16_t    O:1;

    union {
        struct {
            uint16_t mss:14;
            uint16_t rsvd3:2;
        };
        struct {
            uint16_t csum_offset:14;
            uint16_t rsvd4:2;
        };
    };

} __attribute__((packed));

struct rx_desc {    // 16 B

    uint64_t    addr:52;
    uint64_t    rsvd:12;
    uint16_t    len;
    uint16_t    opcode:3;
    uint16_t    rsvd2:13;
    uint32_t    rsvd3;

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

    uint8_t     rsvd1 : 6;
    uint8_t     color : 1;
    uint8_t     enable : 1;

    uint64_t    ring_base;
    uint16_t    ring_size;
    uint64_t    cq_ring_base;

} __attribute__((packed));

uint64_t get_qstate_addr(uint64_t lif, uint32_t qtype, uint32_t qid);

std::pair<uint32_t,uint64_t>
make_doorbell(int upd, int lif, int type, int pid, int qid, int ring, int p_index);

void alloc_queue(uint64_t lif, queue_type qtype, uint32_t qid, uint16_t size);
bool poll_queue(uint64_t lif, queue_type qtype, uint32_t qid, uint32_t max_count, uint16_t *prev_cindex);
void write_queue(uint64_t lif, queue_type qtype, uint32_t qid);
void read_queue(uint64_t lif, queue_type qtype, uint32_t qid);
void print_queue(uint64_t lif, queue_type qtype, uint32_t qid);

uint8_t *alloc_buffer(uint16_t size);
void free_buffer(void *Addr);
void post_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t size);
void consume_buffer(uint64_t lif, queue_type qtype, uint32_t qid, void *buf, uint16_t *size);

#endif
