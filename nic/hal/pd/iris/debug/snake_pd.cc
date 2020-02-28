// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/pd_api.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/hal/pd/iris/debug/snake_pd.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/src/debug/snake.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_snake_test_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_snake_test_create_args_t *args = pd_func_args->pd_snake_test_create;
    snake_test_t *snake = args->snake;
    pd_snake_test_t *snake_pd;

    HAL_TRACE_DEBUG("PD Create");

    // allocate PD
    snake_pd = snake_test_pd_alloc_init();
    if (snake_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    link_pi_pd(snake_pd, snake);

    // program hw
    ret = pd_snake_test_program_hw(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program hw");
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        pd_snake_test_cleanup(snake_pd);
    }
    return ret;
}

hal_ret_t
pd_snake_test_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_snake_test_delete_args_t *args = pd_func_args->pd_snake_test_delete;
    pd_snake_test_t    *snake_test_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->snake != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->snake->pd != NULL), HAL_RET_INVALID_ARG);

    snake_test_pd = (pd_snake_test_t *)args->snake->pd;

    // deprogram HW
    ret = pd_snake_test_deprogram_hw(snake_test_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to deprogram hw");
    }

    // dealloc resources and free
    ret = pd_snake_test_cleanup(snake_test_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed pd snake_test cleanup");
    }

    return ret;
}

hal_ret_t
pd_snake_test_deprogram_hw(pd_snake_test_t *snake_pd)
{
    pd_snake_test_if_t *snake_if;

    for (const void *ptr : *snake_pd->snake_if_list) {
        snake_if = (pd_snake_test_if_t *)ptr;
        pd_snake_test_deprogram_ifs(snake_if);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_snake_test_deprogram_ifs(pd_snake_test_if_t *snake_if)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Deprogramming for hw_lif: {}, oport: {}, slport: {},"
                    "dlport: {}, vlan_insert_en: {} => inp_prop_idx: {}"
                    "nacl_idx: {} ",
                    snake_if->hw_lif_id,
                    snake_if->oport,
                    snake_if->slport,
                    snake_if->dlport,
                    snake_if->vlan_insert_en,
                    snake_if->inp_prop_idx,
                    snake_if->nacl_idx);


    ret = pd_snake_test_loop_depgm_inp_props(snake_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to depgm inp props. err: {}", ret);
        goto end;
    }

    ret = pd_snake_test_loop_depgm_nacl(snake_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to depgm inp props. err: {}", ret);
        goto end;
    }

    ret = pd_snake_test_loop_depgm_omap(snake_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to depgm inp props. err: {}", ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
pd_snake_test_program_hw(pd_snake_test_t *snake_pd)
{
    snake_test_t *snake = snake_pd->snake;
    if (snake->type == types::SNAKE_TEST_TYPE_LOOP) {
        pd_snake_test_program_loop(snake_pd);
    } else if (snake->type == types::SNAKE_TEST_TYPE_UP2UP) {
        pd_snake_test_program_up2up(snake_pd);
    } else if (snake->type == types::SNAKE_TEST_TYPE_ARM_TO_ARM) {
    } else {
        HAL_TRACE_ERR("Unknown snake test type: {}", snake->type);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_snake_test_cleanup(pd_snake_test_t *snake_test_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!snake_test_pd) {
        goto end;
    }

    // delinking PI<->PD
    delink_pi_pd(snake_test_pd, (snake_test_t *)snake_test_pd->snake);

    // freeing PD
    snake_test_pd_free(snake_test_pd);
end:
    return ret;
}


hal_ret_t
pd_snake_test_program_loop (pd_snake_test_t *snake_pd)
{
    hal_ret_t ret = HAL_RET_OK;


    // Populate hw_lif_id & slport
    // - For Internak Mgmt If
    ret = pd_snake_test_loop_form_int_mgmt_lif(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to form int mgmt lif. err: {}", ret);
        goto end;
    }
    // - For Uplinks
    ret = pd_snake_test_loop_form_upifs(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to form uplinks. err: {}", ret);
        goto end;
    }

    // Populate dlport
    ret = pd_snake_test_loop_set_dlports(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to populate dlport. err: {}", ret);
        goto end;
    }

    // Program if list
    ret = pd_snake_test_loop_pgm_ifs(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to pgm ifs. err: {}", ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
pd_snake_test_program_up2up (pd_snake_test_t *snake_pd)
{
    hal_ret_t ret = HAL_RET_OK;


    // Populate hw_lif_id & slport
    // - For Uplinks
    ret = pd_snake_test_loop_form_upifs(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to form uplinks. err: {}", ret);
        goto end;
    }

    // Populate dlport
    ret = pd_snake_test_loop_set_dlports(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to populate dlport. err: {}", ret);
        goto end;
    }

    // Program if list
    ret = pd_snake_test_loop_pgm_ifs(snake_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to pgm ifs. err: {}", ret);
        goto end;
    }

end:
    return ret;
}

hal_ret_t
pd_snake_test_loop_pgm_ifs (pd_snake_test_t *snake_pd)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd_snake_test_if_t *snake_if = NULL;

    /**
     * IF:
     *  - Input Prop
     *  - NACL
     *  - Output Mapping
     */

    for (const void *ptr : *snake_pd->snake_if_list) {
        snake_if = (pd_snake_test_if_t *)ptr;
        HAL_TRACE_DEBUG("Programming for hw_lif: {}, oport: {}, slport: {},"
                        "dlport: {}, vlan_insert_en: {}",
                        snake_if->hw_lif_id,
                        snake_if->oport,
                        snake_if->slport,
                        snake_if->dlport,
                        snake_if->vlan_insert_en);
        // Input Properties
        ret = pd_snake_test_loop_pgm_inp_props(snake_if, snake_pd);
        // NACL
        ret = pd_snake_test_loop_pgm_nacl(snake_if);
        // Output Mapping
        ret = pd_snake_test_loop_pgm_omap(snake_if);
        HAL_TRACE_DEBUG("Programming for hw_lif: {}, oport: {}, slport: {},"
                        "dlport: {}, vlan_insert_en: {} => inp_prop_idx: {}"
                        "nacl_idx: {}",
                        snake_if->hw_lif_id,
                        snake_if->oport,
                        snake_if->slport,
                        snake_if->dlport,
                        snake_if->vlan_insert_en,
                        snake_if->inp_prop_idx,
                        snake_if->nacl_idx);
    }

    return ret;
}

#define om_tmoport data.action_u.output_mapping_set_tm_oport
hal_ret_t
pd_snake_test_loop_pgm_omap (pd_snake_test_if_t *snake_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    output_mapping_actiondata_t data;
    directmap                   *dm_omap = NULL;
    sdk_ret_t                   sdk_ret;

    memset(&data, 0, sizeof(data));

    data.action_id = OUTPUT_MAPPING_SET_TM_OPORT_ID;
    om_tmoport.nports = 1;
    om_tmoport.egress_mirror_en = 1;
    om_tmoport.egress_port1 = snake_if->oport;
    om_tmoport.dst_lif = snake_if->hw_lif_id;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->insert_withid(&data, snake_if->slport);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to program : {}", snake_if->slport);
    } else {
        HAL_TRACE_DEBUG("programmed for lport {}", snake_if->slport);
    }

    return ret;
}

hal_ret_t
pd_snake_test_loop_depgm_omap (pd_snake_test_if_t *snake_if)
{
    hal_ret_t                   ret = HAL_RET_OK;
    directmap                   *dm_omap = NULL;
    sdk_ret_t                   sdk_ret;

    dm_omap = g_hal_state_pd->dm_table(P4TBL_ID_OUTPUT_MAPPING);
    SDK_ASSERT_RETURN((dm_omap != NULL), HAL_RET_ERR);

    sdk_ret = dm_omap->remove(snake_if->slport);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to deprogram : {}", snake_if->slport);
    } else {
        HAL_TRACE_DEBUG("deprogrammed for lport {}", snake_if->slport);
    }

    return ret;
}

hal_ret_t
pd_snake_test_loop_pgm_nacl (pd_snake_test_if_t *snake_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    nacl_swkey_t        key;
    nacl_swkey_mask_t   mask;
    nacl_actiondata_t   data;
    acl_tcam            *acl_tbl = NULL;
    static              uint32_t priority = ACL_SNAKE_TEST_PRIORITY_BEGIN;

    memset(&key, 0, sizeof(key));
    memset(&mask, 0, sizeof(mask));
    memset(&data, 0, sizeof(data));

    acl_tbl = g_hal_state_pd->acl_table();
    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    // (slport) => (dlport, no_drop)

    // Key
    key.entry_inactive_nacl = 0;
    mask.entry_inactive_nacl_mask = 0x1;
    key.control_metadata_src_lport = snake_if->slport;
    mask.control_metadata_src_lport_mask =
        ~(mask.control_metadata_src_lport_mask & 0);

    // Data
    data.action_id = NACL_NACL_PERMIT_ID;
    data.action_u.nacl_nacl_permit.discard_drop = 1;
    data.action_u.nacl_nacl_permit.dst_lport_en = 1;
    data.action_u.nacl_nacl_permit.dst_lport = snake_if->dlport;

    ret = acl_tbl->insert(&key, &mask, &data, priority, &snake_if->nacl_idx);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Programmed nacl at: {}.", snake_if->nacl_idx);
        priority++;
    } else {
        HAL_TRACE_ERR("Unable to program nacl. ret: {}", ret);
    }

    return ret;
}

hal_ret_t
pd_snake_test_loop_depgm_nacl (pd_snake_test_if_t *snake_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    acl_tcam            *acl_tbl = NULL;

    acl_tbl = g_hal_state_pd->acl_table();
    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);
    ret = acl_tbl->remove(snake_if->nacl_idx);
    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("DeProgrammed nacl at: {}.", snake_if->nacl_idx);
    } else {
        HAL_TRACE_ERR("Unable to deprogram nacl. ret: {}", ret);
    }

    return ret;
}

#define inp_prop data.action_u.input_properties_input_properties
hal_ret_t
pd_snake_test_loop_pgm_inp_props (pd_snake_test_if_t *snake_if,
                                  pd_snake_test_t *snake_pd)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    input_properties_swkey_t        key = {0};
    input_properties_actiondata_t   data = {0};
    sdk_hash                        *inp_prop_tbl = NULL;
    snake_test_t                    *snake = snake_pd->snake;


    // Key
    key.capri_intrinsic_lif = snake_if->hw_lif_id;
    if (snake_if->vlan_insert_en) {
        key.p4plus_to_p4_insert_vlan_tag = 1;
    } else {
        key.vlan_tag_valid = 1;
    }
    key.vlan_tag_vid = snake->vlan;

    // Data
    inp_prop.src_lport = snake_if->slport;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((inp_prop_tbl != NULL), HAL_RET_ERR);
    sdk_ret = inp_prop_tbl->insert(&key, &data, &snake_if->inp_prop_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("snake_test: unable to program for "
                      "(lif, vlan): ({}, {})",
                      key.capri_intrinsic_lif, key.vlan_tag_vid);
        goto end;
    } else {
        HAL_TRACE_DEBUG("snake_test: Programmed "
                        "table:input_properties index:{} ",
                        snake_if->inp_prop_idx);
    }
end:
    return ret;
}

hal_ret_t
pd_snake_test_loop_depgm_inp_props (pd_snake_test_if_t *snake_if)
{
    hal_ret_t                       ret = HAL_RET_OK;
    sdk_ret_t                       sdk_ret;
    sdk_hash                        *inp_prop_tbl = NULL;

    inp_prop_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_INPUT_PROPERTIES);
    SDK_ASSERT_RETURN((inp_prop_tbl != NULL), HAL_RET_ERR);

    sdk_ret = inp_prop_tbl->remove(snake_if->inp_prop_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("snake_test: unable to deprogram idx: {}", snake_if->inp_prop_idx);
        goto end;
    } else {
        HAL_TRACE_DEBUG("snake_test: DeProgrammed "
                        "table:input_properties index:{} ",
                        snake_if->inp_prop_idx);
    }
end:
    return ret;
}

hal_ret_t
pd_snake_test_loop_set_dlports (pd_snake_test_t *snake_pd)
{
    snake_test_t *snake = snake_pd->snake;
    pd_snake_test_if_t *snake_if = NULL,
                       *prev_snake_if = NULL,
                       *first_up_snake_if = NULL;


    if (snake->type == types::SNAKE_TEST_TYPE_UP2UP) {
        // Update first uplink if
        for (const void *ptr : *snake_pd->snake_if_list) {
            snake_if = (pd_snake_test_if_t *)ptr;
            // Enters for the first uplink
            if (!first_up_snake_if) {
                first_up_snake_if = snake_if;
            }
            if (prev_snake_if) {
                prev_snake_if->dlport = snake_if->slport;
            }
            prev_snake_if = snake_if;
        }
        // Update last uplink if
        snake_if->dlport = first_up_snake_if->slport;
    } else {
        for (const void *ptr : *snake_pd->snake_if_list) {
            snake_if = (pd_snake_test_if_t *)ptr;
            if (prev_snake_if) {
                // Enters for the first uplink
                if (!first_up_snake_if) {
                    first_up_snake_if = snake_if;
                }
                prev_snake_if->dlport = snake_if->slport;
            }
            prev_snake_if = snake_if;
        }
        // Update last uplink if
        snake_if->dlport = first_up_snake_if->slport;
    }

    return HAL_RET_OK;
}


bool
pd_snake_test_loop_form_upif (void *ht_entry, void *ctxt)
{
    hal_ret_t                ret = HAL_RET_OK;
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    pd_snake_test_t          *snake_pd = (pd_snake_test_t *)ctxt;
    if_t                     *hal_if = NULL;
    pd_snake_test_if_t       *snake_if = NULL;

    hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);

    if (hal_if->if_type != intf::IF_TYPE_UPLINK) {
        goto end;
    }

    if (snake_pd->snake->type == types::SNAKE_TEST_TYPE_UP2UP &&
        hal_if->is_oob_management == true) {
        // Skip oob for up2up
        goto end;
    }

    snake_if = (pd_snake_test_if_t *)g_hal_state_pd->
        snake_test_if_slab()->alloc();
    if (snake_if == NULL) {
        goto end;
    }
    snake_if->oport = uplinkif_get_port_num(hal_if);
    snake_if->vlan_insert_en = 0;
    snake_if->hw_lif_id = if_get_hw_lif_id(hal_if);

    ret = pd_snake_test_allocate_lport(&snake_if->slport);

    ret = snake_pd->snake_if_list->insert(snake_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to insert IF into snake list. err: {}",
                      ret);
        goto end;
    }

    HAL_TRACE_DEBUG("Inserted Uplink hw_lif_id: {}, lport: {}",
                    snake_if->hw_lif_id,
                    snake_if->slport);


end:
    if (snake_if) {
        hal::pd::delay_delete_to_slab(HAL_SLAB_SNAKE_TEST_IF_PD, snake_if);
    }
    return false;
}

hal_ret_t
pd_snake_test_loop_form_upifs (pd_snake_test_t *snake_pd)
{
    hal_ret_t ret = HAL_RET_OK;

    if (!g_hal_state->if_id_ht()->num_entries()) {
        HAL_TRACE_ERR("No uplinks. Can't do snake test");
        return HAL_RET_ERR;
    }

    g_hal_state->if_id_ht()->walk(pd_snake_test_loop_form_upif, snake_pd);
    return ret;
}

hal_ret_t
pd_snake_test_loop_form_int_mgmt_lif (pd_snake_test_t *snake_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_id_t            imgmt_lif_id;
    lif_t               *imgmt_lif;
    pd_snake_test_if_t  *snake_if = NULL;
    uint32_t            hw_lif_id;

    imgmt_lif_id = g_hal_state->mnic_internal_mgmt_lif_id();
    imgmt_lif = find_lif_by_id(imgmt_lif_id);
    if (!imgmt_lif) {
        HAL_TRACE_ERR("Unable to find internal mgmt lif. id: {}",
                      imgmt_lif_id);
        goto end;
    }
    ret = pd_lif_get_hw_lif_id(imgmt_lif, &hw_lif_id);

    snake_if = (pd_snake_test_if_t *)g_hal_state_pd->
        snake_test_if_slab()->alloc();
    if (snake_if == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    snake_if->oport = TM_PORT_DMA;
    snake_if->vlan_insert_en = imgmt_lif->vlan_insert_en;
    snake_if->hw_lif_id = hw_lif_id;
    ret = pd_snake_test_allocate_lport(&snake_if->slport);

    ret = snake_pd->snake_if_list->insert(snake_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to insert IF into snake list. err: {}",
                      ret);
        goto end;
    }

    HAL_TRACE_DEBUG("Inserted Int. Mgmt hw_lif_id: {}, lport: {}",
                    snake_if->hw_lif_id,
                    snake_if->slport);

end:
    if (snake_if) {
        hal::pd::delay_delete_to_slab(HAL_SLAB_SNAKE_TEST_IF_PD, snake_if);
    }
    return ret;
}

hal_ret_t
pd_snake_test_allocate_lport(uint32_t *lport)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;

    rs = g_hal_state_pd->lport_idxr()->alloc(lport);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("failed to alloc lport_id err: {}", rs);
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("snake_test: allocated lport_id:{}", *lport);
    return ret;
}

void
link_pi_pd(pd_snake_test_t *pd_snake_test, snake_test_t *pi_snake_test)
{
    pd_snake_test->snake = pi_snake_test;
    pi_snake_test->pd = pd_snake_test;
}

void
delink_pi_pd(pd_snake_test_t *pd_snake_test, snake_test_t *pi_snake_test)
{
    if (pd_snake_test) {
        pd_snake_test->snake = NULL;
    }
    if (pi_snake_test) {
        pi_snake_test->pd = NULL;
    }
}


}    // namespace pd
}    // namespace hal
