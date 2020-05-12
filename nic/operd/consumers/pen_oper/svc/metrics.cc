//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#include "nic/sdk/lib/metrics/metrics.hpp"
#include "nic/sdk/lib/operd/operd.hpp"
#include "metrics.hpp"

typedef std::shared_ptr<MetricsGetResponse> MetricsGetResponsePtr;

static std::map<std::string, void*> g_handlers;

static void
metrics_read (std::string name, sdk::metrics::key_t key,
              MetricsGetResponsePtr rsp)
{
    void *handler;
    sdk::metrics::counters_t counters;

    if (g_handlers.count(name) == 0) {
        handler = sdk::metrics::metrics_open(name.c_str());
        g_handlers[name] = handler;
    } else {
        handler = g_handlers[name];
    }

    counters = sdk::metrics::metrics_read(handler, key);
    for (uint32_t i = 0; i < counters.size(); i++) {
        ::operd::CountersStatus *status = rsp->add_response();
        ::operd::CounterStatus *counter = status->add_counters();
        counter->set_name(counters[i].first);
        counter->set_value(counters[i].second);
    }
    rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
}

Status
MetricsSvcImpl::MetricsGet(ServerContext* context,
                           ServerReaderWriter<MetricsGetResponse,
                           MetricsGetRequest>* stream) {
    MetricsGetRequest metrics_req;
    std::shared_ptr<MetricsGetResponse> metrics_rsp = nullptr;

    do {
        while (stream->Read(&metrics_req)) {
            sdk::metrics::key_t key;

            if (metrics_req.key().size() != sizeof(key)) {
                metrics_rsp->set_apistatus(
                    types::ApiStatus::API_STATUS_INVALID_ARG);
                stream->Write(*metrics_rsp.get());
                continue;
            }
            memcpy(&key, metrics_req.key().c_str(), sizeof(key));
            metrics_rsp = std::make_shared<MetricsGetResponse>();
            metrics_read(metrics_req.name(), key, metrics_rsp);
            stream->Write(*metrics_rsp.get());
        }
        pthread_yield();
    } while (true);

    return Status::OK;
}
