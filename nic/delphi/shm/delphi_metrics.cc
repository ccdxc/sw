// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/delphi/shm/delphi_metrics.hpp"
#ifdef __x86_64__
#elif __aarch64__
#include "nic/sdk/lib/pal/pal.hpp"
#endif

namespace delphi {
namespace metrics {

#define MOCK_READ_VAL 0xDEADBEEF

// Counter constructor
Counter::Counter(uint64_t *ptr) {
    valptr_ = ptr;
    isDpstats_ = 0;
}

// Counter constructor
Counter::Counter(uint64_t pal_addr) {
    valptr_ = (uint64_t *)pal_addr;
    isDpstats_ = 1;
#ifdef __x86_64__
    valcache_ = 0;
#endif
}

// Add adds a value to current counter value
error Counter::Add(uint64_t addn) {
    if (!isDpstats_) {
        *valptr_ += addn;
    } else {
#ifdef __x86_64__
        // mock read for testing purposes
        valcache_ += addn;
#elif __aarch64__
        uint64_t   pal_addr = (uint64_t)valptr_;
        uint64_t   data = MOCK_READ_VAL;
        auto rc = sdk::lib::pal_reg_read(pal_addr, (uint32_t *)&data, 2);
        if (rc == sdk::lib::PAL_RET_OK) {
            data += addn;
            sdk::lib::pal_reg_write(pal_addr, (uint32_t *)&data, 2);
        }
#endif
    }

    return error::OK();
}

// Incr increments the counter by one
error Counter::Incr() {
    return this->Add(1);
}

// Get returns current value of counter
uint64_t Counter::Get() {
    if (!isDpstats_) {
        return *valptr_;
    } else {
#ifdef __x86_64__
        // mock read for testing purposes
        return valcache_;
#elif __aarch64__
        uint64_t   pal_addr = (uint64_t)valptr_;
        uint64_t   data = MOCK_READ_VAL;
        auto rc = sdk::lib::pal_reg_read(pal_addr, (uint32_t *)&data, 2);
        if (rc == sdk::lib::PAL_RET_OK) {
            return data;
        }
#endif

        return 0;
    }
};

// Set sets the current value of counter
error Counter::Set(uint64_t val) {
    if (!isDpstats_) {
        *valptr_ = val;
    } else {
#ifdef __x86_64__
         valcache_ = val;
#elif __aarch64__
        uint64_t   pal_addr = (uint64_t)valptr_;
        sdk::lib::pal_reg_write(pal_addr, (uint32_t *)&val, 2);
#endif
    }

    return error::OK();
}

// Gauge constructor
Gauge::Gauge(double *ptr) {
    valptr_ = ptr;
    isDpstats_ = 0;
}

// Gauge constructor
Gauge::Gauge(uint64_t pal_addr) {
    valptr_ = (double *)pal_addr;
    isDpstats_ = 1;
#ifdef __x86_64__
    valcache_ = 0;
#endif
}

// Get returns current value of gauge
double Gauge::Get() {
    if (!isDpstats_) {
        return *valptr_;
    } else {
#ifdef __x86_64__
        // mock read for testing purposes
        return valcache_;
#elif __aarch64__
        uint64_t   pal_addr = (uint64_t)valptr_;
        double     data = MOCK_READ_VAL;
        auto rc = sdk::lib::pal_reg_read(pal_addr, (uint32_t *)&data, 2);
        if (rc == sdk::lib::PAL_RET_OK) {
            return data;
        }
#endif

        return 0;
    }
};

// Set sets the current value of gauge
error Gauge::Set(double val) {
    if (!isDpstats_) {
        *valptr_ = val;
    } else {
#ifdef __x86_64__
        valcache_ = val;
#elif __aarch64__
        uint64_t   pal_addr = (uint64_t)valptr_;
        sdk::lib::pal_reg_write(pal_addr, (uint32_t *)&val, 2);
#endif
    }

    return error::OK();
}

// GetDelphiShm returns a delphi shared memory object
delphi::shm::DelphiShmPtr DelphiMetrics::GetDelphiShm() {
    thread_local delphi::shm::DelphiShmPtr client_shm = make_shared<delphi::shm::DelphiShm>();
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
