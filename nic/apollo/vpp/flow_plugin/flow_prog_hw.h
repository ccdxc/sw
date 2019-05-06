//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// Hardware Flow Programming interface

#ifndef __VPP_FLOW_PLUGIN_FLOW_PROG_HW_H__
#define __VPP_FLOW_PLUGIN_FLOW_PROG_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ftl ftl;
typedef struct ftl_entry_s ftentry_t;

// Prototypes

int initialize_pds(void);

ftl* ftl_create(uint32_t table_id,
                uint32_t num_hints,
                uint32_t max_recircs,
                void *key2str,
                void *appdata2str);

int ftl_insert(ftl *obj, ftentry_t *entry, uint32_t hash);

int ftl_remove(ftl *obj, ftentry_t *entry, uint32_t hash);

void ftl_delete(ftl *obj);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_FLOW_PLUGIN_FLOW_PROG_HW_H__
