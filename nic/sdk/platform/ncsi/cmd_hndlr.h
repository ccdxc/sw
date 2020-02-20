/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __CMD_HNDLR_H__
#define __CMD_HNDLR_H__

#include <string>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "pkt-defs.h"
#include "ipc_service.h"
#include "transport.h"
#include "state_machine.h"
#include "ncsi_param_db.h"
#include "platform/utils/mpartition.hpp"

#define SUPPORTED_NCSI_REV          0x1
#define MAX_NCSI_CMDS               256
#define MIN_802_3_FRAME_SZ          60
#define NCSI_HDR_REV_OFFSET         15
#define NCSI_CMD_OPCODE_OFFSET      18

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

using boost::property_tree::ptree;

struct port_stats {
	__le64 frames_rx_ok;
	__le64 frames_rx_all;
	__le64 frames_rx_bad_fcs;
	__le64 frames_rx_bad_all;
	__le64 octets_rx_ok;
	__le64 octets_rx_all;
	__le64 frames_rx_unicast;
	__le64 frames_rx_multicast;
	__le64 frames_rx_broadcast;
	__le64 frames_rx_pause;
	__le64 frames_rx_bad_length;
	__le64 frames_rx_undersized;
	__le64 frames_rx_oversized;
	__le64 frames_rx_fragments;
	__le64 frames_rx_jabber;
	__le64 frames_rx_pripause;
	__le64 frames_rx_stomped_crc;
	__le64 frames_rx_too_long;
	__le64 frames_rx_vlan_good;
	__le64 frames_rx_dropped;
	__le64 frames_rx_less_than_64b;
	__le64 frames_rx_64b;
	__le64 frames_rx_65b_127b;
	__le64 frames_rx_128b_255b;
	__le64 frames_rx_256b_511b;
	__le64 frames_rx_512b_1023b;
	__le64 frames_rx_1024b_1518b;
	__le64 frames_rx_1519b_2047b;
	__le64 frames_rx_2048b_4095b;
	__le64 frames_rx_4096b_8191b;
	__le64 frames_rx_8192b_9215b;
	__le64 frames_rx_other;
	__le64 frames_tx_ok;
	__le64 frames_tx_all;
	__le64 frames_tx_bad;
	__le64 octets_tx_ok;
	__le64 octets_tx_total;
	__le64 frames_tx_unicast;
	__le64 frames_tx_multicast;
	__le64 frames_tx_broadcast;
	__le64 frames_tx_pause;
	__le64 frames_tx_pripause;
	__le64 frames_tx_vlan;
	__le64 frames_tx_less_than_64b;
	__le64 frames_tx_64b;
	__le64 frames_tx_65b_127b;
	__le64 frames_tx_128b_255b;
	__le64 frames_tx_256b_511b;
	__le64 frames_tx_512b_1023b;
	__le64 frames_tx_1024b_1518b;
	__le64 frames_tx_1519b_2047b;
	__le64 frames_tx_2048b_4095b;
	__le64 frames_tx_4096b_8191b;
	__le64 frames_tx_8192b_9215b;
	__le64 frames_tx_other;
	__le64 frames_tx_pri_0;
	__le64 frames_tx_pri_1;
	__le64 frames_tx_pri_2;
	__le64 frames_tx_pri_3;
	__le64 frames_tx_pri_4;
	__le64 frames_tx_pri_5;
	__le64 frames_tx_pri_6;
	__le64 frames_tx_pri_7;
	__le64 frames_rx_pri_0;
	__le64 frames_rx_pri_1;
	__le64 frames_rx_pri_2;
	__le64 frames_rx_pri_3;
	__le64 frames_rx_pri_4;
	__le64 frames_rx_pri_5;
	__le64 frames_rx_pri_6;
	__le64 frames_rx_pri_7;
	__le64 tx_pripause_0_1us_count;
	__le64 tx_pripause_1_1us_count;
	__le64 tx_pripause_2_1us_count;
	__le64 tx_pripause_3_1us_count;
	__le64 tx_pripause_4_1us_count;
	__le64 tx_pripause_5_1us_count;
	__le64 tx_pripause_6_1us_count;
	__le64 tx_pripause_7_1us_count;
	__le64 rx_pripause_0_1us_count;
	__le64 rx_pripause_1_1us_count;
	__le64 rx_pripause_2_1us_count;
	__le64 rx_pripause_3_1us_count;
	__le64 rx_pripause_4_1us_count;
	__le64 rx_pripause_5_1us_count;
	__le64 rx_pripause_6_1us_count;
	__le64 rx_pripause_7_1us_count;
	__le64 rx_pause_1us_count;
	__le64 frames_tx_truncated;
	uint8_t rsvd[312];
};

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
    sdk::platform::utils::mpartition* mempartition;
    transport *xport;

    std::shared_ptr<IpcService> ipc;
    //void (*cmd_hndlr[MAX_NCSI_CMDS])(const void* cmd_pkt, ssize_t cmd_sz);
    CmdHndlrFunc CmdTable[MAX_NCSI_CMDS];
    static NcsiParamDb* NcsiDb[NCSI_CAP_CHANNEL_COUNT];
    static uint64_t mac_addr_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_MIXED_MAC_FILTER_COUNT];
    static uint16_t vlan_filter_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_VLAN_FILTER_COUNT];
    static uint8_t vlan_mode_list[NCSI_CAP_CHANNEL_COUNT];
    static StateMachine *StateM[NCSI_CAP_CHANNEL_COUNT];

    void GetMacStats(uint32_t port, struct GetNicStatsRespPkt& resp);
    void ReadMacStats(uint32_t port, struct port_stats& stats);

    int SendNcsiCmdResponse(const void *buf, ssize_t sz);

    int ValidateCmdPkt(const void *pkt, ssize_t sz);
    int ConfigVlanFilter(uint8_t filter_idx, uint16_t vlan, uint32_t port, bool enable);
    int ConfigVlanMode(uint8_t vlan_mode, uint32_t port, bool enable);
    int ConfigMacFilter(uint8_t filter_idx, const uint8_t* mac_addr, uint32_t port, uint8_t mac_addr_type,
            bool enable);
    ssize_t EnableChannelRx(uint8_t ncsi_chan, bool enable);
    ssize_t EnableChannelTx(uint8_t ncsi_chan, bool enable);
    ssize_t DisableFilters(uint8_t ncsi_chan);
    ssize_t EnableFilters(uint8_t ncsi_chan);

    // NCSI command handlers
    static void ClearInitState(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void SelectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void DeselectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void ResetChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
    static void EnableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz);
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

