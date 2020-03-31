//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <map>
#include <memory>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include "grpc++/grpc++.h"
#include "gen/proto/meta/meta.pb.h"
#include "gen/proto/oper.grpc.pb.h"
#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "nic/apollo/agent/svc/oper.hpp"
#include "nic/sdk/lib/metrics/metrics.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/sdk/lib/operd/operd.hpp"
#include "nic/operd/alerts/alerts.hpp"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using types::Empty;

using pds::OperSvc;
using pds::TechSupportRequest;
using pds::TechSupportResponse;
using pds::MetricsGetRequest;
using pds::MetricsGetResponse;
using pds::AlertsGetResponse;

typedef std::shared_ptr<MetricsGetResponse> MetricsGetResponsePtr;

static std::map<std::string, void*> g_handlers;

static inline std::string
get_techsupport_filename (void)
{
    char timestring[PATH_MAX];
    char filename[PATH_MAX];
    time_t current_time = time(NULL);

    strftime(timestring, PATH_MAX, "%Y%m%d%H%M%S", gmtime(&current_time));
    snprintf(filename, PATH_MAX, "tech-support-%s.tar.gz", timestring);

    return std::string(filename);
}

static inline std::string
get_techsupport_binary (void)
{
    std::string ts_bin_path;
    std::string ts_bin = "/bin/techsupport";

    ts_bin_path = std::string(std::getenv("PDSPKG_TOPDIR"));
    if (ts_bin_path.empty()) {
        ts_bin_path = std::string("/nic/");
    }
    ts_bin_path += ts_bin;

    return ts_bin_path;
}

static inline std::string
get_techsupport_config (void)
{
    std::string ts_cfg_path;
    std::string ts_cfg = "/techsupport.json";

    ts_cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (ts_cfg_path.empty()) {
        ts_cfg_path = std::string("/nic/conf/");
    }
    ts_cfg_path += ts_cfg;

    return ts_cfg_path;
}

static inline std::string
get_techsupport_cmd (std::string ts_dir, std::string ts_file, bool skipcores)
{
    char ts_cmd[PATH_MAX];
    std::string bin_path, ts_bin_path;
    std::string ts_bin, ts_task;

    ts_bin = get_techsupport_binary();
    ts_task = get_techsupport_config();

    snprintf(ts_cmd, PATH_MAX, "%s -c %s -d %s -o %s %s", ts_bin.c_str(),
             ts_task.c_str(), ts_dir.c_str(), ts_file.c_str(),
             skipcores ? "-s" : "");

    return std::string(ts_cmd);
}

static void
metrics_read (std::string name, sdk::metrics::key_t key,
              MetricsGetResponsePtr rsp)
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
        ::pds::CountersStatus *status = rsp->add_response();
        ::pds::CounterStatus *counter = status->add_counters();
        counter->set_name(counters[i].first);
        counter->set_value(counters[i].second);
    }
    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static void
alerts_log_to_obj (sdk::operd::log_ptr log, AlertsGetResponse &rsp)
{
    const char *data = log->data();
    int alert_id = *(int *)data;
    const char *message = data + sizeof(int);
    alert_t prototype = operd::alerts::alerts[alert_id];
    ::pds::Alert *alert = rsp.mutable_response();

    alert->set_name(prototype.name);
    alert->set_category(prototype.category);
    alert->set_description(prototype.description);
    alert->set_message(message);
    if (strcmp(prototype.severity, "DEBUG") == 0) {
        alert->set_severity(pds::DEBUG);
    } else if (strcmp(prototype.severity, "INFO") == 0) {
        alert->set_severity(pds::INFO);
    } else if (strcmp(prototype.severity, "WARN") == 0) {
        alert->set_severity(pds::WARN);
    } else if (strcmp(prototype.severity, "CRITICAL") == 0){
        alert->set_severity(pds::CRITICAL);
    }

    rsp.set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static inline int
get_exit_status (int rc)
{
    int ret = rc;

    if (WIFEXITED(rc)) {
        ret = WEXITSTATUS(rc);
    }
    return ret;
}

Status
OperSvcImpl::TechSupportCollect(ServerContext *context,
                                const TechSupportRequest *req,
                                TechSupportResponse *rsp) {
    int rc;
    std::string tsdir = "/data/techsupport/";
    auto tsrsp = rsp->mutable_response();
    auto tsfile = get_techsupport_filename();
    auto tscmd = get_techsupport_cmd(tsdir, tsfile, req->request().skipcores());

    rc = get_exit_status(system(tscmd.c_str()));
    PDS_TRACE_DEBUG("Techsupport request {}, rc {}", tsfile, rc);
    if (rc != 0) {
        rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
    } else {
        tsrsp->set_filepath(tsdir + tsfile);
        rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    }
    return Status::OK;
}

Status
OperSvcImpl::MetricsGet(ServerContext* context,
                        ServerReaderWriter<MetricsGetResponse,
                        MetricsGetRequest>* stream) {
    pds_obj_key_t obj_key;
    MetricsGetRequest metrics_req;
    std::shared_ptr<MetricsGetResponse> metrics_rsp = nullptr;

    do {
        while (stream->Read(&metrics_req)) {
            sdk::metrics::key_t key;

            if (metrics_req.key().size() != sizeof(key)) {
                PDS_TRACE_DEBUG("Rcvd request with invalid size {}",
                                metrics_req.key().size());
                metrics_rsp->set_apistatus(
                    types::ApiStatus::API_STATUS_INVALID_ARG);
                stream->Write(*metrics_rsp.get());
                continue;
            }
            memcpy(&key, metrics_req.key().c_str(), sizeof(key));
            pds_obj_key_proto_to_api_spec(&obj_key, metrics_req.key());
            PDS_TRACE_DEBUG("Rcvd metrics request, name {}, key {}",
                            metrics_req.name().c_str(), obj_key.str());
            metrics_rsp = std::make_shared<MetricsGetResponse>();
            metrics_read(metrics_req.name(), key, metrics_rsp);
            stream->Write(*metrics_rsp.get());
        }
        pthread_yield();
    } while (true);
    return Status::OK;
}

Status
OperSvcImpl::AlertsGet(ServerContext *context, const Empty *req,
                       ServerWriter<AlertsGetResponse> *stream) {
    sdk::operd::consumer_ptr source = std::make_shared<sdk::operd::region>("alerts");

    while (true) {
        sdk::operd::log_ptr log = source->read();

        if (log != nullptr) {
            AlertsGetResponse resp;

            alerts_log_to_obj(log, resp);
            
            if (!stream->Write(resp)) {
                // Client closed connection
                break;
            }
        }
        sleep(5);
    }
    return Status::OK;
}
