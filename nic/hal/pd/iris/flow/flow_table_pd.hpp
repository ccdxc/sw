// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_IRIS_FLOW_PD_HPP__
#define __HAL_PD_IRIS_FLOW_PD_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/system.pb.h"
#include "gen/proto/table.pb.h"
#include "gen/proto/internal.pb.h"
#include "nic/sdk/include/sdk/mem.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "nic/utils/ftl/ftlv6.hpp"

using table::TableResponse;
using internal::FlowHashGetResponse;


namespace hal {
namespace pd {

class flow_table_pd {
public:
    static flow_table_pd *factory();
    static void destroy(flow_table_pd *ftpd);

    flow_table_pd() {}
    ~flow_table_pd() {}

    hal_ret_t init();
    hal_ret_t insert(void *entry,
                     uint32_t *hash_value, bool hash_valid);
    hal_ret_t update(void *entry,
                     uint32_t *hash_value, bool hash_valid);
    hal_ret_t remove(void *entry);
    hal_ret_t get(void *entry, FlowHashGetResponse *rsp);

    hal_ret_t meta_get(table::TableMetadataResponseMsg *rsp_msg);
    hal_ret_t stats_get(sys::TableStatsEntry *stats_entry);
    hal_ret_t dump(TableResponse *rsp);

private:
    FtlBaseTable *table_;
    std::string table_name_;
    uint32_t table_size_;
    uint32_t oflow_table_size_;

};

} // namespace pd
} // namespace hal

#endif // __HAL_PD_IRIS_FLOW_PD_HPP__
