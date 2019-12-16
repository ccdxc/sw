//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __TABLE_HPP__
#define __TABLE_HPP__

#include "nic/include/base.hpp"
#include "gen/proto/table.pb.h"
#include "gen/proto/table.grpc.pb.h"

using table::TableMetadataResponseMsg;
using table::TableMetadata;
using table::TableMetMetadata;
using table::TableFlowMetadata;
using table::TableHashMetadata;
using table::TableTcamMetadata;
using table::TableIndexMetadata;
using table::TableRequestMsg;
using table::TableResponseMsg;
using table::TableResponse;
using table::TableSpec;
using table::TableKind;
using table::TableIndexMsg;
using table::TableIndexEntry;
using table::TableTcamMsg;
using table::TableTcamEntry;
using table::TableHashMsg;
using table::TableHashEntry;
using table::TableFlowMsg;
using table::TableFlowEntry;
using table::TableMetMsg;
using table::TableMetEntry;

namespace hal {

hal_ret_t table_metadata_get(table::TableMetadataResponseMsg *rsp);
hal_ret_t table_get (TableSpec& spec, TableResponseMsg *rsp);

}    // namespace hal

#endif    // __TABLE_HPP__

