#include "rpc_test.hpp"
#include <gtest/gtest.h>
#include "nic/hal/plugins/alg_rpc/core.hpp"
#include "nic/fte/test/fte_base_test.hpp"

using namespace hal::plugins::alg_rpc;
using namespace fte;

hal_handle_t rpc_test::s_e2e_eph, rpc_test::c_e2e_eph;

void *start_server (void *ptr)
{
    rpc_test      *test = (rpc_test *)ptr;
    std::string    cmd, prefix_cmd;

    prefix_cmd = test->prefix_cmd(test->s_e2e_eph);
    cmd = prefix_cmd + " rpcbind -h 64.0.0.1 && PYTHONPATH=/sw/nic/hal/plugins/alg_rpc/test " + \
          prefix_cmd + " python -c 'import rpc; rpc.testsvr(\"64.0.0.1\")'";
    std::system(cmd.c_str());

    return NULL;
}

void *start_client (void *ptr)
{
    rpc_test      *test = (rpc_test *)ptr;
    std::string     cmd;
    cmd = "PYTHONPATH=/sw/nic/hal/plugins/alg_rpc/test " + test->prefix_cmd(test->c_e2e_eph) + \
           " python -c 'import rpc; rpc.testclt(\"64.0.0.1\")'";
    std::system(cmd.c_str());

    return NULL;
}

void *process_packets (void *ptr)
{
    rpc_test     *test = (rpc_test *)ptr;
    test->process_e2e_packets();

    return NULL;
}

TEST_F(rpc_test, sunrpc_e2e)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;

    SetUpE2ETestCase();
    pthread_create(&mThreadID1, NULL, start_server, (void *)this);
    sleep(5);
    pthread_create(&mThreadID3, NULL, process_packets, (void *)this);
    pthread_create(&mThreadID2, NULL, start_client, (void *)this);

    pthread_join(mThreadID2, NULL);
    pthread_cancel(mThreadID1);
    pthread_cancel(mThreadID3);
    std::system("pkill rpcbind && pkill python -c");
    fte_base_test::CleanUpE2ETestCase();
}
