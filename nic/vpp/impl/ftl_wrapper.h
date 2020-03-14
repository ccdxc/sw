//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Hardware Flow Programming interface

#ifndef __VPP_IMPL_FTL_WRAPPER_H__
#define __VPP_IMPL_FTL_WRAPPER_H__

typedef struct ftl_base ftl;
typedef struct ftl_base ftlv4;
typedef struct ftl_base ftlv6;
typedef struct ftl_base ftll2;
typedef struct ipv4_flow_hash_entry_t v4_flow_entry;
typedef struct flow_hash_entry_t flow_entry;

typedef char* (*key2str_t)(void *key);
typedef char* (*appdata2str_t)(void *data);

#ifdef __cplusplus

#include "gen/p4gen/p4/include/ftl.h"

extern "C" {
#endif

#define FTL_ENTRY_STR_MAX   2048

// max 256 packets are processed by VPP, so 512 flows
#define MAX_FLOW_ENTRIES_PER_BATCH 512

typedef struct flow_flags_s {
    uint8_t log : 1;
    uint8_t update : 1;
} flow_flags_t;

// Prototypes

int initialize_flow(void);

void set_skip_ftl_program(int val);

void set_skip_session_program(int val);

int get_skip_ftl_program(void);

int get_skip_session_program(void);

int session_program(uint32_t ses_id, void *action);

void session_insert(uint32_t ses_id, void *ses_info);

void session_get_addr(uint32_t ses_id, uint8_t **ses_addr,
                      uint32_t *entry_size);

void ftl_init_stats_cache(void);

void ftl_cache_stats(ftl *obj);

void ftl_dump_stats_cache(char *buf, int max_len);

void ftl_dump_stats(ftl *obj, char *buf, int max_len);

ftl * ftl_create(void *key2str, void *appdata2str);

int ftl_insert(ftl *obj, flow_entry *entry, uint32_t hash, uint8_t log, 
               uint8_t update);

int ftl_remove(ftl *obj, flow_entry *entry, uint32_t hash, uint8_t log);

int ftl_clear(ftl *obj, bool clear_global_state, bool clear_thread_local_state);

void ftl_delete(ftl *obj);

int ftl_dump_hw_entries(ftl *obj, char *logfile, uint8_t detail, bool v6);

uint64_t ftl_get_flow_count(ftl *obj, bool v6);

void ftl_set_session_index(flow_entry *entry, uint32_t session);

void ftl_set_epoch(flow_entry *entry, uint8_t val);

uint32_t ftl_get_session_id(flow_entry *entry);

void ftl_set_thread_id(ftl *obj, uint32_t thread_id);

void ftl_set_key_lookup_id(flow_entry *entry, uint16_t lookup_id);

void ftl_set_entry_flow_miss_hit(flow_entry *entry, uint8_t val);

void ftl_set_entry_nexthop(flow_entry *entry, uint32_t nhid, uint32_t nhtype,
                           uint8_t nhvalid);

void ftlv4_set_entry_nexthop(v4_flow_entry *entry, uint32_t nhid,
                             uint32_t nhtype, uint8_t nhvalid);

void ftlv4_set_key_lookup_id(v4_flow_entry *entry, uint16_t lookup_id);

uint16_t ftlv4_get_key_lookup_id(v4_flow_entry *entry);

void ftlv4_set_entry_flow_miss_hit(v4_flow_entry *entry, uint8_t val);

ftlv4 * ftlv4_create(void *key2str,
                     void *appdata2str);

void ftlv4_delete(ftlv4 *obj);

int ftlv4_dump_hw_entries(ftlv4 *obj, char *logfile, uint8_t detail);

int ftlv4_dump_hw_entry(ftlv4 *obj, uint32_t src, uint32_t dst,
                        uint8_t ip_proto, uint16_t sport,
                        uint16_t dport, uint16_t lookup_id,
                        char *buf, int max_len);

void ftlv4_init_stats_cache(void);

void ftlv4_cache_stats(ftlv4 *obj);

void ftlv4_dump_stats(ftlv4 *obj, char *buf, int max_len);

void ftlv4_dump_stats_cache(char *buf, int max_len);

int ftlv4_clear(ftlv4 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv4_get_flow_count(ftlv4 *obj);

void ftlv4_cache_batch_init(void);

void ftlv4_cache_set_key(
             uint32_t sip,
             uint32_t dip,
             uint8_t ip_proto,
             uint16_t src_port,
             uint16_t dst_port,
             uint16_t lookup_id);

void ftlv4_cache_set_nexthop(
             uint32_t nhid,
             uint32_t nhtype,
             uint8_t nh_valid);

int ftlv4_cache_get_count(void);

void ftlv4_cache_advance_count(int val);

int ftlv4_cache_program_index(ftlv4 *obj, uint16_t id);

int ftlv4_cache_delete_index(ftlv4 *obj, uint16_t id);

void ftlv4_cache_set_session_index(uint32_t val);

uint32_t ftlv4_cache_get_session_index(int id);

void ftlv4_cache_set_epoch(uint8_t val);

void ftlv4_cache_set_hash_log(uint32_t val, uint8_t log);

void ftlv4_cache_set_flow_miss_hit(uint8_t val);

void ftlv4_cache_set_update_flag(uint8_t update);

void ftlv4_cache_batch_flush(ftlv4 *obj, int *status);

void ftlv4_set_thread_id(ftlv4 *obj, uint32_t thread_id);

ftl * ftl_create(void *key2str, void *appdata2str);

void ftlv6_delete(ftlv6 *obj);

int ftlv6_dump_hw_entries(ftlv6 *obj, char *logfile, uint8_t detail);

int ftlv6_dump_hw_entry(ftlv6 *obj, uint8_t *src, uint8_t *dst,
                        uint8_t ip_proto, uint16_t sport,
                        uint16_t dport, uint16_t lookup_id,
                        char *buf, int max_len);

void ftlv6_init_stats_cache(void);

void ftlv6_cache_stats(ftlv6 *obj);

void ftlv6_dump_stats(ftlv6 *obj, char *buf, int max_len);

void ftlv6_dump_stats_cache(char *buf, int max_len);

int ftlv6_clear(ftlv6 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv6_get_flow_count(ftlv6 *obj);

void ftlv6_cache_batch_init(void);

void ftlv6_cache_set_key(uint8_t *sip,
                         uint8_t *dip,
                         uint8_t ip_proto,
                         uint16_t src_port,
                         uint16_t dst_port,
                         uint16_t lookup_id);

void ftll2_cache_set_key(uint8_t *src,
                         uint8_t *dst,
                         uint16_t ether_type,
                         uint16_t lookup_id);

void ftlv6_cache_set_nexthop(uint32_t nhid,
                             uint32_t nhtype,
                             uint8_t nh_valid);

int ftlv6_cache_get_count(void);

void ftlv6_cache_advance_count(int val);

int ftlv6_cache_program_index(ftlv6 *obj, uint16_t id);

int ftlv6_cache_delete_index(ftlv6 *obj, uint16_t id);

void ftlv6_cache_set_session_index(uint32_t val);

uint32_t ftlv6_cache_get_session_index(int id);

void ftlv6_cache_set_epoch(uint8_t val);

void ftlv6_cache_set_flow_miss_hit(uint8_t val);

void ftlv6_cache_set_update_flag(uint8_t update);

void ftlv6_cache_set_hash_log(uint32_t val, uint8_t log);

void ftlv6_cache_batch_flush(ftlv6 *obj, int *status);

void ftlv4_set_key(v4_flow_entry *entry,
                   uint32_t sip,
                   uint32_t dip,
                   uint8_t ip_proto,
                   uint16_t src_port,
                   uint16_t dst_port,
                   uint16_t lookup_id);

void ftlv6_set_key(flow_entry *entry,
                   uint8_t *sip,
                   uint8_t *dip,
                   uint8_t ip_proto,
                   uint16_t src_port,
                   uint16_t dst_port,
                   uint16_t lookup_id);

void ftll2_set_key(flow_entry *entry,
                   uint8_t *src,
                   uint8_t *dst,
                   uint16_t ether_type,
                   uint16_t lookup_id);

int ftlv4_remove(ftlv4 *obj, v4_flow_entry *entry, uint32_t hash, uint8_t log);

int ftlv6_remove(ftlv6 *obj, flow_entry *entry, uint32_t hash, uint8_t log);

void ftlv6_set_thread_id(ftlv6 *obj, uint32_t thread_id);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_FTL_WRAPPER_H__
