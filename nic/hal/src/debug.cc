#include "nic/include/base.h"
#include "nic/include/mtrack.hpp"
#include "nic/hal/src/debug.hpp"
#include "sdk/slab.hpp"
#include "nic/include/hal_state.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "nic/hal/lkl/lklshim.hpp"

using sdk::lib::slab;

namespace hal {

bool
mtrack_map_walk_cb (void *ctxt, uint32_t alloc_id,
                    utils::mtrack_info_t *minfo)
{
    debug::MemTrackGetResponse       *response;
    debug::MemTrackGetResponseMsg    *rsp;
    
    if (!ctxt || !minfo) {
        HAL_ABORT(FALSE);
        return true;
    }
    rsp = (debug::MemTrackGetResponseMsg *)ctxt;
    response = rsp->add_response();
    if (!response) {
        // memory allocation failure, stop walking !!
        return true;
    }
    response->set_api_status(types::API_STATUS_OK);
    response->mutable_spec()->set_alloc_id(alloc_id);
    response->mutable_stats()->set_num_allocs(minfo->num_allocs);
    response->mutable_stats()->set_num_frees(minfo->num_frees);

    return false;
}

hal_ret_t
mtrack_get (debug::MemTrackGetRequest& req,
            debug::MemTrackGetResponseMsg *rsp)
{
    utils::g_hal_mem_mgr.walk(rsp, mtrack_map_walk_cb);
    return HAL_RET_OK;
}

hal_ret_t
set_slab_response (slab *s, debug::SlabGetResponseMsg *rsp)
{
    debug::SlabGetResponse  *response;

    response = rsp->add_response();
    if (!response) {
        // memory allocation failure!!
        return HAL_RET_OOM;
    }

    if (!s) {
        response->set_api_status(types::API_STATUS_ERR);
        return HAL_RET_INVALID_ARG;
    }

    response->set_api_status(types::API_STATUS_OK);

    response->mutable_spec()->set_name(s->slab_name());
    response->mutable_spec()->set_id(s->slab_id());
    response->mutable_spec()->set_element_size(s->elem_sz());
    response->mutable_spec()->set_elements_per_block(s->elems_per_block());
    response->mutable_spec()->set_thread_safe(s->thread_safe());
    response->mutable_spec()->set_grow_on_demand(s->grow_on_demand());
    //response->mutable_spec()->set_delay_delete(s->delay_delete());
    response->mutable_spec()->set_zero_on_allocation(s->zero_on_alloc());

    response->mutable_stats()->set_num_elements_in_use(s->num_in_use());
    response->mutable_stats()->set_num_allocs(s->num_allocs());
    response->mutable_stats()->set_num_frees(s->num_frees());
    response->mutable_stats()->set_num_alloc_errors(s->num_alloc_fails());
    response->mutable_stats()->set_num_blocks(s->num_blocks());

    return HAL_RET_OK;
}

hal_ret_t
slab_get_from_req (debug::SlabGetRequest& req, debug::SlabGetResponseMsg *rsp)
{
#if 0
    hal_slab_t  slab_id;
    hal_ret_t   ret = HAL_RET_OK;
    uint32_t i = 0;
    slab *s;
   
    slab_id = (hal_slab_t)req.id();

    if (slab_id < HAL_SLAB_PI_MAX) {
        s = hal::g_hal_state->get_slab(slab_id);
        ret = set_slab_response(s, rsp);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        s = hal::pd::g_hal_state_pd->get_slab(slab_id);
        ret = set_slab_response(s, rsp);
    } else if (slab_id == HAL_SLAB_ALL) {
        for (i = (uint32_t) HAL_SLAB_PI_MIN; i < (uint32_t) HAL_SLAB_PI_MAX; i ++) {
            s = hal::g_hal_state->get_slab((hal_slab_t) i);
            ret = set_slab_response (s, rsp);
        }
        for (i = (uint32_t) HAL_SLAB_PD_MIN; i < (uint32_t) HAL_SLAB_PD_MAX; i ++) {
            s = hal::pd::g_hal_state_pd->get_slab((hal_slab_t) i);
            ret = set_slab_response(s, rsp);
        }
    } else {
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get slab for slab id {}", i ? i : slab_id);
        return ret;
    }
#endif

    return HAL_RET_OK;
}

}    // namespace hal
