//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// header defining all exposed APIs/structures from infra library
//

#ifndef __VPP_INFRA_API_API_H__
#define __VPP_INFRA_API_API_H__

#include <vppinfra/clib.h>
#include <vlib/vlib.h>

#define MAX_NACL_ID     0xFF  // 1 byte

typedef enum {
    PDS_PACKET_OFFSET_FIRST,
    PDS_PACKET_OFFSET_START = PDS_PACKET_OFFSET_FIRST,
    PDS_PACKET_OFFSET_L2,
    PDS_PACKET_OFFSET_L3,
    PDS_PACKET_OFFSET_L4,
    PDS_PACKET_OFFSET_I_L2,
    PDS_PACKET_OFFSET_I_L3,
    PDS_PACKET_OFFSET_I_L4,
    PDS_PACKET_OFFSET_LAST,
} pds_packet_offset;

typedef struct pds_infra_api_reg_s {
    // nacl id for which packets have to redirected.
    u16 nacl_id;
    // vector containing node name which has to receive redirected packets
    u8 *node;
    // frame queue index where packets have to be enqueued. This is applicable
    // only if we need packet in different thread and its mandatory in handoff.
    // set to 0xffffffff if not handoff
    u32 frame_queue_index;
    // thread ID in which packets have to received. Set to 0xFFFF
    // if packet has to be received in same thread as its received.
    u16 handoff_thread;
    // packet offset to be advanced before redirecting packets.
    pds_packet_offset offset;
    // flag to indicate register or unregister. set to true if unregister.
    u8 unreg;
} pds_infra_api_reg_t;

typedef struct pds_infra_nacl_handoff_s {
    u16 node_id;
    u16 handoff_thread;
    u32 frame_queue_index;
    pds_packet_offset offset;
} pds_infra_nacl_handoff_t;

typedef struct pds_infra_api_main_s {
    pds_infra_nacl_handoff_t *nacl_id_to_node;
    u16 drop_node_idx;
} pds_infra_api_main_t;

// this API can be used for registering/unregistering a node to
// receive packets which has corresponding nacl_id set in p4-cpu
// headers for the packet.
int pds_register_nacl_id_to_node(pds_infra_api_reg_t *params);

extern pds_infra_api_main_t infra_api_main;

always_inline void
pds_get_handoff_data_x2 (u16 nacl_id0, u16 nacl_id1,
                         u16 *next0, u16 *next1,
                         u32 *frame_queue_index0, u32 *frame_queue_index1,
                         u16 *thread_id0, u16 *thread_id1,
                         pds_packet_offset *offset0,
                         pds_packet_offset *offset1)
{
    pds_infra_api_main_t *am = &infra_api_main;
    pds_infra_nacl_handoff_t *data0, *data1;
    u16 thread_index = vlib_get_thread_index();

    data0 = &am->nacl_id_to_node[nacl_id0];

    if (PREDICT_TRUE(nacl_id0 == nacl_id1)) {
        if (PREDICT_FALSE(data0->node_id == 0xFFFF)) {
            *next0 = *next1 = am->drop_node_idx;
            goto done;
        }
        *next0 = *next1 = data0->node_id;
        *frame_queue_index0 = *frame_queue_index1 = data0->frame_queue_index;
        *thread_id0 = *thread_id1 =
                (data0->handoff_thread == 0xFFFF) ? thread_index :
                data0->handoff_thread;
        *offset0 = *offset1 = data0->offset;
        goto done;
    }
    if (PREDICT_FALSE(data0->node_id == 0xFFFF)) {
        *next0 = am->drop_node_idx;
        goto next;
    }
    *next0 = data0->node_id;
    *frame_queue_index0 = data0->frame_queue_index;
    *thread_id0 = (data0->handoff_thread == 0xFFFF) ? thread_index :
                  data0->handoff_thread;
    *offset0 = data0->offset;

next:
    data1 = &am->nacl_id_to_node[nacl_id1];
    if (PREDICT_FALSE(data1->node_id == 0xFFFF)) {
        *next1 = am->drop_node_idx;
        goto done;
    }
    *next1 = data1->node_id;
    *frame_queue_index1 = data1->frame_queue_index;
    *thread_id1 = (data1->handoff_thread == 0xFFFF) ? thread_index :
                  data1->handoff_thread;
    *offset1 = data1->offset;

done:
    return;
}

always_inline void
pds_get_handoff_data_x1 (u16 nacl_id0,
                         u16 *next0,
                         u32 *frame_queue_index0,
                         u16 *thread_id0,
                         pds_packet_offset *offset0)
{
    pds_infra_api_main_t *am = &infra_api_main;
    pds_infra_nacl_handoff_t *data0;
    u16 thread_index = vlib_get_thread_index();

    data0 = &am->nacl_id_to_node[nacl_id0];

    if (PREDICT_FALSE(data0->node_id == 0xFFFF)) {
        *next0 = am->drop_node_idx;
        goto done;
    }
    *next0 = data0->node_id;
    *thread_id0 = (data0->handoff_thread == 0xFFFF) ? thread_index :
                  data0->handoff_thread;
    *offset0 = data0->offset;

done:
    return;
}

#endif    // __VPP_INFRA_API_API_H__
