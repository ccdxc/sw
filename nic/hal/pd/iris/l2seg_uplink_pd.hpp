#ifndef __HAL_PD_L2SEG_UPLINK_HPP__
#define __HAL_PD_L2SEG_UPLINK_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

hal_ret_t l2seg_uplink_program_hw(pd_l2seg_uplink_args_t *args);
hal_ret_t l2set_uplink_pgm_input_properties_tbl(pd_l2seg_uplink_args_t *args, 
                                                nwsec_profile_t *nwsec_prof);
hal_ret_t l2set_uplink_upd_input_properties_tbl (pd_l2seg_uplink_args_t *args,
                                                 nwsec_profile_t *nwsec_prof);

} // namespace pd
} // namespace hal


#endif    // __HAL_PD_L2SEG_UPLINK_HPP__

