#ifndef _LKLSHIM_HPP_
#define _LKLSHIM_HPP_

#include "nic/include/base.hpp"
#include "lib/slab/slab.hpp"
#include "lib/list/list.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "lib/ht/ht.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"

#include <netinet/in.h>
#include <memory>
#include <map>

extern "C" {
#include <netinet/in.h>
}
using sdk::lib::ht_ctxt_t;
using sdk::lib::slab;

namespace hal {

bool lklshim_release_client_syn(uint16_t qid) ;
bool lklshim_release_client_syn6(uint16_t qid) ;
} //namespace hal

#endif // _LKLSHIM_HPP_
