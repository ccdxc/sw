
#ifndef __LIB_DRIVER__
#define __LIB_DRIVER__

enum queue_type {
    RX = 0,
    TX = 1,
    ADMIN = 2,
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

struct admin_cmd_desc {
  uint16_t opcode;
  uint16_t cmd_data[31];
};

struct admin_comp_desc {
  uint32_t status:8;
  uint32_t rsvd:8;
  uint32_t comp_index:16;
  uint8_t cmd_data[11];
  uint8_t rsvd2:7;
  uint8_t color:1;
};

typedef struct {
    uint64_t qstate_addr;
    void *qstate;
    uint64_t ring_base;
    uint32_t ring_size;
    uint64_t cq_ring_base;
    void *q; // Q
    uint16_t head;  // producer index
    uint16_t tail;  // consumer index
    void *cq; // CQ
    uint16_t cq_tail;   // consumer index
    uint16_t cq_color;  // expected color
} queue_info_t;

uint64_t get_qstate_addr(uint16_t lif, uint32_t qtype, uint32_t qid);

queue_info_t &get_queue_info(uint16_t lif, queue_type qtype, uint32_t qid);
void set_queue_info(uint16_t lif, queue_type qtype, uint32_t qid, queue_info_t queue_info);

void alloc_queue(uint16_t lif, queue_type qtype, uint32_t qid, uint16_t size);
void write_queue(uint16_t lif, queue_type qtype, uint32_t qid);
void read_queue(uint16_t lif, queue_type qtype, uint32_t qid);
bool poll_queue(uint16_t lif, queue_type qtype, uint32_t qid);

uint8_t *alloc_buffer(uint32_t size);
void free_buffer(uint8_t *buf);

void post_buffer(uint16_t lif, queue_type qtype, uint32_t qid, uint8_t *buf, uint32_t size);
typedef void (*completion_cb)(uint8_t *buf, uint32_t size, void *ctx);
void consume_buffer(uint16_t lif, queue_type qtype, uint32_t qid, completion_cb cb, void *ctx);

bool queue_has_space(uint16_t lif, queue_type qtype, uint32_t qid);

#endif
