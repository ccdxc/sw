/* ============================================================================
 |
 | DirectMapEntry
 | 
 |   - Represents an entry in DirectMap Table.
 |
 * ==========================================================================*/

#ifndef __DIRECTMAP_ENTRY_HPP__
#define __DIRECTMAP_ENTRY_HPP__

#include "nic/include/base.h"
#include "nic/sdk/include/ht.hpp"

using sdk::lib::ht_ctxt_t;


namespace hal {
namespace pd {
extern class hal_state_pd    *g_hal_state_pd;
namespace utils {


/** ---------------------------------------------------------------------------
 *  directmap entry structure
 *
 *      - Stores data 
 *      - hash table context
 *
 * ---------------------------------------------------------------------------*/
typedef struct directmap_entry_s {
    void            *data;
    uint16_t        len;
    uint32_t        index;
    uint32_t        ref_cnt;
    
    ht_ctxt_t       ht_ctxt;
} __PACK__ directmap_entry_t;

directmap_entry_t *directmap_entry_alloc_init (void);
hal_ret_t directmap_entry_free (directmap_entry_t *rwe);

}    // namespace utils
}    // namespace pd
}    // namespace hal

#endif // __DIRECTMAP_ENTRY_HPP__
