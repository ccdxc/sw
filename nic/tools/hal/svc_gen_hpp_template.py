//:: import os
//:: import importlib
//:: import sys
//:: hdr_def = fileName.replace('_pb2.py', '').upper()
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_SVC_${hdr_def}_HPP__
#define __HAL_SVC_${hdr_def}_HPP__

#include "nic/include/base.h"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
//:: if 'WS_TOP' not in os.environ:
//::     # This should have been set, before invoking the template.
//::     assert False
//:: #endif
//:: # Add the proto directory to the path.
//:: ws_top = os.environ['WS_TOP']
//:: fullpath = ws_top + '/nic/gen/proto/hal/'
//:: sys.path.insert(0, fullpath)
//::
//:: fileModule = importlib.import_module(fileName[:-3])
//:: includeFileName = fileName[:-7]
#include "nic/gen/proto/hal/${includeFileName}.grpc.pb.h"
//::     # Remove the _pb2.py from file and store it for now.
//:: for service in fileModule.DESCRIPTOR.services_by_name.items():
//::     pkg = fileModule.DESCRIPTOR.package.lower()

using grpc::ServerContext;
using grpc::Status;
using ${pkg}::${service[0]};

class ${service[0]}ServiceImpl final : public ${service[0]}::Service {
public:
//::     for method in service[1].methods_by_name.items():
//::         input_name = pkg+'::'+method[1].input_type.name
//::         output_name = pkg+'::'+method[1].output_type.name
    Status ${method[0]}(ServerContext *context,
                        const ${input_name} *req,
                        ${output_name} *rsp) override;

//::     #endfor
};

//:: #endfor

#endif   // __HAL_SVC_${hdr_def}_HPP__

