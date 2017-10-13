//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include "nic/model_sim/include/buf_hdr.h"
#include "nic/proxy-e2etest/lib_driver.hpp"
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/proxy-e2etest/flows.hpp"


int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage:" << argv[0] << " <lif>" << std::endl;
    exit(1);
  }

  int ret;
  //uint32_t i;
  uint16_t lif_id = (uint16_t) (atoi(argv[1]) & 0xffff);
  uint32_t port = 0, cos = 0;

  //
  uint8_t *pkt = enic_to_uplink;
  std::vector<uint8_t> ipkt, opkt;

  ipkt.resize(sizeof(enic_to_uplink));
  opkt.resize(sizeof(enic_to_uplink));
  memcpy(ipkt.data(), pkt, sizeof(enic_to_uplink));

  lib_model_connect();

  // --------------------------------------------------------------------------------------------------------//

  // Create Queues
  alloc_queue(lif_id, TX, 0, 1024);
  alloc_queue(lif_id, RX, 0, 1024);
  print_queue(lif_id, TX, 0);
  print_queue(lif_id, RX, 0);

  // --------------------------------------------------------------------------------------------------------//

  // Post tx buffer
  uint8_t *buf = alloc_buffer(ipkt.size());
  assert(buf != NULL);
  memcpy(buf, ipkt.data(), ipkt.size());
  post_buffer(lif_id, TX, 0, buf, ipkt.size());

  // Transmit Packet
  std::cout << "Writing packet to model! size: " << ipkt.size() << " on port: " << port << std::endl;

  printf("\nDOORBELL\n");
  std::pair<uint32_t, uint64_t> db = make_doorbell(
                                                   0x3 /* upd */, lif_id /* lif */, TX /* type */,
                                                   0 /* pid */, 0 /* qid */, 0 /* ring */, 0 /* p_index */);
  step_doorbell(db.first, db.second);

  // Wait for packet to come out of port
  get_next_pkt(opkt, port, cos);
  if (!opkt.size()) {
    std::cout << "NO packet back from model! size: " << opkt.size() << std::endl;
    exit(1);
  } else {
    std::cout << "Got packet back from model! size: " << opkt.size() << " on port: " << port << " cos " << cos
              << std::endl;
    if (ipkt.size() == opkt.size()) {
      if (memcmp(ipkt.data(), opkt.data(), sizeof(enic_to_uplink))) {
        printf("Received packet does not match Sent packet!\n");
        ret = -1;
      } else {
        printf("Received packet matches Sent packet!\n");
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
