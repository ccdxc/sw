/*
 * hntap_switch.hpp
 *
 *  Created on: Sep 17, 2018
 *      Author: sudhiaithal
 */

#ifndef NIC_E2ETESTS_HNTAP_HNTAP_SWITCH_HPP_
#define NIC_E2ETESTS_HNTAP_HNTAP_SWITCH_HPP_


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
#include <list>
#include <linux/if_packet.h>
#include <net/if.h>
#include "netinet/ip.h"
#include "netinet/udp.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"

typedef std::array<unsigned char, 6> mac_address_t;

#define maccpy(_dst, _src) \
{ \
    for (int i = 0; i < 6; i++) { \
        _dst[i] = _src[i]; \
    } \
}

#define INT_TO_ADDR(_addr) \
(_addr & 0xFF), \
(_addr >> 8 & 0xFF), \
(_addr >> 16 & 0xFF), \
(_addr >> 24 & 0xFF)

class HntapPeer {
};

class HntapTunnelPeer : public HntapPeer {
public:
    ip_addr_t ip;
};

class HntapPassThroughSwitch;

class HntapSwitchBase {
public:
    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len) = 0;
    virtual void ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len) = 0;
    virtual void AddDevHandle(dev_handle_t *dev_handle) = 0;
    virtual void* ReceiverThread(void *arg) = 0;

    static void* receiverThread(void *arg);

    virtual void StartReceiver() {
        pthread_t thread;
        int     ret;
        ret = pthread_create(&thread, NULL, this->receiverThread, this);
        if (ret < 0) {
            TLOG("Creating thread for receiver failed");
            assert(0);
        }
    }
};

class HntapSwitch : public HntapSwitchBase {
    std::map<uint32_t,HntapPeer*> ip_peer_map;
    std::map<mac_address_t,HntapPeer*> mac_peer_map;
    std::map<mac_address_t,dev_handle_t*> ep_dev_handle_map;
    std::list<HntapPeer*> peers;
    int sock;
    int rsock;
    void createSocket();
    void setUpInterface(const char* intfName);
    void sendPacket(HntapTunnelPeer *peer, const unsigned char *pkt, uint32_t len);
    HntapPeer* getPeer(mac_addr_t macAddr);
    HntapPeer* getPeer(ip_addr_t *ipAddr);
    dev_handle_t* getDevHandleForEp(mac_addr_t macAddr);
    void addDevHandleForEp(mac_addr_t macAddr, dev_handle_t *dev_handle);
    std::list<dev_handle_t*> devHandles;
public:
    mac_addr_t intfMac;
    uint32_t ipAddr;


    HntapSwitch(const char *intfName);

    virtual void AddDevHandle(dev_handle_t *dev_handle) {
        bool found = (std::find(devHandles.begin(), devHandles.end(), dev_handle) != devHandles.end());
        if (!found) {
            TLOG("Hntap Switch : Adding Dev handle with port :%d \n", dev_handle->port);
            devHandles.push_back(dev_handle);
        }
    }

    uint32_t buildPeerPacket(HntapTunnelPeer *peer, char *peerPacket,
            const unsigned char *pkt, uint32_t len);
    virtual void* ReceiverThread(void *arg);
    virtual void ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len);



    void AddPeer(HntapTunnelPeer *peer) {
        bool found = (std::find(peers.begin(), peers.end(), peer) != peers.end());
        if (!found) {
            TLOG("Adding peer %s\n", ipaddr2str(&peer->ip));
            peers.push_back(peer);
            ip_peer_map[peer->ip.addr.v4_addr] = peer;
        }
    }

    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len);
};


class HntapPassThroughIntfSwitch : public HntapSwitchBase {
public:
    dev_handle_t  *uplink_dev;
    std::string    uplink_intf;
    int sock;
    int if_index;
    std::map<mac_address_t,bool> allowed_macs;
    HntapPassThroughSwitch *parent;

    HntapPassThroughIntfSwitch(std::string intfname);

    void createSocket() {
        this->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(this->sock < 0) {
            TLOG("Socket creation failed!");
            assert(0);
        }
        TLOG("Socket creation success!\n");
    }

    void SetParent(HntapPassThroughSwitch *parent) {
        this->parent = parent;
    }

    void setUpInterface() {
        struct ifreq    req;
        struct sockaddr_ll interfaceAddr;

          bzero(&req, sizeof(req));
          /*strncpy(req.ifr_name, this->uplink_intf.c_str(), IFNAMSIZ - 1);
           if (ioctl(this->sock, SIOCGIFFLAGS, &req) < 0) {
               TLOG("SIOCGIFFLAGS - Promiscuous... failed...\n");
               exit(1);
           }*/

          // Get the ifindex of this interface.
            bzero(&req, sizeof(req));
            strncpy(req.ifr_name, uplink_intf.c_str(), IFNAMSIZ - 1);
            if (ioctl(this->sock, SIOCGIFINDEX, &req) < 0) {
                perror("SIOCGIFINDEX");
                exit(1);
            }
            TLOG("Interface Index = %#x\n",req.ifr_ifindex);
            if_index  = req.ifr_ifindex;



            memset(&interfaceAddr,0,sizeof(interfaceAddr));
            interfaceAddr.sll_ifindex = req.ifr_ifindex;
            interfaceAddr.sll_family = PF_PACKET;
            interfaceAddr.sll_protocol = htons(ETH_P_ALL);

            // Bind interface
            if (bind(this->sock, (struct sockaddr *)&interfaceAddr,sizeof(interfaceAddr)) < 0) {
                TLOG("Bind to device failed.\n");
                exit(1);
            }

            struct packet_mreq mr;
            memset(&mr, 0, sizeof(mr));
            mr.mr_ifindex = if_index;
            mr.mr_type = PACKET_MR_PROMISC;
            if(setsockopt(this->sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
                TLOG("Promiscuous add failed\n");
                exit(1);
            }


           int val = 1;
           if (setsockopt(this->sock, SOL_PACKET, PACKET_AUXDATA, &val,
                          sizeof(val)) == -1 && errno != ENOPROTOOPT) {
                    exit(1);
           }


           TLOG("Interface Index = %#x %s\n",if_index, this->uplink_intf.c_str());
    }

    virtual void AddDevHandle(dev_handle_t *dev_handle) {
        TLOG("Hntap Switch : Setting Dev handle with port :%d (%s) to %s \n",
                dev_handle->port, dev_handle->name, uplink_intf.c_str());
        this->uplink_dev = dev_handle;
    }

    bool isAllowedMac(mac_addr_t macAddr);
    void addAllowedMac(mac_address_t macAddr) { this->allowed_macs[macAddr] = true; }

    virtual void* ReceiverThread(void *arg);
    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len);

    virtual void ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len);

};


class HntapPassThroughSwitch : public HntapSwitchBase {
    std::map<dev_handle_t*, HntapPassThroughIntfSwitch*> dev_handle_passthrough_intf;
    std::map<std::string, HntapPassThroughIntfSwitch*> model_intf_passthrough_intf;
    std::map<mac_address_t,bool> remote_macs;
    std::mutex switch_mutex;
public:

    HntapPassThroughIntfSwitch *getPassThroughIntf(dev_handle_t *dev_handle) {
        std::map<dev_handle_t*,HntapPassThroughIntfSwitch*>::iterator it;

        it = dev_handle_passthrough_intf.find(dev_handle);

        return (it != dev_handle_passthrough_intf.end()) ? it->second : 0;
    }

    HntapPassThroughIntfSwitch *getPassThroughIntf(std::string modelIntf) {
        std::map<std::string,HntapPassThroughIntfSwitch*>::iterator it;

        it = model_intf_passthrough_intf.find(modelIntf);

        return (it != model_intf_passthrough_intf.end()) ? it->second : 0;
    }


    void AddUplinkMap(std::string modelIntf, std::string uplinkIntf) {

        HntapPassThroughIntfSwitch *passThroughIntf =
                new HntapPassThroughIntfSwitch(uplinkIntf);
        TLOG("Adding %s to %s\n", modelIntf.c_str(), uplinkIntf.c_str());
        model_intf_passthrough_intf[modelIntf] = passThroughIntf;
        passThroughIntf->SetParent(this);
    }

    virtual void AddDevHandle(dev_handle_t *dev_handle) {
        HntapPassThroughIntfSwitch *passThroughIntf  = getPassThroughIntf(dev_handle->name);
        if (passThroughIntf == nullptr) {
            TLOG("Pass through interface not specified...\n");
            return;
            /* Should assert as config is invalid */
           //assert(0);
        }
        TLOG("Adding dev handle %s to %s\n",
                dev_handle->name, passThroughIntf->uplink_intf.c_str());
        dev_handle_passthrough_intf[dev_handle] = passThroughIntf;
        passThroughIntf->AddDevHandle(dev_handle);
    }

    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len) {

        struct ether_header *eth = (struct ether_header*)(pkt);
        mac_addr_t macAddr;

        memcpy(&macAddr, eth->ether_shost, sizeof(macAddr));
        if (this->isRemoteMac(macAddr)) {
            TLOG("Dropping packet as source MAC is from remote..\n");
            return;
        }

        HntapPassThroughIntfSwitch *passthroughIntf;

        passthroughIntf = getPassThroughIntf(dev_handle);

        if (passthroughIntf == nullptr) {
            TLOG("Received packet from device with no uplink association\n");
            assert(0);
        }

        passthroughIntf->ProcessPacketFromModelUplink(dev_handle, pkt, len);
    }

    virtual void ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len) {
        /* Each PassThrough Inteface will processs packets from respective uplink independently */
    }

    void* ReceiverThread(void *arg) {
        /* No Op as there are multiple of the, */
        return NULL;
    }

    void UpdateRemoteMac(mac_addr_t macAddr) {
        mac_address_t mac;
        for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
            mac[i] = macAddr[i];
        }

        switch_mutex.lock();
        this->remote_macs[mac] = true;
        switch_mutex.unlock();
    }

    bool isRemoteMac(mac_addr_t macAddr) {
        std::map<mac_address_t,bool>::iterator it;

        mac_address_t mac;
        for (unsigned int i = 0; i < sizeof(mac_addr_t); i++) {
            mac[i] = macAddr[i];
        }
        switch_mutex.lock();
        it = remote_macs.find(mac);
        switch_mutex.unlock();

        return (it != remote_macs.end()) ? it->second : false;
    }

    void AddAllowedMacs(std::list<mac_address_t> macAddrs) {
        for (std::map<std::string,HntapPassThroughIntfSwitch*>::iterator it=model_intf_passthrough_intf.begin();
                it!=model_intf_passthrough_intf.end(); ++it) {
            for (mac_address_t macAddr : macAddrs) {
                TLOG("Adding allowed mac  %02x:%02x:%02x:%02x:%02x:%02x.\n", macAddr[0],
                        macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
                it->second->addAllowedMac(macAddr);
            }
        }
    }

    virtual void StartReceiver() {
        TLOG("Starting Hntap Passthrough receivers...\n");
        for (std::map<dev_handle_t*,HntapPassThroughIntfSwitch*>::iterator it=dev_handle_passthrough_intf.begin();
                it!=dev_handle_passthrough_intf.end(); ++it) {
            TLOG("Starting Hntap Passthrough receiver for %s (%s)...\n", it->first->name,
                    it->second->uplink_intf.c_str());
            it->second->createSocket();
            it->second->setUpInterface();
            it->second->StartReceiver();
        }
    }
};

#endif /* NIC_E2ETESTS_HNTAP_HNTAP_SWITCH_HPP_ */
