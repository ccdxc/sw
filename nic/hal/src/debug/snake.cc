//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/src/debug/snake.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

static inline snake_test_t *
snake_test_alloc (void)
{
    snake_test_t    *snake_test;

    snake_test = (snake_test_t *)g_hal_state->snake_test_slab()->alloc();
    if (snake_test == NULL) {
        return NULL;
    }
    return snake_test;
}

static inline snake_test_t *
snake_test_init (snake_test_t *snake_test)
{
    if (!snake_test) {
        return NULL;
    }
    SDK_SPINLOCK_INIT(&snake_test->slock, PTHREAD_PROCESS_SHARED);

    memset(snake_test, 0, sizeof(snake_test_t));

    return snake_test;
}

static inline snake_test_t *
snake_test_alloc_init (void)
{
    return snake_test_init(snake_test_alloc());
}

static inline hal_ret_t
snake_test_free (snake_test_t *snake_test)
{
    SDK_SPINLOCK_DESTROY(&snake_test->slock);
    hal::delay_delete_to_slab(HAL_SLAB_SNAKE_TEST, snake_test);
    return HAL_RET_OK;
}

static inline hal_ret_t
snake_test_cleanup (snake_test_t *snake_test)
{
    snake_test_free(snake_test);

    return HAL_RET_OK;
}

hal_ret_t
validate_snake_test_create(SnakeTestRequest& req,
                           SnakeTestResponse *rsp)
{
    return HAL_RET_OK;
}

static hal_ret_t
snake_test_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                        ret = HAL_RET_OK;
    pd::pd_snake_test_create_args_t pd_snake_args;
    dllist_ctxt_t                    *lnode = NULL;
    dhl_entry_t                      *dhl_entry = NULL;
    snake_test_t                     *snake = NULL;
    pd::pd_func_args_t               pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    snake = (snake_test_t *)dhl_entry->obj;

    pd_snake_args.snake = snake;
    pd_func_args.pd_snake_test_create = &pd_snake_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SNAKE_TEST_CREATE,
                          &pd_func_args);
     if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create snake test pd, err : {}", ret);
    }
    return ret;
}

static hal_ret_t
snake_test_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    snake_test_t                *snake_test  = NULL;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    snake_test = (snake_test_t *)dhl_entry->obj;

    g_hal_state->set_snake_test(snake_test);

    return ret;
}

hal_ret_t
snake_test_create_abort_cleanup (snake_test_t *snake, hal_handle_t hal_handle)
{
    hal_ret_t                       ret;
    pd::pd_snake_test_delete_args_t pd_snake_args = { 0 };
    pd::pd_func_args_t              pd_func_args = {0};

    if (snake->pd) {
        pd_snake_args.snake = snake;
        pd_func_args.pd_snake_test_delete = &pd_snake_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_SNAKE_TEST_DELETE, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete snake {} pd, err : {}", snake->type,
                          ret);
        }
    }

    hal_handle_free(hal_handle);

    return HAL_RET_OK;
}

static hal_ret_t
snake_test_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret                        = HAL_RET_OK;
    dllist_ctxt_t *lnode                 = NULL;
    dhl_entry_t *dhl_entry               = NULL;
    snake_test_t *snake                  = NULL;
    hal_handle_t hal_handle              = 0;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    snake = (snake_test_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = snake_test_create_abort_cleanup(snake, hal_handle);

    return ret;
}

hal_ret_t
snake_test_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

static hal_ret_t
snake_test_prepare_rsp (SnakeTestResponse *rsp,
                        hal_ret_t ret, hal_handle_t hal_handle)
{
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

hal_ret_t
snake_test_create(SnakeTestRequest& req,
                  SnakeTestResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    snake_test_t    *snake = NULL;
    dhl_entry_t     dhl_entry = { 0 };
    cfg_op_ctxt_t   cfg_ctxt  = { 0 };

    hal_api_trace(" API Begin: Snake Test create ");
    proto_msg_dump(req);

    ret = validate_snake_test_create(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("snake test validation failed, err : {}", ret);
        goto end;
    }

    if(g_hal_state->snake_test()) {
        HAL_TRACE_ERR("Failed to create snake test, exists already. {:#x}",
                      (uint64_t)g_hal_state->snake_test());
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    snake = snake_test_alloc_init();
    if (!snake) {
        HAL_TRACE_ERR("Failed to alloc/init snake test");
        ret = HAL_RET_OOM;
        goto end;
    }

    snake->type = req.type();
    if (req.vlan() == 0) {
        snake->vlan = SNAKE_TEST_LOOP_VLAN_DEF;
    } else {
        snake->vlan = req.vlan();
    }

    snake->hal_handle = hal_handle_alloc(HAL_OBJ_ID_SNAKE_TEST);
    if (snake->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for snake test");
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        snake_test_cleanup(snake);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    dhl_entry.handle  = snake->hal_handle;
    dhl_entry.obj     = snake;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(snake->hal_handle, &cfg_ctxt,
                             snake_test_create_add_cb,
                             snake_test_create_commit_cb,
                             snake_test_create_abort_cb,
                             snake_test_create_cleanup_cb);

end:

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (snake) {
            snake_test_cleanup(snake);
            snake = NULL;
        }
        // HAL_API_STATS_INC(HAL_API_SNAKE_TEST_CREATE_FAIL);
    } else {
        // HAL_API_STATS_INC(HAL_API_SNAKE_TEST_CREATE_SUCCESS);
    }

    snake_test_prepare_rsp(rsp, ret, snake ? snake->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

hal_ret_t
validate_snake_test_delete_req(SnakeTestDeleteRequest& req,
                               SnakeTestDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (req.type() != types::SNAKE_TEST_TYPE_ARM_TO_ARM &&
        req.type() != types::SNAKE_TEST_TYPE_LOOP &&
        req.type() != types::SNAKE_TEST_TYPE_UP2UP) {
        ret =  HAL_RET_INVALID_ARG;
    }
    return ret;
}

hal_ret_t
validate_snake_test_delete(SnakeTestDeleteRequest& req,
                           snake_test_t *snake)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (req.type() != snake->type) {
        ret = HAL_RET_INVALID_ARG;
    }
    return ret;
}

static hal_ret_t
snake_test_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret                        = HAL_RET_OK;
    dllist_ctxt_t *lnode                 = NULL;
    dhl_entry_t *dhl_entry               = NULL;
    snake_test_t *snake                  = NULL;
    pd::pd_snake_test_delete_args_t pd_snake_args = { 0 };
    pd::pd_func_args_t              pd_func_args = {0};

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    snake = (snake_test_t *)dhl_entry->obj;

    pd_snake_args.snake = snake;
    pd_func_args.pd_snake_test_delete = &pd_snake_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SNAKE_TEST_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete snake {} pd, err : {}", snake->type,
                      ret);
    }

    return ret;
}

static hal_ret_t
snake_test_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    snake_test_t       *snake = NULL;
    hal_handle_t       hal_handle = 0;

    SDK_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    snake = (snake_test_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    g_hal_state->set_snake_test(NULL);

    hal_handle_free(hal_handle);

    snake_test_cleanup(snake);

    return ret;
}

static hal_ret_t
snake_test_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

static hal_ret_t
snake_test_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
snake_test_delete(SnakeTestDeleteRequest& req,
                  SnakeTestDeleteResponse *rsp)
{
    hal_ret_t             ret = HAL_RET_OK;
    snake_test_t          *snake = NULL;
    cfg_op_ctxt_t         cfg_ctxt = { 0 };
    dhl_entry_t           dhl_entry = { 0 };

    hal_api_trace(" API Begin: Snake test delete ");
    proto_msg_dump(req);

    // validate the request message
    ret = validate_snake_test_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("snake delete validation failed, err : {}", ret);
        goto end;
    }

    snake = g_hal_state->snake_test();
    if (snake == NULL) {
        HAL_TRACE_ERR("No snake test creatd. Nothing to delete.");
        ret = HAL_RET_SNAKE_TEST_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("Deleting snake test {}", snake->type);

    ret = validate_snake_test_delete(req, snake);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("snake delete validation failed, err : {}", ret);
        goto end;
    }

    dhl_entry.handle = snake->hal_handle;
    dhl_entry.obj = snake;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(snake->hal_handle, &cfg_ctxt,
                             snake_test_delete_del_cb,
                             snake_test_delete_commit_cb,
                             snake_test_delete_abort_cb,
                             snake_test_delete_cleanup_cb);
end:

    if (ret == HAL_RET_OK) {
        // HAL_API_STATS_INC (HAL_API_VRF_DELETE_SUCCESS);
    } else {
        // HAL_API_STATS_INC (HAL_API_VRF_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

hal_ret_t
snake_test_get(SnakeTestResponseMsg *rsp_msg)
{
    hal_ret_t       ret = HAL_RET_OK;
    snake_test_t    *snake_test = g_hal_state->snake_test();

    auto response = rsp_msg->add_response();
    SnakeTestRequest *req = response->mutable_request();

    if (snake_test) {
        req->set_type(snake_test->type);
        req->set_vlan(snake_test->vlan);
    }

    response->set_api_status(types::API_STATUS_OK);

    return ret;
}

}    // namespace hal
