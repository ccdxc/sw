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
tcp_buffer_t::factory (uint32_t seq_start, void *handler_ctx, 
                       data_handler_t handler, tcp_buffer_slab_t slab_info)
{
    tcp_buffer_t *entry = (tcp_buffer_t *)slab_->alloc();

    entry->buff_ = NULL;
    entry->cur_seq_ = seq_start;
    entry->end_buff_seq_ = entry->cur_seq_;
    entry->buff_size_ = 0;
    entry->segments_ = NULL;
    entry->num_segments_ = 0;
    entry->data_handler_ = handler;
    entry->handler_ctx_ = handler_ctx;
    entry->cur_slab_ = 0;
    memcpy(entry->buff_slabs_, slab_info, sizeof(tcp_buffer_slab_t));

    HAL_TRACE_DEBUG("Init TCP buffer with seq_start: {} data handler: {:p} handle_ctx: {:p} slab_2k: {:p}"
                    "slab_4k: {:p} slab_8k {:p} slab_16k{:p}",
                     entry->cur_seq_, (void *)entry->data_handler_, (void *)entry->handler_ctx_, 
                     (void *)entry->buff_slabs_[0], (void *)entry->buff_slabs_[1], 
                     (void *)entry->buff_slabs_[2], (void *)entry->buff_slabs_[3]);

    return entry;
}

//------------------------------------------------------------------------
// Alloc/Initialize the tcp buffer
//------------------------------------------------------------------------
tcp_buffer_t *
tcp_buffer_t::factory (const TCPBuffer &req, void *handler_ctx, data_handler_t handler,
                       tcp_buffer_slab_t slab_info)
{
    tcp_buffer_t *entry = (tcp_buffer_t *)slab_->alloc();

    memcpy(entry->buff_slabs_, slab_info, sizeof(tcp_buffer_slab_t));

    entry->tcp_buff_from_proto(req);
    entry->data_handler_ = handler;
    entry->handler_ctx_  = handler_ctx;

    return entry;
}

//------------------------------------------------------------------------
// Free the entry
//------------------------------------------------------------------------
void tcp_buffer_t::free ()
{
    if (buff_) {
        buff_slabs_[cur_slab_]->free(buff_); 
    }
    if (segments_) {
        HAL_FREE(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF, segments_);
    }

    slab_->free(this);
}

hal_ret_t
tcp_buffer_t::tcp_buff_to_proto (TCPBuffer *rsp)
{
    rsp->set_current_seq(cur_seq_);
    rsp->set_end_buff_seq(end_buff_seq_);
    rsp->set_buff_size(buff_size_);
    rsp->set_num_segments(num_segments_);
    rsp->set_current_slab(cur_slab_);

    for (int i = 0; i < num_segments_; i++) {
        auto seg = rsp->add_segments();
        seg->set_start(segments_[i].start);
        seg->set_end(segments_[i].end);
    }

    if (buff_size_) {
        rsp->set_payload(buff_, buff_size_);
    }

    return HAL_RET_OK;
}

hal_ret_t
tcp_buffer_t::tcp_buff_from_proto (const TCPBuffer &req)
{
    cur_seq_      = req.current_seq();
    end_buff_seq_ = req.end_buff_seq();
    buff_size_    = req.buff_size();
    num_segments_ = req.num_segments();
    segments_     = NULL;
    cur_slab_     = req.current_slab();

    if (buff_size_) {
        buff_ = (uint8_t *)buff_slabs_[cur_slab_]->alloc();
        if (req.payload().size()) {
            memcpy(buff_, req.payload().c_str(), buff_size_);
        }
    }

    if (num_segments_) {
        uint32_t segment_sz = sizeof(segment_t) * MAX_SEGMENTS;

        segments_ = (segment_t *)HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                            segment_sz);
        bzero(segments_, segment_sz);

        for (int i = 0; i < num_segments_; i++) {
            segments_[i].start = req.segments(i).start();
            segments_[i].end   = req.segments(i).end();
        }
    }
    HAL_TRACE_DEBUG("Init TCP buffer cur_seq:{} end_seq:{}, sz:{}, seg:{}, slab:{}, psz:{}",
                    cur_seq_, end_buff_seq_, buff_size_, num_segments_, cur_slab_,
                    req.payload().size());
    return HAL_RET_OK;
}

//------------------------------------------------------------------------
// Helper to insert the segment given the FTE context and data handler
//------------------------------------------------------------------------
hal_ret_t
tcp_buffer_t::insert_segment (fte::ctx_t &ctx, data_handler_t handler)
{
    uint32_t seq = htonl(ctx.cpu_rxhdr()->tcp_seq_num);
    uint8_t  *pkt = ctx.pkt();

    handler_ctx_ = (void *)std::addressof(ctx);
    data_handler_ = handler;
    HAL_TRACE_DEBUG("Packet len: {} payload offset: {} data handler: {}", ctx.pkt_len(),
                                ctx.cpu_rxhdr()->payload_offset, (void *)data_handler_);
    HAL_TRACE_DEBUG("seq: {} curr_seq: {} payload_len: {}", seq, cur_seq_, ctx.payload_len());
    return insert_segment(seq, &pkt[ctx.cpu_rxhdr()->payload_offset], ctx.payload_len());
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
        HAL_TRACE_VERBOSE("Calling data handler1");
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
    }

    uint32_t  buff_sz_reqd = (end-cur_seq_);
    HAL_TRACE_VERBOSE("Curr seq: {} end_buff_seq_: {} buff_sz_reqd: {}", 
                      cur_seq_, end_buff_seq_, buff_sz_reqd);
    // Allocate/expand the buffer
    if (buff_size_ == 0) {
        uint32_t segment_sz = sizeof(segment_t) * MAX_SEGMENTS;
        // Set the buff size to minimum required, we increase as we go
        uint32_t size = MIN_BUFF_SIZE;
        while (buff_sz_reqd > size && size <= MAX_BUFF_SIZE) {
            size = 2*size; cur_slab_++;
        }

        // We hit the max. Return
        if (size > MAX_BUFF_SIZE) {
            cur_slab_ = 0;
            HAL_TRACE_ERR("Max buffer size hit -- bailing");
            return HAL_RET_OOB;
        }

        buff_size_ = size;
        buff_ = (uint8_t *)buff_slabs_[cur_slab_]->alloc();
        SDK_ASSERT_RETURN((buff_ != NULL), HAL_RET_OOB);
        end_buff_seq_ = cur_seq_ + size;

        // first segment to store, allocate buffer
        segments_ = (segment_t *) HAL_MALLOC(hal::HAL_MEM_ALLOC_TCP_REASSEMBLY_BUFF,
                                             segment_sz);
        bzero(segments_, segment_sz);
        HAL_TRACE_DEBUG("Allocate buffer of size: {} current_slab: {}", buff_size_, cur_slab_);
    } else if (end >= end_buff_seq_) {
        uint32_t new_buff_sz = buff_size_, old_slab = cur_slab_;
        while (buff_sz_reqd > new_buff_sz && new_buff_sz <= MAX_BUFF_SIZE) {
            new_buff_sz = 2*new_buff_sz; cur_slab_++;
        }

        // We hit the max. Return
        if (new_buff_sz > MAX_BUFF_SIZE) {
            cur_slab_ = old_slab;
            HAL_TRACE_ERR("Payload exceeds the max buff size");
            return HAL_RET_OOB;
        }
 
        // expand the size of buffer
        uint8_t *new_buff = (uint8_t *)buff_slabs_[cur_slab_]->alloc();
        memcpy(new_buff, buff_, buff_size_);
        buff_slabs_[old_slab]->free(buff_); 

        // Set the buff size to reflect the new buffer
        buff_ = new_buff;
        buff_size_ = new_buff_sz;
        end_buff_seq_ = cur_seq_ +  buff_size_;
        HAL_TRACE_DEBUG("Allocated memory of size: {} end buffer seq: {} cur_slab_: {}", 
                        buff_size_, end_buff_seq_, cur_slab_);
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
            end_buff_seq_ = cur_seq_ + buff_size_;
        }
    }


    return HAL_RET_OK;
}

} // namespace alg_utils
} // namespace plugins
} // namespace hal
