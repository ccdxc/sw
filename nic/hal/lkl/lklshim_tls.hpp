#ifndef _LKLSHIM_HPP_
#define _LKLSHIM_HPP_

#include "nic/include/base.h"
#include "nic/utils/slab/slab.hpp"
#include "nic/include/list.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
//#include "nic/hal/src/tcpcb.hpp"
//#include "nic/hal/src/tlscb.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/hal/src/session.hpp"

#include <netinet/in.h>
#include <memory>
#include <map>
//#include <openssl/crypto.h>
//#include <openssl/x509.h>
//#include <openssl/pem.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>

extern "C" {
#include <netinet/in.h>
}
using hal::utils::ht_ctxt_t;
using hal::utils::slab;

namespace hal {

bool lklshim_release_client_syn(uint16_t qid) ;
} //namespace hal

#endif // _LKLSHIM_HPP_
