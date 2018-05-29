#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/alg_ftp/core.hpp"
#include "ftp_test.hpp"
#include <sched.h>

using namespace hal::plugins::alg_ftp;
using namespace session;
using namespace fte;
using namespace hal::plugins::sfw;

hal_handle_t ftp_test::s_e2e_eph, ftp_test::c_e2e_eph;

void *start_server (void *ptr)
{
    ftp_test       *test = (ftp_test *)ptr;
    std::string     cmd;
    cmd = " python3 -m pyftpdlib -i 64.0.0.1 -p 21 -u admin -P root -d /sw/nic";
    test->run_service(test->s_e2e_eph, cmd);

    return NULL;
}

void *start_active_client (void *ptr)
{
    ftp_test      *test = (ftp_test *)ptr;
    std::string    cmd;
    cmd = " wget ftp://admin:root@64.0.0.1/hal/plugins/alg_ftp/test/ftp_file.txt --no-passive-ftp";
    sleep(1);
    test->run_service(test->c_e2e_eph, cmd);

    return NULL;
}

void *start_passive_client (void *ptr)
{
    ftp_test      *test = (ftp_test *)ptr;
    std::string    cmd;
    cmd = " wget ftp://admin:root@64.0.0.1/hal/plugins/alg_ftp/test/ftp_file.txt";
    sleep(1);
    test->run_service(test->c_e2e_eph, cmd);

    return NULL;
}

void *process_packets (void *ptr)
{
    ftp_test     *test = (ftp_test *)ptr;
    test->process_e2e_packets();

    return NULL;
}

TEST_F(ftp_test, ftp_e2e_active)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;

    SetUpE2ETestCase();
    pthread_create(&mThreadID1, NULL, start_server, (void *)this);
    pthread_create(&mThreadID2, NULL, start_active_client, (void *)this);
    pthread_create(&mThreadID3, NULL, process_packets, (void *)this);

    pthread_join(mThreadID2, NULL);
    pthread_cancel(mThreadID1);
    pthread_cancel(mThreadID3);
    std::system("rm ftp_file.txt");
}

TEST_F(ftp_test, ftp_e2e_passive)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;

    pthread_create(&mThreadID1, NULL, start_server, (void *)this);
    pthread_create(&mThreadID2, NULL, start_passive_client, (void *)this);
    pthread_create(&mThreadID3, NULL, process_packets, (void *)this);

    pthread_join(mThreadID2, NULL);
    pthread_cancel(mThreadID1);
    pthread_cancel(mThreadID3);
    fte_base_test::CleanUpE2ETestCase();
    std::system("rm ftp_file.txt");
}
