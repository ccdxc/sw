#ifndef __L4_H__
#define __L4_H__

#include <base.hpp>

typedef struct port_range_s {
    uint16_t        port_lo;
    uint16_t        port_hi;
} __PACK__ port_range_t;

#endif    // __L4_H__

