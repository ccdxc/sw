#include "fte_base_test.hpp"
#include <gtest/gtest.h>
#include "nic/fte/fte.hpp"
#include <pthread.h>

using namespace fte;

class fte_session_test;

class fte_session_test : public fte_base_test {
protected:
    fte_session_test() {}

    virtual ~fte_session_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        fte_base_test::SetUpTestCase();

        // create topo
        hal_handle_t vrfh = add_vrf();
        hal_handle_t nwh = add_network(vrfh, 0x0A000000, 8, 0xAABB0A000000);
        hal_handle_t l2segh = add_l2segment(nwh, 100);
        hal_handle_t intfh1 = add_uplink(1);
        hal_handle_t intfh2 =  add_uplink(2);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001 , 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002 , 0xAABB0A000002, 0);
    }

    static hal_handle_t client_eph, server_eph;

public:
    static void fte_session_create();
};

hal_handle_t fte_session_test::client_eph, fte_session_test::server_eph;

void fte_session_test::fte_session_create()
{
    hal_ret_t  ret = HAL_RET_OK;

    Tins::TCP tcp = Tins::TCP(100, 101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
}

void* send_flow_miss(void *ptr)
{
    fte_session_test *test = (fte_session_test *)ptr;

    for (int i=0; i<10; i++) {
        test->fte_session_create();
    }

    return NULL;
}

void* send_delete_all(void *ptr)
{
    SessionDeleteResponseMsg delrsp;

    for (int i=0; i<10; i++) {
         hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
         hal::session_delete_all(&delrsp);
         hal::hal_cfg_db_close();
    }
 
    return NULL;
}

void* send_get_all(void *ptr)
{
    SessionGetResponseMsg    getrsp;

    for (int i=0; i<10; i++) {
         hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
         hal::session_get_all(&getrsp);
         hal::hal_cfg_db_close();
    }

    return NULL;
}

TEST_F(fte_session_test, session_create)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;
    pthread_create(&mThreadID1, NULL, send_flow_miss, (void *)this);
    pthread_create(&mThreadID2, NULL, send_delete_all, NULL);
    pthread_create(&mThreadID3, NULL, send_get_all, NULL); 
    pthread_join(mThreadID1, NULL);
    pthread_join(mThreadID2, NULL);
    pthread_join(mThreadID3, NULL);
}
