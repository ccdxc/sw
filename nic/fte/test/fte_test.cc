#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/fte.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/include/hal_mem.hpp"

using namespace fte;
using namespace std;

// test ctx with access to protected members
class test_ctx_t :  public ctx_t {
public:
    using ctx_t::init;

};

class fte_test : public ctx_t,  public ::testing::Test {
protected:
    fte_test() {
    }

    virtual ~fte_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        fte::unregister_features_and_pipelines();
    }

    vector<string> results;

    // Returns exec_handler_t that pushes the result to results vector
    exec_handler_t
    exec_handler(const string& result,
                 pipeline_action_t action = PIPELINE_CONTINUE) {
        return [=](ctx_t&) {
            this->results.push_back(result);
            return action;
        };
    }

    hal_ret_t add_register_feature(std::string name, exec_handler_t handler) {
        add_feature(name);
        return register_feature(name, handler);
    }

};

// add feature
TEST_F(fte_test, add_feature) {
    auto rc = add_feature("f1");
    EXPECT_EQ(rc, HAL_RET_OK);
}

// add dup feature
TEST_F(fte_test, add_dup_feature) {
    auto rc = add_feature("f1");
    EXPECT_EQ(rc, HAL_RET_OK);

    rc = add_feature("f1");
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}

// register non existing feature
TEST_F(fte_test, register_unk_feature) {
    auto rc = register_feature("f1", exec_handler("f1"));
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}


//  register valid feature
TEST_F(fte_test, register_feature) {
    auto rc = add_register_feature("f1", exec_handler("f1"));
    EXPECT_EQ(rc, HAL_RET_OK);
}

//  register-feature with null exec handler
TEST_F(fte_test, register_feature_nil) {
    auto rc = add_register_feature("f1", nullptr);
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}

//  register-pipeline
TEST_F(fte_test, register_pipeline) {
    lifqid_t lifq = {1,1,1};
    add_feature("f1");
    vector<string> features {"f1"};
    auto rc = register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "1", "1", features);
    EXPECT_EQ(rc, HAL_RET_OK);
}

// register duplicate pipeline
TEST_F(fte_test, register_pipeline_dup) {
    lifqid_t lifq = {1,1,1};
    auto rc = register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "1", "1", {});
    rc = register_pipeline("p2", sys::FWD_MODE_ANY, lifq, "1", "1", {});
    EXPECT_EQ(rc, HAL_RET_ENTRY_EXISTS);
}

// Register pipeline wih Unknown feature
TEST_F(fte_test, register_pipeline_unk) {
    lifqid_t lifq = {1,1,1};
    add_feature( "f1");
    vector<string> features {"f1", "f2"};
    auto rc = register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "1", "1", features);
    //EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
    EXPECT_EQ(rc, HAL_RET_OK);
}

TEST_F(fte_test, execute_pipeline) {
    add_feature("f1");
    register_feature("f1", exec_handler("f1"));
    add_feature("f2");
    register_feature("f2", exec_handler("f2"));
    add_feature("f3");
    register_feature("f3", exec_handler("f3"));

    vector<string> features {"f1", "f2", "f3"};
    lifqid_t lifq = {SERVICE_LIF_TCP_PROXY,1,1};
    register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "1", features);
    features = {"f1","f3"};
    lifq = {SERVICE_LIF_TCP_PROXY,1,2};
    register_pipeline("p2", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "2", features);

    ctx_t ctx = {};

    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY,1,1});
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1", "f2", "f3"}));

    results.clear();

    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY,1,2});
    rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1", "f3"}));
}

// execute unknown pipeline
TEST_F(fte_test, execute_pipeline_unk) {
    ctx_t ctx = {};
    ctx.set_arm_lifq({1,1,1});
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}


// pipeline with a block with PIPELINE_END action
TEST_F(fte_test, execute_pipeline_end) {
    add_feature("f1");
    register_feature("f1", exec_handler("f1"));
    add_feature("f2");
    register_feature("f2", exec_handler("f2"));
    add_feature("f3");
    register_feature("f3", exec_handler("f3"));
    add_feature("f4");
    register_feature("f4", exec_handler("f4", PIPELINE_END));

    vector<string> features{"f1", "f2", "f4", "f3"};
    lifqid_t lifq = {SERVICE_LIF_TLS_PROXY,1,1};
    register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TLS_PROXY", "1", features);

    ctx_t ctx = {};
    ctx.set_arm_lifq({SERVICE_LIF_TLS_PROXY,1,1});
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1","f2","f4"}));
}

// pipeline with a block with PIPELINE_RESTART action
TEST_F(fte_test, execute_pipeline_restart) {
    add_feature("f1");
    register_feature("f1", exec_handler("f1"));
    add_feature("f2");
    register_feature("f2", exec_handler("f2"));
    add_feature("f3");
    register_feature("f3", exec_handler("f3"));

    // p1 - run f1
    lifqid_t lifq = {SERVICE_LIF_TCP_PROXY,1,1}; 
    vector<string> features{"f1"};
    register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "1", features);

    // p2 - run f2 and goto p1
    add_feature("restart-p1");
    register_feature("restart-p1",  [](ctx_t &ctx) {
            ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY,1,1});
            return PIPELINE_RESTART;
        });
    features = {"f2", "restart-p1"};
    lifq = {SERVICE_LIF_TCP_PROXY,1,2};
    register_pipeline("p2", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "2", features);

    // p3 - run f3 and goto p2
    add_feature("restart-p2");
    register_feature("restart-p2",  [](ctx_t &ctx) {
            ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY,1,2});
            return PIPELINE_RESTART;
        });
    features = {"f3", "restart-p2"};
    lifq = {SERVICE_LIF_TCP_PROXY,1,3};
    register_pipeline("p3", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "3", features);

    // execute p3
    ctx_t ctx={};
    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY, 1, 3});
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f3", "f2", "f1"}));
}

// pipeline with wildcard LIFs
TEST_F(fte_test, execute_pipeline_wildcard) {
    add_feature("f1");
    register_feature("f1", exec_handler("f1"));
    add_feature("f2");
    register_feature("f2", exec_handler("f2"));
    add_feature("f3");
    register_feature("f3", exec_handler("f3"));
    add_feature("f4");
    register_feature("f4", exec_handler("f4"));

    vector<string> features;

    // p1 - {1, 1, 1} f1
    features = {"f1"};
    lifqid_t lifq = {SERVICE_LIF_TCP_PROXY,1,1};
    register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "1", features);

    // p2 - {1, 1, *} f2
    features = {"f2"};
    lifq = {SERVICE_LIF_TCP_PROXY,1,0};
    register_pipeline("p2", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "0", features, {}, {0x7FF, 0x7, 0});

    // p3 - {1, *, *} f3
    features = {"f3"};
    lifq = {SERVICE_LIF_TCP_PROXY,0,0};
    register_pipeline("p3", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TCP_PROXY", "0", features, {}, {0x7FF, 0, 0});

    // p4 - {*, *, *} f4
    features  = {"f4"};
    lifq = {SERVICE_LIF_TCP_PROXY,0,0};
    register_pipeline("p4", sys::FWD_MODE_ANY, lifq, "", "0", features, {}, {0, 0, 0});

    ctx_t ctx={};

    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY, 1, 1});
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f1"}));
    results.clear();

    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY, 1, 10});
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f2"}));
    results.clear();

    ctx.set_arm_lifq({SERVICE_LIF_TCP_PROXY, 2, 2});
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f3"}));
    results.clear();

    ctx.set_arm_lifq({2, 2, 2});
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f4"}));
    results.clear();
}

//test feature state
TEST_F(fte_test, ctx_state) {
    auto fn1 = [](ctx_t& ctx) {
        uint32_t *st1 = (uint32_t *)ctx.feature_state();
        EXPECT_NE(st1, nullptr);

        *st1 = 0x12345678;

        uint16_t *st2 = (uint16_t *)ctx.feature_state("f2");
        EXPECT_NE(st2, nullptr);

        for (int i = 0; i < 1024; i++) {
            st2[i] = i;
        }
        return PIPELINE_CONTINUE;
    };

    auto fn2 = [](ctx_t& ctx) {
        uint32_t *st1 =  (uint32_t *)ctx.feature_state("f1");
        EXPECT_EQ(*st1, 0x12345678);

        uint16_t *st2 =  (uint16_t *)ctx.feature_state();
        for (int i = 0; i < 1024; i++) {
            EXPECT_EQ(st2[i], i);
        }
        return PIPELINE_CONTINUE;
    };

    feature_info_t info = {};

    info.state_size = sizeof(uint32_t);
    add_feature("f1");
    register_feature("f1", fn1, info);

    info.state_size = 1024*sizeof(uint16_t);
    add_feature("f2");
    register_feature("f2", fn2, info);

    vector<string> features{"f1", "f2"};
    lifqid_t  lifq = {SERVICE_LIF_TLS_PROXY,1,1};
    register_pipeline("p1", sys::FWD_MODE_ANY, lifq, "SERVICE_LIF_TLS_PROXY", "1", features);


    uint16_t num_features;
    size_t sz = feature_state_size(&num_features);
    feature_state_t *st = (feature_state_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_FTE, sz);

    test_ctx_t ctx = {};
    ctx.init({SERVICE_LIF_TLS_PROXY,1,1}, st, num_features);
    execute_pipeline(ctx);
    HAL_FREE(hal::HAL_MEM_ALLOC_FTE, st);
}
