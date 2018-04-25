//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "tcp_buffer.hpp"

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
tcp_buffer_t::factory(uint32_t seq_start, void *handler_ctx, data_handler_t handler)
{
    tcp_buffer_t *entry = (tcp_buffer_t *)slab_->alloc();

    entry->buff_ = NULL;
    entry->buff_size_ = 0;
    entry->cur_seq_ = seq_start;
    entry->segments_ = NULL;
    entry->num_segments_ = 0;
    entry->data_handler_ = handler;
    entry->handler_ctx_ = handler_ctx;

    return entry;
}

//------------------------------------------------------------------------
// Free the entry
//------------------------------------------------------------------------
void tcp_buffer_t::free()
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
// Insert new segment into the buffer, calls the handler with the
// reassembled data.
//------------------------------------------------------------------------
hal_ret_t
tcp_buffer_t::insert_segment(uint32_t seq, uint8_t *payload, size_t payload_len)
{
    uint32_t end = seq + payload_len;

    // If the end of the payload ends before our current sequence number,
    // its old duplicate data
    if (compare_seq_numbers(end, cur_seq_) <= 0) {
        return HAL_RET_OK;
    }

    // If it starts before current sequence number, slice it
    if (compare_seq_numbers(seq, cur_seq_) < 0) {
        uint32_t diff = cur_seq_ - seq;
        seq = cur_seq_;
        payload += diff;
        payload_len -= diff;
    }

    // If the segment matches cur seq and doesn't intersect with any of the
    // stored segments, call the handler without doing a copy
    if ((seq == cur_seq_) &&
        (num_segments_ <= 0 || compare_seq_numbers(end, segments_[0].start) < 0)) {
        size_t processed = data_handler_(handler_ctx_, payload, payload_len);
        HAL_ASSERT_RETURN(processed <= payload_len, HAL_RET_INVALID_ARG);
        cur_seq_ += processed;
        if (processed == payload_len) {
            // all payload is processed, return
            return HAL_RET_OK;
        }
        seq += processed;
        payload += processed;
        payload_len -= processed;
    }

    // Allocate/expand the buffer
    if ((end - cur_seq_) > buff_size_) {
        if (buff_size_ == 0) {
            // first segment to store, allocate buffer
            buff_ = (uint8_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, MIN_BUFF_SIZE);
            buff_size_ = MIN_BUFF_SIZE;
            segments_ = (segment_t *) HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                                 sizeof(segment_t) * MAX_SEGMENTS);
        } else if (2*buff_size_ < MAX_BUFF_SIZE) {
            // expand the size of buffer
            uint8_t *new_buff = (uint8_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                                      2*buff_size_);
            memcpy(new_buff, buff_, buff_size_);
            HAL_FREE(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, buff_);

            buff_ = new_buff;
            buff_size_ *= 2;
        } else {
            HAL_TRACE_ERR("alg_utils::payload execeeds the max buffer size - truncating");
            end = buff_size_ - cur_seq_;
            payload_len = end - seq;
        }
    }


    // Remember the size of reassembled payload (handler is called when there is new data)
    size_t reassembled_payload = (segments_[0].start == cur_seq_) ?
        (segments_[0].end - segments_[0].start) : 0;

    // insert the segment in correct order
    int cur;

    // skip all segments which end before the new segment
    for (cur = 0; cur < num_segments_; cur++) {
        if (compare_seq_numbers(seq, segments_[cur].end) <= 0) {
            break;
        }
    }

    if (cur >= num_segments_) {
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
        // new segment ends before the cur segment.
        // insert the new segment before the cur segment by moving
        // the rest of the segments
        move_segments(cur, 1);
        segments_[cur].start = seq;
        segments_[cur].end = end;
    } else {
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

    if ((segments_[0].start == cur_seq_) &&
        (segments_[0].end - segments_[0].start) > reassembled_payload) {
        size_t processed = data_handler_(handler_ctx_, buff_, segments_[0].end - segments_[0].start);

        HAL_ASSERT_RETURN(processed <= (segments_[0].end - segments_[0].start), HAL_RET_INVALID_ARG);

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
        }
    }

    return HAL_RET_OK;
}

} // namespace alg_utils
} // namespace plugins
} // namespace hal
