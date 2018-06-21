#ifndef _FTE_EXAMPLE_H_
#define _FTE_EXAMPLE_H_

/* Version 1 of the flow_key */
struct flow_key_t {
    uint32_t tid;
    uint32_t sip;
    uint32_t dip;
    uint16_t proto;
    uint16_t sport;
    uint16_t dport;
};

typedef struct flow_state_s {
    uint64_t        create_ts;           // flow create timestamp
    uint32_t        last_pkt_ts;         // last packet timestamp
    uint64_t        packets;             // packet count on this flow
    uint64_t        bytes;               // byte count on this flow
    uint64_t        drop_packets;        // packets dropped for this flow
    uint64_t        drop_bytes;          // bytes dropped for this flow
    uint32_t        exception_bmap;      // exceptions seen on this flow

    uint32_t        tcp_seq_num;
    uint32_t        tcp_ack_num;
    uint32_t        tcp_win_sz;
    int32_t         syn_ack_delta;       // ACK delta of iflow
    uint16_t        tcp_mss;
    uint8_t         tcp_win_scale;
    uint8_t         tcp_ws_option_sent : 1;
    uint8_t         tcp_ts_option_sent : 1;
    uint8_t         tcp_sack_perm_option_sent : 1;
} flow_state_t;

struct flow_t {
    struct flow_key_t flow_key;
    flow_state_t flow_state;
};
#endif
