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
uint32_t hntap_port;

typedef struct hntap_dev_stats_ {
    uint32_t num_pkts_recvd;
    uint32_t num_pkts_sent;
}hntap_dev_stats_t;

hntap_dev_stats_t dev_stats[TAP_ENDPOINT_MAX];

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

enum model_pkt_read_t {
    MODEL_PKT_READ_P4      = 0,
    MODEL_PKT_READ_P4_PLUS = 1
};

static bool
model_read_and_send(model_pkt_read_t read_type, dev_handle_t *dest_dev,
       uint8_t *recv_buf, uint16_t &prev_cindex)
{
    uint16_t rsize = 0;
    bool pkt_sent = false;
    uint32_t port = 0, cos = 0;
    std::vector<uint8_t> opkt;
    uint16_t offset = 0;

    if (read_type == MODEL_PKT_READ_P4_PLUS) {
#define POLL_RETRIES 4
        if ((poll_queue(dest_dev->lif_id, RX, 0, POLL_RETRIES, &prev_cindex))) {
              TLOG("Got some packet\n");
              // Receive Packet
              consume_buffer(dest_dev->lif_id, RX, 0, recv_buf, &rsize);
              if (!rsize) {
                  TLOG("Did not get packet back from host side of model!\n");
              } else {
                  TLOG("Got packet of size %d bytes back from host side of model!\n", rsize);
                  dump_pkt((char *)recv_buf, rsize);
              }
        }
    } else if (read_type == MODEL_PKT_READ_P4) {
        uint16_t count = 0;
        do {
           get_next_pkt(opkt, port, cos);
           if (opkt.size() == 0) {usleep(10000); count++;}
        } while (count < (nw_retries*100) && !opkt.size());
        if (!opkt.size()) {
            TLOG("NO packet back from model! size: %d\n", opkt.size());
        } else {
            TLOG("Got packet back from model! size: %d on port: %d cos %d\n", opkt.size(), port, cos);
            recv_buf = opkt.data();
            rsize = opkt.size();
        }
    }

    if (!rsize) {
        TLOG("Did not get packet back from the model model!\n");
    } else {
        TLOG("Got packet of size %d bytes back from host side of model!\n", rsize);
        dump_pkt((char *)recv_buf, rsize);
        /*
         * Now that we got the packet from the Model, lets send it out on the Host-Tap interface.
         */
        int nwrite;
        if (dest_dev->nat_cb) {
            dest_dev->nat_cb((char*)recv_buf, rsize,  PKT_DIRECTION_TO_DEV);
        }
        if (dest_dev->type == HNTAP_TUN) {
            /*  Tunnelled Packets does not have ethernet header and vlan header.
             *  If incoming packet has it, then packet going to destination should not have it.
             * */
            offset = dest_dev->needs_vlan_tag ?
                    sizeof(struct vlan_header_t) : sizeof(struct ether_header_t);
            nwrite = write(dest_dev->fd, recv_buf + offset, rsize - offset);
        } else if (dest_dev->type == HNTAP_TAP) {
            nwrite = write(dest_dev->fd, recv_buf, rsize);
        }
        if (nwrite < 0) {
          perror("Net-Rx 2: Writing data to host-tap");
          abort();
        } else {
          TLOG("Wrote packet with %lu bytes to Host Tap (Tx)\n", rsize - offset);
        }
        pkt_sent = true;
    }

    return pkt_sent;
}

void
hntap_model_process (dev_handle_t *dev_handle, char *pktbuf, int size)
{
  uint16_t src_lif_id = (uint16_t) (dev_handle->lif_id & 0xffff);
  dev_handle_t *dest_dev_handle = get_dest_dev_handle(dev_handle);
  uint16_t dest_lif_id = (uint16_t)(dest_dev_handle->lif_id & 0xffff);
  uint32_t port = 0, cos = 0;
  uint16_t prev_cindex = 0xFFFF;
  uint8_t *pkt = (uint8_t *) pktbuf;
  std::vector<uint8_t> ipkt, opkt;
  uint8_t *recv_buf = nullptr;
  uint8_t *send_buf = nullptr;
  bool pkt_sent = false;


  if (!hntap_go_thru_model) {
      /*
       * Bypass the model and send packet to Network-tap directly. Test only
       */
      TLOG("Host-Tx: Bypassing model, packet size: %d, on port: %d cos %d\n",
              size, port, cos);
      int nwrite;
      uint16_t offset = 0;
      if (dest_dev_handle->type == HNTAP_TUN) {
          offset = dest_dev_handle->needs_vlan_tag ? sizeof(struct vlan_header_t) : sizeof(struct ether_header_t);
      }
      nwrite = write(dest_dev_handle->fd, pkt + offset, size - offset);
      if (nwrite < 0) {
        perror("Writing data");
        abort();
      } else {
        TLOG("Wrote packet with %lu bytes to %s (Tx)\n", size - offset, hntap_type(dest_dev_handle->tap_ep));
      }
      return;
  }

    lib_model_connect();
    /* Prepare Receiver before sending packet */
    if (dest_dev_handle->tap_ep == TAP_ENDPOINT_HOST) {
      /* If Real lif, on destination, post a buffer */
      // --------------------------------------------------------------------------------------------------------//

      // Create Queues
      alloc_queue(dest_lif_id, TX, 0, 1024);
      alloc_queue(dest_lif_id, RX, 0, 1024);

      // --------------------------------------------------------------------------------------------------------//

      // Post buffer
      recv_buf = alloc_buffer(9126);
      assert(recv_buf != NULL);
      memset(recv_buf, 0, 9126);
      post_buffer(dest_lif_id, RX, 0, recv_buf, 9126);
    } else if (dest_dev_handle->tap_ep == TAP_ENDPOINT_NET) {
      opkt.resize(size);
    } else {
      abort();
    }

  if (dev_handle->tap_ep == TAP_ENDPOINT_HOST) {
      // --------------------------------------------------------------------------------------------------------//
      // Create Queues
      alloc_queue(src_lif_id, TX, 0, 1024);
      alloc_queue(src_lif_id, RX, 0, 1024);
      // --------------------------------------------------------------------------------------------------------//
      // Post tx buffer
      send_buf = alloc_buffer(size);
      assert(send_buf != NULL);
      memcpy(send_buf, pkt, size);
      TLOG("buf %p size %lu\n", send_buf, size);
      //TODO
      dump_pkt((char *)send_buf, size);

      // Transmit Packet
      TLOG("Writing packet to model! size: %d on port: %d\n", size, port);
      post_buffer(src_lif_id, TX, 0, send_buf, size);
  } else if (dev_handle->tap_ep == TAP_ENDPOINT_NET)  {
      // Send packet to Model
      ipkt.resize(size);
      memcpy(ipkt.data(), pkt, size);
      TLOG("Sending packet to model! size: %d on port: %d\n", ipkt.size(), port);
      step_network_pkt(ipkt, dev_handle->port);
  } else {
      abort();
  }

  if (dev_handle->tap_ep == TAP_ENDPOINT_NET &&
          dest_dev_handle->tap_ep == TAP_ENDPOINT_HOST) {
      /*
       * Check if packet is received on the Host side
       * May be HAL/LKL will do proxy initially.
       */
      pkt_sent = model_read_and_send(MODEL_PKT_READ_P4_PLUS,
              dest_dev_handle, recv_buf, prev_cindex);

      if (!pkt_sent) {
          /*
           * Packet not sent to host, check HAL/LKL Proxy responding.
           * Note, device should be the original device now.
           */
          pkt_sent = model_read_and_send(MODEL_PKT_READ_P4,
                  dev_handle, recv_buf, prev_cindex);
      } else {
          goto done;
      }
  }

  if (dest_dev_handle->tap_ep == TAP_ENDPOINT_NET) {
      pkt_sent = model_read_and_send(MODEL_PKT_READ_P4,
              dest_dev_handle, recv_buf, prev_cindex);

      if (pkt_sent) {
          goto done;
      }

  } else if (dest_dev_handle->tap_ep == TAP_ENDPOINT_HOST) {

      pkt_sent = model_read_and_send(MODEL_PKT_READ_P4_PLUS,
              dest_dev_handle, recv_buf, prev_cindex);

      if (pkt_sent) {
          goto done;
      }
  } else {
      abort();
  }

done:
    if (recv_buf) {
        free_buffer(recv_buf);
    }
    if (send_buf) {
        free_buffer(send_buf);
    }
    lib_model_conn_close();
}

void
hntap_pkt_process (dev_handle_t *dev_handle,
        char *pktbuf, int nread)
{
  struct ether_header_t *eth_header;

  TLOG("Host-Tx to Model: packet sent with %d bytes\n", nread);
  if (nread < (int) sizeof(struct ether_header)) return;

  dump_pkt(pktbuf, nread);
  eth_header = (struct ether_header_t *) pktbuf;

  if (hntap_get_etype(eth_header) == ETHERTYPE_IP ||
          (hntap_get_etype(eth_header) == ETHERTYPE_ARP)) {
    TLOG("Ether-type IP, sending to Model\n");
    //hntap_host_client_to_server_nat(pktbuf, nread);
    if (dev_handle->nat_cb) {
        dev_handle->nat_cb(pktbuf, nread, PKT_DIRECTION_FROM_DEV);
    }
    hntap_model_process(dev_handle, pktbuf, nread);
  } else {
    TLOG("Ether-type 0x%x IGNORED\n", ntohs(eth_header->etype));
  }

}

static int
hntap_do_drop_rexmit(dev_handle_t *dev, uint32_t app_port_index, char *pkt, int len)
{
  struct ether_header_t *eth;
  struct vlan_header_t *vlan;
  struct ipv4_header_t *ip;
  struct tcp_header_t *tcp;
  uint16_t etype;

  if (!hntap_drop_rexmit) return(0);

  eth = (struct ether_header_t *)pkt;
  if (ntohs(eth->etype) == ETHERTYPE_VLAN) {
    vlan = (struct vlan_header_t*)pkt;
    etype = ntohs(vlan->etype);
    ip = (ipv4_header_t *)(vlan+1);
  } else {
    etype = ntohs(eth->etype);
    ip = (ipv4_header_t *)(eth+1);
  }

  if (etype == ETHERTYPE_IP) {

    if (ip->protocol == IPPROTO_TCP) {
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
                TLOG("Got stuff on type : %s\n", hntap_type(dev_handle->tap_ep));
                uint16_t offset = 0;
                if (dev_handle->type == HNTAP_TUN) {
                    offset = dev_handle->needs_vlan_tag ? sizeof(struct vlan_header_t) : sizeof(struct ether_header_t);
                }
                if ((nread = read(dev_handle->fd, p + offset, PKTBUF_LEN)) < 0) {
                  TLOG("Read from host-tap failed - %s\n", strerror(errno));
                  abort();
                }

                if (dev_handle->type == HNTAP_TUN) {
                    if (p[offset] != 0x45) {
                      TLOG("Not an IP packet 0x%x\n", p[offset]);
                      continue;
                    }
                }
                dev_stats[dev_handle->tap_ep].num_pkts_recvd++;
                TLOG("%s:%d Read %d bytes \n", hntap_type(dev_handle->tap_ep),
                        dev_stats[dev_handle->tap_ep].num_pkts_recvd, nread);
                if (dev_handle->pre_process) {
                    dev_handle->pre_process(pktbuf, PKTBUF_LEN);
                }
                int ret = dump_pkt(pktbuf, nread + offset, dev_handle->tap_ports[0]);
                 if (ret == -1) {
                     TLOG("Not a desired packet\n");
                     continue;
                 }
                 if (hntap_do_drop_rexmit(dev_handle, 0, pktbuf, nread + offset)) {
                         TLOG("Retransmitted TCP packet, seqno: 0x%x, dropping\n", dev_handle->flowtcp[0].seq);
                     continue;
                 }
                 hntap_pkt_process(dev_handle, pktbuf, nread + offset);
            }
        }
     }
}


