//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Hardware Flow Programming interface

#ifndef __VPP_FLOW_HW_PROGRAM_H__
#define __VPP_FLOW_HW_PROGRAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nic/utils/ftl/ftl_structs.hpp>

typedef struct ftlv4 ftlv4;
typedef struct ftlv6 ftlv6;
typedef struct ftlv4_entry_t ftlv4_entry_t;
typedef struct ftlv6_entry_t ftlv6_entry_t;

// Prototypes

int initialize_flow(void);

void session_insert(uint32_t ses_id, void *ses_info);

void session_get_addr(uint32_t ses_id, uint8_t **ses_addr,
                      uint32_t *entry_size);

ftlv4 * ftlv4_create(void *key2str,
                     void *appdata2str,
                     uint32_t thread_id);

int ftlv4_insert(ftlv4 *obj, ftlv4_entry_t *entry, uint32_t hash);

int ftlv4_remove(ftlv4 *obj, ftlv4_entry_t *entry, uint32_t hash);

void ftlv4_delete(ftlv4 *obj);

int ftlv4_dump_hw_entries(ftlv4 *obj, char *logfile, uint8_t detail);

void ftlv4_dump_stats(ftlv4 *obj, char *buf, int max_len);

int ftlv4_clear(ftlv4 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv4_get_flow_count(ftlv4 *obj);

void ftlv4_dump_stats_summary(ftlv4 **obj_arr, uint32_t obj_count,
                              char *buf, int max_len);

ftlv6 * ftlv6_create(void *key2str,
                     void *appdata2str,
                     uint32_t thread_id);

int ftlv6_insert(ftlv6 *obj, ftlv6_entry_t *entry, uint32_t hash);

int ftlv6_remove(ftlv6 *obj, ftlv6_entry_t *entry, uint32_t hash);

void ftlv6_delete(ftlv6 *obj);

int ftlv6_dump_hw_entries(ftlv6 *obj, char *logfile, uint8_t detail);

void ftlv6_dump_stats(ftlv6 *obj, char *buf, int max_len);

int ftlv6_clear(ftlv6 *obj, bool clear_global_state,
                bool clear_thread_local_state);

uint64_t ftlv6_get_flow_count(ftlv6 *obj);

void ftlv6_dump_stats_summary(ftlv6 **obj_arr, uint32_t obj_count,
                              char *buf, int max_len);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_HW_PROGRAM_H__

