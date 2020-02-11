/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __RBT_TRANSPORT_H__
#define __RBT_TRANSPORT_H__

#include <string>
#include <sys/ioctl.h>
#include <net/ethernet.h>

namespace sdk {
namespace platform {
namespace ncsi {

#define NCSI_PROTOCOL_ETHERTYPE 0x88F8

#if 0
uint8_t dummy_rsp[60] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x88, 0xf8, 0x00, 0x01,
0x00, 0xa4, 0x81, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00};
#endif

class rbt_transport : public transport {
private:
    int sock_fd;
    int ifindex;
    struct ifreq ifr;
    struct sockaddr_ll sock_addr;

public:
    rbt_transport(const char* iface_name)
    {
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), iface_name);
    };

    int Init() {
        int rc;
        const unsigned char ether_broadcast_addr[]=
                {0xff,0xff,0xff,0xff,0xff,0xff};

        printf("opening raw socket now...\n");
        sock_fd = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(NCSI_PROTOCOL_ETHERTYPE));

        memset(&sock_addr, 0, sizeof(sock_addr));

        if (ioctl(sock_fd, SIOCGIFINDEX, &ifr)==-1) {
                printf("%s",strerror(errno));
        }
        ifindex=ifr.ifr_ifindex;

        sock_addr.sll_family=AF_PACKET;
        sock_addr.sll_ifindex=ifindex;
        sock_addr.sll_halen=ETHER_ADDR_LEN;
        sock_addr.sll_protocol=htons(NCSI_PROTOCOL_ETHERTYPE);
        memcpy(sock_addr.sll_addr,ether_broadcast_addr,ETHER_ADDR_LEN);

        printf("Opened RAW socket and seting socket options now\n");
        rc = setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr));

        if (rc < 0) {
            printf("socket option SO_BINDTODEVICE failed\n");
            return rc;
        }
        else {
            printf("Opened socket succesfully for interface: %s\n", ifr.ifr_name);
            //SendPkt(dummy_rsp, sizeof(dummy_rsp));
            return sock_fd;
        }
    };
    ssize_t SendPkt(const void *buf, size_t len) { return sendto(sock_fd, buf, len, 0, (struct sockaddr*)&sock_addr,sizeof(sock_addr)); };
    ssize_t RecvPkt(void *buf, size_t len) { return recvfrom(sock_fd, buf, 1500 /*ETH_FRAME_LEN*/, 0, NULL, NULL); };

};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__RBT_TRANSPORT_H__

