#include <l2segment.hpp>
#include <l2segment_api.hpp>

namespace hal {


// ----------------------------------------------------------------------------
// Returns L2 segment PD
// ----------------------------------------------------------------------------
void *
l2seg_get_pd(l2seg_t *seg)
{
    return seg->pd;
}

// ----------------------------------------------------------------------------
// Returns L2 segment ID
// ----------------------------------------------------------------------------
l2seg_id_t
l2seg_get_l2seg_id(l2seg_t *seg)
{
    return seg->seg_id;
}

// ----------------------------------------------------------------------------
// Returns access encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_acc_encap_type(l2seg_t *seg)
{
    return seg->access_encap.type;
}

// ----------------------------------------------------------------------------
// Returns access encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_acc_encap_val(l2seg_t *seg)
{
    return seg->access_encap.val;
}

// ----------------------------------------------------------------------------
// Returns fabric encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_fab_encap_type(l2seg_t *seg)
{
    return seg->fabric_encap.type;
}

// ----------------------------------------------------------------------------
// Returns fabric encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_fab_encap_val(l2seg_t *seg)
{
    return seg->fabric_encap.val;
}

}
