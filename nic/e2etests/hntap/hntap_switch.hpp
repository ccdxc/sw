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

    HntapPassThroughIntfSwitch(std::string intfname);

    void createSocket() {
        this->sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(this->sock < 0) {
            TLOG("Socket creation failed!");
            assert(0);
        }
        TLOG("Socket creation success!\n");
    }

    void setUpInterface() {
        struct ifreq    req;

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

           // Bind interface
           if (setsockopt(this->sock, SOL_SOCKET, SO_BINDTODEVICE,
                   this->uplink_intf.c_str(), IFNAMSIZ - 1) == -1)    {
               TLOG("SO_BINDTODEVICE failed...\n");
               exit(1);
           }
           TLOG("Interface Index = %#x %s\n",if_index, this->uplink_intf.c_str());
    }

    virtual void AddDevHandle(dev_handle_t *dev_handle) {
        TLOG("Hntap Switch : Setting Dev handle with port :%d (%s) to %s \n",
                dev_handle->port, dev_handle->name, uplink_intf.c_str());
        this->uplink_dev = dev_handle;
    }

    virtual void* ReceiverThread(void *arg);
    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len);

    virtual void ProcessUplinkReceivedPacket(const unsigned char *pkt, uint32_t len);

};


class HntapPassThroughSwitch : public HntapSwitchBase {
    std::map<dev_handle_t*, HntapPassThroughIntfSwitch*> dev_handle_passthrough_intf;
    std::map<std::string, HntapPassThroughIntfSwitch*> model_intf_passthrough_intf;
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
    }

    virtual void AddDevHandle(dev_handle_t *dev_handle) {
        HntapPassThroughIntfSwitch *passThroughIntf  = getPassThroughIntf(dev_handle->name);
        if (passThroughIntf == nullptr) {
            TLOG("Pass through interface not specified...\n");
            assert(0);
        }
        TLOG("Adding dev handle %s to %s\n",
                dev_handle->name, passThroughIntf->uplink_intf.c_str());
        dev_handle_passthrough_intf[dev_handle] = passThroughIntf;
        passThroughIntf->AddDevHandle(dev_handle);
    }

    virtual void ProcessPacketFromModelUplink(dev_handle_t *dev_handle,
            const unsigned char *pkt, uint32_t len) {

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
