//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Memhash interface

#ifndef __VPP_FLOW_PLUGIN_FLOW_MEMHASH_H__
#define __VPP_FLOW_PLUGIN_FLOW_MEMHASH_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_hash mem_hash;

// Prototypes

int initialize_pds(void);
mem_hash* mem_hash_create(uint32_t table_id, uint32_t num_hints,
                          uint32_t max_recircs, void *key2str,
                          void *appdata2str);
int mem_hash_insert(mem_hash *obj, void *key, void *app_data);
void mem_hash_delete(mem_hash *obj);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_PLUGIN_FLOW_MEMHASH_H__
