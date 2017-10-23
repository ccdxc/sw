#include "nic/include/fte_db.hpp"

namespace fte {

alg_entry_t *alloc_and_insert_alg_entry(alg_entry_t *entry);
void alg_completion_hdlr (fte::ctx_t& ctx, bool status);

}
