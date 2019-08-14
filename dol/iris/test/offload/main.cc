
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
#include "nic/sdk/platform/utils/qstate_mgr.hpp"
#include "nic/sdk/platform/capri/capri_state.hpp"
#include "crypto_rsa_testvec.hpp"
#include "crypto_ecdsa_testvec.hpp"
#include "crypto_asym.hpp"

DEFINE_uint64(hal_port, 50054, "TCP port of the HAL's gRPC server");
DEFINE_string(hal_ip, "localhost", "IP of HAL's gRPC server");
DEFINE_string(test_group, "", "Test group to run");
DEFINE_uint64(poll_interval, 60, "Polling interval in seconds");
DEFINE_uint64(long_poll_interval, 300,
              "Polling interval for longer running tests in seconds");
#ifdef __x86_64__
DEFINE_string(script_dir, "/sw/nic/third-party/nist-cavp",
              "Script directory path)");
DEFINE_string(engine_path, "/sw/nic/build/x86_64/iris/lib/libtls_pse.so",
              "SSL engine library full path");
DEFINE_string(nicmgr_config_file, "/sw/platform/src/app/nicmgrd/etc/accel.json",
              "nicmgr json configuration filename (full path)");
#else
DEFINE_string(script_dir, "/data/third-party/nist-cavp",
              "Script directory path)");
DEFINE_string(engine_path, "/nic/lib/libtls_pse.so",
              "SSL engine library full path");
DEFINE_string(nicmgr_config_file, "/platform/etc/nicmgrd/accel.json",
              "nicmgr json configuration filename (full path)");
#endif

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
bool run_rsa_testvectors;
bool run_ecdsa_testvectors;
uint32_t run_acc_scale_tests_map;

/*
 * test case ID (legacy usage from storage)
 */
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

    const string                test_name;
    std::function<bool(void *)> test_fn;
    void                        *test_param;
    bool                        test_success;
};

/*
 * RSA testvectors
 */
typedef struct {
    string                      testvec_fname;
    crypto_rsa::rsa_key_create_type_t key_create_type;
    int                         pad_mode;
} rsa_vector_entry_t;

const static vector<rsa_vector_entry_t> rsa_testvectors =
{
    {"rsa-testvectors/SigGen15_186-3.req",
     crypto_rsa::RSA_KEY_CREATE_SIGN, RSA_PKCS1_PADDING},

    // For signature verification, we want to produce the ciphered output
    // so the encryption method is used.
    {"rsa-testvectors/SigVer15_186-3.req",
      crypto_rsa::RSA_KEY_CREATE_ENCRYPT, RSA_PKCS1_PADDING},

    //{"rsa-testvectors/SigGen931_186-3.req",
    // crypto_rsa::RSA_KEY_CREATE_SIGN, RSA_X931_PADDING},

#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    {"rsa-testvectors/SigGen15_186-3_TruncatedSHAs.req",
     crypto_rsa::RSA_KEY_CREATE_SIGN, RSA_PKCS1_PADDING},
    {"rsa-testvectors/SigVer15_186-3_TruncatedSHAs.req",
     crypto_rsa::RSA_KEY_CREATE_ENCRYPT, RSA_PKCS1_PADDING},
#endif
};

/*
 * ECDSA testvectors
 */
typedef struct {
    string                      testvec_fname;
    crypto_ecdsa::ecdsa_key_create_type_t key_create_type;
} ecdsa_vector_entry_t;

const static vector<ecdsa_vector_entry_t> ecdsa_testvectors =
{
    {"ecdsa-testvectors/SigGen.req",
     crypto_ecdsa::ECDSA_KEY_CREATE_SIGN},

    // For signature verification, we want to produce the ciphered output
    // so the encryption method is used.
    {"ecdsa-testvectors/SigVer.req",
      crypto_ecdsa::ECDSA_KEY_CREATE_VERIFY},

#ifdef OPENSSL_WITH_TRUNCATED_SHA_SUPPORT
    {"ecdsa-testvectors/SigGen_TruncatedSHAs.req",
     crypto_ecdsa::ECDSA_KEY_CREATE_SIGN},
    {"ecdsa-testvectors/SigVer_TruncatedSHAs.req",
     crypto_ecdsa::ECDSA_KEY_CREATE_VERIFY},
#endif

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

/*
 * Return certain flags for use by utils module so it does not
 * have to link in tests.cc
 */
uint64_t
poll_interval(void)
{
    return FLAGS_poll_interval;
}

uint64_t
long_poll_interval(void)
{
    return FLAGS_long_poll_interval;
}

static int
sdk_trace_cb (sdk_trace_level_e trace_level,
              const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    OFFL_LOG_DEBUG("{}", logbuf);
    va_end(args);
    return 0;
}

int
common_setup(void)
{
    sdk::lib::logger::init(sdk_trace_cb);

#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
                sdk::lib::PAL_RET_OK);
    if (init_host_mem() < 0) {
        OFFL_FUNC_ERR("Host mem init failed (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("Host mem initialized");

#elif 0 //__aarch64__
#if !defined(APOLLO) && !defined(ARTEMIS)
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) ==
                sdk::lib::PAL_RET_OK);
#endif
#endif

    // Initialize hal interface
    hal_if::init_hal_if();
    OFFL_FUNC_INFO("HAL client initialized");

    // Initialize storage hbm memory
    if (utils::hbm_buf_init() < 0) {
        OFFL_FUNC_ERR("HBM buf init failed (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("HBM buf initialized");

#ifdef __x86_64__

    // Initialize model client
    if (lib_model_connect() < 0) {
        OFFL_FUNC_ERR("Failed to connect with model (is model running?)");
        return -1;
    }
    OFFL_FUNC_INFO("Model client initialized");
#endif

    crypto_asym::init(FLAGS_engine_path.c_str());
    tests::test_generic_eos_ignore();
    return 0;
}

/*
 * Driver for RSA testvectors
 */
static bool
rsa_testvectors_run(void *test_param)
{
    vector<rsa_vector_entry_t>  *testvectors = 
                      static_cast<vector<rsa_vector_entry_t> *>(test_param);
    uint32_t        failure_count;

    /*
     * Run the vectors with the specified mem_type
     */
    auto testvectors_run = [](const rsa_vector_entry_t& entry,
                              dp_mem_type_t mem_type) -> bool
    {
        crypto_rsa::rsa_testvec_params_t          testvec_params;
        crypto_rsa::rsa_testvec_pre_push_params_t pre_params;
        crypto_rsa::rsa_testvec_push_params_t     push_params;
        crypto_rsa::rsa_testvec_t                 *rsa_testvec;
        offload_base_params_t                     base_params;
        bool                                      success;

        testvec_params.key_create_type(entry.key_create_type).
                       pad_mode(entry.pad_mode).
                       d_mem_type(mem_type).
                       e_mem_type(mem_type).
                       n_mem_type(mem_type).
                       msg_mem_type(mem_type).
                       sig_mem_type(mem_type).
                       dma_desc_mem_type(mem_type).
                       status_mem_type(mem_type);
        OFFL_LOG_INFO("Running RSA vector {} with mem_type {}",
                      entry.testvec_fname, mem_type);
        rsa_testvec = new crypto_rsa::rsa_testvec_t(
                          testvec_params.base_params(base_params));
        success = rsa_testvec->pre_push(pre_params.scripts_dir(FLAGS_script_dir).
                                                   testvec_fname(entry.testvec_fname));;
        if (success) {
            success = rsa_testvec->push(push_params.rsa_ring(crypto_asym::asym_ring));
        }
        if (success) {
            success = rsa_testvec->post_push();
        }
        if (success) {
            success = rsa_testvec->full_verify();
        }
        rsa_testvec->rsp_file_output(mem_type == DP_MEM_TYPE_HBM ?
                                     "hbm" : "host");
        delete rsa_testvec;
        return success;
    };

    failure_count = 0;
    for (uint32_t i = 0; i < testvectors->size(); i++) {
        const rsa_vector_entry_t& entry = testvectors->at(i);

        /*
         * 1st run with HBM
         */
        if (!testvectors_run(entry, DP_MEM_TYPE_HBM)) {
            failure_count++;
        }

#ifdef __x86_64__

        /*
         * 2nd run with host mem
         */
        if (!testvectors_run(entry, DP_MEM_TYPE_HOST_MEM)) {
            failure_count++;
        }
#endif
    }

    return failure_count == 0;
}

/*
 * Driver for ECDSA testvectors
 */
static bool
ecdsa_testvectors_run(void *test_param)
{
    vector<ecdsa_vector_entry_t>  *testvectors = 
                      static_cast<vector<ecdsa_vector_entry_t> *>(test_param);
    uint32_t        failure_count;

    /*
     * Run the vectors with the specified mem_type
     */
    auto testvectors_run = [](const ecdsa_vector_entry_t& entry,
                              dp_mem_type_t mem_type) -> bool
    {
        crypto_ecdsa::ecdsa_testvec_params_t          testvec_params;
        crypto_ecdsa::ecdsa_testvec_pre_push_params_t pre_params;
        crypto_ecdsa::ecdsa_testvec_push_params_t     push_params;
        crypto_ecdsa::ecdsa_testvec_t                 *ecdsa_testvec;
        offload_base_params_t                     base_params;
        bool                                      success;

        testvec_params.key_create_type(entry.key_create_type).
                       q_mem_type(mem_type).
                       d_mem_type(mem_type).
                       k_mem_type(mem_type).
                       msg_mem_type(mem_type).
                       sig_mem_type(mem_type).
                       dma_desc_mem_type(mem_type).
                       status_mem_type(mem_type);
        OFFL_LOG_INFO("Running ECDSA vector {} with mem_type {}",
                      entry.testvec_fname, mem_type);
        ecdsa_testvec = new crypto_ecdsa::ecdsa_testvec_t(
                          testvec_params.base_params(base_params));
        success = ecdsa_testvec->pre_push(pre_params.scripts_dir(FLAGS_script_dir).
                                                     testvec_fname(entry.testvec_fname));;
        if (success) {
            success = ecdsa_testvec->push(push_params.ecdsa_ring(crypto_asym::asym_ring));
        }
        if (success) {
            success = ecdsa_testvec->post_push();
        }
        if (success) {
            success = ecdsa_testvec->full_verify();
        }
        ecdsa_testvec->rsp_file_output(mem_type == DP_MEM_TYPE_HBM ?
                                     "hbm" : "host");
        delete ecdsa_testvec;
        return success;
    };

    failure_count = 0;
    for (uint32_t i = 0; i < testvectors->size(); i++) {
        const ecdsa_vector_entry_t& entry = testvectors->at(i);

        /*
         * 1st run with HBM
         */
        if (!testvectors_run(entry, DP_MEM_TYPE_HBM)) {
            failure_count++;
        }

#ifdef __x86_64__

        /*
         * 2nd run with host mem
         */
        if (!testvectors_run(entry, DP_MEM_TYPE_HOST_MEM)) {
            failure_count++;
        }
#endif
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
              << "\nTest group:     "        << FLAGS_test_group
              << "\nScripts dir:    "        << FLAGS_script_dir 
              << "\nOpenssl engine path: "   << FLAGS_engine_path
              << std::endl;

    if (FLAGS_test_group == "") {
        run_rsa_testvectors = true;
        run_ecdsa_testvectors = true;
    } else if (FLAGS_test_group == "rsa") {
        run_rsa_testvectors = true;
    } else if (FLAGS_test_group == "ecdsa") {
        run_ecdsa_testvectors = true;
    }

    if (common_setup() < 0)  {
        OFFL_FUNC_ERR("Failed common setup");
        return 1;
    }

    /*
     * Indicate to model that config is done
     */
    CONFIG_DONE();

    if (run_rsa_testvectors) {
        test_suite.push_back(test_entry_t("FIPS RSA", &rsa_testvectors_run,
                                          (void *)&rsa_testvectors));
    }
    if (run_ecdsa_testvectors) {
        test_suite.push_back(test_entry_t("FIPS ECDSA", &ecdsa_testvectors_run,
                                          (void *)&ecdsa_testvectors));
    }

    OFFL_LOG_INFO("Formed test suite with {} cases", test_suite.size());
    OFFL_LOG_INFO("Running test cases");

    overall_success = true;
    for (tcid = 0; tcid < test_suite.size(); tcid++) {
        test_entry_t& test_entry = test_suite.at(tcid);

        gettimeofday(&start, NULL);
        OFFL_LOG_INFO(" Starting test #: {} name: {}", tcid, test_entry.test_name);

        TESTCASE_BEGIN(tcid, 0);
        test_entry.test_success = test_entry.test_fn(test_entry.test_param);
        TESTCASE_END(tcid, 0);

        gettimeofday(&end, NULL);
        OFFL_LOG_INFO(" Finished test #: {} name: {} status {} time {}",
                      tcid, test_entry.test_name, test_entry.test_success,
                      end.tv_sec - start.tv_sec);
        overall_success &= test_entry.test_success;
    }

    OFFL_LOG_INFO("");
    OFFL_LOG_INFO("Consolidated Test Report");
    OFFL_LOG_INFO("--------------------------------------------------------------");
    OFFL_LOG_INFO("Number\t\tName\t\t\t\tResult\n");
    OFFL_LOG_INFO("--------------------------------------------------------------");

    for (size_t i = 0; i < test_suite.size(); i++) {
        test_entry_t& test_entry = test_suite.at(i);
        OFFL_LOG_INFO("{}\t\t{}\t\t\t{}", i+1, test_entry.test_name,
                      test_entry.test_success ? "Success" : "Failure");
    }

    OFFL_LOG_INFO("");
    OFFL_LOG_INFO("Overall Report: {}",
                  overall_success ? "SUCCESS" : "FAILURE");

    EXIT_SIMULATION();

    if (offl::logger::logger()) {
        offl::logger::logger()->flush();
    }
    fflush(stdout);
    exit(overall_success ? 0 : 1);
}
