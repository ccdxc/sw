#ifndef CAP_AXI4_SLAVE_H
#define CAP_AXI4_SLAVE_H
#include "pen_axi4_slave.h"
#include "cap_axi_decoders.h"

using cap_axi4_slave_t = pen_axi4_slave_t<cap_axi_ar_bundle_t, cap_axi_r_bundle_t, cap_axi_aw_bundle_t, cap_axi_w_bundle_t, cap_axi_b_bundle_t>;
#endif
