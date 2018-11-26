// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SDK_METRICS_H_
#define _DELPHI_SDK_METRICS_H_

#include "nic/delphi/utils/utils.hpp"
#include "nic/delphi/shm/shm.hpp"

namespace delphi {
namespace metrics {

#define DEFAULT_METRIC_TBL_SIZE 2500

// Counter class implements a shared memory based counter
class Counter {
public:
    Counter(uint64_t *ptr);
    Counter(uint64_t pal_addr);
    error Add(uint64_t addn);
    error Incr();
    error Set(uint64_t val);
    uint64_t Get();
    static inline int32_t Size() { return sizeof(uint64_t);};
private:
    uint64_t   *valptr_;
    bool       isDpstats_;
#ifdef __x86_64__
    uint64_t   valcache_; // mock values for testing purposes
#endif
};
typedef std::shared_ptr<Counter> CounterPtr;

// Gauge implements a shared memory based gauge
class Gauge {
public:
    Gauge(double *ptr);
    Gauge(uint64_t pal_addr);
    error Set(double val);
    double Get();
    static inline int32_t Size() { return sizeof(double);};
private:
    double     *valptr_;
    bool       isDpstats_;
#ifdef __x86_64__
    double     valcache_; // mock values for testing purposes
#endif
};
typedef std::shared_ptr<Gauge> GaugePtr;

// forward declaration
class DelphiMetrics;
typedef std::shared_ptr<DelphiMetrics> DelphiMetricsPtr;

class MetricsFactory
{
public:
    virtual DelphiMetricsPtr Create(char *key, char *val) = 0;
    virtual DelphiMetricsPtr Create(char *key, uint64_t pal_addr) = 0;
};


// register a new kind of metrics
#define REGISTER_METRICS(klass) \
    class klass##MetricsFactory : public delphi::metrics::MetricsFactory { \
    public: \
        klass##MetricsFactory() \
        { \
            delphi::metrics::DelphiMetrics::RegisterMetrics(#klass, this); \
        } \
        delphi::metrics::DelphiMetricsPtr Create(char *key, char *val) { \
            return make_shared<klass>(key, val); \
        } \
        delphi::metrics::DelphiMetricsPtr Create(char *key, uint64_t pal_addr) { \
            return make_shared<klass>(key, pal_addr); \
        } \
    }; \
    static klass##MetricsFactory global_##klass##MetricsFactory;

// DelphiMetrics is the base class for all metrics objects
class DelphiMetrics {
public:
    DelphiMetrics() {};
    virtual string DebugString() = 0;
    virtual void *Raw() = 0;
    static delphi::shm::DelphiShmPtr GetDelphiShm();
    // GetFactoryMap gets the factory db
    static inline map<string, MetricsFactory*> *GetFactoryMap() {
        static map<string, MetricsFactory*> factories;
        return &factories;
    }
    // registers a factory class for each kind
    static inline void RegisterMetrics(string kind, MetricsFactory *factry) {
        try {
            map<string, MetricsFactory*> *fctries = DelphiMetrics::GetFactoryMap();
            LogDebug("Registering metrics kind {}", kind);
            fctries->insert(std::pair<string, MetricsFactory*>(kind, factry));
        } catch (...){}
    }

    // creates an object of specific kind
    static inline DelphiMetricsPtr Create(const string &name, char *key, char *val) {
        map<string, MetricsFactory*> fctries = *(DelphiMetrics::GetFactoryMap());
        MetricsFactory *fctry = fctries[name];
        assert(fctry != NULL);

        return fctry->Create(key, val);
    }
    static inline DelphiMetricsPtr Create(const string &name, char *key, uint64_t pal_addr) {
        map<string, MetricsFactory*> fctries = *(DelphiMetrics::GetFactoryMap());
        MetricsFactory *fctry = fctries[name];
        assert(fctry != NULL);

        return fctry->Create(key, pal_addr);
    }
};


class DelphiMetricsIterator {
public:
    explicit DelphiMetricsIterator(string met_name) {
        // get the shared memory object
        delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
        assert(shm != NULL);

        // get the table
        static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(met_name);
        assert(tbl != NULL);

        tbl_iter_ = tbl->Iterator();
        met_name_ = met_name;
    }
    inline void Next() {
        tbl_iter_.Next();
    }
    inline DelphiMetricsPtr Get() {
        if (!tbl_iter_.IsDpstats()) {
            return DelphiMetrics::Create(met_name_, tbl_iter_.Key(), tbl_iter_.Value());
        } else {
            uint64_t pal_addr = *(uint64_t *)tbl_iter_.Value();
            return DelphiMetrics::Create(met_name_, tbl_iter_.Key(), pal_addr);
        }
    }
    inline bool IsNil() {
        return tbl_iter_.IsNil();
    }
    inline bool IsNotNil() {
        return tbl_iter_.IsNotNil();
    }
private:
    delphi::shm::TableIterator tbl_iter_;
    string met_name_;
};

} // namespace metrics
} // namespace delphi

#endif // _DELPHI_SDK_METRICS_H_
