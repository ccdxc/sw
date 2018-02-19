//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// directmap entry
//  - represents an entry in directmap table
//  - used when sharing is enabled for directmap
//------------------------------------------------------------------------------
#ifndef __SDK_DIRECTMAP_ENTRY_HPP__
#define __SDK_DIRECTMAP_ENTRY_HPP__

#include "sdk/base.hpp"
#include "sdk/mem.hpp"
#include "sdk/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace sdk {
namespace table {

typedef struct directmap_entry_s {
    void            *data;
    uint16_t        len;
    uint32_t        index;
    uint32_t        ref_cnt;
    
    ht_ctxt_t       ht_ctxt;
} __PACK__ directmap_entry_t;

directmap_entry_t *directmap_entry_alloc_init (void);
sdk_ret_t directmap_entry_free (directmap_entry_t *rwe);

}    // namespace table
}    // namespace sdk

#endif // __SDK_DIRECTMAP_ENTRY_HPP__
