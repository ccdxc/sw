#include "nic/include/base.h"
#include "nic/include/ip.h"
#include "nic/sdk/include/ht.hpp"
#include "nic/fte/fte.hpp"
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include "nic/hal/plugins/eplearn/common/trans.hpp"

namespace hal {
namespace eplearn {

void *trans_get_ip_entry_key_func(void *entry) {
    HAL_ASSERT(entry != NULL);
    return (void *)(((trans_t *)entry)->ip_entry_key_ptr());
}

uint32_t trans_compute_ip_entry_hash_func(void *key, uint32_t ht_size) {
    return sdk::lib::hash_algo::fnv_hash(key,
                                         sizeof(trans_ip_entry_key_t)) % ht_size;
}

bool trans_compare_ip_entry_key_func(void *key1, void *key2) {
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (memcmp(key1, key2, sizeof(trans_ip_entry_key_t)) == 0) {
        return true;
    }
    return false;
}

}  //  namespace eplearn
}  //  namespace hal
