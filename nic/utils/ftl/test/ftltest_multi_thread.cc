//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "ftltest_base.hpp"
#include "ftltest_main.hpp"
#include "thread_pool.hpp"

using namespace std;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;

class multi_thread: public ftl_test_base {
public:
    void insert_bulk_async(uint32_t count, sdk_ret_t expret,
                           bool with_hash = false, uint32_t hash_32b = 0,
                           vector<future<sdk_ret_t>> *fut_rs = NULL) {
        for (auto i = 0; i < count; i++) {
            auto fut = enqueue_job(i, &ftl_test_base::insert_helper,
                                   this, i, expret, with_hash,
                                   hash_32b);
            if (fut_rs) {
                fut_rs->push_back(move(fut));
            }
        }
    }

    void remove_bulk_async(uint32_t count, sdk_ret_t expret,
                           bool with_hash = false, uint32_t hash_32b = 0,
                           vector<future<sdk_ret_t>> *fut_rs = NULL) {
        for (auto i = 0; i < count; i++) {
            auto fut = enqueue_job(i, &ftl_test_base::remove_helper,
                                   this, i, expret, with_hash,
                                   hash_32b);
            if (fut_rs) {
                fut_rs->push_back(move(fut));
            }
        }
    }

    void update_bulk_async(uint32_t count, sdk_ret_t expret,
                           bool with_hash = false, uint32_t hash_32b = 0,
                           vector<future<sdk_ret_t>> *fut_rs = NULL) {
        for (auto i = 0; i < count; i++) {
            auto fut = enqueue_job(i, &ftl_test_base::update_helper,
                                   this, i, expret, with_hash,
                                   hash_32b);
            if (fut_rs) {
                fut_rs->push_back(move(fut));
            }
        }
    }

    void get_bulk_async(uint32_t count, sdk_ret_t expret,
                        bool with_hash = false,
                        vector<future<sdk_ret_t>> *fut_rs = NULL) {
        for (auto i = 0; i < count; i++) {
            auto fut = enqueue_job(i, &ftl_test_base::get_helper,
                                   this, i, expret, with_hash);
            if (fut_rs) {
                fut_rs->push_back(move(fut));
            }
        }
    }

protected:
    sdk_trace_level_e trace_level;

    multi_thread() {
        trace_level = g_trace_level;
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
    }

    ~multi_thread() {
        g_trace_level = trace_level;
    }

    void get_stats() {
        sdk_table_stats_t tstat;
        sdk_table_api_stats_t astat;
        char buff[10000];

        for (auto i=0; i < g_no_of_threads; i++) {
            auto f = enqueue_job(i, &multi_thread::get_stats_,
                                 this, &tstat, &astat);
            f.get();
            table_stats.accumulate(&tstat);
            api_stats.accumulate(&astat);
            SDK_TRACE_INFO("=================== STATS THREAD ID: %d ===================", i);
            astat.print(buff, 10000);
            SDK_TRACE_INFO("%s", buff);
            tstat.print(buff, 10000);
            SDK_TRACE_INFO("%s", buff);
        }
    }

    void assign_thread_id() {
        for (auto i=0; i < g_no_of_threads; i++) {
            enqueue_job(i, &multi_thread::set_thread_id, this, i);
        }
    }

private:
    vector<ThreadPool*> thread_pools_;

    void SetUp() {
        ftl_test_base::SetUp();
        // Create thread pools
        for (auto i=0; i < g_no_of_threads; i++) {
            thread_pools_.push_back(new ThreadPool(1));
        }
        assign_thread_id();
    }

    void TearDown() {
        ftl_test_base::TearDown();
        // destroy thread pools
        for (auto i=0; i < g_no_of_threads; i++) {
            thread_pools_[i]->~ThreadPool();
        }
    }

    template<class F, class... Args>
    auto enqueue_job(uint32_t index, F&& f, Args&&... args)
        -> future<typename result_of<F(Args...)>::type> {
        auto worker_index =  index % g_no_of_threads;
        return thread_pools_[worker_index]->enqueue(forward<F>(f),
                                                    forward<Args>(args)...);
    }

    void get_stats_(sdk_table_stats_t *tstat,
                    sdk_table_api_stats_t *astat) {
        table->stats_get(astat, tstat);
    }
};

TEST_F(multi_thread, num16K)
{
    insert_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    get_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    update_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    remove_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
}

TEST_F(multi_thread, insert_iterate_remove)
{
    insert_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    remove_bulk_async(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    thread iterate_th(&multi_thread::iterate, this);
    iterate_th.join();
}

TEST_F(multi_thread, insert_remove_full_mesh_iterate)
{
    insert_bulk_async(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH,
                      HASH_VALUE);
    remove_bulk_async(MAX_COUNT, sdk::SDK_RET_OK, WITH_HASH,
                      HASH_VALUE);
}

TEST_F(multi_thread, insert1M_with_hash)
{
    insert_bulk_async(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
}

TEST_F(multi_thread, remove1M_with_hash)
{
    insert_bulk_async(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
    remove_bulk_async(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
}
