#ifndef __HAL_PD_IF_HPP__
#define __HAL_PD_IF_HPP__


#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "gen/p4gen/p4/include/p4pd.h"

namespace hal {
namespace pd {

#define HAL_MAX_LPORTS        2048

struct pd_if_s {
} __PACK__;


hal_ret_t if_l2seg_get_encap_rewrite(if_t *pi_if, l2seg_t *pi_l2seg, uint32_t *encap_data,
                                     uint32_t *rewrite_idx, uint32_t *tnnl_rewrite_idx);
hal_ret_t if_l2seg_get_multicast_rewrite_data(if_t *pi_if, l2seg_t *pi_l2seg, lif_t *enic_lif, 
                                              p4_replication_data_t *data);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_IF_HPP__
