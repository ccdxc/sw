/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __CMD_HNDLR_H__
#define __CMD_HNDLR_H__

#include <string>
#include <memory>
#include "pkt-defs.h"
#include "ipc_service.h"
#include "transport.h"
#include "state_machine.h"
#include "ncsi_param_db.h"

#define SUPPORTED_NCSI_REV          0x1
#define MAX_NCSI_CMDS               256
#define MIN_802_3_FRAME_SZ          60
#define NCSI_HDR_REV_OFFSET         16
#define NCSI_CMD_OPCODE_OFFSET      18

/* NCSI capabilities */
#define NCSI_CAP_HW_ARB                                 (0)
#define NCSI_CAP_HOST_NC_DRV_STATUS                     (0 << 1)
#define NCSI_CAP_NC_TO_MC_FLOW_CTRL                     (0 << 2)
#define NCSI_CAP_MC_TO_NC_FLOW_CTRL                     (0 << 3)
#define NCSI_CAP_ALL_MCAST_ADDR_SUPPORT                 (1 << 4)
#define NCSI_CAP_HW_ARB_IMPL_STATUS                     (1 << 5) //1 means not impl

#define NCSI_CAP_BCAST_FILTER_ARP                       (1)
#define NCSI_CAP_BCAST_FILTER_DHCP_CLIENT               (1 << 1)
#define NCSI_CAP_BCAST_FILTER_DHCP_SERVER               (1 << 2)
#define NCSI_CAP_BCAST_FILTER_NETBIOS                   (1 << 3)

#define NCSI_CAP_MCAST_IPV6_NEIGH_ADV                   (1)
#define NCSI_CAP_MCAST_IPV6_ROUTER_ADV                  (1 << 1)
#define NCSI_CAP_MCAST_DHCPV6_RELAY                     (1 << 2)
#define NCSI_CAP_MCAST_DHCPV6_MCAST_SERVER_TO_CLIENT    (1 << 3)
#define NCSI_CAP_MCAST_IPV6_MLD                         (1 << 4)
#define NCSI_CAP_MCAST_IPV6_NEIGH_SOL                   (1 << 5)

#define NCSI_CAP_BUFFERRING                             0x2000 //i.e 8192 bytes

#define NCSI_CAP_AEN_CTRL_LINK_STATUS_CHANGE            (1)
#define NCSI_CAP_AEN_CTRL_CONFIG_REQUIRED               (1 << 1)
#define NCSI_CAP_AEN_CTRL_HOST_NC_DRV_STATUS_CHANGE     (0 >> 2)
#define NCSI_CAP_AEN_CTRL_OEM_SPECIFIC                  (0 >> 16)

#define NCSI_CAP_MCAST_FILTER_COUNT                     0
#define NCSI_CAP_UCAST_FILTER_COUNT                     0
#define NCSI_CAP_MIXED_MAC_FILTER_COUNT                 0x8
#define NCSI_CAP_VLAN_FILTER_COUNT	                    0xF
#define NCSI_CAP_VLAN_MODE_SUPPORT	                    0x3
#define NCSI_CAP_CHANNEL_COUNT  	                    0x2


/* Response codes */
#define NCSI_RSP_COMMAND_COMPLETED      0x0
#define NCSI_RSP_COMMAND_FAILED         0x1
#define NCSI_RSP_COMMAND_UNAVAILABLE    0x2
#define NCSI_RSP_COMMAND_UNSUPPORTED    0x3

// pensando specific response code
#define NCSI_RSP_INTERNAL_ERR           0x8000 

/* Reason codes */
#define NCSI_REASON_NO_ERR              0x0
#define NCSI_REASON_INTF_INIT_REQRD     0x1
#define NCSI_REASON_INVALID_PARAM       0x2
#define NCSI_REASON_CHAN_NOT_RDY        0x3
#define NCSI_REASON_PKG_NOT_RDY         0x4
#define NCSI_REASON_INVLD_PLD_LEN       0x5

#define NCSI_REASON_INVLD_VLAN          0x0B07
#define NCSI_REASON_INVLD_MAC_ADDR      0x0E08

#define NCSI_REASON_UNKNOWN_CMD         0x7FFF

// pensando specific reason code
#define NCSI_REASON_INTERNAL_ERR        0x8000 
#define NCSI_REASON_INVALID_CMD_ERR     0x8001 

class StateMachine;

namespace sdk {
namespace platform {
namespace ncsi {

struct NcsiStats {
    //Incremented for each valid NCSI command packet
    uint32_t valid_cmd_rx_cnt;        

    //rx_drop_cnt includes short pkts, invalid NCSI header rev and invalid channel id
    uint32_t rx_drop_cnt;

    //Incremented if cmd is not implemented
    uint32_t unsup_cmd_rx_cnt;

    //Incremented if NCSI header checksum is wrong
    uint32_t invalid_chksum_rx_cnt;        
    
    //Addition of all Received packets
    uint32_t rx_total_cnt;
    
    //Incremented for each transmitted packet
    uint32_t tx_total_cnt;
};

typedef void (*CmdHndlrFunc)(void *obj, const void* cmd_pkt, ssize_t cmd_sz);

class CmdHndler {
//class CmdHndler : public IpcService {

public:
    CmdHndler(std::shared_ptr<IpcService> IpcObj, transport *xport);
    int HandleCmd(const void* pkt, ssize_t sz);
private:

    struct NcsiStats stats;
    transport *xport;

    std::shared_ptr<IpcService> ipc;
    //void (*cmd_hndlr[MAX_NCSI_CMDS])(const void* cmd_pkt, ssize_t cmd_sz);
    CmdHndlrFunc CmdTable[MAX_NCSI_CMDS];
    static NcsiParamDb NcsiDb[NCSI_CAP_CHANNEL_COUNT];
    static uint64_t mac_addr_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_MIXED_MAC_FILTER_COUNT];
    static uint16_t vlan_filter_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_VLAN_FILTER_COUNT];
    static StateMachine *StateM[NCSI_CAP_CHANNEL_COUNT];

    int SendNcsiCmdResponse(const void *buf, ssize_t sz);

    int ValidateCmdPkt(const void *pkt, ssize_t sz);
    int ConfigVlanFilter(uint16_t vlan, uint32_t port, bool enable);
    int ConfigVlanMode(uint8_t vlan_mode, uint32_t port, bool enable);
    int ConfigMacFilter(uint64_t mac_addr, uint32_t port, uint8_t mac_addr_type,
            bool enable);
    void ChannelEnable(const void *cmd_pkt, ssize_t cmd_sz, bool enable);
    void ChannelEnableNwTx(const void *cmd_pkt, ssize_t cmd_sz, bool enable);
    
    // NCSI command handlers
    static void ClearInitState(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void SelectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void DeselectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void DisableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void ResetChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void DisableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void SetLink(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetLinkStatus(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void SetVlanFilter(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void DisableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void SetMacAddr(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableBcastFilter(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void DisableBcastFilter(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void EnableGlobalMcastFilter(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void DisableGlobalMcastFilter(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void GetVersionId(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetCapabilities(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetParams(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetNicPktStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetNcsiStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void GetNcsiPassthruStats(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void GetPackageStatus(void *obj, const void *cmd_pkt,
            ssize_t cmd_sz);
    static void GetPackageUUID(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
};

} // namespace ncsi
} // namespace platform
} // namespace sdk

#endif //__CMD_HNDLR_H__

