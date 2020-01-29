//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Hardware Flow Programming interface

#ifndef __VPP_IMPL_FTL_WRAPPER_H__
#define __VPP_IMPL_FTL_WRAPPER_H__

typedef struct ftl_base ftlv4;
typedef struct ftl_base ftlv6;
typedef struct ipv4_flow_hash_entry_t v4_flow_entry;
typedef struct flow_hash_entry_t v6_flow_entry;

#ifdef __cplusplus

#include "gen/p4gen/p4/include/ftl.h"

extern "C" {
#endif

// Prototypes

int initialize_flow(void);

void set_skip_ftl_program(int val);

void set_skip_session_program(int val);

int session_program(uint32_t ses_id, void *action);

void session_insert(uint32_t ses_id, void *ses_info);

void session_get_addr(uint32_t ses_id, uint8_t **ses_addr,
                      uint32_t *entry_size);

ftlv4 * ftlv4_create(void *key2str,
                     void *appdata2str,
                     uint32_t thread_id);

void ftlv4_delete(ftlv4 *obj);

int ftlv4_dump_hw_entries(ftlv4 *obj, char *logfile, uint8_t detail);

int ftlv4_dump_hw_entry(ftlv4 *obj, uint32_t src, uint32_t dst,
                        uint8_t ip_proto, uint16_t sport,
                        uint16_t dport, uint16_t lookup_id,
                        char *buf, int max_len);

void ftlv4_dump_stats(ftlv4 *obj, char *buf, int max_len);

int ftlv4_clear(ftlv4 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv4_get_flow_count(ftlv4 *obj);

void ftlv4_dump_stats_summary(ftlv4 **obj_arr, uint32_t obj_count,
                              char *buf, int max_len);

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

void ftlv4_cache_batch_flush(ftlv4 *obj, int *status);

ftlv6 * ftlv6_create(void *key2str,
                     void *appdata2str,
                     uint32_t thread_id);

void ftlv6_delete(ftlv6 *obj);

int ftlv6_dump_hw_entries(ftlv6 *obj, char *logfile, uint8_t detail);

int ftlv6_dump_hw_entry(ftlv6 *obj, uint8_t *src, uint8_t *dst,
                        uint8_t ip_proto, uint16_t sport,
                        uint16_t dport, uint16_t lookup_id,
                        char *buf, int max_len);

void ftlv6_dump_stats(ftlv6 *obj, char *buf, int max_len);

int ftlv6_clear(ftlv6 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv6_get_flow_count(ftlv6 *obj);

void ftlv6_dump_stats_summary(ftlv6 **obj_arr, uint32_t obj_count,
                              char *buf, int max_len);

void ftlv6_cache_batch_init(void);

void ftlv6_cache_set_key(
             uint8_t *sip,
             uint8_t *dip,
             uint8_t ip_proto,
             uint16_t src_port,
             uint16_t dst_port,
             uint16_t lookup_id);

void ftlv6_cache_set_nexthop(
             uint32_t nhid,
             uint32_t nhtype,
             uint8_t nh_valid);

int ftlv6_cache_get_count(void);

void ftlv6_cache_advance_count(int val);

int ftlv6_cache_program_index(ftlv6 *obj, uint16_t id);

int ftlv6_cache_delete_index(ftlv6 *obj, uint16_t id);

void ftlv6_cache_set_session_index(uint32_t val);

uint32_t ftlv6_cache_get_session_index(int id);

void ftlv6_cache_set_epoch(uint8_t val);

void ftlv6_cache_set_hash_log(uint32_t val, uint8_t log);

void ftlv6_cache_batch_flush(ftlv6 *obj, int *status);

void ftlv4_set_key(v4_flow_entry *entry,
                   uint32_t sip,
                   uint32_t dip,
                   uint8_t ip_proto,
                   uint16_t src_port,
                   uint16_t dst_port,
                   uint16_t lookup_id);

void ftlv6_set_key(v6_flow_entry *entry,
                   uint8_t *sip,
                   uint8_t *dip,
                   uint8_t ip_proto,
                   uint16_t src_port,
                   uint16_t dst_port,
                   uint16_t lookup_id,
                   uint8_t key_type);

int ftlv4_remove(ftlv4 *obj, v4_flow_entry *entry, uint32_t hash, uint8_t log);

int ftlv6_remove(ftlv6 *obj, v6_flow_entry *entry, uint32_t hash, uint8_t log);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_FTL_WRAPPER_H__
