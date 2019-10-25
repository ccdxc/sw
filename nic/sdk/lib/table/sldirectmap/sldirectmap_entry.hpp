//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// stateless directmap entry
//  - represents an entry in directmap table
//  - used when sharing is enabled for directmap
//------------------------------------------------------------------------------

#ifndef __SDK_LIB_TABLE_SLDIRECTMAP_ENTRY_HPP__
#define __SDK_LIB_TABLE_SLDIRECTMAP_ENTRY_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/ht/ht.hpp"

using sdk::lib::ht_ctxt_t;

namespace sdk {
namespace table {

typedef struct sldirectmap_entry_s {
    void            *data;
    uint16_t        len;
    uint32_t        index;
    uint32_t        ref_cnt;

    ht_ctxt_t       ht_ctxt;
} __PACK__ sldirectmap_entry_t;

sldirectmap_entry_t *sldirectmap_entry_alloc_init (void);
sdk_ret_t sldirectmap_entry_free (sldirectmap_entry_t *rwe);

}    // namespace table
}    // namespace sdk

#endif    // __SDK_LIB_TABLE_SLDIRECTMAP_ENTRY_HPP__

