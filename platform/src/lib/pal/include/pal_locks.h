#ifndef _PAL_LOCKS_H_
#define _PAL_LOCKS_H_

#include "pal_types.h"

/*
 * Locks are Co-Operative.
 *
 * It is expected that the developer uses these APIs for
 * process level synchronization to access PAL library.
 * Both Read locks (shareable by multiple applications) and
 * Write locks (Exclusive access) are provided.
 */
pal_lock_ret_t pal_wr_lock(pal_lock_id_t lock_id);

pal_lock_ret_t pal_rd_lock(pal_lock_id_t lock_id);

pal_lock_ret_t pal_wr_unlock(pal_lock_id_t lock_id);

pal_lock_ret_t pal_rd_unlock(pal_lock_id_t lock_id);

#endif

