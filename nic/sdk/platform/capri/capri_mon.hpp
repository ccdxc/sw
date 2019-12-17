// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
/*
 * capri_mon.hpp
 * Rahul Shekhar (Pensando Systems)
 */

#ifndef __CAPRI_MON_HPP__
#define __CAPRI_MON_HPP__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "asic/rw/asicrw.hpp"
#include "third-party/asic/capri/verif/apis/cap_freq_api.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/verif/apis/sdram_csr_ipxact.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_mc_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_dpp_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_dpr_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_pics_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_wa_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_te_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_mpu_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_hens_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_mpns_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_pbc_c_hdr.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_pbm_c_hdr.h"
#include "third-party/asic/capri/verif/apis/ns_soc_ip.h"

namespace sdk {
namespace platform {
namespace capri {

typedef struct hbmerrcause_s {
    uint32_t offset;
    std::string message;
} hbmerrcause_t;

#define MAX_CHANNEL   8
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)       (sizeof (a) / sizeof ((a)[0]))
#endif
#define CAP_NWL_ADDR(channel_num) (((channel_num * \
                    (CAP_ADDR_BASE_MC_MC_1_OFFSET - CAP_ADDR_BASE_MC_MC_0_OFFSET)) + \
                    CAP_ADDR_BASE_MC_MC_0_OFFSET) + CAP_MC_CSR_MCH_DHS_APB_BYTE_ADDRESS)

#define CAP_MC_ADDR(channel_num) ((channel_num * \
                    (CAP_ADDR_BASE_MC_MC_1_OFFSET - CAP_ADDR_BASE_MC_MC_0_OFFSET)) + \
                    CAP_ADDR_BASE_MC_MC_0_OFFSET)

#define MC_STA_OFFSET 4

#define NOC_REGISTER_RESET_VALUE 0x80010

pen_adjust_perf_status_t capri_adjust_perf(int chip_id, int inst_id,
                                          pen_adjust_index_t &idx,
                                          pen_adjust_perf_type_t perf_type);
void capri_set_half_clock(int chip_id, int inst_id);
sdk_ret_t capri_unravel_hbm_intrs(bool *iscattrip, bool *iseccerr, bool logging);

} // namespace capri
} // namespace platform
} // namespace sdk

#endif
