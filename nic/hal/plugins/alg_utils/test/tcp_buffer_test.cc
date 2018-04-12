#include <gtest/gtest.h>
#include <vector>
#include <utility>

#include "nic/hal/plugins/alg_utils/tcp_buffer.hpp"

using namespace hal::plugins::alg_utils;

class tcp_buffer_test : public ::testing::Test {
public:
    void tcp_buffer_init(uint32_t start_seq) {
        result_.len = 0;
        tcp_buffer_ = tcp_buffer_t::factory(start_seq, &result_, [](void *ctx, uint8_t *data, size_t len) {
                result_t *res = (result_t *)ctx;
                EXPECT_NE(len, 0);
                // std::cout << "out: start=" << res->len << " len=" << len << "\n";
                memcpy(res->buff + res->len, data, len);
                res->len += len;
                return len;
            });
    }

    static int myrandom (int i) { return std::rand()%i;}

protected:
    tcp_buffer_test() {
    }

    virtual ~tcp_buffer_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {

    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }

    tcp_buffer_t *tcp_buffer_;

    struct result_t {
        uint8_t buff[1500];
        size_t len;
    } result_;
};

TEST_F(tcp_buffer_test, insert_segment)
{
    uint8_t input[1500];
    std::vector<std::pair<uint32_t, uint32_t>> segments = {
        {0, 10},
        {15, 30},
        {8, 13},
        {35, 100},
        {5, 38},
        {120, 150},
        {100, 120},
        {125, 175},
        {200, 250},
        {300, 350},
        {370, 400},
        {175, 200},
        {260, 300},
        {250, 270},
        {325, 375},
        {370, 400},
        {400, 500},
        {495, 1000},
        {1000,1500},
    };
    
    for (uint i = 0; i < sizeof(input); i++) {
        input[i] = myrandom(255);
    }
    
    uint32_t start_seq = 0xFFFFFF00;
    tcp_buffer_init(start_seq);


    for (auto seg : segments) {
        uint32_t start, end;
        std::tie(start, end) = seg;
        // std::cout << "in: start=" << start << " len=" << end-start << "\n";
        tcp_buffer_->insert_segment(start_seq+start, input+start, end-start);
    }

    EXPECT_EQ(result_.len, sizeof(input));
    for (uint i = 0; i < sizeof(input); i++) {
        EXPECT_EQ(result_.buff[i], input[i]) << "i=" << i;
    }
}
