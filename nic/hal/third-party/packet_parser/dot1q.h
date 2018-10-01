#ifndef __DOT1Q_H__
#define __DOT1Q_H__

#include "types.h"

struct dot1q {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    __u16 vlan_id_h:4,
          cfi:1,
          user_priority:3,
          vlan_id_l:8;
	u16 ether_type;
#elif __BYTE_ORDER == __BIG_ENDIAN
    __u16 user_priority:3,
          cfi:1,
          vlan_id:12;
	u16 ether_type;
#else
# error "Please fix endianness defines"
#endif
};

static inline u8 dot1q_user_priority(const struct dot1q *dot1q)
{
    return dot1q->user_priority;
}

static inline u8 dot1q_cfi(const struct dot1q *dot1q)
{
    return dot1q->cfi;
}

static inline u16 dot1q_vlan_id(const struct dot1q *dot1q)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return dot1q->vlan_id_l | (dot1q->vlan_id_h << 8);

#elif __BYTE_ORDER == __BIG_ENDIAN
    return dot1q->vlan_id;
#else
# error "Please fix endianness defines"
#endif

}

#endif /* __DOT1Q_H__ */
