//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "tcp_buffer.hpp"
#include "nic/fte/fte_ctx.hpp"

using namespace fte;

namespace hal {
namespace plugins {
namespace alg_utils {

// static memeber initializer
slab* tcp_buffer_t::slab_ = slab::factory("tcp_buffer_t", hal::HAL_SLAB_TCP_REASSEMBLY_BUFF,
                                           sizeof(tcp_buffer_t), 16, true, true, true);

//------------------------------------------------------------------------
// Alloc/Initialize the tcp buffer
//------------------------------------------------------------------------
tcp_buffer_t *
tcp_buffer_t::factory (uint32_t seq_start, void *handler_ctx, data_handler_t handler)
{
    tcp_buffer_t *entry = (tcp_buffer_t *)slab_->alloc();

    entry->buff_ = NULL;
    entry->free_buff_size_ = 0;
    entry->buff_size_ = 0;
    entry->cur_seq_ = seq_start;
    entry->segments_ = NULL;
    entry->num_segments_ = 0;
    entry->data_handler_ = handler;
    entry->handler_ctx_ = handler_ctx;

    HAL_TRACE_DEBUG("Init TCP buffer with seq_start: {} data handler: {:#x} handle_ctx: {:#x}",
                     entry->cur_seq_, (void *)entry->data_handler_, (void *)entry->handler_ctx_);

    return entry;
}

//------------------------------------------------------------------------
// Free the entry
//------------------------------------------------------------------------
void tcp_buffer_t::free ()
{
    if (buff_) {
        HAL_FREE(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, buff_);
    }
    if (segments_) {
        HAL_FREE(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, segments_);
    }

    slab_->free(this);
}

//------------------------------------------------------------------------
// Helper to insert the segment given the FTE context and data handler
//------------------------------------------------------------------------
hal_ret_t
tcp_buffer_t::insert_segment (fte::ctx_t &ctx, data_handler_t handler) 
{
    uint32_t payload_len = (ctx.pkt_len() - ctx.cpu_rxhdr()->payload_offset);
    uint32_t seq = htonl(ctx.cpu_rxhdr()->tcp_seq_num);
    uint8_t  *pkt = ctx.pkt();

    handler_ctx_ = (void *)std::addressof(ctx);
    data_handler_ = handler;
    HAL_TRACE_DEBUG("Packet len: {} payload offset: {} data handler: {}", ctx.pkt_len(), 
                                ctx.cpu_rxhdr()->payload_offset, (void *)data_handler_);
    HAL_TRACE_DEBUG("seq: {} curr_seq: {} payload_len: {}", seq, cur_seq_, payload_len);
    return insert_segment(seq, &pkt[ctx.cpu_rxhdr()->payload_offset], payload_len);
}

//------------------------------------------------------------------------
// Insert new segment into the buffer, calls the handler with the
// reassembled data.
//------------------------------------------------------------------------
hal_ret_t
tcp_buffer_t::insert_segment (uint32_t seq, uint8_t *payload, size_t payload_len)
{
    uint32_t end = seq + payload_len;
    bool     data_handler_invoked = false;

    // If the end of the payload ends before our current sequence number,
    // its old duplicate data
    if (compare_seq_numbers(end, cur_seq_) <= 0) {
        HAL_TRACE_DEBUG("Payload ends before current seq: {} payload_len: {}", seq, payload_len);
        return HAL_RET_OK;
    }

    // If it starts before current sequence number, slice it
    if (compare_seq_numbers(seq, cur_seq_) < 0) {
        HAL_TRACE_DEBUG("Payload starts before current -- slicing seq: {} cur_seq_: {}", seq, cur_seq_);
        uint32_t diff = cur_seq_ - seq;
        seq = cur_seq_;
        payload += diff;
        payload_len -= diff;
    }

    // If the segment matches cur seq and doesn't intersect with any of the
    // stored segments, call the handler without doing a copy
    if ((seq == cur_seq_) &&
        (num_segments_ <= 0 || compare_seq_numbers(end, segments_[0].start) < 0)) {
        HAL_TRACE_DEBUG("Calling data handler1");
        size_t processed = data_handler_(handler_ctx_, payload, payload_len);
        SDK_ASSERT_RETURN(processed <= payload_len, HAL_RET_INVALID_ARG);
        cur_seq_ += processed;
        if (processed == payload_len) {
            // all payload is processed, return
            return HAL_RET_OK;
        }
        seq += processed;
        payload += processed;
        payload_len -= processed;
        data_handler_invoked = true;
        HAL_TRACE_DEBUG("Processed bytes: {} seq: {} end: {}", processed, seq, end);
    }

    // Allocate/expand the buffer
    if ((end - cur_seq_) > free_buff_size_) {
        if (free_buff_size_ == 0) {
            // Set the buff size to minimum required, we increase as we go
            free_buff_size_ = buff_size_ = ((end-cur_seq_) > MIN_BUFF_SIZE)?((end-cur_seq_)+1):MIN_BUFF_SIZE;
            // first segment to store, allocate buffer
            buff_ = (uint8_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, free_buff_size_);
            segments_ = (segment_t *) HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                                 sizeof(segment_t) * MAX_SEGMENTS);
            HAL_TRACE_DEBUG("Allocate segment of size: {}", buff_size_);
        } else if (2*buff_size_ < MAX_BUFF_SIZE) {
            // expand the size of buffer
            uint32_t new_buff_sz = 2*buff_size_;
            uint8_t *new_buff = (uint8_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                                      new_buff_sz);
            memcpy(new_buff, buff_, buff_size_);
            HAL_FREE(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, buff_);

            // Set the free buff size to reflect the new buffer
            free_buff_size_ += (new_buff_sz - buff_size_);
            buff_ = new_buff;
            buff_size_ = new_buff_sz;
        } else {
            uint32_t seq_diff = (seq - cur_seq_);
            // Check if we are seeing a packet in the future and
            // we have enough to hold some of the payload atleast
            if (seq_diff < free_buff_size_) {
                HAL_TRACE_ERR("alg_utils::payload execeeds the max buffer size - truncating");
                end = seq + (free_buff_size_ - seq_diff);
                payload_len = (end - seq);     
            } else {
                HAL_TRACE_ERR("alg_utils::payload execeeds the max buffer size - bailing");
            }
        }
    }


    // Remember the size of reassembled payload (handler is called when there is new data)
    size_t reassembled_payload = (segments_[0].start == cur_seq_) ?
        (segments_[0].end - segments_[0].start) : 0;

    // insert the segment in correct order
    int cur = 0;

    // skip all segments which end before the new segment
    for (cur = 0; cur < num_segments_; cur++) {
        if (compare_seq_numbers(seq, segments_[cur].end) <= 0) {
            break;
        }
    }

    if (cur >= num_segments_) {
        HAL_TRACE_DEBUG("Adding new segment at the end");
        // add new segment at the end
        if (cur < MAX_SEGMENTS) {
            segments_[cur].start = seq;
            segments_[cur].end = end;
            num_segments_++;
        } else {
            HAL_TRACE_ERR("alg_utils:: max segments reached - dropping segment");
            return HAL_RET_OOB;
        }
    }  else if (compare_seq_numbers(end, segments_[cur].start) < 0) {
        HAL_TRACE_DEBUG("Move the segments and insert current segment");
        // new segment ends before the cur segment.
        // insert the new segment before the cur segment by moving
        // the rest of the segments
        move_segments(cur, 1);
        segments_[cur].start = seq;
        segments_[cur].end = end;
    } else {
        HAL_TRACE_DEBUG("Overlapping segment found, merging segments");
        // cur segment overlaps the new segment
        // adjust the cur segment and merge all the
        // overlapping segments
        if (compare_seq_numbers(seq, segments_[cur].start) < 0)
            segments_[cur].start = seq;

        if (compare_seq_numbers(end, segments_[cur].end) > 0)
            segments_[cur].end = end;

        // merge all overlaping segments
        int next = cur + 1;
        while(next < num_segments_ &&
              compare_seq_numbers(segments_[cur].end, segments_[next].start) >= 0) {
            if (compare_seq_numbers(segments_[next].end, segments_[cur].end) > 0) {
                segments_[cur].end = segments_[next].end;
            }
            next++;
        }

        // move the rest of the segments
        move_segments(next, cur + 1 - next);
    }

    /* copy payload and call the handler */
    memcpy(buff_ + (seq - cur_seq_), payload, payload_len);
    free_buff_size_ -= ((seq - cur_seq_) + payload_len);

    // Invoke handler if we havent already
    if (data_handler_invoked == false && (segments_[0].start == cur_seq_) &&
        (segments_[0].end - segments_[0].start) > reassembled_payload) {
        size_t processed = data_handler_(handler_ctx_, buff_, segments_[0].end - segments_[0].start);

        SDK_ASSERT_RETURN(processed <= (segments_[0].end - segments_[0].start), HAL_RET_INVALID_ARG);

        if (processed > 0) {
            segments_[0].start += processed;

            // drop the first segment if it is processed completely
            if (segments_[0].start == segments_[0].end) {
                move_segments(1, -1);
            }

            // Move the payload of rest of the segments to the begining
            for (int i = 0; i < num_segments_; i++) {
                uint8_t *old_loc = buff_ + (segments_[i].start - cur_seq_);
                uint8_t *new_loc = old_loc - processed;

                memmove(new_loc, old_loc,  segments_[i].end - segments_[i].start);
            }

            cur_seq_ += processed;
            free_buff_size_ += processed;
        }
    }
  

    return HAL_RET_OK;
}

} // namespace alg_utils
} // namespace plugins
} // namespace hal
