//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#include "api.h"

pds_infra_api_main_t infra_api_main;

int
pds_register_nacl_id_to_node (pds_infra_api_reg_t *params)
{
    pds_infra_api_main_t *am = &infra_api_main;
    vlib_node_t *lookup_node,
                *reg_node;
    vlib_main_t *vm = vlib_get_main();
    pds_infra_nacl_handoff_t *nacl_id_to_node = NULL;

    if ((!params) || (params->nacl_id > MAX_NACL_ID) ||
        (!params->node && params->frame_queue_index)) {
        return -1;
    }

    if (params->unreg) {
        goto unreg;
    }

    if ((params->handoff_thread != 0xFFFF) &&
        (params->handoff_thread > vec_len (vlib_worker_threads))) {
        return -1;
    }

    lookup_node = vlib_get_node_by_name(vm, (u8 *) "pds-p4cpu-hdr-lookup");
    reg_node = vlib_get_node_by_name(vm, params->node);
    if (!lookup_node || !reg_node) {
        return -1;
    }

    vlib_worker_thread_barrier_sync(vm);
    nacl_id_to_node = &am->nacl_id_to_node[params->nacl_id];
    nacl_id_to_node->node_id =
            vlib_node_add_next(vm, lookup_node->index,
                               reg_node->index);
    nacl_id_to_node->frame_queue_index = params->frame_queue_index;
    nacl_id_to_node->handoff_thread = params->handoff_thread;
    nacl_id_to_node->offset = params->offset;
    vlib_worker_thread_barrier_release(vm);
    return 0;

unreg:
    // there is no way to remove already added next node
    vlib_worker_thread_barrier_sync(vm);
    clib_memset(nacl_id_to_node, ~0,
                sizeof(pds_infra_nacl_handoff_t));
    vlib_worker_thread_barrier_release(vm);
    return 0;
}

static clib_error_t *
pds_infra_api_init (vlib_main_t * vm)
{
    pds_infra_api_main_t *am = &infra_api_main;
    vlib_node_t *lookup_node;

    vec_validate(am->nacl_id_to_node, MAX_NACL_ID);
    clib_memset(am->nacl_id_to_node, ~0,
                sizeof(pds_infra_nacl_handoff_t) * (MAX_NACL_ID+1));
    lookup_node = vlib_get_node_by_name(vm, (u8 *) "pds-p4cpu-hdr-lookup");
    am->drop_node_idx = vlib_node_add_named_next(vm, lookup_node->index,
                                                 "pds-error-drop");

    return NULL;
}

VLIB_INIT_FUNCTION(pds_infra_api_init);
