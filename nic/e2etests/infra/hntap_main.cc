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


int main(int argv, char *argc[])
{
#define MAX_DEV_HANDLES 2
  dev_handle_t* host_tap_hdl;
  dev_handle_t* net_tap_hdl;
  setlinebuf(stdout);
  setlinebuf(stderr);
  dev_handle_t *dev_handles[MAX_DEV_HANDLES];
  ptree             pt;
  const char*cfg_file = nullptr;

  int opt = 0;
  while ((opt = getopt(argv, argc, "p:n:xf:")) != -1) {
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

  for (ptree::value_type &ep_pairs: pt.get_child("")) {
      //std::cout <<ep_pairs.second;
      std::string src_ep_name = ep_pairs.second.get<std::string>("src.name");
      bool src_ep_local =  ep_pairs.second.get<bool>("src.local");
      int src_lif_id = ep_pairs.second.get<int>("src.lif_id");
      int src_port = ep_pairs.second.get<int>("src.port");
      std::string dst_ep_name = ep_pairs.second.get<std::string>("dst.name");
      bool dst_ep_local =  ep_pairs.second.get<bool>("dst.local");
      int dst_lif_id = ep_pairs.second.get<int>("dst.lif_id");
      int dst_port = ep_pairs.second.get<int>("dst.port");
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

      dev_handles[0] = host_tap_hdl;

      type = dst_ep_local ? TAP_ENDPOINT_HOST : TAP_ENDPOINT_NET;
      net_tap_hdl = hntap_create_tap_device(type,
              dst_ep_name.c_str());

      /* Create tap interface for Network-tap */
      if (net_tap_hdl == nullptr) {
          TLOG("Error creating tap interface %s!\n", dst_ep_name.c_str());
          abort();
      }
      net_tap_hdl->tap_ports[0] = hntap_port;
      net_tap_hdl->lif_id = dst_lif_id;
      net_tap_hdl->port = dst_port ? dst_port - 1 : dst_port;
      dev_handles[1] = net_tap_hdl;

      TLOG("  Setup done, listening on tap devices..\n");
      add_dev_handle_tap_pair(host_tap_hdl, net_tap_hdl);
      hntap_do_select_loop(dev_handles, MAX_DEV_HANDLES);
  }

  return(0);
}
