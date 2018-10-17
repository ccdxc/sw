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

typedef std::tuple<mac_address_t, uint32_t, uint32_t> mac_config;
typedef std::vector<mac_config> mac_config_list;

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


class mac_config_t {
public:
    mac_address_t mac;
    uint32_t port;
    uint32_t uplink_vlan;
};

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
    std::map<mac_address_t, mac_config_t*> mac_configs;
    std::map<uint32_t, bool> vlan_configs;
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

    bool isAllowedMac(mac_addr_t macAddr, uint32_t port, uint32_t uplink_vlan);
    bool isAllowedMac(mac_addr_t macAddr);
    bool isAllowedVlan(uint32_t vlan_id);
    void addMacConfig(mac_address_t macAddr, uint32_t port, uint32_t uplink_vlan) {
        mac_config_t *cfg = new mac_config_t();
        cfg->port = port;
        cfg->mac = macAddr;
        cfg->uplink_vlan = uplink_vlan;
        this->mac_configs[cfg->mac] = cfg;
        this->vlan_configs[uplink_vlan] = true;
    }

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
        mac_addr_t smacAddr,dmacAddr;

        memcpy(&smacAddr, eth->ether_shost, sizeof(mac_addr_t));
        memcpy(&dmacAddr, eth->ether_dhost, sizeof(mac_addr_t));

        /* Model might flood the pack back, make sure we drop it */
        if (this->isRemoteMac(smacAddr)) {
            TLOG("Dropping packet as source MAC (%s) is from remote..\n",
                    macaddr2str(smacAddr));
            return;
        }

        HntapPassThroughIntfSwitch *passthroughIntf;

        passthroughIntf = getPassThroughIntf(dev_handle);

        if (passthroughIntf == nullptr) {
            TLOG("Received packet from device with no uplink association\n");
            assert(0);
        }

        TLOG("Sending packet with smac %s dmac %s to outside uplink.\n",
                macaddr2str(smacAddr),  macaddr2str(dmacAddr));
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

        TLOG("Adding remote mac %s\n", macaddr2str(macAddr));

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

    void AddMacConfigs(mac_config_list macConfigs) {
        for (std::map<std::string,HntapPassThroughIntfSwitch*>::iterator it=model_intf_passthrough_intf.begin();
                it!=model_intf_passthrough_intf.end(); ++it) {
            for (mac_config mac_config : macConfigs) {
                TLOG("Adding allowed mac  %02x:%02x:%02x:%02x:%02x:%02x.\n", std::get<0>(mac_config)[0],
                        std::get<0>(mac_config)[1], std::get<0>(mac_config)[2],std::get<0>(mac_config)[3],
                        std::get<0>(mac_config)[4], std::get<0>(mac_config)[5]);
                it->second->addMacConfig(std::get<0>(mac_config),
                        std::get<1>(mac_config), std::get<2>(mac_config));
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
