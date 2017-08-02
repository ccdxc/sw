#include "if_pd_utils.hpp"
#include "lif_pd.hpp"
#include "uplinkif_pd.hpp"
#include <interface_api.hpp>

using namespace hal;

namespace hal {
namespace pd {

uint32_t
if_get_hw_lif_id(if_t *pi_if) 
{
    lif_t           *pi_lif;
    pd_lif_t        *pd_lif;
    pd_uplinkif_t   *pd_upif;
    intf::IfType    if_type;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT(pi_lif != NULL);

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            HAL_ASSERT(pi_lif != NULL);

            return pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            return pd_upif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return 0;
}

uint32_t
if_get_encap_vlan(if_t *pi_if, l2seg_t *pi_l2seg)
{
    intf::IfType    if_type;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            return pi_if->encap_vlan;
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            // Get fabric encap from l2seg
            if (pi_l2seg->fabric_encap.type == types::ENCAP_TYPE_DOT1Q) {
                return pi_l2seg->fabric_encap.val;
            } else {
                // TODO: Handle if its VXLAN
            }
            break;
        case intf::IF_TYPE_TUNNEL:
            // TODO: Handle Tunnels
            break;
        default:
            HAL_ASSERT(0);
    }

    return 0;

}

}   // namespace pd
}   // namespace hal
