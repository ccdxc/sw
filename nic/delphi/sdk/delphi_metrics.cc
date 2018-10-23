// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/delphi/sdk/delphi_metrics.hpp"

namespace delphi {
namespace metrics {

// Counter constructor
Counter::Counter(uint64_t *ptr) {
    valptr_ = ptr;
}

// Add adds a value to current counter value
error Counter::Add(uint64_t addn) {
    *valptr_ += addn;

    return error::OK();
}

// Incr increments the counter by one
error Counter::Incr() {
    *valptr_ += 1;

    return error::OK();
}

// Set sets the current value of counter
error Counter::Set(uint64_t val) {
    *valptr_ = val;

    return error::OK();
}

// Gauge constructor
Gauge::Gauge(double *ptr) {
    valptr_ = ptr;
}

// Set sets the current value of gauge
error Gauge::Set(double val) {
    *valptr_ = val;

    return error::OK();
}

// GetDelphiShm returns a delphi shared memory object
delphi::shm::DelphiShmPtr DelphiMetrics::GetDelphiShm() {
    static delphi::shm::DelphiShmPtr client_shm = make_shared<delphi::shm::DelphiShm>();
    if (!client_shm->IsMapped()) {
        error err = client_shm->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true);
        if (err.IsNotOK()) {
            LogError("Error memory mapping shared memory. Err: {}", err);
            return nullptr;
        }
    }

    return client_shm;
}


} // namespace metrics
} // namespace delphi
