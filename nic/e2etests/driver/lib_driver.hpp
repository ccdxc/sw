
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

struct tx_cq_desc {
    uint32_t status:8;
    uint32_t rsvd:8;
    uint32_t comp_index:16;
    uint32_t rsvd2[2];
    uint32_t rsvd3:31;
    uint32_t color:1;
}__attribute__((packed));

struct rx_cq_desc {
   uint32_t status:8;
   uint32_t rsvd:8;
   uint32_t comp_index:16;
   uint32_t rss_hash;
   uint16_t csum;
   uint16_t vlan_tci;
   uint32_t len:14;
   uint32_t rsvd2:2;
   uint32_t rss_type:4;
   uint32_t rsvd3:4;
   uint32_t csum_tcp_ok:1;
   uint32_t csum_tcp_bad:1;
   uint32_t csum_udp_ok:1;
   uint32_t csum_udp_bad:1;
   uint32_t csum_ip_ok:1;
   uint32_t csum_ip_bad:1;
   uint32_t V:1;
   uint32_t color:1;
}__attribute__((packed));


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


typedef void (*pkt_read_cb)(uint8_t *buf, uint32_t len, void *ctxt);
void consume_queue(uint64_t lif, queue_type qtype, uint32_t qid,
        pkt_read_cb pkt_cb, void *ctx);
void tx_consume_queue(uint64_t lif, queue_type qtype, uint32_t qid);
bool queue_has_space(uint64_t lif, queue_type qtype, uint32_t qid);

#endif
