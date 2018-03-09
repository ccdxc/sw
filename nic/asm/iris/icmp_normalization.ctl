#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct icmp_normalization_k k;
struct phv_                 p;

k = {
    icmp_typeCode                                   = 0x1001; // icmp_type = 8;
    l4_metadata_icmp_deprecated_msgs_drop           = 0x1;
    l4_metadata_icmp_redirect_msg_drop              = 0x1;
    l4_metadata_icmp_invalid_code_action            = 0x1;
};

p = {
    icmp_typeCode                                   = 0x0801;
};
