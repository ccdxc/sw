//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/base.hpp"
#include "nic/include/hal_mem.hpp"
#include "lib/slab/slab.hpp"
#include "nic/fte/fte_ctx.hpp"

using namespace fte;
using session::TCPBuffer;
using session::TCPSegment;

namespace hal {
namespace plugins {
namespace alg_utils {

typedef slab* (tcp_buffer_slab_t) [4];

using sdk::lib::slab;

typedef class tcp_buffer_t tcp_buffer;

class tcp_buffer_t {
public:
    typedef size_t (*data_handler_t)(void *ctx, uint8_t *data, size_t len);

    static tcp_buffer_t *factory(uint32_t seq_start, void *handler_ctx,
                                 data_handler_t handler, tcp_buffer_slab_t slab_info);
    static tcp_buffer_t *factory (const TCPBuffer &req, void *handler_ctx,
                                  data_handler_t handler, tcp_buffer_slab_t slab_info);
    void free();

    hal_ret_t insert_segment(uint32_t seq, uint8_t *payload, size_t payload_len);
    hal_ret_t insert_segment(fte::ctx_t &ctx, data_handler_t handler);

    hal_ret_t tcp_buff_to_proto(TCPBuffer *rsp);
    hal_ret_t tcp_buff_from_proto (const TCPBuffer &req);
    tcp_buffer_t(const tcp_buffer_t&) = delete;
    tcp_buffer_t() = delete;

private:

    static const int MAX_SEGMENTS = 32;
    static const int MIN_BUFF_SIZE = 2048;
    static const int MAX_BUFF_SIZE = 16*1024;

    // As defined by RFC 1982 - 2 ^ (SERIAL_BITS - 1)
    static const uint32_t seq_number_diff = 2147483648U;

    // slab for tcp_buffer_t
    static slab *slab_;

    // Compares sequence numbers as defined by RFC 1982.
    int compare_seq_numbers(uint32_t seq1, uint32_t seq2)
    {
        if (seq1 == seq2) {
            return 0;
        }
        if (seq1 < seq2) {
            return (seq2 - seq1 < seq_number_diff) ? -1 : 1;
        }
        else {
            return (seq1 - seq2 > seq_number_diff) ? -1 : 1;
        }
    }

    void move_segments(int start, int offset)
    {
        if (offset) {
            // drop the last segments if it exceeds max size
            if (num_segments_ + offset >= MAX_SEGMENTS) {
                HAL_TRACE_ERR("alg_utils: max segments reached - dropping last segment");
                num_segments_ =  MAX_SEGMENTS - offset - 1;
            }
            if (start < num_segments_) {
                memmove(&segments_[start+offset], &segments_[start],
                        sizeof(segment_t)*(num_segments_-start));
            }
            num_segments_ += offset;
        }
    }

    struct segment_t {
        uint32_t start;
        uint32_t end;
    } __PACK__ ;

    uint8_t        *buff_;
    uint32_t       end_buff_seq_;
    size_t         buff_size_;
    uint32_t       cur_seq_;
    segment_t      *segments_;
    uint8_t        num_segments_;
    data_handler_t data_handler_;
    void           *handler_ctx_;
    uint8_t        cur_slab_;
    tcp_buffer_slab_t buff_slabs_;
};

} // namespace alg_utils
} // namespace plugins
} // namespace hal
