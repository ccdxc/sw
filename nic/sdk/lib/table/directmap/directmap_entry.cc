//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "directmap_entry.hpp"

namespace sdk {
namespace table {

//---------------------------------------------------------------------------
// allocate a rw entry instance
//---------------------------------------------------------------------------
static inline directmap_entry_t*
directmap_entry_alloc (void)
{
    directmap_entry_t *rwe;

    rwe = (directmap_entry_t *)SDK_CALLOC(SDK_MEM_ALLOC_ID_DM_ENTRY,
                                          sizeof(directmap_entry_t));
    if (rwe == NULL) {
        return NULL;
    }

    return rwe;
}

//---------------------------------------------------------------------------
// initialize a rwe instance
//---------------------------------------------------------------------------
directmap_entry_t *
directmap_entry_init (directmap_entry_t *rwe)
{
    if (!rwe) {
        return NULL;
    }
    memset(rwe, 0, sizeof(directmap_entry_t));

    // initialize meta information
    rwe->ht_ctxt.reset();

    return rwe;
}

//---------------------------------------------------------------------------
// allocate and initialize a rw entry instance
//---------------------------------------------------------------------------
directmap_entry_t *
directmap_entry_alloc_init (void)
{
    return directmap_entry_init(directmap_entry_alloc());
}

//---------------------------------------------------------------------------
// free rw entry instance
//---------------------------------------------------------------------------
sdk_ret_t
directmap_entry_free (directmap_entry_t *rwe)
{
    SDK_FREE(SDK_MEM_ALLOC_ID_DM_ENTRY, rwe);
    return SDK_RET_OK;
}

}   // namespace table
}   // namespace sdk

