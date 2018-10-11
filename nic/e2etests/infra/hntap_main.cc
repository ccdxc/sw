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
#include <sys/ioctl.h>
#include <net/if.h>
#include "netinet/ip.h"
#include "netinet/udp.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"
#include "nic/e2etests/hntap/hntap_switch.hpp"

#include <grpc++/grpc++.h>
#include "gen/proto/interface.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using intf::LifSpec;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::LifRequestMsg;
using intf::LifResponseMsg;
using intf::LifQStateMapEntry;
using intf::QStateSetReq;
using types::Empty;

using boost::property_tree::ptree;

#define BUF_SIZE 9126
#define HSWITCH_SEND_PORT 12345
#define HSWITCH_RECV_PORT 54321
#define IP_IDENTIFICATION HSWITCH_RECV_PORT

extern uint32_t nw_retries;
extern uint16_t hntap_port;
extern bool hntap_drop_rexmit;
extern bool hntap_go_thru_model;

const char*cfg_file = nullptr;
HntapPassThroughSwitch *passThroughswitch = NULL;


const unsigned char lif_mac_addr_start[]  = { 0x00, 0x02, 0x00, 0x00, 0x01, 0x01};
const unsigned char uplink_mac_addr_start[] = {0x00, 0x03, 0x00, 0x00, 0x01, 0x01};

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) :
        intf_stub_(Interface::NewStub(channel)) {
        channel_ = channel;
    }

    void wait_until_ready(void) {
        while (channel_->GetState(true) != GRPC_CHANNEL_READY) {
            std::cout << "Waiting for HAL to be ready ..." << std::endl;
            sleep(5);
        }
    }

    void lif_get_all (void) {
        LifGetRequestMsg    req_msg;
        LifGetResponseMsg   rsp_msg;
        ClientContext       context;
        Status              status;
        uint32_t            num_rsp;

        req_msg.add_request();
        status = intf_stub_->LifGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            num_rsp = rsp_msg.response_size();
            TLOG("Num of Lifs : %d\n", num_rsp);
            for (uint32_t i = 0; i < num_rsp; i ++) {
                TLOG("Lif ID : %lu , Hw Lif ID : %lu\n",
                        rsp_msg.response(i).spec().key_or_handle().lif_id(),
                        rsp_msg.response(i).status().hw_lif_id());
                lif_map[rsp_msg.response(i).spec().key_or_handle().lif_id()] =
                        rsp_msg.response(i).status().hw_lif_id();
            }
        } else {
            std::cout << "Lif Get Failed" << std::endl;
        }
    }

    uint64_t get_hw_lif_id(uint64_t lif_id) {
        std::map<uint64_t,uint64_t>::iterator it;

        it = lif_map.find(lif_id);

        return (it != lif_map.end()) ? it->second : 0;

    }


private:
    std::unique_ptr<Interface::Stub> intf_stub_;
    std::shared_ptr<Channel> channel_;
    std::map<uint64_t,uint64_t> lif_map;
};


static HntapSwitch *
hntapSwitchInit(const char *intfName, std::list<std::string> peerIps) {
    HntapSwitch *hswitch = new HntapSwitch(intfName);

    for (std::string ip: peerIps) {
        HntapTunnelPeer *peer = new HntapTunnelPeer();
        inet_pton(AF_INET, ip.c_str(), &(peer->ip.addr.v4_addr));
        peer->ip.addr.v4_addr = ntohl(peer->ip.addr.v4_addr);
        hswitch->AddPeer(peer);
    }

    return hswitch;
}

static HntapPassThroughSwitch *
hntapPassThroughSwitchInit() {
    return new HntapPassThroughSwitch();
}

static void load_passthrough_config(const char*cfg_file);

static void
signalHandler( int signum ) {
   TLOG("*************HNTAP Reloading Config begin *************\n");
   load_passthrough_config(cfg_file);
   TLOG("*************HNTAP Reloading Config end   *************\n");
}

static int
str2mac(const char* mac, mac_address_t &out){
    if( 6 == sscanf( mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &out[0], &out[1], &out[2],&out[3], &out[4], &out[5] ) ){
        return 0;
    }else{
        return 1;
    }
}

static void
load_passthrough_config(const char*cfg_file) {

    ptree pt;
    int   ret;
    mac_address_t allowed_mac;
    std::list<mac_address_t> allowedMacs;

    if (passThroughswitch == nullptr) {
        TLOG("Hntap not in pass through mode, no config reload.");
        return;
    }
    std::ifstream json_cfg(cfg_file);
    read_json(json_cfg, pt);
    boost::optional< ptree& > hswitch_cfg =  pt.get_child_optional("switch");
    if (hswitch_cfg) {
      ptree& hswitchCfg  = hswitch_cfg.get();
      boost::optional< ptree& > passthrough_cfg =  hswitchCfg.get_child_optional("passthrough-mode");
      if (passthrough_cfg) {
          // This should be called after uplink map.
          ptree& passthroughCfg  = passthrough_cfg.get();
          boost::optional< ptree& > allowed_macs =  passthroughCfg.get_child_optional("allowed-macs");
          if (allowed_macs) {
              for (auto& allowedMac : passthroughCfg.get_child("allowed-macs")) {
                  std::string strMac = allowedMac.second.get_value<std::string>();
                  ret = str2mac(strMac.c_str(), allowed_mac);
                  if (ret != 0) {
                      TLOG("Invalid mac adddress : %s", (strMac.c_str()));
                      assert(0);
                  }
                  TLOG("Read Allowed mac adddress : %s\n", (strMac.c_str()));
                  allowedMacs.push_back(allowed_mac);
              }
              passThroughswitch->AddAllowedMacs(allowedMacs);
          }
      }
    }
}

int main(int argv, char *argc[])
{
  dev_handle_t* host_tap_hdl = nullptr;
  dev_handle_t *dev_handle_pairs[10][2];
  uint32_t dev_handle_pair_cnt = 0;
  setlinebuf(stdout);
  setlinebuf(stderr);
  dev_handle_t **dev_handles;
  ptree             pt;
  std::string    svc_endpoint;
  char mac_addr_str[20];
  const unsigned char *mac_addr;
  HntapSwitchBase *hswitch = NULL;
  std::string hswitchOutIntf;
  std::list<std::string>peers;
  std::list<mac_address_t> allowedMacs;

  grpc_init();
  if (getenv("HAL_GRPC_PORT")) {
      svc_endpoint = "localhost:" + std::string(getenv("HAL_GRPC_PORT"));
  } else {
      svc_endpoint = "localhost:50054";
  }

  // create gRPC channel to connect to HAL and wait until it is ready
  hal_client hclient(grpc::CreateChannel(svc_endpoint,
                                         grpc::InsecureChannelCredentials()));
  hclient.lif_get_all();

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

  ptree& devices =  pt.get_child("devices");
  boost::optional< ptree& > hswitch_cfg =  pt.get_child_optional("switch");
  if (hswitch_cfg) {
      TLOG("Hntap Switch config is present..\n");
      ptree& hswitchCfg  = hswitch_cfg.get();
      boost::optional< ptree& > tun_cfg =  hswitchCfg.get_child_optional("tunnel-mode");
      boost::optional< ptree& > passthrough_cfg =  hswitchCfg.get_child_optional("passthrough-mode");

      if (tun_cfg) {
          TLOG("Hntap Switch in tunnel mode..\n");
          ptree& tunCfg  = tun_cfg.get();
          hswitchOutIntf = tunCfg.get<std::string>("out-interface");
          for (auto& peer :  tunCfg.get_child("peers")) {
              peers.push_back(peer.second.get_value<std::string>());
          }
          hswitch = hntapSwitchInit(hswitchOutIntf.c_str(), peers);
      } else if (passthrough_cfg) {
          TLOG("Hntap Switch in passthrough mode..\n");
          ptree& passthroughCfg  = passthrough_cfg.get();
          boost::optional< ptree& > uplink_map =  passthroughCfg.get_child_optional("uplink-map");
          if (uplink_map) {
              passThroughswitch = hntapPassThroughSwitchInit();
              for (auto& uplinkMap :  passthroughCfg.get_child("uplink-map")) {
                  passThroughswitch->AddUplinkMap(uplinkMap.first, uplinkMap.second.data());
              }
              // This should be called after uplink map.
              load_passthrough_config(cfg_file);
              hswitch = passThroughswitch;
          }
      } else {
          TLOG("Hntap Switch Config is empty..\n");
      }
  }

  uint32_t dev_handle_cnt = devices.size();
  uint32_t i = 0;
  uint32_t lif_mac_offset = 0;
  uint32_t uplink_mac_offset = 0;
  uint32_t offset;

  TLOG("Number of devices to create : %d\n", dev_handle_cnt);
  dev_handles = (dev_handle_t**)malloc(sizeof(dev_handle_t) * dev_handle_cnt);
  uint32_t pair_id = 0;
  for (ptree::value_type &ep_pairs: devices) {
      //std::cout <<ep_pairs.second;
      std::string src_ep_name = ep_pairs.second.get<std::string>("name");
      bool src_ep_local =  ep_pairs.second.get<bool>("local");
      int src_lif_id = ep_pairs.second.get<int>("lif_id");
      int src_port = ep_pairs.second.get<int>("port");
      uint64_t hw_lif_id = 0;
      //TODO :ADD LOG Here

      /* Create tap interface for -tap */
      tap_endpoint_t type = src_ep_local ? TAP_ENDPOINT_HOST : TAP_ENDPOINT_NET;
    
    if(src_ep_local) {
        mac_addr = lif_mac_addr_start;
        offset = lif_mac_offset++;
    } else {
       mac_addr = uplink_mac_addr_start;
       offset = uplink_mac_offset++;
    }
    sprintf(mac_addr_str,  "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2],
            mac_addr[3], mac_addr[4], mac_addr[5] + offset);
      host_tap_hdl = hntap_create_tap_device(type, src_ep_name.c_str(), mac_addr_str);
      if (host_tap_hdl == nullptr ) {
        TLOG("Error creating tap interface %s!\n", src_ep_name.c_str());
        abort();
      }

      host_tap_hdl->port = src_port ? src_port - 1 : 0;
      if (src_ep_local) {
          hw_lif_id = hclient.get_hw_lif_id(src_lif_id);
          if (!hw_lif_id) {
              TLOG("LIF ID %d not found in HAL\n", src_lif_id);
	      if (hntap_go_thru_model) {
                  abort();
              }
          }
          type = TAP_ENDPOINT_HOST;
      } else {
          type = TAP_ENDPOINT_NET;
          if (hswitch != nullptr) {
              hswitch->AddDevHandle(host_tap_hdl);
          }
      }

      host_tap_hdl->tap_ports[0] = hntap_port;
      host_tap_hdl->lif_id = hw_lif_id;

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

  if (hswitch) {
      TLOG("Starting Hntap Switch Receiver..\n");
      hswitch->StartReceiver();
  }

  TLOG("  Setup done, listening on tap devices..\n");
  if (!hntap_go_thru_model) {
      for (uint32_t i = 0; i < dev_handle_pair_cnt; i++) {
          add_dev_handle_tap_pair(dev_handle_pairs[i][0],
                  dev_handle_pairs[i][1]);
      }
  }

  signal(SIGUSR1, signalHandler);
  hntap_work_loop(dev_handles, dev_handle_cnt, true, hswitch);
  return(0);
}



