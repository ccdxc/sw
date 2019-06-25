//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_STRUCTS_HPP__
#define __FTLITE_STRUCTS_HPP__
namespace ftlite {
namespace internal {

struct base_entry_t {
    static uint32_t count;
public:
    uint32_t size() {
        return sizeof(*this);
    }
};

} // namespace ftlite
} // namespace internal
#endif
