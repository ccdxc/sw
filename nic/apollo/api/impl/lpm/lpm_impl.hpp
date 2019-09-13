//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// APIs declared in this file need to be implemented by the pipeline in a
/// pipeline specific manner
///
//----------------------------------------------------------------------------

#ifndef __LPM_IMPL_HPP__
#define __LPM_IMPL_HPP__

uint32_t lpm_entry_key_size(itree_type_t tree_type);
uint32_t lpm_keys_per_table(itree_type_t tree_type);
uint32_t lpm_stages(itree_type_t tree_type, uint32_t num_intrvls);
sdk_ret_t lpm_add_key_to_stage(itree_type_t tree_type, lpm_stage_info_t *stage,
                               lpm_inode_t *lpm_inode);
sdk_ret_t lpm_add_key_to_last_stage(itree_type_t tree_type,
                                    lpm_stage_info_t *stage,
                                    lpm_inode_t *lpm_inode);
sdk_ret_t lpm_set_default_data(itree_type_t tree_type, lpm_stage_info_t *stage,
                               uint32_t default_data);
sdk_ret_t lpm_write_stage_table(itree_type_t tree_type,
                                lpm_stage_info_t *stage);
sdk_ret_t lpm_write_last_stage_table(itree_type_t tree_type,
                                     lpm_stage_info_t *stage);


#endif    // __LPM_IMPL_HPP__
