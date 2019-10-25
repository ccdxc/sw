//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "sldirectmap_entry.hpp"

namespace sdk {
namespace table {

//---------------------------------------------------------------------------
// allocate a sldirectmap entry instance
//---------------------------------------------------------------------------
static inline sldirectmap_entry_t *
sldirectmap_entry_alloc (void)
{
    sldirectmap_entry_t *dme;

    dme = (sldirectmap_entry_t *)SDK_CALLOC(SDK_MEM_ALLOC_LIB_DIRECT_MAP_ENTRY,
                                          sizeof(sldirectmap_entry_t));
    if (dme == NULL) {
        return NULL;
    }

    return dme;
}

//---------------------------------------------------------------------------
// initialize a sldirectmap entry instance
//---------------------------------------------------------------------------
sldirectmap_entry_t *
sldirectmap_entry_init (sldirectmap_entry_t *dme)
{
    if (!dme) {
        return NULL;
    }
    memset(dme, 0, sizeof(sldirectmap_entry_t));

    // initialize meta information
    dme->ht_ctxt.reset();

    return dme;
}

//---------------------------------------------------------------------------
// allocate and initialize a sldirectmap entry instance
//---------------------------------------------------------------------------
sldirectmap_entry_t *
sldirectmap_entry_alloc_init (void)
{
    return sldirectmap_entry_init(sldirectmap_entry_alloc());
}

//---------------------------------------------------------------------------
// free sldirectmap entry instance
//---------------------------------------------------------------------------
sdk_ret_t
sldirectmap_entry_free (sldirectmap_entry_t *dme)
{
    SDK_FREE(SDK_MEM_ALLOC_LIB_DIRECT_MAP_ENTRY, dme);
    return SDK_RET_OK;
}

}   // namespace table
}   // namespace sdk

