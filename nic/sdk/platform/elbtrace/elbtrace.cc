//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the mputrace implementation for setting trace
/// registers in ASIC
///
//===----------------------------------------------------------------------===//

#include "third-party/asic/elba/model/utils/elb_csr_py_if.h"
#include "platform/utils/mpart_rsvd.hpp"
#include "platform/elba/csrint/csr_init.hpp"
#include "platform/csr/asicrw_if.hpp"
#include "platform/elbtrace/elbtrace.hpp"
#include "lib/pal/pal.hpp"

namespace sdk {
namespace platform {

#define roundup(x, y) ((((x) + ((y)-1)) / (y)) * (y)) /* to any y */
  //Neel. todo: Below todo comment is from capri. check what needs to be done for Elba
// TODO. Need to read it from memory or file. Temporary fix for SDK compilation
#define MREGION_MPU_TRACE_ADDR                                                 \
    (MREGION_BASE_ADDR + MREGION_MPU_TRACE_START_OFFSET)
  //#define TRACE_END (MREGION_MPU_TRACE_ADDR + MREGION_MPU_TRACE_SIZE) //todo: req SW to make it deeper for SDP and DMA

#define REGION_SPLIT  (MREGION_MPU_TRACE_SIZE/4)
#define MPU_TRACE_SIZE (REGION_SPLIT)
#define SDP_PHV_TRACE_SIZE (REGION_SPLIT)
#define SDP_CTL_TRACE_SIZE (REGION_SPLIT)
#define DMA_TRACE_SIZE (REGION_SPLIT)

#define TRACE_BASE roundup(MREGION_MPU_TRACE_ADDR, 4096)
#define TRACE_END (MREGION_MPU_TRACE_ADDR + MPU_TRACE_SIZE)

  //Neel. todo: what shd be the base address for sdp and dma?
#define SDPTRACE_PHV_BASE roundup(TRACE_END+1, 4096)   //todo
#define SDPTRACE_PHV_END (SDPTRACE_PHV_BASE + SDP_PHV_TRACE_SIZE - 4096) //todo
#define SDPTRACE_CTL_BASE roundup(SDPTRACE_PHV_END+1, 4096)   //todo
#define SDPTRACE_CTL_END (SDPTRACE_CTL_BASE + SDP_CTL_TRACE_SIZE - 4096) //todo
#define DMATRACE_BASE    roundup(SDPTRACE_CTL_END+1, 4096) //todo
#define DMATRACE_END     (DMATRACE_BASE + DMA_TRACE_SIZE - 4096) //todo

// Placeholder for the global state in mputrace app
mputrace_global_state_t g_state = {};

void
usage (char *argv[])
{
    std::cerr << "Usage: " << std::endl
              << "       " << argv[0] << " conf <cfg_file>" << std::endl
              << "       " << argv[0] << " dump <out_file>" << std::endl
              << "       " << argv[0] << " show" << std::endl
              << "       " << argv[0] << " reset" << std::endl;
}

static inline int
mputrace_error (char *argv[])
{
    std::cerr << "Invalid operation " << argv[1] << std::endl;
    usage(argv);
    exit(EXIT_FAILURE);
}

static inline void
elbtrace_init(void)
{
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
           sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);
#endif

    sdk::platform::elba::csr_init();
}

static int
mputrace_handle_options (int argc, char *argv[])
{
    int ret = 0;
    std::map<std::string, int> oper = {
        {"conf_mpu", MPUTRACE_CONFIG},
        {"dump_mpu", MPUTRACE_DUMP},
        {"reset_mpu", MPUTRACE_RESET},
        {"show_mpu", MPUTRACE_SHOW},
        {"conf_sdp",  SDPTRACE_CONFIG},
        {"dump_sdp",  SDPTRACE_DUMP},
        {"reset_sdp", SDPTRACE_RESET},
        {"show_sdp",  SDPTRACE_SHOW},
        {"conf_dma",  DMATRACE_CONFIG},
        {"dump_dma",  DMATRACE_DUMP},
        {"reset_dma", DMATRACE_RESET},
        {"show_dma",  DMATRACE_SHOW},
    };

    g_state.trace_base = TRACE_BASE;
    g_state.trace_end = TRACE_END;
    g_state.sdptrace_phv_base = SDPTRACE_PHV_BASE;
    g_state.sdptrace_phv_end  = SDPTRACE_PHV_END;
    g_state.sdptrace_ctl_base = SDPTRACE_CTL_BASE;
    g_state.sdptrace_ctl_end  = SDPTRACE_CTL_END;
    g_state.dmatrace_base = DMATRACE_BASE;
    g_state.dmatrace_end  = DMATRACE_END;
    switch (oper[std::string((const char *)argv[1])]) {
    case MPUTRACE_CONFIG: 
    case SDPTRACE_CONFIG:
    case DMATRACE_CONFIG:
        if (argc < MPUTRACE_MAX_ARG) {
            std::cerr << "Specify config filename" << std::endl;
            usage(argv);
            exit(EXIT_FAILURE);
            break;
        }
        elbtrace_init();
	if (oper[std::string((const char *)argv[1])] == MPUTRACE_CONFIG) {
	  elbtrace_cfg(argv[2], "mpu");
	}
	else if (oper[std::string((const char *)argv[1])] == SDPTRACE_CONFIG) {
	  cout << "configuring SDP now" << endl;
	  elbtrace_cfg(argv[2], "sdp");
	}
	else if (oper[std::string((const char *)argv[1])] == DMATRACE_CONFIG) {
	  elbtrace_cfg(argv[2], "dma");
	}

        break;
    case MPUTRACE_DUMP:
    case SDPTRACE_DUMP:
    case DMATRACE_DUMP:
        if (argc < MPUTRACE_MAX_ARG) {
            std::cerr << "Specify dump filename" << std::endl;
            usage(argv);
            exit(EXIT_FAILURE);
        }
        elbtrace_init();
	if (oper[std::string((const char *)argv[1])] == MPUTRACE_DUMP) {
	  elbtrace_dump(argv[2], "mpu");
	}
	else if (oper[std::string((const char *)argv[1])] == SDPTRACE_DUMP) {
	  elbtrace_dump(argv[2], "sdp");
	}
	else if (oper[std::string((const char *)argv[1])] == DMATRACE_DUMP) {
	  elbtrace_dump(argv[2], "dma");
	}
        break;
    case MPUTRACE_RESET:
        elbtrace_init();
        mputrace_reset();
        break;
    case SDPTRACE_RESET:
        elbtrace_init();
        sdptrace_reset();
        break;
    case DMATRACE_RESET:
        elbtrace_init();
        dmatrace_reset();
        break;
    case MPUTRACE_SHOW:

      cout << "MREGION_MPU_TRACE_ADDR  " << hex << MREGION_MPU_TRACE_ADDR << endl;
      cout << "MREGION_BASE_ADDR 	        "   << hex << MREGION_BASE_ADDR 	     << endl;
      cout << "MREGION_MPU_TRACE_START_OFFSET  " << hex << MREGION_MPU_TRACE_START_OFFSET << endl;
      cout << "MREGION_MPU_TRACE_SIZE  " << hex << MREGION_MPU_TRACE_SIZE << endl;

      cout << "REGION_SPLIT     " << hex << REGION_SPLIT   << endl;
      cout << "MPU_TRACE_SIZE   " << hex << MPU_TRACE_SIZE  << endl;
      cout << "SDP_PHV_TRACE_SIZE   " << hex << SDP_PHV_TRACE_SIZE  << endl;
      cout << "SDP_CTL_TRACE_SIZE   " << hex << SDP_CTL_TRACE_SIZE  << endl;
      cout << "DMA_TRACE_SIZE   " << hex << DMA_TRACE_SIZE  << "\n" <<  endl;

      cout << " MPUTRACE_BASE	   " << hex << TRACE_BASE		     << endl;
      cout << " MPUTRACE_END       " << hex << TRACE_END                      << endl;
      cout << " SDPTRACE_PHV_BASE  " << hex << SDPTRACE_PHV_BASE  << endl;
      cout << " SDPTRACE_PHV_END   " << hex << SDPTRACE_PHV_END   << endl;
      cout << " SDPTRACE_CTL_BASE  " << hex << SDPTRACE_CTL_BASE  << endl;
      cout << " SDPTRACE_CTL_END   " << hex << SDPTRACE_CTL_END   << endl;
      cout << " DMATRACE_BASE      " << hex << DMATRACE_BASE      << endl;
      cout << " DMATRACE_END       " << hex << DMATRACE_END       << endl;



        elbtrace_init();
        mputrace_show();
        break;
    case SDPTRACE_SHOW:
        elbtrace_init();
        sdptrace_show();
        break;
    case DMATRACE_SHOW:
        elbtrace_init();
        dmatrace_show();
        break;
    default:
        ret = mputrace_error(argv);
    }
    return ret;
}

}    //    end namespace platform
}    //    end namespace sdk

int
main (int argc, char *argv[])
{
    int ret = 0;

    //    sdk::platform::pal_init("foo");
    if (argc < MPUTRACE_MAX_ARG - 1) {
        sdk::platform::usage(argv);
        exit(EXIT_FAILURE);
    }

    ret = sdk::platform::mputrace_handle_options(argc, argv);

    return ret;
}
