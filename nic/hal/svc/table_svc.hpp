// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __TABLE_SVC_HPP__
#define __TABLE_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/table.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using table::Table;
using types::Empty;
using table::TableMetadataResponseMsg;
using table::TableMetadata;
using table::TableMetMetadata;
using table::TableFlowMetadata;
using table::TableHashMetadata;
using table::TableTcamMetadata;
using table::TableIndexMetadata;
using table::TableRequestMsg;
using table::TableResponseMsg;
using table::TableSpec;

class TableServiceImpl final : public Table::Service {
public:
    Status TableMetadataGet(ServerContext *context,
                            const Empty *req,
                            TableMetadataResponseMsg *rsp) override;

    Status TableGet(ServerContext *context,
                    const TableRequestMsg *req,
                    TableResponseMsg *rsp) override;
};


#endif  // __TABLE_SVC_HPP__

