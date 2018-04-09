#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <assert.h>
#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"

using boost::property_tree::ptree;

extern uint32_t nw_retries;
extern uint16_t hntap_port;
extern bool hntap_drop_rexmit;
extern bool hntap_go_thru_model;


int main(int argv, char *argc[])
{
  dev_handle_t* host_tap_hdl = nullptr;
  dev_handle_t *dev_handle_pairs[10][2];
  uint32_t dev_handle_pair_cnt = 0;
  setlinebuf(stdout);
  setlinebuf(stderr);
  dev_handle_t **dev_handles;
  ptree             pt;
  const char*cfg_file = nullptr;

  int opt = 0;
  while ((opt = getopt(argv, argc, "p:n:xf:s")) != -1) {
    switch (opt) {
    case 'p':
        hntap_port = atoi(optarg);
    TLOG( "Port numer=%d\n", hntap_port);
        break;
    case 'n':
        nw_retries = atoi(optarg);
    TLOG( "NW Retries=%d\n", nw_retries);
    break;
    case 'x':
    hntap_drop_rexmit = true;
        break;
    case 's':
    hntap_go_thru_model = false;
        break;
    case 'f':
        cfg_file = optarg;
        break;
    case '?':
    default:
        TLOG( "usage: ./hntap [-n <NW Retries>] [-x] \n");
    exit(-1);
        break;
    }

  }
  TLOG("Starting Host/network Tapper..\n");

  if (cfg_file == nullptr) {
      TLOG("Configuration file not specified..\n");
      exit(-1);

  }
  std::ifstream json_cfg(cfg_file);
  read_json(json_cfg, pt);

  uint32_t dev_handle_cnt = pt.size();
  uint32_t i = 0;

  TLOG("Number of devices to create : %d\n", dev_handle_cnt);
  dev_handles = (dev_handle_t**)malloc(sizeof(dev_handle_t) * dev_handle_cnt);
  uint32_t pair_id = 0;
  for (ptree::value_type &ep_pairs: pt) {
      //std::cout <<ep_pairs.second;
      std::string src_ep_name = ep_pairs.second.get<std::string>("name");
      bool src_ep_local =  ep_pairs.second.get<bool>("local");
      int src_lif_id = ep_pairs.second.get<int>("lif_id");
      int src_port = ep_pairs.second.get<int>("port");
      //TODO :ADD LOG Here

      /* Create tap interface for -tap */
      tap_endpoint_t type = src_ep_local ? TAP_ENDPOINT_HOST : TAP_ENDPOINT_NET;
      host_tap_hdl = hntap_create_tap_device(type, src_ep_name.c_str());
      if (host_tap_hdl == nullptr ) {
        TLOG("Error creating tap interface %s!\n", src_ep_name.c_str());
        abort();
      }
      host_tap_hdl->tap_ports[0] = hntap_port;
      host_tap_hdl->lif_id = src_lif_id;
      host_tap_hdl->port = src_port ? src_port - 1 : 0;

      dev_handles[i++] = host_tap_hdl;
      TLOG("Added configuration for device %s , type : %s\n",
              src_ep_name.c_str(), type == TAP_ENDPOINT_HOST ? "Host" : "Network");

      if (!hntap_go_thru_model) {
          dev_handle_pairs[dev_handle_pair_cnt][(pair_id) %2] = host_tap_hdl;
          if (pair_id++) {
              dev_handle_pair_cnt++;
              pair_id = 0;
          }
      }
  }

  TLOG("  Setup done, listening on tap devices..\n");
  if (!hntap_go_thru_model) {
      for (uint32_t i = 0; i < dev_handle_pair_cnt; i++) {
          add_dev_handle_tap_pair(dev_handle_pairs[i][0],
                  dev_handle_pairs[i][1]);
      }
  }
  hntap_work_loop(dev_handles, dev_handle_cnt, true);
  return(0);
}
