// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>

#include "oper.hpp"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/oper.grpc.pb.h"
#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "grpc++/grpc++.h"

using grpc::Status;
using grpc::ServerContext;

using types::Empty;

using pds::OperSvc;
using pds::TechSupportRequest;
using pds::TechSupportResponse;

static std::string
get_command (std::string base)
{
    char timestring[PATH_MAX];
    char filename[PATH_MAX];
    time_t current_time = time(NULL);

    strftime(timestring, PATH_MAX, "%Y%m%d%H%M%S", gmtime(&current_time));
    snprintf(filename, PATH_MAX, "/nic/bin/techsupport "
             "-c /sw/nic/conf/apulu/techsupport.json "
             "-d /%s/ -o tech-support-%s.gz", base.c_str(), timestring);

    return std::string(filename);
}

Status
OperSvcImpl::TechSupportCollect(ServerContext *context,
                                const TechSupportRequest *req,
                                TechSupportResponse *rsp) {
    int rc;
    auto tsrsp = rsp->mutable_response();

    auto filename = get_command("/data/techsupport");
    rc = system(filename.c_str());
    if (rc == -1) {
        rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
    } else {
        tsrsp->set_filepath(filename);
        rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    }
    return Status::OK;
}
