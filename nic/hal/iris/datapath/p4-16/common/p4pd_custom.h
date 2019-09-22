#ifndef __P4PD_CUSTOM_H__

#define __P4PD_CUSTOM_H__

//ISSUE_1223
typedef struct __attribute__((__packed__)) __p4_replication_data_t {
    uint64_t rewrite_index:12;
    uint64_t is_tunnel:1;
    uint64_t is_qid:1;
    uint64_t repl_type:2;
    uint64_t qid_or_vnid:24;
    uint64_t tunnel_rewrite_index:10;
    uint64_t lport:11;
    uint64_t qtype:3;
} p4_replication_data_t;

// ISSUE_1252
typedef struct __attribute__((__packed__)) __flow_hash_appdata {
    uint8_t export_en;
    uint32_t flow_index;
} flow_hash_appdata_t;

typedef struct __attribute__((__packed__)) __flow_hash_overflow_appdata {
    uint8_t export_en;
    uint32_t flow_index;
} flow_hash_overflow_appdata_t;

#endif /* __P4PD_CUSTOM_H__ */
