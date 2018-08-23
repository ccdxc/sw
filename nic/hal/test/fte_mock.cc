// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/base.hpp"
#include "nic/fte/fte_core.hpp"

namespace fte {

void fte_start(void *ctxt)
{
}

//  FTE Pipeline
hal_ret_t register_pipeline(const std::string& name,
                            lifqid_t& lifq,
                            const std::string& lif,
                            const std::string& qid,
                            const std::vector<std::string> &features_outbound,
                            const std::vector<std::string> &features_inbound,
                            const lifqid_t &lifq_mask)
{
    return HAL_RET_OK;
}

hal_ret_t add_feature(const std::string& name)
{
    return HAL_RET_OK;
}

}    // namespace fte
