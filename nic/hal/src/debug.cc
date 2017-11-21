#include "nic/include/base.h"
#include "nic/include/mtrack.hpp"
#include "nic/hal/src/debug.hpp"

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

}    // namespace hal
