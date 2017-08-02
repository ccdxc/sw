#include <gtest/gtest.h>
#include <vector>

#include "../fte.hpp"

using namespace fte;
using namespace std;

class fte_test : public ::testing::Test {
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
};

//  register valid feature
TEST_F(fte_test, register_feature) {
    auto rc = register_feature(1, "f1", exec_handler("f1"));
    EXPECT_EQ(rc, HAL_RET_OK);
}

//  register-feature with null exec handler
TEST_F(fte_test, register_feature_nil) {
    auto rc = register_feature(1, "f1", nullptr);
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}

//  register duplicate feature 
TEST_F(fte_test, register_feature_dup) {
    register_feature(1, "f1", exec_handler("f1"));
    auto rc = register_feature(1, "f2", exec_handler("f2"));
    EXPECT_EQ(rc, HAL_RET_DUP_INS_FAIL);
}

//  register-pipeline
TEST_F(fte_test, register_pipeline) {
    register_feature(1, "f1", exec_handler("f1"));
    vector<feature_id_t> features {1};
    auto rc = register_pipeline("p1", {1,1,1}, &features[0], features.size());
    EXPECT_EQ(rc, HAL_RET_OK);
}

// register duplicate pipeline
TEST_F(fte_test, register_pipeline_dup) {
    auto rc = register_pipeline("p1", {1,1,1}, nullptr, 0);
    rc = register_pipeline("p2", {1,1,1}, nullptr, 0);
    EXPECT_EQ(rc, HAL_RET_DUP_INS_FAIL);
}

// Register pipeline wih Unknown feature
TEST_F(fte_test, register_pipeline_unk) {
    register_feature(1, "f1", exec_handler("f1"));
    vector<feature_id_t> features {1, 2};
    auto rc = register_pipeline("p1", {1,1,1}, &features[0], features.size());
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}

// execute pipeline
TEST_F(fte_test, execute_pipeline) {
    register_feature(1, "f1", exec_handler("f1"));
    register_feature(2, "f2", exec_handler("f2"));
    register_feature(3, "f3", exec_handler("f3"));

    vector<feature_id_t> features {1,2,3};
    register_pipeline("p1", {1,1,1}, &features[0], features.size());
    features = {1,3};
    register_pipeline("p2", {1,1,2}, &features[0], features.size());

    ctx_t ctx;

    ctx.arm_lifq = {1,1,1};
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1", "f2", "f3"}));

    results.clear();

    ctx.arm_lifq = {1,1,2};
    rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1", "f3"}));
}

// execute unknown pipeline
TEST_F(fte_test, execute_pipeline_unk) {
    ctx_t ctx;
    ctx.arm_lifq = {1,1,1};
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_INVALID_ARG);
}


// pipeline with a block with PIPELINE_END action
TEST_F(fte_test, execute_pipeline_end) {
    register_feature(1, "f1", exec_handler("f1"));
    register_feature(2, "f2", exec_handler("f2"));
    register_feature(3, "f3", exec_handler("f3"));
    register_feature(4, "f4", exec_handler("f4", PIPELINE_END));

    vector<feature_id_t> features{1,2,4,3};
    register_pipeline("p1", {2,1,1}, &features[0], features.size());

    ctx_t ctx;
    ctx.arm_lifq = {2,1,1};
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f1","f2","f4"}));
}

// pipeline with a block with PIPELINE_RESTART action
TEST_F(fte_test, execute_pipeline_restart) {
    register_feature(1, "f1", exec_handler("f1"));
    register_feature(2, "f2", exec_handler("f2"));
    register_feature(3, "f3", exec_handler("f3"));

    // p1 - run f1
    vector<feature_id_t> features{1};
    register_pipeline("p1", {1,1,1}, &features[0], features.size());

    // p2 - run f2 and goto p1
    register_feature(111, "restart-p1", [](ctx_t &ctx) {
            ctx.arm_lifq = {1,1,1};
            return PIPELINE_RESTART;
        });
    features = {2, 111}; 
    register_pipeline("p2", {1,1,2}, &features[0], features.size());

    // p3 - run f3 and goto p2
    register_feature(112, "restart-p2", [](ctx_t &ctx) {
            ctx.arm_lifq = {1,1,2};
            return PIPELINE_RESTART;
        });
    features = {3, 112}; 
    register_pipeline("p3", {1,1,3}, &features[0], features.size());

    // execute p3
    ctx_t ctx;
    ctx.arm_lifq = {1, 1, 3};
    auto rc = execute_pipeline(ctx);
    EXPECT_EQ(rc, HAL_RET_OK);
    EXPECT_EQ(results, vector<string>({"f3", "f2", "f1"}));
}

// pipeline with wildcard LIFs
TEST_F(fte_test, execute_pipeline_wildcard) {
    register_feature(1, "f1", exec_handler("f1"));
    register_feature(2, "f2", exec_handler("f2"));
    register_feature(3, "f3", exec_handler("f3"));
    register_feature(4, "f4", exec_handler("f4"));

    vector<feature_id_t> features;

    // p1 - {1, 1, 1} f1
    features = {1};
    register_pipeline("p1", {1,1,1}, &features[0], features.size());

    // p2 - {1, 1, *} f2
    features = {2};
    register_pipeline("p2", {1,1,0}, &features[0], features.size(), {0x7FF, 0x7, 0});

    // p3 - {1, *, *} f3
    features = {3};
    register_pipeline("p3", {1,0,0}, &features[0], features.size(), {0x7FF, 0, 0});

    // p4 - {*, *, *} f4
    features  = {4};
    register_pipeline("p4", {0,0,0}, &features[0], features.size(), {0, 0, 0});

    ctx_t ctx;

    ctx.arm_lifq = {1, 1, 1};
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f1"}));
    results.clear();

    ctx.arm_lifq = {1, 1, 10};
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f2"}));
    results.clear();

    ctx.arm_lifq = {1, 2, 2};
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f3"}));
    results.clear();

    ctx.arm_lifq = {2, 2, 2};
    execute_pipeline(ctx);
    EXPECT_EQ(results, vector<string>({"f4"}));
    results.clear();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
