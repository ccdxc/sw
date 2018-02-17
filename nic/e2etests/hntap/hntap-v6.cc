#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <zmq.h>
#include <assert.h>
#include "nic/model_sim/include/buf_hdr.h"
#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "nic/model_sim/include/lib_model_client.h"
#include "nic/model_sim/include/buf_hdr.h"
#include "nic/e2etests/driver/lib_driver.hpp"
#include "nic/e2etests/hntap/dev.hpp"
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/lib/packet.hpp"

#define PKTBUF_LEN        2000
#define HNTAP_LIF_ID      15
uint32_t hntap_port;

bool hntap_go_thru_model = true; // Go thru model for host<->nw talk
bool hntap_drop_rexmit = false;
bool hntap_allow_udp = false;
uint32_t nw_retries = 0;

#define ETH_ADDR_LEN 6

std::map<dev_handle_t*,dev_handle_t*> dev_route_map;

void add_dev_handle_tap_pair(dev_handle_t *dev, dev_handle_t *other_dev)
{
    dev_route_map[dev] = other_dev;
    dev_route_map[other_dev] = dev;
}

void remove_dev_handle_tap_pair(dev_handle_t *dev, dev_handle_t *other_dev)
{
    std::map<dev_handle_t*,dev_handle_t*>::iterator it;

    it=dev_route_map.find(dev);
    dev_route_map.erase(it);

    it=dev_route_map.find(other_dev);
    dev_route_map.erase(it);
}

static dev_handle_t*
get_dest_dev_handle(dev_handle_t *dev)
{
    std::map<dev_handle_t*,dev_handle_t*>::iterator it;

    it = dev_route_map.find(dev);
    return it->second;
}

void
hntap_host_tx_to_model (dev_handle_t *dev_handle, char *pktbuf, int size)
{
  uint16_t lif_id = (uint16_t) (HNTAP_LIF_ID & 0xffff);
  uint32_t port = 0, cos = 0, count;

  uint8_t *pkt = (uint8_t *) pktbuf;
  std::vector<uint8_t> ipkt, opkt;
  uint8_t *buf;

  ipkt.resize(size);
  opkt.resize(size);
  memcpy(ipkt.data(), pkt, size);

  if (!hntap_go_thru_model) {

      /*
       * Bypass the model and send packet to Network-tap directly. Test only
       */
      memcpy(opkt.data(), pkt, size);
      TLOG("Host-Tx: Bypassing model, packet size: %d (%d), on port: %d cos %d\n",
              opkt.size(), size, port, cos);
      goto send_to_nettap;
  }

  lib_model_connect();

  // --------------------------------------------------------------------------------------------------------//

  // Create Queues
  alloc_queue(lif_id, TX, 0, 1024);
  alloc_queue(lif_id, RX, 0, 1024);

  // --------------------------------------------------------------------------------------------------------//

  // Post tx buffer
  buf = alloc_buffer(ipkt.size());
  assert(buf != NULL);
  memcpy(buf, ipkt.data(), ipkt.size());
  TLOG("buf %p size %lu\n", buf, ipkt.size());
  //TODO
  dump_pkt6((char *)buf, ipkt.size());

  // Transmit Packet
  TLOG("Writing packet to model! size: %d on port: %d\n", ipkt.size(), port);
  post_buffer(lif_id, TX, 0, buf, ipkt.size());
  // Wait for packet to come out of port
  count = 0;
  do {
     get_next_pkt(opkt, port, cos);
     if (opkt.size() == 0) {usleep(10000); count++;}
  } while (count < (nw_retries*100) && !opkt.size());
  if (!opkt.size()) {
      TLOG("NO packet back from model! size: %d\n", opkt.size());
  } else {
      TLOG("Got packet back from model! size: %d on port: %d cos %d\n", opkt.size(), port, cos);
  }

  lib_model_conn_close();

 send_to_nettap:

  if (opkt.size()) {
    /*
     * Now that we got the packet from the Model, lets send it out on the Net-Tap interface.
     */
    int nwrite;
    //hntap_net_client_to_server_nat((char *)opkt.data(), opkt.size());
    dev_handle_t *dest_dev_handle = get_dest_dev_handle(dev_handle);
    dest_dev_handle->nat_cb((char *)opkt.data(), opkt.size(), PKT_DIRECTION_TO_DEV);
    if (hntap_go_thru_model) {
        nwrite = write(dest_dev_handle->fd, opkt.data()+sizeof(struct ether_header_t), opkt.size()-sizeof(struct ether_header_t));
    } else {

        /*
         * When not going thru model, we'll send the original packet as-is to the net-tap.
	 */

        nwrite = write(dest_dev_handle->fd, opkt.data() + sizeof(struct vlan_header_t), opkt.size() - sizeof(struct vlan_header_t));
    }

    if (nwrite < 0) {
        perror("Host-Tx: Writing data to net-tap");
    } else {
        TLOG("Wrote packet with %lu bytes (%d) to Network Tap (Tx)\n", opkt.size() - sizeof(struct ether_header_t), nwrite);
    }
  }
}

void
hntap_handle_host_tx (dev_handle_t *dev_handle,
        char *pktbuf, int nread)
{
  struct ether_header_t *eth_header;

  TLOG("Host-Tx to Model: packet sent with %d bytes\n", nread);
  if (nread < (int) sizeof(struct ether_header)) return;

  dump_pkt6(pktbuf, nread);
  eth_header = (struct ether_header_t *) pktbuf;

  if (hntap_get_etype(eth_header) == ETHERTYPE_IPV6) {
    TLOG("Ether-type IP, sending to Model\n");
    //hntap_host_client_to_server_nat(pktbuf, nread);
    dev_handle->nat_cb(pktbuf, nread, PKT_DIRECTION_FROM_DEV);
    hntap_host_tx_to_model(dev_handle, pktbuf, nread);
  } else {
    TLOG("Ether-type 0x%x IGNORED\n", ntohs(eth_header->etype));
  }

}

void
hntap_net_rx_to_model (dev_handle_t *dev_handle, char *pktbuf, int size)
{
  uint16_t lif_id = (uint16_t)(HNTAP_LIF_ID & 0xffff);
  uint32_t port = 0;
  uint16_t rsize = 0;
  uint16_t prev_cindex = 0xFFFF;

  uint8_t *pkt = (uint8_t *) pktbuf;
  std::vector<uint8_t> ipkt,opkt;
  uint8_t *buf;
  bool got_nw_pkt = false;


  ipkt.resize(size);
  opkt.resize(size);

  memcpy(ipkt.data(), pkt, size);

  if (!hntap_go_thru_model) {

      /*
       * Bypass the model and send packet to Host-tap directly. Test only.
       */
      buf = (uint8_t *) pktbuf;
      rsize = size;
      TLOG("Net-Rx: Bypassing model, packet size %d!\n", rsize);
      goto send_to_hosttap;

  }

  lib_model_connect();

  // --------------------------------------------------------------------------------------------------------//

  // Create Queues
  alloc_queue(lif_id, TX, 0, 1024);
  alloc_queue(lif_id, RX, 0, 1024);

  // --------------------------------------------------------------------------------------------------------//

  // Post buffer
  buf = alloc_buffer(9126);
  assert(buf != NULL);
  memset(buf, 0, 9126);
  post_buffer(lif_id, RX, 0, buf, 9126);
  // Send packet to Model
  TLOG("Sending packet to model! size: %d on port: %d\n", ipkt.size(), port);
  step_network_pkt(ipkt, port);

#define POLL_RETRIES 4

  if (poll_queue(lif_id, RX, 0, POLL_RETRIES, &prev_cindex)) {
    TLOG("Got some packet\n");
    // Receive Packet
    consume_buffer(lif_id, RX, 0, buf, &rsize);
    if (!rsize) {
        TLOG("Did not get packet back from host side of model!\n");
    } else {
        TLOG("Got packet of size %d bytes back from host side of model!\n", rsize);
    }
  } else {
    uint32_t port = 0, cos = 0;
    uint32_t count = 0;
    do {
       get_next_pkt(opkt, port, cos);
       if (opkt.size() == 0) {usleep(10000); count++;}
    } while (count < (nw_retries*100) && !opkt.size());
    if (!opkt.size()) {
        TLOG("NO packet back from nw side of model! size: %d\n", opkt.size());
    } else {
        TLOG("Got packet back from nw side of model! size: %d on port: %d cos %d\n", opkt.size(), port, cos);
	got_nw_pkt = true;
    }
    
  }

  if (got_nw_pkt) {

    /*
     * Now that we got the packet from the Model, lets send it out on the Net-Tap interface.
     */
    int nwrite;

    //hntap_net_client_to_server_nat((char *)opkt.data(), opkt.size());
    dev_handle->nat_cb((char *)opkt.data(), opkt.size(), PKT_DIRECTION_TO_DEV);

    if ((nwrite = write(dev_handle->fd, opkt.data()+sizeof(struct ether_header_t), opkt.size()-sizeof(struct ether_header_t))) < 0){
      perror("Net-Rx: Writing data to net-tap");
    } else {
      TLOG("Wrote packet with %lu bytes to Network Tap (Rx)\n", opkt.size() - sizeof(struct ether_header_t));
    }
  }

  if (!rsize && poll_queue(lif_id, RX, 0, POLL_RETRIES, &prev_cindex)) {
      TLOG("Got some packet\n");
    // Receive Packet                                                                                                                                            
    consume_buffer(lif_id, RX, 0, buf, &rsize);
    if (!rsize) {
        TLOG("Did not get packet back from host side of model!\n");
    } else {
        TLOG("Got packet of size %d bytes back from host side of model!\n", rsize);
    }
  }


 send_to_hosttap:

  if (rsize) {
    dump_pkt6((char *)buf, rsize);
    /*
     * Now that we got the packet from the Model, lets send it out on the Host-Tap interface.
     */
    int nwrite;

    //hntap_host_server_to_client_nat((char*)buf,rsize);
    dev_handle_t *dest_dev = get_dest_dev_handle(dev_handle);
    dest_dev->nat_cb((char*)buf, rsize,  PKT_DIRECTION_TO_DEV);
    if (hntap_go_thru_model) {

        nwrite = write(dest_dev->fd, buf + sizeof(struct vlan_header_t),
		       rsize - sizeof(struct vlan_header_t));
    } else {

        /*
         * When not going thru model, we'll send the original packet as-is to the host-tap.
         */
        nwrite = write(dest_dev->fd, buf + sizeof(struct ether_header_t), rsize - sizeof(struct ether_header_t));
    }

    if (nwrite < 0) {
      perror("Net-Rx: Writing data to host-tap");
    } else {
      TLOG("Wrote packet with %lu bytes to Host Tap (Tx)\n", rsize - sizeof(struct vlan_header_t));
    }
  }

  lib_model_conn_close();
}

void
hntap_handle_net_rx (dev_handle_t *dev_handle, char *pktbuf, int nread)
{
  struct ether_header_t *eth_header;

  TLOG("Net-Rx to Model: packet sent with %d bytes\n", nread);
  if (nread < (int) sizeof(struct ether_header)) return;

  dump_pkt6(pktbuf, nread);
  eth_header = (struct ether_header_t *) pktbuf;
  if ((ntohs(eth_header->etype) == ETHERTYPE_IPV6) ||
      (ntohs(eth_header->etype) == ETHERTYPE_VLAN)) {
      TLOG("Ether-type IP, sending to Model\n");
      //hntap_net_server_to_client_nat(pktbuf, nread);
      dev_handle->nat_cb(pktbuf, nread, PKT_DIRECTION_FROM_DEV);
      hntap_net_rx_to_model(dev_handle, pktbuf, nread);
  } else {
    TLOG("Ether-type 0x%x IGNORED\n", ntohs(eth_header->etype));
  }

}

int
hntap_do_drop_rexmit(dev_handle_t *dev, uint32_t app_port_index, char *pkt, int len)
{
  struct ether_header_t *eth;
  struct vlan_header_t *vlan;
  struct ipv6_header_t *ip;
  struct tcp_header_t *tcp;
  uint16_t etype;

  if (!hntap_drop_rexmit) return(0);

  eth = (struct ether_header_t *)pkt;
  if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
    vlan = (struct vlan_header_t*)pkt;
    etype = ntohs(vlan->etype);
    ip = (ipv6_header_t *)(vlan+1);
  } else {
    etype = ntohs(eth->etype);
    ip = (ipv6_header_t *)(eth+1);
  }

  if (etype == ETHERTYPE_IPV6) {

    if (ip->nexthdr == IPPROTO_TCP) {
      tcp = (struct tcp_header_t*)(ip+1);
#if 0
      if (ntohl(tcp->seq) == *seqnum) {
          TLOG("Same sequence-number 0x%x\n", *seqnum);
          return(1);
      }
      *seqnum = ntohl(tcp->seq);
#endif
      if(tcp->rst)
          return(1);	
      
      dev->seqnum[app_port_index] = ntohl(tcp->seq);
      struct tcp_header_t *flowtcp = &dev->flowtcp[app_port_index];

      if (tcp->seq == flowtcp->seq &&
          tcp->ack_seq == flowtcp->ack_seq &&
          tcp->fin == flowtcp->fin &&
          tcp->syn == flowtcp->syn &&
          tcp->rst == flowtcp->rst &&
          tcp->psh == flowtcp->psh &&
          tcp->ack == flowtcp->ack &&
          tcp->urg == flowtcp->urg &&
          tcp->ece == flowtcp->ece &&
          tcp->cwr == flowtcp->cwr) {
          TLOG("Same tcp header fields\n");
          return(1);
      }
      memcpy(flowtcp, tcp, sizeof(tcp_header_t));
    }
  }
  return(0);
}

void
hntap_do_select_loop (dev_handle_t *dev_handles[], uint32_t max_handles)
{
  int		maxfd;
  uint16_t  nread;
  char	    pktbuf[PKTBUF_LEN];
  char      *p = pktbuf;
  unsigned  long int num_hosttap_pkts = 0, num_nettap_pkts = 0;
  int	    ret;

  while (1) {

      fd_set rd_set;
      FD_ZERO(&rd_set);
      maxfd = -1;
      for (uint32_t i = 0 ; i < max_handles; i++) {
        FD_SET(dev_handles[i]->fd, &rd_set);
        if (dev_handles[i]->fd > maxfd) {
            maxfd = dev_handles[i]->fd;
        }
      }

        TLOG("Select on host_tap_fd %d, net_tap_fd %d\n", dev_handles[0]->fd, dev_handles[1]->fd);
        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR){
                TLOG("Ret = %d, errno %d\n", ret, errno);
                continue;
        }

        if (ret < 0) {
          TLOG("select() failed: %s\n", strerror(errno));
          abort();
        }
        TLOG("Got something\n");
        for (uint32_t i = 0 ; i < max_handles; i++) {
            if (FD_ISSET(dev_handles[i]->fd, &rd_set)) {
                dev_handle_t *dev_handle = dev_handles[i];
                TLOG("Got stuff on type : %d\n", dev_handle->tap_ep);
                if (dev_handle->tap_ep == TAP_ENDPOINT_HOST) {
                    if (dev_handle->type == HNTAP_TUN) {
                        /*
                         *  Data received from host-tap. We'll read it and write to the Host-mem for model to read (Host-Tx path).
                         */
                        if ((nread = read(dev_handle->fd, p+sizeof(struct vlan_header_t), PKTBUF_LEN)) < 0) {
                          TLOG("Read from host-tap failed - %s\n", strerror(errno));
                          abort();
                        }

                        if (p[sizeof(struct vlan_header_t)] != 0x60) {
                          TLOG("Not an IPv6 packet 0x%x\n", p[sizeof(struct vlan_header_t)]);
                          continue;
                        }

                        num_hosttap_pkts++;
                        TLOG("Host-Tap %d: Read %d bytes from host tap interface\n", num_hosttap_pkts, nread);
                        if (dev_handle->pre_process) {
                            dev_handle->pre_process(pktbuf, PKTBUF_LEN);
                        }
                        int ret = dump_pkt6(pktbuf, nread + sizeof(struct vlan_header_t), dev_handle->tap_ports[0]);
                         if (ret == -1) {
                             TLOG("Not a desired TCP packet\n");
                             continue;
                         }

                        if (hntap_do_drop_rexmit(dev_handle, 0,
                                pktbuf, nread + sizeof(struct vlan_header_t))) {
                                TLOG("Retransmitted TCP packet, seqno: 0x%x, dropping\n", dev_handle->flowtcp[0].seq);
                            continue;
                        }

                        /*
                         * Setup and write to the Host-memory interface for model to read.
                         */
                        hntap_handle_host_tx(dev_handle, pktbuf, nread + sizeof(struct vlan_header_t));
                    } else if (dev_handle->type == HNTAP_TAP) {

                    } else {
                        abort();
                    }

                } else if (dev_handle->tap_ep == TAP_ENDPOINT_NET)  {
                    if (dev_handle->type == HNTAP_TUN) {
                        TLOG("Got stuff on net_tap_fd\n");
                        /*
                        * Data received from net-tap. We'll read it and write to the ZMQ to teh model (Net Rx path).
                        */
                        if ((nread = read(dev_handle->fd, p+sizeof(struct ether_header_t), PKTBUF_LEN)) < 0) {
                        perror("Read from net-tap failed!");
                        abort();
                        }

                        if (p[sizeof(struct ether_header_t)] != 0x60) {
                        TLOG("Not an IPv6 packet 0x%x\n", p[sizeof(struct ether_header_t)]);
                        continue;
                        }
                        num_nettap_pkts++;
                        TLOG("Net-Tap %d: Read %d bytes from network tap interface\n", num_nettap_pkts, nread);
                        if (dev_handle->pre_process) {
                            dev_handle->pre_process(pktbuf, PKTBUF_LEN);
                        }
                        TLOG("Added ether header\n");
                        int ret = dump_pkt6(pktbuf, nread + sizeof(struct ether_header_t));
                        if (ret == -1) {
                          TLOG("Not a desired TCP packet\n");
                            continue;
                        }

                       if (hntap_do_drop_rexmit(dev_handle, 0,
                               pktbuf, nread + sizeof(struct vlan_header_t))) {
                               TLOG("Retransmitted TCP packet, seqno: 0x%x, dropping\n", dev_handle->flowtcp[0].seq);
                           continue;
                       }

                      /*
                       * Write to the ZMQ interface to modeal for Network Rx packet path.
                       */
                      hntap_handle_net_rx(dev_handle, pktbuf, nread + sizeof(struct ether_header_t));

                    } else if (dev_handle->type == HNTAP_TAP) {

                    } else {
                        abort();
                    }

                } else {
                    abort();
                }
            }
        }
     }
}


