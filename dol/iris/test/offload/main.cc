
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/time.h>
#include "gflags/gflags.h"
#include "hal_if.hpp"
#include "utils.hpp"
#include "tests.hpp"
#include "logger.hpp"
#include "c_if.h"
#include "lib_model_client.h"
#include "crypto_rsa_testvec.hpp"
#include "crypto_asym.hpp"

DEFINE_uint64(hal_port, 50054, "TCP port of the HAL's gRPC server");
DEFINE_string(hal_ip, "localhost", "IP of HAL's gRPC server");
DEFINE_string(test_group, "", "Test group to run");
DEFINE_uint64(poll_interval, 60, "Polling interval in seconds");
DEFINE_uint64(long_poll_interval, 300,
              "Polling interval for longer running tests in seconds");
DEFINE_string(script_dir, "/sw/nic/third-party/nist-cavp",
              "Script directory path)");
DEFINE_string(engine_path, "/sw/nic/build/x86_64/iris/lib/libdol_engine.so",
              "SSL engine library full path");
DEFINE_string(nicmgr_config_file, "/sw/platform/src/app/nicmgrd/etc/accel.json",
              "nicmgr json configuration filename (full path)");

/*
 * Satisfy extern requirements for sstorage libs
 */
DEFINE_bool(rtl, false,
            "Test being run under RTL");
DEFINE_bool(with_rtl_skipverify, false,
            "Test being run under RTL with --skipverify in effect");
DEFINE_uint64(num_pdma_queues, 3,
              "number of queues for PDMA test (in power of 2)");
DEFINE_uint64(acc_scale_chain_replica, 0,
              "Accelerator chain replication count (in power of 2)");

DEFINE_uint64(acc_scale_submissions, 2,
              "number of accelerator queue submissions for scale testing (in power of 2)");
DEFINE_uint64(acc_scale_iters, 1,
              "Number of iterations for accelerator scale testing (0 = infinite)");
DEFINE_uint64(nvme_scale_iters, 64,
              "Number of iterations for NVME scale testing (0 = infinite)");
bool run_nicmgr_tests;
bool run_pdma_tests;
bool run_rsa_testvectors = true;
uint32_t run_acc_scale_tests_map;

size_t tcid;

class test_entry_t
{
public:
    test_entry_t(const string& test_name,
                 std::function<bool(void *)> test_fn,
                 void *test_param) :
        test_name(test_name),
        test_fn(test_fn),
        test_param(test_param),
        test_success(false)
    {
    }

    const string&                 test_name;
    std::function<bool(void *)>   test_fn;
    void                          *test_param;
    bool                          test_success;
};

/*
 * RSA testvectors
 */
const static vector<string>     rsa_testvectors = {
    "rsa-testvectors/SigGen15_186-3.txt",
    //"rsa-testvectors/SigGen931_186-3.txt",
};

/*
 * Runnable test suite
 */
static vector<test_entry_t>     test_suite;

void
sig_handler(int sig)
{
    void        *array[16];
    size_t      size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 16);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int
common_setup(void)
{
    // Initialize hal interface
    hal_if::init_hal_if();
    OFFL_FUNC_INFO("HAL client initialized");

    if (init_host_mem() < 0) {
        OFFL_FUNC_ERR("Host mem init failed (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("Host mem initialized");

    // Initialize storage hbm memory
    if (utils::hbm_buf_init() < 0) {
        OFFL_FUNC_ERR("HBM buf init failed (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("HBM buf initialized");

    // Initialize model client
    if (lib_model_connect() < 0) {
        OFFL_FUNC_ERR("Failed to connect with model (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("Model client initialized");

    crypto_asym::init(FLAGS_engine_path.c_str());
    tests::test_generic_eos_ignore();
    return 0;
}

static bool
rsa_testvectors_drive(void *test_param)
{
    vector<string>                            *testvectors;
    offload_base_params_t base_params;
    crypto_rsa::rsa_testvec_params_t          testvec_params;
    crypto_rsa::rsa_testvec_pre_push_params_t pre_params;
    crypto_rsa::rsa_testvec_push_params_t     push_params;
    crypto_rsa::rsa_testvec_t                 *rsa_testvec;
    uint32_t                                  failure_count;

    testvectors = static_cast<vector<string> *>(test_param);
    failure_count = 0;
    for (uint32_t i = 0; i < testvectors->size(); i++) {
        const string& vec_name(testvectors->at(i));

        OFFL_LOG_INFO("Running RSA vector {}", vec_name);
        rsa_testvec = new crypto_rsa::rsa_testvec_t(testvec_params.base_params(base_params));
        rsa_testvec->pre_push(pre_params.testvec_fname(FLAGS_script_dir + "/" +
                                                       vec_name));
        rsa_testvec->push(push_params.rsa_ring(crypto_asym::asym_ring));
        rsa_testvec->post_push();
        if (!rsa_testvec->full_verify()) {
            failure_count++;
        }
        delete rsa_testvec;
    }

    return failure_count == 0;
}

int
main(int argc,
     char **argv)
{
    struct timeval      start;
    struct timeval      end;
    bool                overall_success;

    offl::logger::init(true, "info");

    gflags::ParseCommandLineFlags(&argc, &argv, true);
    signal(SIGSEGV, sig_handler);

    std::cout << "Input - hal_port: "        << FLAGS_hal_port 
              << "\nInput - hal_ip: "        << FLAGS_hal_ip 
              << "\nScripts dir: "           << FLAGS_script_dir 
              << "\nOpenssl engine path: "   << FLAGS_engine_path
              << std::endl;

    if (common_setup() < 0)  {
        OFFL_FUNC_ERR("Failed common setup");
        return 1;
    }

    if (run_rsa_testvectors) {
        test_suite.push_back(test_entry_t("RSA", &rsa_testvectors_drive,
                                          (void *)&rsa_testvectors));
    }

    OFFL_LOG_INFO("Formed test suite with {} cases", test_suite.size());
    OFFL_LOG_INFO("Running test cases");

    overall_success = true;
    for (tcid = 0; tcid < test_suite.size(); tcid++) {
        auto test_entry = test_suite.at(tcid);

        gettimeofday(&start, NULL);
        OFFL_LOG_INFO(" Starting test #: {} name: {}", tcid, test_entry.test_name);

        test_entry.test_success = test_entry.test_fn(test_entry.test_param);
        gettimeofday(&end, NULL);
        OFFL_LOG_INFO(" Finished test #: {} name: {} status {} time {}",
                      tcid, test_entry.test_name, test_entry.test_success,
                      end.tv_sec - start.tv_sec);
        overall_success &= test_entry.test_success;
    }

    OFFL_LOG_INFO("Overall Report: {}",
                  overall_success ? "SUCCESS" : "FAILURE");

    if (offl::logger::logger()) {
        offl::logger::logger()->flush();
    }
    fflush(stdout);
    exit(overall_success ? 0 : 1);
}
