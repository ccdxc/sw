// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/pd/gft/pd_utils.hpp"
#include "lif_pd.hpp"
#include "enicif_pd.hpp"
#include "uplinkif_pd.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/p4/gft/include/defines.h"

using namespace hal;

namespace hal {
namespace pd {


//-----------------------------------------------------------------------------
// Given a PI If, get its hw lif id
//-----------------------------------------------------------------------------
hal_ret_t
if_get_hw_lif_id(if_t *pi_if, uint32_t *hw_lif_id) 
{
    lif_t           *pi_lif = NULL;
    pd_lif_t        *pd_lif = NULL;
    pd_uplinkif_t   *pd_upif = NULL;
    intf::IfType    if_type;

    HAL_ASSERT(pi_if != NULL);
    HAL_ASSERT(hw_lif_id != NULL);

    *hw_lif_id = INVALID_INDEXER_INDEX;

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT_GOTO(pi_lif != NULL, end); 

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            HAL_ASSERT_GOTO(pd_lif != NULL, end);

            *hw_lif_id =  pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            *hw_lif_id = pd_upif->hw_lif_id;
            break;
        default:
            HAL_ASSERT_GOTO(0, end); 
    }

end:
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// Check if gft table type is ingress
//-----------------------------------------------------------------------------
bool 
gft_match_prof_is_ingress(gft_table_type_t type)
{
    if (type == GFT_TABLE_TYPE_WILDCARD_INGRESS || 
        type == GFT_TABLE_TYPE_EXACT_MATCH_INGRESS) {
        return true;
    }

    return false;
}


}   // namespace pd
}   // namespace hal
