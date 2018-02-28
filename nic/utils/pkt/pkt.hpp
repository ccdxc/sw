// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef    __PKT_HPP__
#define    __PKT_HPP__

namespace hal {
namespace utils {

hal_ret_t tcp_rst_from_syn_pkt(uint8_t *pkt, uint16_t l2_offset,
                               uint16_t l3_offset, uint16_t l4_offset)

}    // namespace hal
}    // utils

#endif    // __PKT_HPP__

