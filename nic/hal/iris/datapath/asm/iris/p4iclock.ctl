#include "ingress.h"
#include "INGRESS_p.h"

struct p4i_clock_d  d;
struct phv_         p;

d = {
    gettimeofday_ms_d.multiplier_ms = 2638829;
    //gettimeofday_ms_d.multiplier_ms = 5277656;
    //gettimeofday_ms_d.multiplier_ms = 10555312;
};

r4 = 0x3FFFFFFFFFF;
