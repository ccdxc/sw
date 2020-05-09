#include "egress.h"
#include "EGRESS_p.h"

struct clock_d  d;
struct phv_     p;

d = {
    //gettimeofday_d.multiplier_ns = 0x99999999;
    //gettimeofday_d.multiplier_ns = 0x133333332;
    gettimeofday_d.multiplier_ns = 0x266666668;
};

//r4 = 4167;
//r4 = 2083;
//r4 = 1042;
r4 = 0x3FFFFFFFFFF;
