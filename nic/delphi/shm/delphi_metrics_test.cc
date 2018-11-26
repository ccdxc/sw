// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
#include <unistd.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include "gtest/gtest.h"

#include "nic/delphi/shm/delphi_metrics.hpp"
#include "nic/sdk/include/sdk/pal.hpp"

namespace {
using namespace std;

// forward declaration
class TestMetric;
typedef std::shared_ptr<TestMetric> TestMetricPtr;

// forward declaration
class TestMetricsIterator;

// test_metric_t: c-struct for test metric
typedef struct test_metric_ {
    uint64_t   rx_counter;
    uint64_t   tx_counter;
    double     rx_rate;
    double     tx_rate;
} test_metric_t;

// TestMetric class
class TestMetric : public delphi::metrics::DelphiMetrics {
private:
    delphi::metrics::CounterPtr   rx_counter_;
    delphi::metrics::CounterPtr   tx_counter_;
    delphi::metrics::GaugePtr     rx_rate_;
    delphi::metrics::GaugePtr     tx_rate_;

    int32_t                       key_;
    char                          *shm_ptr_;
    uint64_t                      pal_addr_;
public:
    TestMetric(int32_t key, char *ptr);
    TestMetric(int32_t key, uint64_t pal_addr);
    TestMetric(char *kptr, char *vptr) : TestMetric(*(int32_t *)kptr, vptr){ };
    TestMetric(char *kptr, uint64_t pal_addr) : TestMetric(*(int32_t *)kptr, pal_addr){ };
    int32_t GetKey() { return key_; };
    void * Raw() { return shm_ptr_; };
    delphi::metrics::CounterPtr RxCounter() { return rx_counter_; };
    delphi::metrics::CounterPtr TxCounter() { return tx_counter_; };
    delphi::metrics::GaugePtr RxRate() { return rx_rate_; };
    delphi::metrics::GaugePtr TxRate() { return tx_rate_; };
    static int32_t Size();
    static delphi::error  CreateTable();
    static TestMetricPtr  NewTestMetric(int32_t key);
    static TestMetricPtr  NewDpTestMetric(int32_t key, uint64_t pal_addr);
    static delphi::error  Publish(int32_t key, test_metric_t *mptr);
    static TestMetricPtr Find(int32_t key);
    static TestMetricPtr FindDp(int32_t key);
    delphi::error Delete();
    virtual string DebugString();
    static TestMetricsIterator Iterator();
};
REGISTER_METRICS(TestMetric);

class TestMetricsIterator {
public:
    explicit TestMetricsIterator(delphi::shm::TableIterator tbl_iter) {
        tbl_iter_ = tbl_iter;
    }
    inline void Next() {
        tbl_iter_.Next();
    }
    inline TestMetricPtr Get() {
        int32_t *key = (int32_t *)tbl_iter_.Key();
        if (!tbl_iter_.IsDpstats()) {
            return make_shared<TestMetric>(*key, tbl_iter_.Value());
        } else {
            uint64_t pal_addr = *(uint64_t *)tbl_iter_.Value();
            return make_shared<TestMetric>(*key, pal_addr);
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
};

// TestMetric constructor
TestMetric::TestMetric(int32_t key, char *ptr) {
    key_ = key;
    shm_ptr_ = ptr;

    // instantiate fields
    rx_counter_ = make_shared<delphi::metrics::Counter>((uint64_t *)ptr);
    ptr += delphi::metrics::Counter::Size();
    tx_counter_ = make_shared<delphi::metrics::Counter>((uint64_t *)ptr);
    ptr += delphi::metrics::Counter::Size();
    rx_rate_ = make_shared<delphi::metrics::Gauge>((double *)ptr);
    ptr += delphi::metrics::Gauge::Size();
    tx_rate_ = make_shared<delphi::metrics::Gauge>((double *)ptr);
    ptr += delphi::metrics::Gauge::Size();
}

// TestMetric constructor
TestMetric::TestMetric(int32_t key, uint64_t pal_addr) {
    key_ = key;
    pal_addr_ = pal_addr;

    // instantiate fields
    rx_counter_ = make_shared<delphi::metrics::Counter>(pal_addr);
    pal_addr += delphi::metrics::Counter::Size();
    tx_counter_ = make_shared<delphi::metrics::Counter>(pal_addr);
    pal_addr += delphi::metrics::Counter::Size();
    rx_rate_ = make_shared<delphi::metrics::Gauge>(pal_addr);
    pal_addr += delphi::metrics::Gauge::Size();
    tx_rate_ = make_shared<delphi::metrics::Gauge>(pal_addr);
    pal_addr += delphi::metrics::Gauge::Size();
}


int32_t TestMetric::Size() {
    int32_t sz = 0;

    // calculate the shared memory size
    sz += delphi::metrics::Counter::Size();
    sz += delphi::metrics::Counter::Size();
    sz += delphi::metrics::Gauge::Size();
    sz += delphi::metrics::Gauge::Size();

    return sz;
}

// CreateTable creates a table for metrics
delphi::error  TestMetric::CreateTable() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("TestMetric", DEFAULT_METRIC_TBL_SIZE);

    return delphi::error::OK();
}

// NewTestMetric creates a new test metric
TestMetricPtr TestMetric::NewTestMetric(int32_t key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("TestMetric", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    char *shmptr = (char *)tbl->Create((char *)&key, sizeof(key), TestMetric::Size());

    // return an instance of TestMetric
    return make_shared<TestMetric>(key, shmptr);
}

// NewDpTestMetric creates a new test metric in PAL memory
TestMetricPtr TestMetric::NewDpTestMetric(int32_t key, uint64_t pal_addr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("TestMetric", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    auto err = tbl->CreateDpstats((char *)&key, sizeof(key), pal_addr, TestMetric::Size());
    assert(err.IsOK());

    // return an instance of TestMetric
    return make_shared<TestMetric>(key, pal_addr);
}

// Find finds a metrics by key
TestMetricPtr TestMetric::Find(int32_t key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // find the key
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("TestMetric");
    char *shmptr = (char *)tbl->Find((char *)&key, sizeof(key));
    if (shmptr == NULL) {
        return NULL;
    }

    // return an instance of TestMetric
    return make_shared<TestMetric>(key, shmptr);
}

// FindDp finds a metrics by key
TestMetricPtr TestMetric::FindDp(int32_t key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // find the key
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("TestMetric");
    uint64_t *pal_addr = (uint64_t *)tbl->Find((char *)&key, sizeof(key));
    if (pal_addr == NULL) {
        return NULL;
    }

    // return an instance of TestMetric
    return make_shared<TestMetric>(key, *pal_addr);
}

// Publish publishes a metric atomically
delphi::error TestMetric::Publish(int32_t key, test_metric_t *mptr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("TestMetric");
    assert(tbl != NULL);

    // publish to hash table
    return tbl->Publish((char *)&key, sizeof(key), (char *)mptr, TestMetric::Size());
}

// Delete deletes the metric instance
delphi::error TestMetric::Delete() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("TestMetric");
    assert(tbl != NULL);

    // delete the key
    return tbl->Delete((char *)&key_, sizeof(key_));
}

// Iterator returns an iterator for metrics
TestMetricsIterator TestMetric::Iterator() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("TestMetric");
    assert(tbl != NULL);

    return TestMetricsIterator(tbl->Iterator());
}

// DebugString prints the contents of the metrics object
string TestMetric::DebugString() {
    stringstream outstr;
    outstr << "TestMetric {" << endl;
    outstr << "    Key: " << key_ << endl;
    outstr << "    RxCounter: " << RxCounter()->Get() << endl;
    outstr << "    TxCounter: " << TxCounter()->Get() << endl;
    outstr << "    RxRate: " << RxRate()->Get() << endl;
    outstr << "    TxRate: " << TxRate()->Get() << endl;
    outstr << "}" << endl;

    return outstr.str();
}

class DelphiMetricTest : public testing::Test {
protected:
    delphi::shm::DelphiShmPtr srv_shm_;
public:
    virtual void SetUp() {
        usleep(1000);
        // create the server
        srv_shm_ = make_shared<delphi::shm::DelphiShm>();
        delphi::error err = srv_shm_->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true);
        assert(err.IsOK());

#ifdef __x86_64__
        assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_MOCK) == sdk::lib::PAL_RET_OK);
#endif
    }
    virtual void TearDown() {
        usleep(1000);
        srv_shm_->MemUnmap();
    }
};

TEST_F(DelphiMetricTest, BasicMetricsTest) {
    usleep(1000);

    // create a new metric
    int32_t key = 100;
    TestMetricPtr tmptr = TestMetric::NewTestMetric(key);
    ASSERT_TRUE((tmptr != NULL)) << "Failed to create the metric";

    // verify default values are zero
    ASSERT_EQ(tmptr->RxCounter()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(tmptr->RxRate()->Get(), 0) << "invalid default value for gauge";

    // increment the counter and verify its incremented
    tmptr->RxCounter()->Incr();
    tmptr->RxCounter()->Incr();
    ASSERT_EQ(tmptr->RxCounter()->Get(), 2) << "invalid value for counter";

    // add to counter and verify its value
    tmptr->TxCounter()->Add(3);
    tmptr->TxCounter()->Add(3);
    ASSERT_EQ(tmptr->TxCounter()->Get(), 6) << "invalid value for counter";
    LogInfo("Counter values: {}, {}", tmptr->RxCounter()->Get(), tmptr->TxCounter()->Get());
    srv_shm_->Kvstore()->DumpInfo();

    // test setting the gauge value
    tmptr->RxRate()->Set(5);
    ASSERT_EQ(tmptr->RxRate()->Get(), 5) << "invalid value for gauge";
    tmptr->RxRate()->Set(3);
    ASSERT_EQ(tmptr->RxRate()->Get(), 3) << "invalid value for gauge";

    // display metrics
    printf("Metrics: \n%s\n", tmptr->DebugString().c_str());

    // dump the hash entry
    delphi::shm::TableMgrUptr tbl = srv_shm_->Kvstore()->Table("TestMetric");
    tbl->DumpEntry((char *)&key, sizeof(key));
    void *ptr = tbl->Find((char *)&key, sizeof(key));
    ASSERT_TRUE((ptr != NULL)) << "key was not found in hash table";

    // iterate over all stats and count it
    int iter_count = 0;
    for (auto iter = TestMetric::Iterator(); iter.IsNotNil(); iter.Next()) {
        auto tmp = iter.Get();
        printf("Iterator Metrics: %s\n", tmp->DebugString().c_str());
        iter_count++;
    }
    ASSERT_EQ(iter_count, 1) << "Got invalid number of metrics";

    // try generic iterator
    iter_count = 0;
    delphi::metrics::DelphiMetricsIterator miter("TestMetric");
    for (; miter.IsNotNil(); miter.Next()) {
        auto tmp = miter.Get();
        printf("DelphiMetricsIterator: %s\n", tmp->DebugString().c_str());
        iter_count++;
    }
    ASSERT_EQ(iter_count, 1) << "Got invalid number of metrics";

    delphi::error err = tmptr->Delete();
    ASSERT_TRUE(err.IsOK()) << "Failed to delete the metric";
    srv_shm_->Kvstore()->DumpInfo();

    // verify the key is removed from the hash table
    ptr = tbl->Find((char *)&key, sizeof(key));
    ASSERT_TRUE((ptr == NULL)) << "key was not deleted from the hash table";
}

TEST_F(DelphiMetricTest, TestDpMetric) {
    // create a new datapath metric
    int32_t key = 100;
    TestMetricPtr tmptr = TestMetric::NewDpTestMetric(key, (uint64_t)0x1001);
    ASSERT_TRUE((tmptr != NULL)) << "Failed to create the dp metric";
    delphi::shm::TableMgrUptr tbl = srv_shm_->Kvstore()->Table("TestMetric");

    // set the values to zero and read it back
    tmptr->RxCounter()->Set(0);
    tmptr->RxRate()->Set(0);
    ASSERT_EQ(tmptr->RxCounter()->Get(), 0) << "invalid value for counter";
    ASSERT_EQ(tmptr->RxRate()->Get(), 0) << "invalid value for gauge";

    // make sure we can find the key
    auto fptr = TestMetric::FindDp(key);
    ASSERT_TRUE((fptr != NULL)) << "Failed to find the dp metric";

    // increment the counter and verify its incremented
    tmptr->RxCounter()->Incr();
    tmptr->RxCounter()->Incr();
    ASSERT_EQ(tmptr->RxCounter()->Get(), 2) << "invalid value for counter";
    ASSERT_EQ(fptr->RxCounter()->Get(), 0) << "invalid value for counter";

    // iterate over all stats and count it
    int iter_count = 0;
    for (auto iter = TestMetric::Iterator(); iter.IsNotNil(); iter.Next()) {
        auto tmp = iter.Get();
        printf("Iterator Metrics: %s\n", tmp->DebugString().c_str());
        iter_count++;
    }
    ASSERT_EQ(iter_count, 1) << "Got invalid number of metrics";

    delphi::error err = tmptr->Delete();
    ASSERT_TRUE(err.IsOK()) << "Failed to delete the metric";

    // verify the key is removed from the hash table
    void *ptr = tbl->Find((char *)&key, sizeof(key));
    ASSERT_TRUE((ptr == NULL)) << "key was not deleted from the hash table";
}

TEST_F(DelphiMetricTest, TestPublish) {
    usleep(1000);

    // create a new metric
    int32_t key = 100;
    /*
    TestMetricPtr tmptr = TestMetric::NewTestMetric(key);
    ASSERT_TRUE((tmptr != NULL)) << "Failed to create the metric";
    delphi::shm::TableMgrUptr tbl = srv_shm_->Kvstore()->Table("TestMetric");

    // verify default values are zero
    ASSERT_EQ(tmptr->RxCounter()->Get(), 0) << "invalid default value for counter";
    ASSERT_EQ(tmptr->RxRate()->Get(), 0) << "invalid default value for gauge";
*/

    // create the table
    TestMetric::CreateTable();

    // publish new values
    test_metric_t stats = {
        rx_counter: 10,
        tx_counter: 10,
        rx_rate: 10.0,
        tx_rate: 10.0,
    };
    TestMetric::Publish(key, &stats);

    // display metrics
    delphi::shm::TableMgrUptr tbl = srv_shm_->Kvstore()->Table("TestMetric");
    tbl->DumpEntry((char *)&key, sizeof(key));
    TestMetricPtr tmptr = TestMetric::Find(key);
    ASSERT_TRUE((tmptr != NULL)) << "key was not found in hash table";
    printf("Metrics: \n%s\n", tmptr->DebugString().c_str());

    // verify new values are set
    ASSERT_EQ(tmptr->RxCounter()->Get(), 10) << "invalid value for counter";

    delphi::error err = tmptr->Delete();
    ASSERT_TRUE(err.IsOK()) << "Failed to delete the metric";

    // verify the key is removed from the hash table
    void *ptr = tbl->Find((char *)&key, sizeof(key));
    ASSERT_TRUE((ptr == NULL)) << "key was not deleted from the hash table";
}
} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
