//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// LPM API implementation provided by apulu pipeline
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/lpm/lpm_priv.hpp"

uint32_t
lpm_entry_key_size (itree_type_t tree_type)
{
    return 0;
}

uint32_t
lpm_keys_per_table (itree_type_t tree_type)
{
    return 0;
}

uint32_t
lpm_stages (itree_type_t tree_type, uint32_t num_intrvls)
{
    return 0;
}

sdk_ret_t
lpm_add_key_to_stage (itree_type_t tree_type, lpm_stage_info_t *stage,
                      lpm_inode_t *lpm_inode)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
lpm_add_key_to_last_stage (itree_type_t tree_type,
                           lpm_stage_info_t *stage,
                           lpm_inode_t *lpm_inode)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
lpm_set_default_data (itree_type_t tree_type, lpm_stage_info_t *stage,
                      uint32_t default_data)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
lpm_write_stage_table (itree_type_t tree_type, lpm_stage_info_t *stage)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
lpm_write_last_stage_table (itree_type_t tree_type,
                            lpm_stage_info_t *stage)
{
    return SDK_RET_INVALID_OP;
}
