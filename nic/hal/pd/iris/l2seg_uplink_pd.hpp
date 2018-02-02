#ifndef __HAL_PD_L2SEG_UPLINK_HPP__
#define __HAL_PD_L2SEG_UPLINK_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

hal_ret_t l2seg_uplink_program_hw(pd_add_l2seg_uplink_args_t *args);
hal_ret_t l2seg_uplink_pgm_input_properties_tbl(pd_add_l2seg_uplink_args_t *args, 
                                                nwsec_profile_t *nwsec_prof);
hal_ret_t l2seg_uplink_upd_input_properties_tbl (pd_add_l2seg_uplink_args_t *args,
                                                 nwsec_profile_t *nwsec_prof);

hal_ret_t l2seg_uplink_deprogram_hw(pd_del_l2seg_uplink_args_t *args);
hal_ret_t 
l2seg_uplink_depgm_input_properties_tbl(pd_del_l2seg_uplink_args_t *args);
} // namespace pd
} // namespace hal


#endif    // __HAL_PD_L2SEG_UPLINK_HPP__

