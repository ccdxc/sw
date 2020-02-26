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
OperSvcImpl::TechSupportStart(ServerContext *context,
                              const TechSupportRequest *req,
                              TechSupportResponse *rsp) {
    int rc;
    auto tsrsp = rsp->mutable_response();

    if (!req->request().has_dst() ||
        req->request().dst().path() == "") {

        rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }
    rc = system(get_command(req->request().dst().path()).c_str());
    if (rc == -1) {
        tsrsp->set_jobstatus(
            pds::TechSupportStatus::TECHSUPPORT_STATUS_UNKNOWN);
    } else {
        tsrsp->set_jobstatus(
            pds::TechSupportStatus::TECHSUPPORT_STATUS_COMPLETED);
    }

    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    return Status::OK;
}

Status
OperSvcImpl::TechSupportStop(ServerContext *context,
                             const Empty *req,
                             TechSupportResponse *rsp) {
    rsp->mutable_response()->set_jobstatus(
        pds::TechSupportStatus::TECHSUPPORT_STATUS_UNKNOWN);
    rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
    return Status::OK;
}

Status
OperSvcImpl::TechSupportGet(ServerContext *context,
                            const Empty *req,
                            TechSupportResponse *rsp) {
    rsp->mutable_response()->set_jobstatus(
        pds::TechSupportStatus::TECHSUPPORT_STATUS_UNKNOWN);
    rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
    return Status::OK;
}
