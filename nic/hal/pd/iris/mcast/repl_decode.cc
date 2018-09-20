// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/hal/pd/iris/mcast/repl_decode.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/build/iris/gen/datapath/p4/include/p4pd.h"

namespace hal {
namespace pd {

const char *
repl_type_to_str(uint32_t type)
{
    switch(type) {
        case TM_REPL_TYPE_DEFAULT: return "DEFAULT";
        case TM_REPL_TYPE_TO_CPU_REL_COPY: return "CPU_REL_COPY";
        case TM_REPL_TYPE_HONOR_INGRESS: return "HON_ING";
        default: return "INVALID";
    }
}

uint32_t repl_entry_data_to_str(void *repl_entry_data,
                                char *buff, uint32_t buff_size)
{
    uint32_t b = 0;
    p4_replication_data_t *data = (p4_replication_data_t *)repl_entry_data;

    b = snprintf(buff, buff_size, "\n\t\tType: %s, lport: %lu,"
                 "(is_qid: %lu, qid/vnid: %lu), rw_idx: %lu,"
                 "is_tunnel: %lu, tnnl_rw_index: %lu,"
                 "qtype: %lu",
                 repl_type_to_str(data->repl_type),
                 data->lport,
                 data->is_qid,
                 data->qid_or_vnid,
                 data->rewrite_index,
                 data->is_tunnel,
                 data->tunnel_rewrite_index,
                 data->qtype);

    return b;
}

}
}
