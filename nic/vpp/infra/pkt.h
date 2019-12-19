//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_PKT_H__
#define __VPP_INFRA_PKT_H__

#include <p4_cpu_hdr_utils.h>

#define PDS_PACKET_LOOP_START                                               \
    u32 _n_left_from, *_from;                                               \
    vlib_buffer_t *_bufs[VLIB_FRAME_SIZE], **_b;                            \
    u16 _nexts[VLIB_FRAME_SIZE], *_next;                                    \
    _from = vlib_frame_vector_args (from_frame);                            \
    _n_left_from = from_frame->n_vectors;                                   \
    vlib_get_buffers (vm, _from, _bufs, _n_left_from);                      \
    _b = _bufs;                                                             \
    _next = _nexts;                                                         \
    while (_n_left_from > 0) {                                              \

#define PDS_PACKET_DUAL_LOOP_START(HDR_PREFETCH_TYPE, DATA_PREFETCH_TYPE)   \
    while (_n_left_from >= 4) {                                             \
        /* Prefetch next iteration. */                                      \
        {                                                                   \
            vlib_prefetch_buffer_header (_b[2], HDR_PREFETCH_TYPE);         \
            vlib_prefetch_buffer_header (_b[3], HDR_PREFETCH_TYPE);         \
            vlib_prefetch_buffer_data(_b[2], DATA_PREFETCH_TYPE);           \
            vlib_prefetch_buffer_data(_b[3], DATA_PREFETCH_TYPE);           \
        }                                                                   \

#define PDS_PACKET_DUAL_LOOP_END                                            \
    _b += 2;                                                                \
    _next += 2;                                                             \
    _n_left_from -= 2;                                                      \
}                                                                           \

#define PDS_PACKET_SINGLE_LOOP_START                                        \
    while (_n_left_from > 0) {                                              \

#define PDS_PACKET_SINGLE_LOOP_END                                          \
    _b += 1;                                                                \
    _next += 1;                                                             \
    _n_left_from -= 1;                                                      \
}                                                                           \

#define PDS_PACKET_LOOP_END                                                 \
    }                                                                       \
    vlib_buffer_enqueue_to_next (vm, node, _from,                           \
                                 _nexts, from_frame->n_vectors);

#define PDS_PACKET_LOOP_END_NO_ENQUEUE                                      \
    }                                                                       \

#define PDS_PACKET_TRACE_LOOP_START                                         \
    u32 _n_left_from, *_from;                                               \
    vlib_buffer_t *_bufs[VLIB_FRAME_SIZE], **_b;                            \
    _from = vlib_frame_vector_args (from_frame);                            \
    _n_left_from = from_frame->n_vectors;                                   \
    vlib_get_buffers (vm, _from, _bufs, _n_left_from);                      \
    _b = _bufs;                                                             \
    while (_n_left_from > 0) {                                              \

#define PDS_PACKET_TRACE_DUAL_LOOP_START                                    \
    while (_n_left_from >= 4) {                                             \
        {                                                                   \
            vlib_prefetch_buffer_with_index (vm, _from[2], LOAD);            \
            vlib_prefetch_buffer_with_index (vm, _from[3], LOAD);            \
        }                                                                   \

#define PDS_PACKET_TRACE_DUAL_LOOP_END                                      \
    _b += 2;                                                                \
    _n_left_from -= 2;                                                      \
}                                                                           \

#define PDS_PACKET_TRACE_SINGLE_LOOP_START                                  \
    while (_n_left_from > 0) {                                              \

#define PDS_PACKET_TRACE_SINGLE_LOOP_END                                    \
    _b += 1;                                                                \
    _n_left_from -= 1;                                                      \
}                                                                           \

#define PDS_PACKET_TRACE_LOOP_END  PDS_PACKET_LOOP_END_NO_ENQUEUE

#define PDS_PACKET_BUFFER(_id)              (_b[_id])
#define PDS_PACKET_NEXT_NODE(_id)           (_next[_id])
#define PDS_PACKET_NEXT_NODE_PTR(_id)       (_next + _id)
#define PDS_PACKET_NEXT_NODE_ARR            (_nexts)
#define PDS_PACKET_BUFFER_INDEX_PTR(_id)    (_from + _id)

uint8_t
pds_p4_rx_get_l2_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l2_offset;
}

uint8_t
pds_p4_rx_get_l3_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l3_offset;
}

uint8_t
pds_p4_rx_get_l4_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l4_offset;
}

uint8_t
pds_p4_rx_get_inner_l2_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l2_inner_offset;
}

uint8_t
pds_p4_rx_get_inner_l3_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l3_inner_offset;
}

uint8_t
pds_p4_rx_get_inner_l4_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->l4_inner_offset;
}

uint8_t
pds_p4_rx_get_payload_offset (p4_rx_cpu_hdr_t *hdr)
{
    return hdr->payload_offset;
}

#endif    // __VPP_INFRA_LOOKUP_PKT_H__
