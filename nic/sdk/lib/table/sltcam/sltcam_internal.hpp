//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// stateless tcam table management library
//------------------------------------------------------------------------------
#ifndef __SDK_SLTCAM_INTERNAL_HPP__
#define __SDK_SLTCAM_INTERNAL_HPP__

namespace sdk {
namespace table {
namespace sltcam {

typedef union handle_ {
    struct {
        uint32_t index : 32;
        uint8_t valid : 1;
        uint32_t spare : 31;
    };
    uint64_t value;
} __attribute__((__packed__)) handle_t;

} // namespace sltcam
} // namespace table
} // namespace sdk

#endif    // __SDK_TCAM_INTERNAL_HPP__
