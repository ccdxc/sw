#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/fte_softq.hpp"

using namespace fte;
using namespace std;

class fte_softq_test : public ::testing::Test {
protected:
    fte_softq_test() {
    }

    virtual ~fte_softq_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
        int a[100], b[100];
        for (int i=0; i <100; i++) {
            op.push_back((void *)(a+i));
            data.push_back((void *)(b+i));
        }
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }

    vector<void*> op;
    vector<void*> data;
};

void *op2, *data2;

TEST_F(fte_softq_test, enqueue_dqueue) {
    auto q = mpscq_t::alloc(4);

    for (int i = 0; i < 16; i++) {
        EXPECT_FALSE(q->dequeue(&op2, &data2));
        EXPECT_TRUE(q->enqueue(op[i], data[i]));
        EXPECT_TRUE(q->dequeue(&op2, &data2));
        EXPECT_EQ(op2, op[i]);
        EXPECT_EQ(data2, data[i]);
        EXPECT_FALSE(q->dequeue(&op2, &data2));
    }
}

TEST_F(fte_softq_test, queue_full) {
    auto q = mpscq_t::alloc(16);

    for (int i = 0; i < 16; i++){
        EXPECT_TRUE(q->enqueue(op[i], data[i]));
    }
    EXPECT_FALSE(q->enqueue(op[16], data[16]));

    for (int i = 0; i < 4; i++){
        EXPECT_TRUE(q->dequeue(&op2, &data2));
        EXPECT_EQ(op2, op[i]);
        EXPECT_EQ(data2, data[i]);
    }

    for (int i = 16; i < 20; i++){
        EXPECT_TRUE(q->enqueue(op[i], data[i]));
    }
    EXPECT_FALSE(q->enqueue(op[20], data[20]));

    for (int i = 4; i < 20; i++){
        EXPECT_TRUE(q->dequeue(&op2, &data2));
        EXPECT_EQ(op2, op[i]);
        EXPECT_EQ(data2, data[i]);
    }
    EXPECT_FALSE(q->dequeue(&op2, &data2));
}
