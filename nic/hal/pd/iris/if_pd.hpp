#ifndef __HAL_PD_IF_HPP__
#define __HAL_PD_IF_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

#define HAL_MAX_LPORTS        2048

struct pd_if_s {
} __PACK__;


hal_ret_t if_l2seg_get_encap_rewrite(if_t *pi_if, l2seg_t *pi_l2seg, uint32_t *encap_data,
                                     uint32_t *rewrite_idx, uint32_t *tnnl_rewrite_idx);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_IF_HPP__
