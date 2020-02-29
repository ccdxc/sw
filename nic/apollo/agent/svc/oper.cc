// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include <map>
#include <memory>
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
#include "nic/sdk/lib/metrics/metrics.hpp"
#include "nic/apollo/core/trace.hpp"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using types::Empty;

using pds::OperSvc;
using pds::TechSupportRequest;
using pds::TechSupportResponse;
using pds::MetricsGetRequest;
using pds::MetricsGetResponse;

typedef std::shared_ptr<MetricsGetResponse> MetricsGetResponsePtr;

static std::map<std::string, void*> g_handlers;

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

static void
metrics_read (std::string name, sdk::metrics::key_t key, MetricsGetResponsePtr rsp)
{
    void *handler;
    sdk::metrics::metrics_counters_t counters;
    
    if (g_handlers.count(name) == 0) {
        handler = sdk::metrics::metrics_open(name.c_str());
        g_handlers[name] = handler;
    } else {
        handler = g_handlers[name];
    }

    counters = sdk::metrics::metrics_read(handler, key);

    for (uint32_t i = 0; i < counters.size(); i++) {
        ::pds::MetricsStatus *status = rsp->mutable_response();
        ::pds::CounterStatus *counter = status->mutable_status()->add_counters();
        counter->set_name(counters[i].first);
        counter->set_value(counters[i].second);
    }

    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
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

Status
OperSvcImpl::MetricsGet(ServerContext* context,
                        ServerReaderWriter<MetricsGetResponse, MetricsGetRequest>* stream) {
    MetricsGetRequest metrics_req;
    std::shared_ptr<MetricsGetResponse> metrics_rsp = nullptr;
    do {
        while (stream->Read(&metrics_req)) {
            sdk::metrics::key_t key;
            if (metrics_req.key().size() != sizeof(key)) {
                PDS_TRACE_DEBUG("Received request with invalid size of %i",
                                metrics_req.key().size());
                metrics_rsp->set_apistatus(
                    types::ApiStatus::API_STATUS_INVALID_ARG);
                stream->Write(*metrics_rsp.get());
                continue;
            }
            memcpy(&key, metrics_req.key().c_str(), sizeof(key));
            PDS_TRACE_DEBUG("Rcvd metrics request: name {}, key {}",
                            metrics_req.name(), key);
            metrics_rsp = std::make_shared<MetricsGetResponse>();
            metrics_read(metrics_req.name(), key, metrics_rsp);
            stream->Write(*metrics_rsp.get());
        }
        pthread_yield();
    } while (true);
    return Status::OK;
}
