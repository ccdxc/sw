//  Hello World client
#include <zmq.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "nic/model_sim/include/buf_hdr.h"
#include "nic/e2etests/driver/lib_driver.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/e2etests/proxy/flows.hpp"

int main (int argc, char *argv[])
{
  if (argc < 2) {
    std::cout << "Usage:" << argv[0] << " <lif>" << std::endl;
    exit(1);
  }

  int ret;
  //uint32_t i;
  uint16_t lif_id = (uint16_t)(atoi(argv[1]) & 0xffff);
  uint32_t port = 0;

  //
  uint8_t *pkt = uplink_to_enic;
  std::vector<uint8_t> ipkt;

  ipkt.resize(sizeof(*pkt));
  memcpy(ipkt.data(), pkt, sizeof(*pkt));

  lib_model_connect();

  // --------------------------------------------------------------------------------------------------------//

  // Create Queues
  alloc_queue(lif_id, TX, 0, 1024);
  alloc_queue(lif_id, RX, 0, 1024);

  // --------------------------------------------------------------------------------------------------------//

  // Post buffer
  uint8_t *buf = alloc_buffer(9126);
  assert(buf != NULL);
  memset(buf, 0, 9126);
  post_buffer(lif_id, RX, 0, buf, 9126);

  // Send packet to Model
  std::cout << "Sending packet to model! size: " << ipkt.size() << " on port: " << port << std::endl;
  step_network_pkt(ipkt, port);

  // Receive Packet
  uint16_t size;
  consume_buffer(lif_id, RX, 0, buf, &size);
  if (!size) {
    std::cout << "Did not get packet back from model!" << std::endl;
  } else {
    std::cout << "Got packet back from model!" << std::endl;
    if (ipkt.size() == size) {
      if (memcmp(ipkt.data(), buf, size)) {
        printf("Received packet does not match Sent packet!\n");
        ret = -1;
      } else {
        printf("Received packet matches Sent packet.\n");
        ret = 0;
      }
    } else {
      printf("Received packet is smaller than Sent packet.\n");
      ret = -1;
    }
  }

  lib_model_conn_close();

  return ret;
}
