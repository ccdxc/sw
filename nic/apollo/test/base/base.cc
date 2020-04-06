//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/test/base/vpp_mock.hpp"
#include "nic/apollo/test/base/base.hpp"

//----------------------------------------------------------------------------
// tracing routines
//----------------------------------------------------------------------------

// callback invoked for debug traces. This is sample implementation, hence
// doesn't check whether user enabled traces at what level, it always prints
// the traces but with a simple header prepended that tells what level the
// trace is spwed at ... in reality, you call your favorite logger here
sdk_trace_level_e g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
static int
trace_cb (uint32_t mod_id, sdk_trace_level_e trace_level,
          const char *format, ...)
{
    va_list args;
    const char *pfx;
    struct timespec tp_;
    char logbuf[1024];

    if (trace_level == sdk::lib::SDK_TRACE_LEVEL_NONE) {
        return 0;
    }

    if (trace_level > g_trace_level) {
        return 0;
    }

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        pfx = "[E]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        pfx = "[W]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        pfx = "[I]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        pfx = "[D]";
        break;

    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
    default:
        // pfx = "[V]";
        // fprintf(stdout, "[V] %s\n", logbuf);
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &tp_);
    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    va_end(args);
    fprintf(stdout, "%s [%lu.%9lu] %s\n", pfx, tp_.tv_sec, tp_.tv_nsec, logbuf);
    fflush(stdout);
    return 0;
}

//----------------------------------------------------------------------------
// test case setup and teardown routines
//----------------------------------------------------------------------------

// called at the beginning of all test cases in this class,
// initializes PDS HAL
void
pds_test_base::SetUpTestCase(test_case_params_t& params)
{
    pds_init_params_t init_params;

    if (vpp_ipc_mock_mode()) {
        vpp_thread_create();
    }

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = trace_cb;
    init_params.pipeline  = ::pipeline_get();
    init_params.cfg_file  = std::string(params.cfg_file);
    init_params.memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
    pds_init(&init_params);
}

// called at the end of all test cases in this class,
// cleanup PDS HAL and quit
void
pds_test_base::TearDownTestCase(void)
{
    pds_teardown();
}

//----------------------------------------------------------------------------
// packet related routines
//----------------------------------------------------------------------------

void
dump_packet (std::vector<uint8_t> data)
{
    int count = 0;
    std::cout << "Packet length : " << data.size() << "\n";
    for (std::vector<uint8_t>::const_iterator i = data.begin(); i != data.end();
         ++i) {
        if (count != 0 && count % 16 == 0)
            std::cout << "\n";
        // std::cout << *i << ' ';
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)*i
                  << " ";
        count++;
    }
    std::cout << "\n";
}

void
send_packet (const uint8_t *tx_pkt, uint32_t tx_pkt_len, uint32_t tx_port,
             const uint8_t *exp_rx_pkt, uint32_t exp_rx_pkt_len,
             uint32_t exp_rx_port)
{
#ifdef SIM
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;
    uint32_t port = 0;
    uint32_t cos = 0;
    uint32_t i = 0;
    uint32_t tcscale = 1;
    int tcid = 0;
    int tcid_filter = 0;

    if (getenv("TCSCALE")) {
        tcscale = atoi(getenv("TCSCALE"));
    }
    if (getenv("TCID")) {
        tcid_filter = atoi(getenv("TCID"));
    }

    tcid++;
    if (tcid_filter != 0 && tcid != tcid_filter) {
        std::cout << "TCID not matching, skipping the send packet\n"
                  << std::endl;
        return;
    }

    ipkt.resize(tx_pkt_len);
    memcpy(ipkt.data(), tx_pkt, tx_pkt_len);
    epkt.resize(exp_rx_pkt_len);
    memcpy(epkt.data(), exp_rx_pkt, exp_rx_pkt_len);
    std::cout << "Testing Host to Switch" << std::endl;
    for (i = 0; i < tcscale; i++) {
        testcase_begin(tcid, i + 1);
        step_network_pkt(ipkt, tx_port);
        if (!getenv("SKIP_VERIFY")) {
            get_next_pkt(opkt, port, cos);
            dump_packet(opkt);
            EXPECT_TRUE(opkt == epkt);
            EXPECT_TRUE(port == exp_rx_port);
        }
        testcase_end(tcid, i + 1);
    }
#endif
}
