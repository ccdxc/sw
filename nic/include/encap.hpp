#ifndef __ENCAP_HPP__
#define __ENCAP_HPP__

#include "nic/include/base.h"
#include "nic/gen/proto/hal/types.pb.h"

// possible encap types
typedef enum encap_type_e {
    ENCAP_TYPE_NONE,
    ENCAP_TYPE_DOT1Q,
    ENCAP_TYPE_VXLAN,
    ENCAP_TYPE_IPINIP,
    ENCAP_TYPE_IPSEC,
    ENCAP_TYPE_MAX,
} encap_type_t;

// encap is defined by its type and value
typedef struct encap_s {
    types::encapType    type;
    uint32_t            val;
} __PACK__ encap_t;

#endif    // __ENCAP_HPP__

