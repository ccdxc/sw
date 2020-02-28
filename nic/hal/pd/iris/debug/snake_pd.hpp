// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SNAKE_PD_HPP__
#define __SNAKE_PD_HPP__

#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

using hal::pd::utils::acl_tcam_entry_handle_t;

namespace hal {
namespace pd {

typedef struct pd_snake_test_if_s {
    uint32_t hw_lif_id;
    uint32_t oport;
    uint32_t slport;
    uint32_t dlport;
    bool vlan_insert_en;
    uint32_t inp_prop_idx;
    acl_tcam_entry_handle_t nacl_idx;
} pd_snake_test_if_t;

typedef struct pd_snake_test_s {
    block_list      *snake_if_list;

    snake_test_t    *snake;
} pd_snake_test_t;

static inline pd_snake_test_t *
snake_test_pd_alloc (void)
{
    pd_snake_test_t    *snake_test_pd;

    snake_test_pd = (pd_snake_test_t *)g_hal_state_pd->snake_test_slab()->alloc();
    if (snake_test_pd == NULL) {
        return NULL;
    }

    return snake_test_pd;
}

static inline pd_snake_test_t *
snake_test_pd_init (pd_snake_test_t *snake_test_pd)
{
    if (!snake_test_pd) {
        return NULL;
    }

    snake_test_pd->snake_if_list = block_list::factory(sizeof(pd_snake_test_if_t));
    snake_test_pd->snake = NULL;
    return snake_test_pd;
}

static inline pd_snake_test_t *
snake_test_pd_alloc_init (void)
{
    return snake_test_pd_init(snake_test_pd_alloc());
}

static inline hal_ret_t
snake_test_pd_mem_free (pd_snake_test_t *snake_test)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_SNAKE_TEST_PD, snake_test);
    return HAL_RET_OK;
}

static inline hal_ret_t
snake_test_pd_free (pd_snake_test_t *snake_test)
{
    if (snake_test->snake_if_list) {
        block_list::destroy(snake_test->snake_if_list);
    }
    snake_test_pd_mem_free(snake_test);
    return HAL_RET_OK;
}

hal_ret_t pd_snake_test_program_hw(pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_program_loop(pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_program_up2up(pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_allocate_lport(uint32_t *lport);
hal_ret_t pd_snake_test_loop_form_int_mgmt_lif(pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_loop_pgm_ifs (pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_loop_pgm_inp_props (pd_snake_test_if_t *snake_if,
                                            pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_loop_depgm_inp_props (pd_snake_test_if_t *snake_if);
hal_ret_t pd_snake_test_loop_set_dlports (pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_loop_form_upifs (pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_loop_pgm_nacl (pd_snake_test_if_t *snake_if);
hal_ret_t pd_snake_test_loop_depgm_nacl (pd_snake_test_if_t *snake_if);
hal_ret_t pd_snake_test_loop_pgm_omap (pd_snake_test_if_t *snake_if);
hal_ret_t pd_snake_test_loop_depgm_omap (pd_snake_test_if_t *snake_if);
void link_pi_pd(pd_snake_test_t *pd_snake_test, snake_test_t *pi_snake_test);
void delink_pi_pd(pd_snake_test_t *pd_snake_test, snake_test_t *pi_snake_test);
hal_ret_t pd_snake_test_cleanup(pd_snake_test_t *snake_test_pd);
hal_ret_t pd_snake_test_deprogram_hw(pd_snake_test_t *snake_pd);
hal_ret_t pd_snake_test_deprogram_ifs(pd_snake_test_if_t *snake_if);

} // namespace pd
} // namespace hal

#endif    // __SNAKE_PD_HPP__
