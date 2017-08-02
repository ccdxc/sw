/* 
 * ----------------------------------------------------------------------------
 *
 * interface_api.hpp
 *
 * Interface APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __L2SEGMENT_API_HPP__
#define __L2SEGMENT_API_HPP__

namespace hal {

void *l2seg_get_pd(l2seg_t *seg);
l2seg_id_t l2seg_get_l2seg_id(l2seg_t *seg);
types::encapType l2seg_get_acc_encap_type(l2seg_t *seg);
uint32_t l2seg_get_acc_encap_val(l2seg_t *seg);
types::encapType l2seg_get_fab_encap_type(l2seg_t *seg);
uint32_t l2seg_get_fab_encap_val(l2seg_t *seg);
} // namespace hal


#endif // __L2SEGMENT_API_HPP__
