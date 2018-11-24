//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "directmap_entry.hpp"

namespace sdk {
namespace table {

//---------------------------------------------------------------------------
// allocate a directmap entry instance
//---------------------------------------------------------------------------
static inline directmap_entry_t *
directmap_entry_alloc (void)
{
    directmap_entry_t *dme;

    dme = (directmap_entry_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIB_DIRECT_MAP_ENTRY,
                                          sizeof(directmap_entry_t));
    if (dme == NULL) {
        return NULL;
    }

    return dme;
}

//---------------------------------------------------------------------------
// initialize a directmap entry instance
//---------------------------------------------------------------------------
directmap_entry_t *
directmap_entry_init (directmap_entry_t *dme)
{
    if (!dme) {
        return NULL;
    }
    memset(dme, 0, sizeof(directmap_entry_t));

    // initialize meta information
    dme->ht_ctxt.reset();

    return dme;
}

//---------------------------------------------------------------------------
// allocate and initialize a directmap entry instance
//---------------------------------------------------------------------------
directmap_entry_t *
directmap_entry_alloc_init (void)
{
    return directmap_entry_init(directmap_entry_alloc());
}

//---------------------------------------------------------------------------
// free directmap entry instance
//---------------------------------------------------------------------------
sdk_ret_t
directmap_entry_free (directmap_entry_t *dme)
{
    SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECT_MAP_ENTRY, dme);
    return SDK_RET_OK;
}

}   // namespace table
}   // namespace sdk

