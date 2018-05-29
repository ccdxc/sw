#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/alg_tftp/core.hpp"
#include "tftp_test.hpp"

using namespace hal::plugins::alg_tftp;
using namespace session;
using namespace fte;
using namespace hal::plugins::sfw;

hal_handle_t tftp_test::s_e2e_eph, tftp_test::c_e2e_eph;

void *start_server (void *ptr)
{
    tftp_test      *test = (tftp_test *)ptr;
    std::string     cmd;
    cmd = " /bin/tftpy_server.py -i 64.0.0.1 -r /sw/nic";
    test->run_service(test->s_e2e_eph, cmd);

    return NULL;
}

void *start_client (void *ptr)
{
    tftp_test      *test = (tftp_test *)ptr;
    std::string     cmd;
    cmd = " /bin/tftpy_client.py -H 64.0.0.1 -D /hal/plugins/alg_tftp/test/tftp_file.txt";
    test->run_service(test->c_e2e_eph, cmd);

    return NULL;
}

void *process_packets (void *ptr)
{
    tftp_test     *test = (tftp_test *)ptr;
    test->process_e2e_packets();
    
    return NULL;
}

TEST_F(tftp_test, tftp_e2e)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;

    SetUpE2ETestCase();
    pthread_create(&mThreadID1, NULL, start_server, (void *)this);
    pthread_create(&mThreadID2, NULL, start_client, (void *)this);
    pthread_create(&mThreadID3, NULL, process_packets, (void *)this);

    pthread_join(mThreadID2, NULL);
    pthread_cancel(mThreadID1);
    pthread_cancel(mThreadID3);    
}
