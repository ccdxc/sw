
#include "../defines.h"

#define BIT(n)					(1 << n)

#define LG2_RX_DESC_SIZE        (4)
#define LG2_RX_CMPL_DESC_SIZE   (4)
#define LG2_RX_QSTATE_SIZE      (6)

#define  RSS_TYPE_NONE         	(0)

#define  RSS_TYPE_IPV4         	BIT(0)
#define  RSS_TYPE_IPV4_TCP     	BIT(1)
#define  RSS_TYPE_IPV4_UDP     	BIT(2)
#define  RSS_TYPE_IPV6         	BIT(3)
#define  RSS_TYPE_IPV6_TCP     	BIT(4)
#define  RSS_TYPE_IPV6_UDP     	BIT(5)
#define  RSS_TYPE_IPV6_EX      	BIT(6)
#define  RSS_TYPE_IPV6_TCP_EX  	BIT(7)
#define  RSS_TYPE_IPV6_UDP_EX  	BIT(8)

#define  RSS_NONE              (RSS_TYPE_NONE)
#define  RSS_IPV4              (RSS_TYPE_IPV4)
#define  RSS_IPV4_TCP          (RSS_TYPE_IPV4 | RSS_TYPE_IPV4_TCP)
#define  RSS_IPV4_UDP          (RSS_TYPE_IPV4 | RSS_TYPE_IPV4_UDP)
#define  RSS_IPV6              (RSS_TYPE_IPV6)
#define  RSS_IPV6_TCP          (RSS_TYPE_IPV6 | RSS_TYPE_IPV6_TCP)
#define  RSS_IPV6_UDP          (RSS_TYPE_IPV6 | RSS_TYPE_IPV6_UDP)
