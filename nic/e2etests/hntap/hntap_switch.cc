/*
 * hntap_switch.cc
 *
 *  Created on: Sep 17, 2018
 *      Author: sudhiaithal
 */

#include "nic/e2etests/hntap/hntap_switch.hpp"
 #include <linux/if_packet.h>
#include "nic/e2etests/lib/packet.hpp"

#define BUF_SIZE 9126
#define HSWITCH_SEND_PORT 12345
#define HSWITCH_RECV_PORT 54321
#define IP_IDENTIFICATION HSWITCH_RECV_PORT

/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}


void HntapSwitch::createSocket() {
    this->sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if(this-> sock < 0) {
        TLOG("Socket creation failed!");
        assert(0);
    }

    this->rsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(this->rsock < 0) {
        TLOG("Socket creation failed!");
        assert(0);
    }
    TLOG("Socket creation success!\n");
}

void HntapSwitch::setUpInterface(const char* intfName) {
    struct ifreq    req;
    // Get the ifindex of this interface.
      bzero(&req, sizeof(req));
      strncpy(req.ifr_name, intfName, IFNAMSIZ - 1);
      if (ioctl(this->sock, SIOCGIFINDEX, &req) < 0) {
          perror("SIOCGIFINDEX");
          exit(1);
      }
      TLOG("Interface Index = %#x\n",req.ifr_ifindex);

      // Get the MAC address of this interface.
      bzero(&req, sizeof(req));
      strcpy(req.ifr_name, intfName);
      if (ioctl(this->sock, SIOCGIFHWADDR, &req) < 0) {
          perror("SIOCGIFHWADDR");
          exit(1);
      }
      maccpy(intfMac, ((uint8_t *)&req.ifr_hwaddr.sa_data));
      TLOG("Mac address : %s\n",macaddr2str(intfMac));

      /* Retrieve the IP address, broadcast address, and subnet mask. */
       if (ioctl(this->sock, SIOCGIFADDR, &req) == 0)
       {
           ipAddr = ((struct sockaddr_in *)(&req.ifr_addr))->sin_addr.s_addr;
           printf("IP address: %d.%d.%d.%d\n", INT_TO_ADDR(ipAddr));
       }

      bzero(&req, sizeof(req));
      strncpy(req.ifr_name, intfName, IFNAMSIZ - 1);
       if (ioctl(this->rsock, SIOCGIFINDEX, &req) < 0) {
           perror("SIOCGIFINDEX");
           TLOG("SIOCGIFINDEX.. failed..\n");
       }

       //req.ifr_flags |= IFF_PROMISC;
       if (ioctl(this->rsock, SIOCGIFFLAGS, &req) < 0) {
           TLOG("SIOCGIFFLAGS - Promiscuous... failed...\n");
           exit(1);
       }

       // Bind interface
       if (setsockopt(this->rsock, SOL_SOCKET, SO_BINDTODEVICE,
               intfName, IFNAMSIZ - 1) == -1)    {
           TLOG("SO_BINDTODEVICE failed...\n");
           exit(1);
       }
       TLOG("Interface Index = %#x %s\n",req.ifr_ifindex, intfName);
}


HntapSwitch::HntapSwitch(const char *intfName) {
    sock = 0;
    bzero(&intfMac, sizeof(mac_addr_t));
    ipAddr = 0;
    rsock = 0;
    this->createSocket();
    this->setUpInterface(intfName);
}

void* HntapSwitchBase::receiverThread(void *arg) {

    struct HntapSwitchBase *hswitch = (struct HntapSwitchBase*)arg;
    return hswitch->ReceiverThread(arg);
}

void* HntapSwitch::ReceiverThread(void *arg) {
   struct HntapSwitch *hswitch = (struct HntapSwitch*)arg;
   int numbytes;
   uint8_t buf[BUF_SIZE];

   TLOG("Starting Receiver Thread on socket...\n");
   while(1) {
       numbytes = recvfrom(hswitch->rsock, buf, BUF_SIZE, 0, NULL, NULL);
       if (numbytes <= 0) {
           continue;
       }
       TLOG("Received %d bytes\n", numbytes);
       hswitch->ProcessUplinkReceivedPacket(buf, numbytes);
   }
   pthread_exit(NULL);
}


bool HntapPassThroughIntfSwitch::isAllowedMac(mac_addr_t macAddr,
        uint32_t port, uint32_t uplink_vlan) {
    std::map<mac_address_t,mac_config_t*>::iterator it;

    mac_address_t mac;
    for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
        mac[i] = macAddr[i];
    }
    it = mac_configs.find(mac);

    if (it != mac_configs.end() && it->second->port == port &&
            it->second->uplink_vlan == uplink_vlan) {
        return true;
    }

    return false;
}

bool HntapPassThroughIntfSwitch::isAllowedMac(mac_addr_t macAddr) {
    std::map<mac_address_t,mac_config_t*>::iterator it;

    mac_address_t mac;
    for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
        mac[i] = macAddr[i];
    }
    it = mac_configs.find(mac);

    if (it != mac_configs.end()) {
        return true;
    }

    return false;
}

bool HntapPassThroughIntfSwitch::isAllowedVlan(uint32_t vlan_id) {
    std::map<uint32_t, bool>::iterator it;

    it = vlan_configs.find(vlan_id);

    if (it != vlan_configs.end()) {
        return true;
    }

    return false;
}

static bool
is_broadcast(mac_addr_t mac_addr) {

    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if (mac_addr[i] != 0xff) {
            return false;
        }
    }
    return true;
}


union {
        struct cmsghdr  cmsg;
        char            buf[CMSG_SPACE(sizeof(struct tpacket_auxdata))];
} cmsg_buf;


#define VLAN_VALID(hdr, hv)     ((hv)->tp_vlan_tci != 0 || ((hdr)->tp_status & TP_STATUS_VLAN_VALID))

#ifdef TP_STATUS_VLAN_TPID_VALID
#define VLAN_TPID(hdr, hv)     (((hv)->tp_vlan_tpid || ((hdr)->tp_status & TP_STATUS_VLAN_TPID_VALID)) ? (hv)->tp_vlan_tpid : ETH_P_8021Q)
#else
#define VLAN_TPID(hdr, hv)     (ETH_P_8021Q)
#endif

static int
read_vlan_tag(struct msghdr *msg_hdr, uint16_t *tpid, uint16_t* vlan_id) {
   struct cmsghdr *cmsg;
   struct tpacket_auxdata *aux;

    for (cmsg = CMSG_FIRSTHDR(msg_hdr); cmsg; cmsg = CMSG_NXTHDR(msg_hdr, cmsg)) {
            if (cmsg->cmsg_len < CMSG_LEN(sizeof(struct tpacket_auxdata)) ||
                cmsg->cmsg_level != SOL_PACKET ||
                cmsg->cmsg_type != PACKET_AUXDATA) {
                    /*
                     * This isn't a PACKET_AUXDATA auxiliary
                     * data item.
                     */
                    continue;
            }
            aux = (struct tpacket_auxdata *)CMSG_DATA(cmsg);
            if (!VLAN_VALID(aux, aux)) {
                    /*
                     * There is no VLAN information in the
                     * auxiliary data.
                     */
                    continue;
            } else {
                 *tpid = (VLAN_TPID(aux, aux));
                 *vlan_id = (aux->tp_vlan_tci);
                 return 0;
            }
     }

    return -1;
}

void* HntapPassThroughIntfSwitch::ReceiverThread(void *arg) {
   struct HntapPassThroughIntfSwitch *hswitch = (struct HntapPassThroughIntfSwitch*)arg;
   int numbytes;
   uint8_t buf[BUF_SIZE];
   uint8_t new_buf[BUF_SIZE];
   uint8_t *final_buf;
   mac_addr_t dmacAddr;
   mac_addr_t smacAddr;
   struct ether_header *eth;
   struct sockaddr_ll  address;
   uint8_t dest[ETH_ALEN] = {0x08, 0x00, 0x27, 0xbf, 0x95, 0xfd};
   struct msghdr msg;
   struct iovec iov[1];
   uint16_t vlan_tpid, vlan_id;

   address.sll_ifindex = this->if_index;
   address.sll_halen = ETH_ALEN;
   memcpy(address.sll_addr, dest, ETH_ALEN);
   bzero(&msg, sizeof(struct msghdr));
   msg.msg_iov = &iov[0];
   msg.msg_iovlen = 1;
   msg.msg_name = &address;
   msg.msg_namelen = sizeof(address);
   iov[0].iov_base=buf;
   iov[0].iov_len=BUF_SIZE;

   msg.msg_control    = &cmsg_buf;
   msg.msg_controllen = sizeof(cmsg_buf);

   TLOG("Starting Receiver Thread on socket : %d (%s) \n", this->sock,
           this->uplink_intf.c_str());
   while(1) {

       numbytes = recvmsg(hswitch->sock, &msg, 0);
        if (numbytes <= 0) {
            continue;
        }

       eth = (struct ether_header*)(buf);
       memcpy(&dmacAddr, eth->ether_dhost, sizeof(mac_addr_t));
       memcpy(&smacAddr, eth->ether_shost, sizeof(mac_addr_t));
       vlan_id = 0;
       vlan_tpid = 0;
       read_vlan_tag(&msg, &vlan_tpid, &vlan_id);


       /* Make sure Smac is not one which is present in this host
        * It could be possible as we are in promiscuous mode.
        * Allow only broadcast and allowed macs only.
        */
       if (!this->isAllowedMac(smacAddr)  && this->isAllowedVlan(vlan_id)  &&
                   (is_broadcast(dmacAddr) ||
                       this->isAllowedMac(dmacAddr, this->uplink_dev->port, vlan_id))) {

           final_buf = buf;
           if (vlan_tpid != 0) {
               TLOG("Vlan ID read successfully :%d (%d)\n", vlan_id, vlan_tpid);
               memcpy(new_buf, buf, sizeof(ether_header));
               vlan_header_t *vlan_hdr =  (vlan_header_t*)(new_buf);
               vlan_hdr->tpid = htons(vlan_tpid);
               vlan_hdr->vlan_tag = htons(vlan_id);
               vlan_hdr->etype = ((struct ether_header*)(buf))->ether_type;
               memcpy(new_buf + sizeof(vlan_header_t),
                       buf + sizeof(struct ether_header), numbytes - sizeof(ether_header));
               final_buf = new_buf;
               numbytes = numbytes + 4;
           }
           TLOG("Received %d bytes from : %d (%s) \n",
               numbytes, hswitch->sock, this->uplink_intf.c_str());
           hswitch->ProcessUplinkReceivedPacket(final_buf, numbytes);
       } else {
           //TLOG("Dropping Received %d bytes from : %d (%s) \n",
           //    numbytes, hswitch->sock, this->uplink_intf.c_str());
       }
   }

   pthread_exit(NULL);
}


void HntapSwitch::ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len) {
    mac_address_t macAddr;

    struct ether_header *eth = (struct ether_header*)(pkt);
    struct udphdr *udp = (struct udphdr*)(pkt +
            sizeof(struct ether_header) + sizeof(struct ip));
    uint32_t data_pkt_len = ntohs(udp->len) - sizeof(struct udphdr);
    char *data = (char *)(pkt +
            sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr));
    struct ip *iphdr = (struct ip*)(pkt + sizeof(struct ether_header));

    if (udp->dest != htons(HSWITCH_RECV_PORT)) {
        TLOG("Dropping unknown packet received...\n");
        return;
    }
    ip_addr_t ip_addr { 0 };
    ip_addr.addr.v4_addr = ntohl(iphdr->ip_src.s_addr);
    HntapPeer *peer = this->getPeer(&ip_addr);
    if (peer == NULL) {
        TLOG("Received packet from unknown peer : %s, %s, len : %d\n",
                ipaddr2str(&ip_addr), macaddr2str(eth->ether_shost), len);
    } else {
        struct ether_header *internalEth = (struct ether_header*)(data);
        TLOG("Received packet from  peer : %s, Mac : %s\n",
                ipaddr2str(&ip_addr), macaddr2str(internalEth->ether_shost));
        for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
            macAddr[i] = internalEth->ether_shost[i];
        }
        this->mac_peer_map[macAddr] = peer;

        dev_handle_t *dev_handle = getDevHandleForEp(internalEth->ether_dhost);
        if (dev_handle != NULL) {
            TLOG("Sending packet from  peer : %s, Source Mac : %s Destination Mac : %s to Port \n",
                ipaddr2str(&ip_addr), macaddr2str(eth->ether_shost),
                macaddr2str(eth->ether_dhost),
                dev_handle->port);
            /* Send Packet to model uplink */
            hntap_model_send_process(dev_handle, data, data_pkt_len);
        } else {
            TLOG("No destination Uplink found for Dmac : %s\n", macaddr2str(eth->ether_dhost));
            for (dev_handle_t *dev_handle : this->devHandles) {
                TLOG("Sending packet from  peer : %s, Source Mac : %s Destination Mac : %s to Port \n",
                    ipaddr2str(&ip_addr), macaddr2str(eth->ether_shost),
                    macaddr2str(eth->ether_dhost),
                    dev_handle->port);
                hntap_model_send_process(dev_handle, data, data_pkt_len);
                //In host pinned mode send it to both uplinks
                //This might have to be revisted for smart mode.
                //break;
            }
        }
    }
}

void HntapSwitch::sendPacket(HntapTunnelPeer *peer,
        const unsigned char *pkt, uint32_t len) {
    char buffer[BUF_SIZE];
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(HSWITCH_RECV_PORT);
    sin.sin_addr.s_addr = htonl(peer->ip.addr.v4_addr);
    uint32_t peerPacketeLen = buildPeerPacket(peer, buffer, pkt, len);
    if(sendto(this->sock, buffer, peerPacketeLen, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        TLOG("sendto() error\n");
        assert(0);
    }
}

uint32_t HntapSwitch::buildPeerPacket(HntapTunnelPeer *peer, char *peerPacket,
        const unsigned char *pkt, uint32_t len) {
    struct ip *ip = (struct ip *) peerPacket;
    struct udphdr *udp = (struct udphdr *) (peerPacket + sizeof(struct ip));
    unsigned char *data = (unsigned char*) (peerPacket + sizeof(struct ip) + sizeof(struct udphdr));
    memset(peerPacket, 0, BUF_SIZE);


    ip->ip_hl = 5;
    ip->ip_v = 4;
    ip->ip_tos = 16; // Low delay
    ip->ip_len = sizeof(struct ip) + sizeof(struct udphdr) + len;
    ip->ip_id = htons(IP_IDENTIFICATION);
    ip->ip_ttl = 64; // hops
    ip->ip_p = IPPROTO_UDP; // UDP

    // Source IP address, can use spoofed address here!!!

    ip->ip_dst.s_addr = htonl(peer->ip.addr.v4_addr);
    ip->ip_src.s_addr = this->ipAddr;


    // Fabricate the UDP header. Source port number, redundant
    udp->source = htons(HSWITCH_SEND_PORT);
    // Destination port number
    udp->dest = htons(HSWITCH_RECV_PORT);
    udp->len = htons(sizeof(struct udphdr) + len);
    memcpy(data, pkt, len);

    // Calculate the checksum for integrity
    ip->ip_sum = csum((unsigned short *)peerPacket, sizeof(struct ip) + sizeof(struct udphdr) + len);

    return ip->ip_len;
}


HntapPeer* HntapSwitch::getPeer(mac_addr_t macAddr) {
    std::map<mac_address_t,HntapPeer*>::iterator it;

    mac_address_t mac;
    for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
        mac[i] = macAddr[i];
    }
    it = mac_peer_map.find(mac);

    return (it != mac_peer_map.end()) ? it->second : 0;

}

dev_handle_t* HntapSwitch::getDevHandleForEp(mac_addr_t macAddr) {
    std::map<mac_address_t, dev_handle_t*>::iterator it;

    mac_address_t mac;
    for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
        mac[i] = macAddr[i];
    }
    it = ep_dev_handle_map.find(mac);

    return (it != ep_dev_handle_map.end()) ? it->second : 0;

}

void HntapSwitch::addDevHandleForEp(mac_addr_t macAddr, dev_handle_t *dev_handle) {
    mac_address_t mac_addr;
    for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
        mac_addr[i] = macAddr[i];
    }
    ep_dev_handle_map[mac_addr] = dev_handle;
}


HntapPeer* HntapSwitch::getPeer(ip_addr_t *ipAddr) {
    std::map<uint32_t,HntapPeer*>::iterator it;

    ip_addr_t ip_addr;

    memcpy(&ip_addr, ipAddr, sizeof(ip_addr_t));

    it = ip_peer_map.find(ip_addr.addr.v4_addr);

    return (it != ip_peer_map.end()) ? it->second : 0;

}


void HntapSwitch::ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
        const unsigned char *pkt, uint32_t len) {
    struct ether_header *eth = (struct ether_header*)(pkt);
    mac_addr_t macAddr;

    memcpy(&macAddr, eth->ether_shost, sizeof(macAddr));
    HntapTunnelPeer *peer = static_cast<HntapTunnelPeer*>(getPeer(macAddr));
    if (peer != NULL) {
        TLOG("Dropping packet based on Dejavu Check..\n");
        return;
    }

    if (dev_handle != NULL) {
        /* Learn from which dev the packet came from */
        memcpy(&macAddr, eth->ether_shost, sizeof(macAddr));
        dev_handle_t *cur_dev_handle = this->getDevHandleForEp(macAddr);
        if (cur_dev_handle != dev_handle) {
            /* Dev handle changed  or not known*/
            this->addDevHandleForEp(macAddr, dev_handle);
        }
    }


    memcpy(&macAddr, eth->ether_dhost, sizeof(macAddr));
    peer = static_cast<HntapTunnelPeer*>(getPeer(macAddr));
    if (peer != NULL) {
        TLOG("Sending packet with destination for Dmac : %s, to Peer : %s\n",
                macaddr2str(macAddr), ipaddr2str((&peer->ip)));
        this->sendPacket(peer, pkt, len);
    } else {
        // Iterate and print values of the list
        TLOG("No destination found for Dmac : %s\n", macaddr2str(macAddr));
        for (HntapPeer *peer : this->peers) {
            HntapTunnelPeer *tunPeer = static_cast<HntapTunnelPeer*>(peer);
            TLOG("Sending packet with destination for Dmac : %s, to Peer : %s\n",
                macaddr2str(macAddr), ipaddr2str((&tunPeer->ip)));
            this->sendPacket(tunPeer, pkt, len);
        }
    }

}


HntapPassThroughIntfSwitch::HntapPassThroughIntfSwitch(std::string intfName) {
    uplink_intf = intfName;
}


void HntapPassThroughIntfSwitch::ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len) {

    struct ether_header *eth = (struct ether_header*)(pkt);
    mac_addr_t macAddr;

    /* Make sure we don't loop the packet in prom mode */
    memcpy(&macAddr, eth->ether_shost, sizeof(macAddr));
    this->parent->UpdateRemoteMac(macAddr);

    TLOG("Sending Pkt from Uplink to model :%d(%s) \n", this->sock,
            this->uplink_intf.c_str());
    dump_pkt((char *)pkt, len);
    hntap_model_send_process(this->uplink_dev, (char*)pkt, len);
}


void HntapPassThroughIntfSwitch::ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
        const unsigned char *pkt, uint32_t len) {

    struct sockaddr_ll  address;
    uint8_t dest[ETH_ALEN] = {0x08, 0x00, 0x27, 0xbf, 0x95, 0xfd};
    struct msghdr msg;
    struct iovec iov[1];
    int send_len;

    assert(dev_handle == this->uplink_dev);

    address.sll_ifindex = this->if_index;
    address.sll_halen = ETH_ALEN;
    memcpy(address.sll_addr, dest, ETH_ALEN);
    bzero(&msg, sizeof(struct msghdr));
    msg.msg_iov = &iov[0];
    msg.msg_iovlen = 1;
    msg.msg_name = &address;
    msg.msg_namelen = sizeof(address);
    iov[0].iov_base = (void*)pkt;
    iov[0].iov_len = len;

    struct sockaddr_ll sadr_ll;
    sadr_ll.sll_ifindex = this->if_index; // index of interface
    sadr_ll.sll_halen = 6; // length of destination mac address
    sadr_ll.sll_addr[0] = dest[0];
    sadr_ll.sll_addr[1] = dest[1];
    sadr_ll.sll_addr[2] = dest[2];
    sadr_ll.sll_addr[3] = dest[3];
    sadr_ll.sll_addr[4] = dest[4];
    sadr_ll.sll_addr[5] = dest[5];

    TLOG("Sending Pkt from model :%d(%s) \n", this->sock,
            this->uplink_intf.c_str());
    dump_pkt((char *)pkt, len);
    send_len = sendto(this->sock, pkt,len, 0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
    if( send_len<0 ) {
        TLOG("sendto() error %d %d %d\n", errno, this->sock, len);
        assert(0);
    }
}
