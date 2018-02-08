
#include "../defines.h"

#define LG2_RX_DESC_SIZE        (4)
#define LG2_RX_CMPL_DESC_SIZE   (4)
#define LG2_RX_QSTATE_SIZE      (6)

// RSS type flags
#define  RSS_TYPE_NONE         (0)
#define  RSS_TYPE_IPV4         (1)
#define  RSS_TYPE_IPV4_TCP     (2)
#define  RSS_TYPE_IPV4_UDP     (4)
#define  RSS_TYPE_IPV6         (8)
#define  RSS_TYPE_IPV6_TCP     (16)
#define  RSS_TYPE_IPV6_UDP     (32)
#define  RSS_TYPE_IPV6_EX      (64)
#define  RSS_TYPE_IPV6_TCP_EX  (128)
#define  RSS_TYPE_IPV6_UDP_EX  (256)

// RSS enum
#define  RSS_NONE              (RSS_TYPE_NONE)
#define  RSS_IPV4              (RSS_TYPE_IPV4)
#define  RSS_IPV4_TCP          (RSS_TYPE_IPV4 | RSS_TYPE_IPV4_TCP)
#define  RSS_IPV4_UDP          (RSS_TYPE_IPV4 | RSS_TYPE_IPV4_UDP)
#define  RSS_IPV6              (RSS_TYPE_IPV6)
#define  RSS_IPV6_TCP          (RSS_TYPE_IPV6 | RSS_TYPE_IPV6_TCP)
#define  RSS_IPV6_UDP          (RSS_TYPE_IPV6 | RSS_TYPE_IPV6_UDP)
