// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#pragma once

#include "nic/include/base.hpp"
#include "gen/proto/system.pb.h"

namespace fte {

typedef struct lifqid_s lifqid_t;
struct lifqid_s {
    uint64_t lif : 11;
    uint64_t qtype: 3;
    uint64_t qid : 24;
} __PACK__;

inline std::ostream& operator<<(std::ostream& os, const lifqid_t& lifq)
{
    return os << fmt::format("{{lif={}, qtype={}, qid={}}}",
                             lifq.lif, lifq.qtype, lifq.qid);
}

inline bool operator==(const lifqid_t& lifq1, const lifqid_t& lifq2)
{
    return (((lifq1.lif == lifq2.lif) && (lifq1.qtype == lifq2.qtype) && \
            (lifq1.qid == lifq2.qid)));
}

//  FTE Pipeline
hal_ret_t register_pipeline(const std::string& name,
                            const sys::ForwardMode fwdmode,
                            lifqid_t& lifq,
                            const std::string& lif,
                            const std::string& qid,
                            const std::vector<std::string> &features_outbound,
                            const std::vector<std::string> &features_inbound = {},
                            const lifqid_t &lifq_mask = lifqid_t{0x7FF, 0x7, 0xFFFFFF});

hal_ret_t add_feature(const std::string& name);

// FTE pkt loop (infinite loop)
void fte_start(void *ctxt);

// diable all fte threads
void disable_fte(void);

// FTE Init routine
hal_ret_t init(void);

}    // namespace fte
