#include "directmap_entry.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

namespace hal {
namespace pd {
namespace utils {

// allocate a rw entry instance
static inline directmap_entry_t*
directmap_entry_alloc (void)
{
    directmap_entry_t       *rwe;

    rwe = (directmap_entry_t *)hal::pd::g_hal_state_pd->directmap_entry_slab()->alloc();
    if (rwe == NULL) {
        return NULL;
    }

    return rwe;
}

// initialize a rwe instance
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

// allocate and initialize a rw entry instance
directmap_entry_t *
directmap_entry_alloc_init (void)
{
    return directmap_entry_init(directmap_entry_alloc());
}

// free rw entry instance
hal_ret_t
directmap_entry_free (directmap_entry_t *rwe)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_DIRECTMAP_ENTRY, rwe);
    return HAL_RET_OK;
}

}
}
}



