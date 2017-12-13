
#ifndef _ETH_HPP_
#define _ETH_HPP_

#include <memory>
#include <map>
#include "nic/include/base.h"
#include "nic/hal/src/lif.hpp"

namespace hal {

    extern hal_ret_t eth_rss_init(
        uint32_t hw_lif_id, lif_rss_info_t *rss, lif_queue_info_t *qinfo);

}  // namespace hal

#endif // _ETH_HPP_
