/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include <cstdio>
#include <cstring>
#include <arpa/inet.h>

#include "cmd_hndlr.h"
#include "lib/logger/logger.hpp"
#include "platform/fru/fru.hpp"
#include "lib/pal/pal.hpp"

//FIXME: Bharat needs to fix this smac hack
#define NCSI_CMD_BEGIN_BANNER() \
{\
    memset(resp.rsp.NcsiHdr.eth_hdr.h_source, 0x2, sizeof(resp.rsp.NcsiHdr.eth_hdr.h_source)); \
    SDK_TRACE_INFO("-------------- NCSI Cmd --------------"); \
    SDK_TRACE_INFO("cmd: %s", __FUNCTION__); \
}

#define NCSI_CMD_END_BANNER() \
{ \
    SDK_TRACE_INFO("cmd: %s, response code: 0x%x, reason code: 0x%x, response status: %d", \
            __FUNCTION__, ntohs(resp.rsp.code), ntohs(resp.rsp.reason), ret); \
    SDK_TRACE_INFO("-------------- NCSI Cmd End --------------"); \
}

namespace sdk {
namespace platform {
namespace ncsi {

uint64_t CmdHndler::mac_addr_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_MIXED_MAC_FILTER_COUNT];
uint16_t CmdHndler::vlan_filter_list[NCSI_CAP_CHANNEL_COUNT][NCSI_CAP_VLAN_FILTER_COUNT];
uint8_t CmdHndler::vlan_mode_list[NCSI_CAP_CHANNEL_COUNT];

StateMachine* CmdHndler::StateM[NCSI_CAP_CHANNEL_COUNT];
NcsiParamDb* CmdHndler::NcsiDb[NCSI_CAP_CHANNEL_COUNT];

struct GetCapRespPkt get_cap_resp;
struct GetVersionIdRespPkt get_version_resp;

std::string fw_ver_file = "/nic/etc/VERSION.json";
ptree prop_tree;

static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;
    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}


void populate_fw_name_ver()
{
    std::string fw_git_sha;
    std::string delimiter = ".";
    uint8_t ver_id[4] = {0,};
    uint32_t idx = 0;
    size_t pos = 0;
    std::string token;


    if (access(fw_ver_file.c_str(), R_OK) < 0) {
        SDK_TRACE_ERR("fw version file %s has no read permissions",
                fw_ver_file.c_str());
    }
    else
    {
        boost::property_tree::read_json(fw_ver_file, prop_tree);
        fw_git_sha = prop_tree.get<std::string>("sw.sha", "");
        strncpy((char*)get_version_resp.fw_name, fw_git_sha.c_str(),
                sizeof(get_version_resp.fw_name));
        //    get_version_resp.fw_version
        std::string version = prop_tree.get<std::string>("sw.version", "");
        std::string s = version.substr(0, version.find("-"));
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            std::cout << token << std::endl;
            ver_id[idx] = std::stoi(token);
            idx++;
            s.erase(0, pos + delimiter.length());
        }

        std::cout << s << std::endl;
        ver_id[idx] = std::stoi(s);

        get_version_resp.fw_version = (ver_id[0] | (ver_id[1] << 8) |
                (ver_id[2] << 16) | (ver_id[3] << 24));
    }
}

#define PORT_MAC_STAT_REPORT_SIZE   (1024)
#define CAPRI_HBM_REG_PORT_STATS    "port_stats"

void CmdHndler::ReadMacStats(uint32_t port, struct port_stats& stats)
{
    uint64_t port_stats_base = 0;

    if (port == 0x11010001) //first uplink
        port_stats_base = mempartition->start_addr(CAPRI_HBM_REG_PORT_STATS);

    else if (port == 0x11020001) //second uplink
        port_stats_base = mempartition->start_addr(CAPRI_HBM_REG_PORT_STATS) +
            PORT_MAC_STAT_REPORT_SIZE;
    else if (port == 0x11030001) //BX port
        port_stats_base = mempartition->start_addr(CAPRI_HBM_REG_PORT_STATS) +
            (2 * PORT_MAC_STAT_REPORT_SIZE);

    if (port_stats_base)
        sdk::lib::pal_mem_read(port_stats_base, (uint8_t *)&stats,
                sizeof(struct port_stats));
    else
        SDK_TRACE_ERR("ReadMacStats: Invalid port number: %d\n", port);

    SDK_TRACE_DEBUG("Stats for port: %d\n", port);
    SDK_TRACE_DEBUG("======================================");
    SDK_TRACE_DEBUG(" frames_rx_ok                :%ld",      stats.frames_rx_ok              );
    SDK_TRACE_DEBUG(" frames_rx_all               :%ld",      stats.frames_rx_all             );
    SDK_TRACE_DEBUG(" frames_rx_bad_fcs           :%ld",      stats.frames_rx_bad_fcs         );
    SDK_TRACE_DEBUG(" frames_rx_bad_all           :%ld",      stats.frames_rx_bad_all         );
    SDK_TRACE_DEBUG(" octets_rx_ok                :%ld",      stats.octets_rx_ok              );
    SDK_TRACE_DEBUG(" octets_rx_all               :%ld",      stats.octets_rx_all             );
    SDK_TRACE_DEBUG(" frames_rx_unicast           :%ld",      stats.frames_rx_unicast         );
    SDK_TRACE_DEBUG(" frames_rx_multicast         :%ld",      stats.frames_rx_multicast       );
    SDK_TRACE_DEBUG(" frames_rx_broadcast         :%ld",      stats.frames_rx_broadcast       );
    SDK_TRACE_DEBUG(" frames_rx_pause             :%ld",      stats.frames_rx_pause           );
    SDK_TRACE_DEBUG(" frames_rx_bad_length        :%ld",      stats.frames_rx_bad_length      );
    SDK_TRACE_DEBUG(" frames_rx_undersized        :%ld",      stats.frames_rx_undersized      );
    SDK_TRACE_DEBUG(" frames_rx_oversized         :%ld",      stats.frames_rx_oversized       );
    SDK_TRACE_DEBUG(" frames_rx_fragments         :%ld",      stats.frames_rx_fragments       );
    SDK_TRACE_DEBUG(" frames_rx_jabber            :%ld",      stats.frames_rx_jabber          );
    SDK_TRACE_DEBUG(" frames_rx_pripause          :%ld",      stats.frames_rx_pripause        );
    SDK_TRACE_DEBUG(" frames_rx_stomped_crc       :%ld",      stats.frames_rx_stomped_crc     );
    SDK_TRACE_DEBUG(" frames_rx_too_long          :%ld",      stats.frames_rx_too_long        );
    SDK_TRACE_DEBUG(" frames_rx_vlan_good         :%ld",      stats.frames_rx_vlan_good       );
    SDK_TRACE_DEBUG(" frames_rx_dropped           :%ld",      stats.frames_rx_dropped         );
    SDK_TRACE_DEBUG(" frames_rx_less_than_64b     :%ld",      stats.frames_rx_less_than_64b   );
    SDK_TRACE_DEBUG(" frames_rx_64b               :%ld",      stats.frames_rx_64b             );
    SDK_TRACE_DEBUG(" frames_rx_65b_127b          :%ld",      stats.frames_rx_65b_127b        );
    SDK_TRACE_DEBUG(" frames_rx_128b_255b         :%ld",      stats.frames_rx_128b_255b       );
    SDK_TRACE_DEBUG(" frames_rx_256b_511b         :%ld",      stats.frames_rx_256b_511b       );
    SDK_TRACE_DEBUG(" frames_rx_512b_1023b        :%ld",      stats.frames_rx_512b_1023b      );
    SDK_TRACE_DEBUG(" frames_rx_1024b_1518b       :%ld",      stats.frames_rx_1024b_1518b     );
    SDK_TRACE_DEBUG(" frames_rx_1519b_2047b       :%ld",      stats.frames_rx_1519b_2047b     );
    SDK_TRACE_DEBUG(" frames_rx_2048b_4095b       :%ld",      stats.frames_rx_2048b_4095b     );
    SDK_TRACE_DEBUG(" frames_rx_4096b_8191b       :%ld",      stats.frames_rx_4096b_8191b     );
    SDK_TRACE_DEBUG(" frames_rx_8192b_9215b       :%ld",      stats.frames_rx_8192b_9215b     );
    SDK_TRACE_DEBUG(" frames_rx_other             :%ld",      stats.frames_rx_other           );
    SDK_TRACE_DEBUG(" frames_tx_ok                :%ld",      stats.frames_tx_ok              );
    SDK_TRACE_DEBUG(" frames_tx_all               :%ld",      stats.frames_tx_all             );
    SDK_TRACE_DEBUG(" frames_tx_bad               :%ld",      stats.frames_tx_bad             );
    SDK_TRACE_DEBUG(" octets_tx_ok                :%ld",      stats.octets_tx_ok              );
    SDK_TRACE_DEBUG(" octets_tx_total             :%ld",      stats.octets_tx_total           );
    SDK_TRACE_DEBUG(" frames_tx_unicast           :%ld",      stats.frames_tx_unicast         );
    SDK_TRACE_DEBUG(" frames_tx_multicast         :%ld",      stats.frames_tx_multicast       );
    SDK_TRACE_DEBUG(" frames_tx_broadcast         :%ld",      stats.frames_tx_broadcast       );
    SDK_TRACE_DEBUG(" frames_tx_pause             :%ld",      stats.frames_tx_pause           );
    SDK_TRACE_DEBUG(" frames_tx_pripause          :%ld",      stats.frames_tx_pripause        );
    SDK_TRACE_DEBUG(" frames_tx_vlan              :%ld",      stats.frames_tx_vlan            );
    SDK_TRACE_DEBUG(" frames_tx_less_than_64b     :%ld",      stats.frames_tx_less_than_64b   );
    SDK_TRACE_DEBUG(" frames_tx_64b               :%ld",      stats.frames_tx_64b             );
    SDK_TRACE_DEBUG(" frames_tx_65b_127b          :%ld",      stats.frames_tx_65b_127b        );
    SDK_TRACE_DEBUG(" frames_tx_128b_255b         :%ld",      stats.frames_tx_128b_255b       );
    SDK_TRACE_DEBUG(" frames_tx_256b_511b         :%ld",      stats.frames_tx_256b_511b       );
    SDK_TRACE_DEBUG(" frames_tx_512b_1023b        :%ld",      stats.frames_tx_512b_1023b      );
    SDK_TRACE_DEBUG(" frames_tx_1024b_1518b       :%ld",      stats.frames_tx_1024b_1518b     );
    SDK_TRACE_DEBUG(" frames_tx_1519b_2047b       :%ld",      stats.frames_tx_1519b_2047b     );
    SDK_TRACE_DEBUG(" frames_tx_2048b_4095b       :%ld",      stats.frames_tx_2048b_4095b     );
    SDK_TRACE_DEBUG(" frames_tx_4096b_8191b       :%ld",      stats.frames_tx_4096b_8191b     );
    SDK_TRACE_DEBUG(" frames_tx_8192b_9215b       :%ld",      stats.frames_tx_8192b_9215b     );
    SDK_TRACE_DEBUG(" frames_tx_other             :%ld",      stats.frames_tx_other           );
    SDK_TRACE_DEBUG(" frames_tx_pri_0             :%ld",      stats.frames_tx_pri_0           );
    SDK_TRACE_DEBUG(" frames_tx_pri_1             :%ld",      stats.frames_tx_pri_1           );
    SDK_TRACE_DEBUG(" frames_tx_pri_2             :%ld",      stats.frames_tx_pri_2           );
    SDK_TRACE_DEBUG(" frames_tx_pri_3             :%ld",      stats.frames_tx_pri_3           );
    SDK_TRACE_DEBUG(" frames_tx_pri_4             :%ld",      stats.frames_tx_pri_4           );
    SDK_TRACE_DEBUG(" frames_tx_pri_5             :%ld",      stats.frames_tx_pri_5           );
    SDK_TRACE_DEBUG(" frames_tx_pri_6             :%ld",      stats.frames_tx_pri_6           );
    SDK_TRACE_DEBUG(" frames_tx_pri_7             :%ld",      stats.frames_tx_pri_7           );
    SDK_TRACE_DEBUG(" frames_rx_pri_0             :%ld",      stats.frames_rx_pri_0           );
    SDK_TRACE_DEBUG(" frames_rx_pri_1             :%ld",      stats.frames_rx_pri_1           );
    SDK_TRACE_DEBUG(" frames_rx_pri_2             :%ld",      stats.frames_rx_pri_2           );
    SDK_TRACE_DEBUG(" frames_rx_pri_3             :%ld",      stats.frames_rx_pri_3           );
    SDK_TRACE_DEBUG(" frames_rx_pri_4             :%ld",      stats.frames_rx_pri_4           );
    SDK_TRACE_DEBUG(" frames_rx_pri_5             :%ld",      stats.frames_rx_pri_5           );
    SDK_TRACE_DEBUG(" frames_rx_pri_6             :%ld",      stats.frames_rx_pri_6           );
    SDK_TRACE_DEBUG(" frames_rx_pri_7             :%ld",      stats.frames_rx_pri_7           );
    SDK_TRACE_DEBUG(" tx_pripause_0_1us_count     :%ld",      stats.tx_pripause_0_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_1_1us_count     :%ld",      stats.tx_pripause_1_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_2_1us_count     :%ld",      stats.tx_pripause_2_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_3_1us_count     :%ld",      stats.tx_pripause_3_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_4_1us_count     :%ld",      stats.tx_pripause_4_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_5_1us_count     :%ld",      stats.tx_pripause_5_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_6_1us_count     :%ld",      stats.tx_pripause_6_1us_count   );
    SDK_TRACE_DEBUG(" tx_pripause_7_1us_count     :%ld",      stats.tx_pripause_7_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_0_1us_count     :%ld",      stats.rx_pripause_0_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_1_1us_count     :%ld",      stats.rx_pripause_1_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_2_1us_count     :%ld",      stats.rx_pripause_2_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_3_1us_count     :%ld",      stats.rx_pripause_3_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_4_1us_count     :%ld",      stats.rx_pripause_4_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_5_1us_count     :%ld",      stats.rx_pripause_5_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_6_1us_count     :%ld",      stats.rx_pripause_6_1us_count   );
    SDK_TRACE_DEBUG(" rx_pripause_7_1us_count     :%ld",      stats.rx_pripause_7_1us_count   );
    SDK_TRACE_DEBUG(" rx_pause_1us_count          :%ld",      stats.rx_pause_1us_count        );
    SDK_TRACE_DEBUG(" frames_tx_truncated         :%ld",      stats.frames_tx_truncated       );

    SDK_TRACE_DEBUG("======================================");

}

void CmdHndler::GetMacStats(uint32_t port, struct GetNicStatsRespPkt& resp)
{
    struct port_stats p_stats = {0,};

    if (port == 0)
        port = 0x11010001; //first uplink
    else if (port == 1)
        port = 0x11020001; //second uplink
    else {
        SDK_TRACE_ERR("Invalid port number: %d. Skipping reading mac stats",
                port);
        return;
    }

    ReadMacStats(port, p_stats);

    memcpy(&resp.rx_bytes, memrev((uint8_t*)&p_stats.octets_rx_all, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.tx_bytes, memrev((uint8_t*)&p_stats.octets_tx_total, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.rx_uc_pkts, memrev((uint8_t*)&p_stats.frames_rx_unicast, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.rx_mc_pkts, memrev((uint8_t*)&p_stats.frames_rx_multicast, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.rx_bc_pkts, memrev((uint8_t*)&p_stats.frames_rx_broadcast, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.tx_uc_pkts, memrev((uint8_t*)&p_stats.frames_tx_unicast, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.tx_mc_pkts, memrev((uint8_t*)&p_stats.frames_tx_multicast, sizeof(uint64_t)), sizeof(uint64_t));
    memcpy(&resp.tx_bc_pkts, memrev((uint8_t*)&p_stats.frames_tx_broadcast, sizeof(uint64_t)), sizeof(uint64_t));
    resp.fcs_err = htonl(p_stats.frames_rx_bad_fcs);
    //resp.align_err = p_stats.
    //resp.false_carrier = p_stats.
    resp.runt_pkts = htonl(p_stats.frames_rx_undersized);
    resp.jabber_pkts = htonl(p_stats.frames_rx_jabber);
    //resp.rx_pause_xon = ;
    resp.rx_pause_xoff = htonl(p_stats.frames_rx_pause);
    //resp.tx_pause_xon = p_stats.
    resp.tx_pause_xoff = htonl(p_stats.frames_tx_pause);
    //resp.tx_s_collision = p_stats.
    //resp.tx_m_collision = p_stats.
    //resp.l_collision = p_stats.
    //resp.e_collision = p_stats.
    //resp.rx_ctl_frames = p_stats.
    resp.rx_64_frames = htonl(p_stats.frames_rx_64b);
    resp.rx_127_frames = htonl(p_stats.frames_rx_65b_127b);
    resp.rx_255_frames = htonl(p_stats.frames_rx_128b_255b);
    resp.rx_511_frames = htonl(p_stats.frames_rx_512b_1023b);
    resp.rx_1023_frames = htonl(p_stats.frames_rx_1024b_1518b);
    resp.rx_1522_frames = htonl(p_stats.frames_rx_1519b_2047b);
    resp.rx_9022_frames = htonl(p_stats.frames_rx_1519b_2047b + p_stats.frames_rx_2048b_4095b + p_stats.frames_rx_4096b_8191b +p_stats.frames_rx_8192b_9215b);
    resp.tx_64_frames = htonl(p_stats.frames_tx_64b);
    resp.tx_127_frames = htonl(p_stats.frames_tx_65b_127b);
    resp.tx_255_frames = htonl(p_stats.frames_tx_128b_255b);
    resp.tx_511_frames = htonl(p_stats.frames_tx_256b_511b);
    resp.tx_1023_frames = htonl(p_stats.frames_tx_512b_1023b);
    resp.tx_1522_frames = htonl(p_stats.frames_tx_1024b_1518b);
    resp.tx_9022_frames = htonl(p_stats.frames_tx_1519b_2047b + p_stats.frames_tx_2048b_4095b + p_stats.frames_tx_4096b_8191b + p_stats.frames_tx_8192b_9215b);
    memcpy(&resp.rx_valid_bytes, memrev((uint8_t*)&p_stats.octets_rx_ok, sizeof(uint64_t)), sizeof(uint64_t));
    //resp.rx_runt_pkts = p_stats.
    ////resp.rx_jabber_pkts = p_stats.

    SDK_TRACE_DEBUG("Response message: ");
    SDK_TRACE_DEBUG("cnt_hi                :%ld", resp.cnt_hi            );
    SDK_TRACE_DEBUG("cnt_lo                :%ld", resp.cnt_lo            );
    SDK_TRACE_DEBUG("rx_bytes              :%ld", resp.rx_bytes          );
    SDK_TRACE_DEBUG("tx_bytes              :%ld", resp.tx_bytes          );
    SDK_TRACE_DEBUG("rx_uc_pkts            :%ld", resp.rx_uc_pkts        );
    SDK_TRACE_DEBUG("rx_mc_pkts            :%ld", resp.rx_mc_pkts        );
    SDK_TRACE_DEBUG("rx_bc_pkts            :%ld", resp.rx_bc_pkts        );
    SDK_TRACE_DEBUG("tx_uc_pkts            :%ld", resp.tx_uc_pkts        );
    SDK_TRACE_DEBUG("tx_mc_pkts            :%ld", resp.tx_mc_pkts        );
    SDK_TRACE_DEBUG("tx_bc_pkts            :%ld", resp.tx_bc_pkts        );
    SDK_TRACE_DEBUG("fcs_err               :%ld", resp.fcs_err           );
    SDK_TRACE_DEBUG("align_err             :%ld", resp.align_err         );
    SDK_TRACE_DEBUG("false_carrier         :%ld", resp.false_carrier     );
    SDK_TRACE_DEBUG("runt_pkts             :%ld", resp.runt_pkts         );
    SDK_TRACE_DEBUG("jabber_pkts           :%ld", resp.jabber_pkts       );
    SDK_TRACE_DEBUG("rx_pause_xon          :%ld", resp.rx_pause_xon      );
    SDK_TRACE_DEBUG("rx_pause_xoff         :%ld", resp.rx_pause_xoff     );
    SDK_TRACE_DEBUG("tx_pause_xon          :%ld", resp.tx_pause_xon      );
    SDK_TRACE_DEBUG("tx_pause_xoff         :%ld", resp.tx_pause_xoff     );
    SDK_TRACE_DEBUG("tx_s_collision        :%ld", resp.tx_s_collision    );
    SDK_TRACE_DEBUG("tx_m_collision        :%ld", resp.tx_m_collision    );
    SDK_TRACE_DEBUG("l_collision           :%ld", resp.l_collision       );
    SDK_TRACE_DEBUG("e_collision           :%ld", resp.e_collision       );
    SDK_TRACE_DEBUG("rx_ctl_frames         :%ld", resp.rx_ctl_frames     );
    SDK_TRACE_DEBUG("rx_64_frames          :%ld", resp.rx_64_frames      );
    SDK_TRACE_DEBUG("rx_127_frames         :%ld", resp.rx_127_frames     );
    SDK_TRACE_DEBUG("rx_255_frames         :%ld", resp.rx_255_frames     );
    SDK_TRACE_DEBUG("rx_511_frames         :%ld", resp.rx_511_frames     );
    SDK_TRACE_DEBUG("rx_1023_frames        :%ld", resp.rx_1023_frames    );
    SDK_TRACE_DEBUG("rx_1522_frames        :%ld", resp.rx_1522_frames    );
    SDK_TRACE_DEBUG("rx_9022_frames        :%ld", resp.rx_9022_frames    );
    SDK_TRACE_DEBUG("tx_64_frames          :%ld", resp.tx_64_frames      );
    SDK_TRACE_DEBUG("tx_127_frames         :%ld", resp.tx_127_frames     );
    SDK_TRACE_DEBUG("tx_255_frames         :%ld", resp.tx_255_frames     );
    SDK_TRACE_DEBUG("tx_511_frames         :%ld", resp.tx_511_frames     );
    SDK_TRACE_DEBUG("tx_1023_frames        :%ld", resp.tx_1023_frames    );
    SDK_TRACE_DEBUG("tx_1522_frames        :%ld", resp.tx_1522_frames    );
    SDK_TRACE_DEBUG("tx_9022_frames        :%ld", resp.tx_9022_frames    );
    SDK_TRACE_DEBUG("rx_valid_bytes        :%ld", resp.rx_valid_bytes    );
    SDK_TRACE_DEBUG("rx_runt_pkts          :%ld", resp.rx_runt_pkts      );
    SDK_TRACE_DEBUG("rx_jabber_pkts        :%ld", resp.rx_jabber_pkts    );


}

CmdHndler::CmdHndler(std::shared_ptr<IpcService> IpcObj, transport *XportObj) {
    memset(&stats, 0, sizeof(struct NcsiStats));
    memset(CmdTable, 0, sizeof(CmdTable));
    memset(mac_addr_list, 0, sizeof(mac_addr_list));
    memset(vlan_filter_list, 0, sizeof(vlan_filter_list));
    memset(vlan_mode_list, 0, sizeof(vlan_mode_list));
    //memset(NcsiDb, 0, sizeof(NcsiDb));

    for (uint8_t ncsi_channel = 0; ncsi_channel < NCSI_CAP_CHANNEL_COUNT;
            ncsi_channel++) {
        StateM[ncsi_channel] = new StateMachine();
        NcsiDb[ncsi_channel] = new NcsiParamDb();
    }

    ipc = IpcObj;
    xport = XportObj;

    CmdTable[CMD_CLEAR_INIT_STATE]        = ClearInitState;
    CmdTable[CMD_SELECT_PACKAGE]          = SelectPackage;
    CmdTable[CMD_DESELECT_PACKAGE]        = DeselectPackage;
    CmdTable[CMD_EN_CHAN]                 = EnableChan;
    CmdTable[CMD_DIS_CHAN]                = EnableChan;
    CmdTable[CMD_RESET_CHAN]              = ResetChan;
    CmdTable[CMD_EN_CHAN_NW_TX]           = EnableChanNwTx;
    CmdTable[CMD_DIS_CHAN_NW_TX]          = EnableChanNwTx;
    CmdTable[CMD_SET_LINK]                = SetLink;
    CmdTable[CMD_GET_LINK_STATUS]         = GetLinkStatus;
    CmdTable[CMD_SET_VLAN_FILTER]         = SetVlanFilter;
    CmdTable[CMD_EN_VLAN]                 = EnableVlan;
    CmdTable[CMD_DIS_VLAN]                = DisableVlan;
    CmdTable[CMD_SET_MAC_ADDR]            = SetMacAddr;
    CmdTable[CMD_EN_BCAST_FILTER]         = EnableBcastFilter;
    CmdTable[CMD_DIS_BCAST_FILTER]        = DisableBcastFilter;
    CmdTable[CMD_EN_GLOBAL_MCAST_FILTER]  = EnableGlobalMcastFilter;
    CmdTable[CMD_DIS_GLOBAL_MCAST_FILTER] = DisableGlobalMcastFilter;
    CmdTable[CMD_GET_VER_ID]              = GetVersionId;
    CmdTable[CMD_GET_CAP]                 = GetCapabilities;
    CmdTable[CMD_GET_PARAMS]              = GetParams;
    CmdTable[CMD_GET_NIC_STATS]           = GetNicPktStats;
    CmdTable[CMD_GET_NCSI_STATS]          = GetNcsiStats;
    CmdTable[CMD_GET_NCSI_PASSTHRU_STATS] = GetNcsiPassthruStats;
    CmdTable[CMD_GET_PACKAGE_STATUS]      = GetPackageStatus;
    CmdTable[CMD_GET_PACKAGE_UUID]        = GetPackageUUID;

    get_cap_resp.cap = htonl(NCSI_CAP_HW_ARB | NCSI_CAP_HOST_NC_DRV_STATUS | NCSI_CAP_NC_TO_MC_FLOW_CTRL | NCSI_CAP_MC_TO_NC_FLOW_CTRL | NCSI_CAP_ALL_MCAST_ADDR_SUPPORT | NCSI_CAP_HW_ARB_IMPL_STATUS);
    get_cap_resp.bc_cap = htonl(NCSI_CAP_BCAST_FILTER_ARP | NCSI_CAP_BCAST_FILTER_DHCP_CLIENT | NCSI_CAP_BCAST_FILTER_DHCP_SERVER | NCSI_CAP_BCAST_FILTER_NETBIOS);
    get_cap_resp.mc_cap = htonl(NCSI_CAP_MCAST_IPV6_NEIGH_ADV | NCSI_CAP_MCAST_IPV6_ROUTER_ADV | NCSI_CAP_MCAST_DHCPV6_RELAY | NCSI_CAP_MCAST_DHCPV6_MCAST_SERVER_TO_CLIENT | NCSI_CAP_MCAST_IPV6_MLD | NCSI_CAP_MCAST_IPV6_NEIGH_SOL);
    get_cap_resp.buf_cap = htonl(NCSI_CAP_BUFFERRING);
    get_cap_resp.aen_cap = htonl(NCSI_CAP_AEN_CTRL_LINK_STATUS_CHANGE | NCSI_CAP_AEN_CTRL_CONFIG_REQUIRED | NCSI_CAP_AEN_CTRL_HOST_NC_DRV_STATUS_CHANGE | NCSI_CAP_AEN_CTRL_OEM_SPECIFIC);
    get_cap_resp.vlan_cnt = NCSI_CAP_VLAN_FILTER_COUNT;
    get_cap_resp.mc_cnt = NCSI_CAP_MCAST_FILTER_COUNT;
    get_cap_resp.uc_cnt = NCSI_CAP_UCAST_FILTER_COUNT;
    get_cap_resp.mixed_cnt = NCSI_CAP_MIXED_MAC_FILTER_COUNT;
    get_cap_resp.vlan_mode = NCSI_CAP_VLAN_MODE_SUPPORT;
    get_cap_resp.channel_cnt = NCSI_CAP_CHANNEL_COUNT;

    //ncsi version 1.1.0 and alpha is 0
    get_version_resp.ncsi_version = htonl(0xF1F1FF00);
    get_version_resp.pci_ids[0] = htons(0x1dd8); //VID
    get_version_resp.pci_ids[1] = htons(0x1002); //DID
    get_version_resp.pci_ids[2] = htons(0xdead); //SUBVID
    get_version_resp.pci_ids[3] = htons(0xbeef); //SUBDID

    /* Pensando IANA enterprise ID as per:
     * https://www.iana.org/assignments/enterprise-numbers/enterprise-numbers */
    get_version_resp.mf_id = htonl(51886);

    //std::string hal_cfg_path = std::getenv("HAL_CONFIG_PATH");
    char* hal_cfg_path = std::getenv("HAL_CONFIG_PATH");

    //if (hal_cfg_path.empty())
    if (!hal_cfg_path)
        hal_cfg_path = "./";

//    SDK_TRACE_INFO("HAL_CONFIG_PATH: %s", hal_cfg_path.c_str());
    SDK_TRACE_INFO("HAL_CONFIG_PATH: %s", hal_cfg_path);

    sdk::lib::device *device = sdk::lib::device::factory("/sysconfig/config0/device.conf");
    std::string mpart_json = sdk::platform::utils::mpartition::get_mpart_file_path(hal_cfg_path, "iris", device->get_feature_profile());

    mempartition = sdk::platform::utils::mpartition::factory(mpart_json.c_str());

    //populate_fw_name_ver();

    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);

}

int CmdHndler::SendNcsiCmdResponse(const void *buf, ssize_t sz)
{
    ssize_t ret;

    ret = xport->SendPkt(buf, sz);

    if (ret < 0) {
        SDK_TRACE_ERR("%s: sending cmd response failed with error code: %d",
                __FUNCTION__, ret);
        return -1;
    }
    else {
        SDK_TRACE_DEBUG("%s: Response sent", __FUNCTION__);
        stats.tx_total_cnt++;
    }

    return 0;
}

int CmdHndler::ConfigVlanFilter(uint8_t filter_idx, uint16_t vlan,
        uint32_t port, bool enable)
{
    ssize_t ret;
    VlanFilterMsg vlan_msg;

    vlan_msg.filter_id = filter_idx;
    vlan_msg.port = port;
    vlan_msg.vlan_id = vlan;
    vlan_msg.enable = enable;

    NcsiDb[vlan_msg.port]->UpdateNcsiParam(vlan_msg);

    ret = this->ipc->PostMsg(vlan_msg);

    if (!ret)
        vlan_filter_list[port][filter_idx] = vlan;

    return ret;
}

int CmdHndler::ConfigMacFilter(uint8_t filter_idx, const uint8_t* mac_addr,
        uint32_t port, uint8_t type, bool enable)
{
    ssize_t ret;
    MacFilterMsg mac_filter_msg;

    mac_filter_msg.filter_id = filter_idx;
    mac_filter_msg.port = port;
    memcpy(mac_filter_msg.mac_addr, mac_addr, sizeof(mac_filter_msg.mac_addr));
    mac_filter_msg.addr_type = type;
    mac_filter_msg.enable = enable;

    NcsiDb[mac_filter_msg.port]->UpdateNcsiParam(mac_filter_msg);

    ret = this->ipc->PostMsg(mac_filter_msg);

    if (!ret)
        memcpy(&mac_addr_list[port][filter_idx], mac_addr,
                sizeof(mac_filter_msg.mac_addr));

    return ret;
}

int CmdHndler::ConfigVlanMode(uint8_t vlan_mode, uint32_t port, bool enable)
{
    ssize_t ret;
    VlanModeMsg vlan_mode_msg;

    vlan_mode_msg.filter_id = port;
    vlan_mode_msg.port = port;
    vlan_mode_msg.mode = vlan_mode;
    vlan_mode_msg.enable = enable;

    NcsiDb[vlan_mode_msg.port]->UpdateNcsiParam(vlan_mode_msg);

    ret = this->ipc->PostMsg(vlan_mode_msg);

    if (!ret)
        vlan_mode_list[port] = vlan_mode;

    return ret;
}
void CmdHndler::SetVlanFilter(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetVlanFilterCmdPkt *cmd = (SetVlanFilterCmdPkt *)cmd_pkt;
    uint16_t vlan_id = (ntohs(cmd->vlan) & 0xFFFF);
    uint8_t filter_idx = (cmd->index);

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();

    SDK_TRACE_INFO("ncsi_channel: filter_idx: 0x%x, channel: 0x%x vlan_id: 0x%x, "
            "enable: 0x%x ", filter_idx, cmd->cmd.NcsiHdr.channel, vlan_id,
            cmd->enable & 0x1);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_VLAN_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan validity e.g. vlan can't be 0
    if (vlan_id) {
        ret = hndlr->ConfigVlanFilter(filter_idx, vlan_id,
                cmd->cmd.NcsiHdr.channel, (cmd->enable & 0x1) ? true:false);
        if (ret) {
            SDK_TRACE_ERR("Failed to set vlan filter");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        }
    }
    else
    {
        SDK_TRACE_ERR("vlan_id: 0 is not valid");
        resp.rsp.reason = htons(NCSI_REASON_INVLD_VLAN);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_VLAN_FILTER);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::ClearInitState(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_CLEAR_INIT_STATE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with "
                    "current ncsi state: 0x%x", cmd,
                    StateM[cmd->cmd.NcsiHdr.channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Implement clear initial state command here

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_CLEAR_INIT_STATE);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::SelectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    uint8_t ncsi_channel = 0;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));
    resp.rsp.NcsiHdr.channel = 0;

    NCSI_CMD_BEGIN_BANNER();

    SDK_TRACE_INFO("ncsi_channel: 0x%x", ncsi_channel);

    sm_ret = StateM[ncsi_channel]->UpdateState(CMD_SELECT_PACKAGE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with "
                    "current ncsi state: 0x%x", cmd,
                    StateM[ncsi_channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //TODO: Enable the filters which were applied on channel

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SELECT_PACKAGE);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DeselectPackage(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DESELECT_PACKAGE);

    if (sm_ret) {
        if (sm_ret == INVALID) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INVALID_CMD_ERR);
            SDK_TRACE_ERR("cmd: %x failed as its invalid cmd with "
                    "current ncsi state: 0x%x", cmd,
                    StateM[cmd->cmd.NcsiHdr.channel]->GetCurState());

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    // disable both channel and disable chan tx for deselect package
    for (uint8_t ncsi_channel = 0; ncsi_channel< NCSI_CAP_CHANNEL_COUNT;
            ncsi_channel++) {
        hndlr->EnableChannelRx(ncsi_channel, false);
        hndlr->EnableChannelTx(ncsi_channel, false);
    }
error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DESELECT_PACKAGE);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

ssize_t CmdHndler::EnableFilters(uint8_t ncsi_chan)
{
    ssize_t ret;
    //struct EnableBcastFilterMsg bcast_filter_msg;
    //struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    struct VlanFilterMsg vlan_filter_msg;
    struct VlanModeMsg vlan_mode_msg;
    struct MacFilterMsg mac_filter_msg;

    //TODO: enable bcast and mcast filters as well

    // enable vlan mode
    memset(&vlan_mode_msg, 0, sizeof(vlan_mode_msg));
    vlan_mode_msg.filter_id = ncsi_chan;
    vlan_mode_msg.port = ncsi_chan;
    vlan_mode_msg.mode = vlan_mode_list[ncsi_chan];

    ret = ipc->PostMsg(vlan_mode_msg);
    if (ret) {
        SDK_TRACE_ERR("IPC Failed to disable vlan mode on %d channel",
                ncsi_chan);
        return ret;
    }

    // enable vlan filters
    for (uint8_t idx=0; idx < NCSI_CAP_VLAN_FILTER_COUNT; idx++) {
        if (vlan_filter_list[ncsi_chan][idx]) {
            vlan_filter_msg.filter_id = (ncsi_chan * idx);
            vlan_filter_msg.port = ncsi_chan;
            vlan_filter_msg.vlan_id = vlan_filter_list[ncsi_chan][idx];
            vlan_filter_msg.enable = true;

            ret = ipc->PostMsg(vlan_filter_msg);
            if (ret) {
                SDK_TRACE_ERR("IPC Failed to enable vlan filters on %d channel"
                        , ncsi_chan);
                return ret;
            }
        }
    }

    //enable mac filters
    for (uint8_t idx=0; idx < NCSI_CAP_MIXED_MAC_FILTER_COUNT; idx++) {
        if (mac_addr_list[ncsi_chan][idx]) {
            mac_filter_msg.filter_id = (ncsi_chan * idx);
            mac_filter_msg.port = ncsi_chan;
            memcpy(mac_filter_msg.mac_addr, &mac_addr_list[ncsi_chan][idx],
                    sizeof(mac_filter_msg.mac_addr));
            mac_filter_msg.enable = true;

            ret = ipc->PostMsg(mac_filter_msg);
            if (ret) {
                SDK_TRACE_ERR("IPC Failed to enable mac filters on %d channel",
                        ncsi_chan);
                return ret;
            }
        }
    }

    return ret;
}

ssize_t CmdHndler::DisableFilters(uint8_t ncsi_chan)
{
    ssize_t ret;
    struct EnableBcastFilterMsg bcast_filter_msg;
    struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    struct VlanFilterMsg vlan_filter_msg;
    struct VlanModeMsg vlan_mode_msg;
    struct MacFilterMsg mac_filter_msg;

    //disable bcast filters
    memset(&bcast_filter_msg, 0, sizeof(bcast_filter_msg));
    bcast_filter_msg.filter_id = ncsi_chan;
    bcast_filter_msg.port = ncsi_chan;

    ret = ipc->PostMsg(bcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("IPC Failed to disable bcast filters on %d channel",
                ncsi_chan);
        return ret;
    }

    //disable mcast filters
    memset(&mcast_filter_msg, 0, sizeof(mcast_filter_msg));
    mcast_filter_msg.filter_id = ncsi_chan;
    mcast_filter_msg.port = ncsi_chan;

    ret = ipc->PostMsg(mcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("IPC Failed to disable mcast filters on %d channel",
                ncsi_chan);
        return ret;
    }

    // disable vlan mode
    memset(&vlan_mode_msg, 0, sizeof(vlan_mode_msg));
    vlan_mode_msg.filter_id = ncsi_chan;
    vlan_mode_msg.port = ncsi_chan;
    vlan_mode_msg.mode = 0;

    ret = ipc->PostMsg(vlan_mode_msg);
    if (ret) {
        SDK_TRACE_ERR("IPC Failed to disable vlan mode on %d channel",
                ncsi_chan);
        return ret;
    }

    // disable vlan filters
    for (uint8_t idx=0; idx < NCSI_CAP_VLAN_FILTER_COUNT; idx++) {
        if (vlan_filter_list[ncsi_chan][idx]) {
            vlan_filter_msg.filter_id = (ncsi_chan * idx);
            vlan_filter_msg.port = ncsi_chan;
            vlan_filter_msg.vlan_id = vlan_filter_list[ncsi_chan][idx];
            vlan_filter_msg.enable = false;

            ret = ipc->PostMsg(vlan_filter_msg);
            if (ret) {
                SDK_TRACE_ERR("IPC Failed to disable vlan filters on %d channel"
                        , ncsi_chan);
                return ret;
            }
        }
    }

    //disable mac filters
    for (uint8_t idx=0; idx < NCSI_CAP_MIXED_MAC_FILTER_COUNT; idx++) {
        if (mac_addr_list[ncsi_chan][idx]) {
            mac_filter_msg.filter_id = (ncsi_chan * idx);
            mac_filter_msg.port = ncsi_chan;
            memcpy(mac_filter_msg.mac_addr, &mac_addr_list[ncsi_chan][idx],
                    sizeof(mac_filter_msg.mac_addr));
            mac_filter_msg.enable = false;

            ret = ipc->PostMsg(mac_filter_msg);
            if (ret) {
                SDK_TRACE_ERR("IPC Failed to disable mac filters on %d channel"
                        , ncsi_chan);
                return ret;
            }
        }
    }

    return ret;
}

ssize_t CmdHndler::EnableChannelRx(uint8_t ncsi_chan, bool enable)
{
    ssize_t ret;
    struct EnableChanMsg enable_ch_msg;

    if (enable)
        EnableFilters(ncsi_chan);
    else
        DisableFilters(ncsi_chan);

    enable_ch_msg.enable = enable;
    enable_ch_msg.port = ncsi_chan;
    enable_ch_msg.filter_id = ncsi_chan;

    NcsiDb[enable_ch_msg.port]->UpdateNcsiParam(enable_ch_msg);

    ret = ipc->PostMsg(enable_ch_msg);
    if (ret)
        SDK_TRACE_ERR("IPC Failed to enable/disable channel");

    return ret;
}

void CmdHndler::EnableChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
    const uint8_t *buf = (uint8_t *)cmd_pkt;
    uint8_t opcode = buf[NCSI_CMD_OPCODE_OFFSET];
    bool enable;

    if (opcode == CMD_EN_CHAN)
        enable = true;
    else if (CMD_DIS_CHAN)
        enable = false;
    else
        SDK_TRACE_ERR("Invalid opcode: 0x%x\n", opcode);

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x, enable: 0x%x",
            cmd->cmd.NcsiHdr.channel, enable);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(
            enable ? CMD_EN_CHAN : CMD_DIS_CHAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    if (hndlr->EnableChannelRx(cmd->cmd.NcsiHdr.channel, enable)) {
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        SDK_TRACE_ERR("cmd: %x failed due to internal err in ipc", cmd);
    }

error_out:
    if (enable)
        resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_CHAN);
    else
        resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_CHAN);

    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::ResetChan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct ResetChanMsg reset_ch_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct ResetChanCmdPkt *cmd = (ResetChanCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_RESET_CHAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    // first disable Rx and Tx for this channel
    hndlr->EnableChannelRx(cmd->cmd.NcsiHdr.channel, false);
    hndlr->EnableChannelTx(cmd->cmd.NcsiHdr.channel, false);

    // reset the channel
    reset_ch_msg.reset = true;
    reset_ch_msg.port = cmd->cmd.NcsiHdr.channel;
    reset_ch_msg.filter_id = cmd->cmd.NcsiHdr.channel;

    NcsiDb[reset_ch_msg.port]->UpdateNcsiParam(reset_ch_msg);

    ret = hndlr->ipc->PostMsg(reset_ch_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to reset channel");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

    // zero out the local database of filters
    if (!resp.rsp.code) {
        memset(mac_addr_list, 0, sizeof(mac_addr_list));
        memset(vlan_filter_list, 0, sizeof(vlan_filter_list));
        memset(vlan_mode_list, 0, sizeof(vlan_mode_list));
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_RESET_CHAN);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;

}

ssize_t CmdHndler::EnableChannelTx(uint8_t ncsi_chan, bool enable)
{
    ssize_t ret;
    struct EnableChanTxMsg enable_ch_tx_msg;

    enable_ch_tx_msg.enable = enable;
    enable_ch_tx_msg.port = ncsi_chan;
    enable_ch_tx_msg.filter_id = ncsi_chan;

    NcsiDb[enable_ch_tx_msg.port]->UpdateNcsiParam(enable_ch_tx_msg);

    ret = ipc->PostMsg(enable_ch_tx_msg);
    if (ret)
        SDK_TRACE_ERR("IPC Failed to enable/disable channel");

    return ret;
}
void CmdHndler::EnableChanNwTx(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
    const uint8_t *buf = (uint8_t *)cmd_pkt;
    uint8_t opcode = buf[NCSI_CMD_OPCODE_OFFSET];
    bool enable;

    if (opcode == CMD_EN_CHAN_NW_TX)
        enable = true;
    else if (CMD_DIS_CHAN_NW_TX)
        enable = false;
    else
        SDK_TRACE_ERR("Invalid opcode: 0x%x\n", opcode);

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x, enable: 0x%x",
            cmd->cmd.NcsiHdr.channel, enable);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(
            enable ? CMD_EN_CHAN_NW_TX : CMD_DIS_CHAN_NW_TX);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    if (hndlr->EnableChannelTx(cmd->cmd.NcsiHdr.channel, enable)) {
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        SDK_TRACE_ERR("cmd: %x failed due to internal err in ipc", cmd);
    }

error_out:
    if (enable)
        resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_CHAN_NW_TX);
    else
        resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_CHAN_NW_TX);

    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::SetLink(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct SetLinkMsg set_link_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetLinkCmdPkt *cmd = (SetLinkCmdPkt *)cmd_pkt;
    uint32_t oem_field_valid = (cmd->mode) & (1 << 11);
    uint8_t set_link = cmd->oem_mode & 0x1;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x oem_field_valid: 0x%x, set_link: 0x%x ",
            cmd->cmd.NcsiHdr.channel, oem_field_valid, set_link);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_LINK);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    if (oem_field_valid) {
        set_link_msg.link_up = set_link;
        set_link_msg.port = cmd->cmd.NcsiHdr.channel;
        set_link_msg.filter_id = cmd->cmd.NcsiHdr.channel;

        NcsiDb[set_link_msg.port]->UpdateNcsiParam(set_link_msg);

        ret = hndlr->ipc->PostMsg(set_link_msg);
        if (ret) {
            SDK_TRACE_ERR("Failed to set link");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
        }
    }
    else
    {
        SDK_TRACE_ERR("Only OEM specific link settings are allowed");
        resp.rsp.reason = htons(NCSI_REASON_INVALID_PARAM);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_LINK);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::GetLinkStatus(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    bool link_status;
    uint32_t status;
    uint8_t link_speed;
    NcsiStateErr sm_ret;
    struct GetLinkStatusRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    hndlr->ipc->GetLinkStatus(cmd->cmd.NcsiHdr.channel, link_status,
            link_speed);
    status = ((link_status ? 1:0) | (link_speed << 1) | (0x3 << 5)/* autoneg */
            | (1 << 20) /* serdes used */ );

    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    //HACK: keep the src mac address as 0x2 as of now. Need to fix in better way
    memset(resp.rsp.NcsiHdr.eth_hdr.h_source, 0x2, 
            sizeof(resp.rsp.NcsiHdr.eth_hdr.h_source));

    //NCSI_CMD_BEGIN_BANNER();
    resp.status = htonl(status);
    SDK_TRACE_INFO("ncsi_channel: 0x%x, link_status: 0x%x",
            cmd->cmd.NcsiHdr.channel, ntohl(resp.status));

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_LINK_STATUS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_LINK_STATUS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_LINK_STATUS_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    //NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::EnableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct EnVlanCmdPkt *cmd = (EnVlanCmdPkt *)cmd_pkt;
    uint32_t vlan_mode = cmd->mode;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x vlan_mode: 0x%x",
            cmd->cmd.NcsiHdr.channel, vlan_mode);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_VLAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan mode validity. we support only vlan mode 1 and 2
    if (vlan_mode > 0 && vlan_mode < 3) {
        ret = hndlr->ConfigVlanMode(vlan_mode, cmd->cmd.NcsiHdr.channel, true);
        if (ret) {
            SDK_TRACE_ERR("Failed to set vlan Mode");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);

            goto error_out;
        }
    }
    else
    {
        SDK_TRACE_ERR("vlan_mode: 0x%x is not supported", vlan_mode);
        resp.rsp.reason = htons(NCSI_REASON_INVALID_PARAM);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);

        goto error_out;
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_VLAN);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::DisableVlan(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_VLAN);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check vlan mode validity. we support only vlan mode 1 and 2
    ret = hndlr->ConfigVlanMode( /*don't care*/ 0, cmd->cmd.NcsiHdr.channel,
            false);
    if (ret) {
        SDK_TRACE_ERR("Failed to set vlan Mode");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_VLAN);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::SetMacAddr(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct SetMacAddrCmdPkt *cmd = (SetMacAddrCmdPkt *)cmd_pkt;
    uint8_t mac_addr_type = ((cmd->at_e & 0xE0) >> 5);
    uint64_t mac_addr = *((uint64_t *)cmd->mac);
    uint8_t filter_idx = cmd->index;
    bool enable = (cmd->at_e & 0x1) ? true:false;
    uint8_t mac_filter_num = cmd->index;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();

    SDK_TRACE_INFO("ncsi_channel: 0x%x, mac_addr: %x:%x:%x:%x:%x:%x, "
            "mac_addr_type: 0x%x, enable: 0x%x ", cmd->cmd.NcsiHdr.channel,
            cmd->mac[5], cmd->mac[4], cmd->mac[3], cmd->mac[2], cmd->mac[1],
            cmd->mac[0], mac_addr_type, enable);

    //valid mac address type are 0(Ucast) & 1(Mcast) only
    if (mac_addr_type > 1) {
        SDK_TRACE_ERR("mac_addr_type: 0x%x is not valid", mac_addr_type);
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

    if(mac_filter_num > 8) {
        SDK_TRACE_ERR("mac_filter_num: 0x%x is not out of range",
                mac_filter_num);
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_SET_MAC_ADDR);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //Check mac addr validity e.g. mac addr can't be 0
    if (mac_addr) {
#if 0
        if (mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num]) {

            ret = hndlr->ConfigMacFilter(mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num],
                    cmd->cmd.NcsiHdr.channel, mac_addr_type, false);

            if (ret) {
                SDK_TRACE_ERR("Failed to remove old mac filter");
                resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
                resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);

                goto error_out;
            }
        }
#endif
        ret = hndlr->ConfigMacFilter(filter_idx, cmd->mac, cmd->cmd.NcsiHdr.channel,
                mac_addr_type, enable);

        if (ret) {
            SDK_TRACE_ERR("Failed to set mac filter");
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);

            goto error_out;
        }
        else {
            mac_addr_list[cmd->cmd.NcsiHdr.channel][mac_filter_num] = mac_addr;
        }
    }
    else
    {
        SDK_TRACE_ERR("All zero mac_addr is not supported");
        resp.rsp.reason = htons(NCSI_REASON_INVLD_MAC_ADDR);
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_SET_MAC_ADDR);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();
    return;
}

void CmdHndler::EnableBcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    struct EnableBcastFilterMsg bcast_filter_msg;

    const struct EnBcastFilterCmdPkt *cmd = (EnBcastFilterCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_BCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    bcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.enable_arp = !!(ntohl(cmd->mode) & NCSI_CAP_BCAST_FILTER_ARP);
    bcast_filter_msg.enable_dhcp_client = !!(ntohl(cmd->mode) & NCSI_CAP_BCAST_FILTER_DHCP_CLIENT);
    bcast_filter_msg.enable_dhcp_server = !!(ntohl(cmd->mode) & NCSI_CAP_BCAST_FILTER_DHCP_SERVER);
    bcast_filter_msg.enable_netbios = !!(ntohl(cmd->mode) & NCSI_CAP_BCAST_FILTER_NETBIOS);

    NcsiDb[bcast_filter_msg.port]->UpdateNcsiParam(bcast_filter_msg);

    ret = hndlr->ipc->PostMsg(bcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to program bcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_BCAST_FILTER);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DisableBcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    EnableBcastFilterMsg bcast_filter_msg;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_BCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    bcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    bcast_filter_msg.enable_arp = false;
    bcast_filter_msg.enable_dhcp_client = false;
    bcast_filter_msg.enable_dhcp_server = false;
    bcast_filter_msg.enable_netbios = false;

    NcsiDb[bcast_filter_msg.port]->UpdateNcsiParam(bcast_filter_msg);

    ret = hndlr->ipc->PostMsg(bcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to disable bcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_BCAST_FILTER);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::EnableGlobalMcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    const struct EnGlobalMcastFilterCmdPkt *cmd =
        (EnGlobalMcastFilterCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_EN_GLOBAL_MCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    mcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.enable_ipv6_neigh_adv = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_IPV6_NEIGH_ADV);
    mcast_filter_msg.enable_ipv6_router_adv = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_IPV6_ROUTER_ADV);
    mcast_filter_msg.enable_dhcpv6_relay = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_DHCPV6_RELAY);
    mcast_filter_msg.enable_dhcpv6_mcast = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_DHCPV6_MCAST_SERVER_TO_CLIENT);
    mcast_filter_msg.enable_ipv6_mld = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_IPV6_MLD);
    mcast_filter_msg.enable_ipv6_neigh_sol = !!(ntohl(cmd->mode) & NCSI_CAP_MCAST_IPV6_NEIGH_SOL);

    NcsiDb[mcast_filter_msg.port]->UpdateNcsiParam(mcast_filter_msg);

    ret = hndlr->ipc->PostMsg(mcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to program mcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_EN_GLOBAL_MCAST_FILTER);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::DisableGlobalMcastFilter(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct NcsiRspPkt resp;
    struct EnableGlobalMcastFilterMsg mcast_filter_msg;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_DIS_GLOBAL_MCAST_FILTER);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    mcast_filter_msg.filter_id = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.port = cmd->cmd.NcsiHdr.channel;
    mcast_filter_msg.enable_ipv6_neigh_adv = false;
    mcast_filter_msg.enable_ipv6_router_adv = false;
    mcast_filter_msg.enable_dhcpv6_relay = false;
    mcast_filter_msg.enable_dhcpv6_mcast = false;
    mcast_filter_msg.enable_ipv6_mld = false;
    mcast_filter_msg.enable_ipv6_neigh_sol = false;

    NcsiDb[mcast_filter_msg.port]->UpdateNcsiParam(mcast_filter_msg);

    ret = hndlr->ipc->PostMsg(mcast_filter_msg);
    if (ret) {
        SDK_TRACE_ERR("Failed to dsiable mcast filters");
        resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
        resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_DIS_GLOBAL_MCAST_FILTER);
    resp.rsp.NcsiHdr.length = htons(NCSI_FIXED_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetVersionId(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetVersionIdRespPkt& resp = get_version_resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    //memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now we are ignoring the ncsi state machine for this cmd

    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_VER_ID);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_VER_ID_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetCapabilities(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetCapRespPkt resp = get_cap_resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memcpy(&resp, &get_cap_resp, sizeof(get_cap_resp));
    memset(&resp.rsp, 0, sizeof(resp.rsp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_CAP);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_CAP);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_CAP_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetParams(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetParamRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
    NcsiDb[cmd->cmd.NcsiHdr.channel]->GetNcsiParamRespPacket(resp);

    memset(&resp.rsp, 0, sizeof(resp.rsp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_PARAMS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PARAMS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_PARAM_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNicPktStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetNicStatsRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NIC_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    hndlr->GetMacStats(cmd->cmd.NcsiHdr.channel, resp);

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NIC_STATS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_NIC_STATS_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNcsiStats(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetNCSIStatsRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NCSI_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    resp.rx_cmds = htonl(hndlr->stats.valid_cmd_rx_cnt);
    resp.dropped_cmds = htonl(hndlr->stats.rx_drop_cnt);
    resp.cmd_type_errs = htonl(hndlr->stats.unsup_cmd_rx_cnt);
    resp.cmd_csum_errs = htonl(hndlr->stats.invalid_chksum_rx_cnt);
    resp.rx_pkts = htonl(hndlr->stats.rx_total_cnt);

    /* include this response as part os tx stats counter*/
    resp.tx_pkts = htonl(hndlr->stats.tx_total_cnt + 1);

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NCSI_STATS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_NCSI_STATS_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetNcsiPassthruStats(void *obj, const void *cmd_pkt,
		ssize_t cmd_sz)
{
    ssize_t ret;
    NcsiStateErr sm_ret;
    struct GetPassThruStatsRespPkt resp;
    struct port_stats p_stats;
    struct mgmt_port_stats *mgmt_p_stats = (struct mgmt_port_stats *) &p_stats;
    uint64_t passthru_tx_pkts;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;
    uint32_t port = 0x11030001; //BX port in capri

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    sm_ret = StateM[cmd->cmd.NcsiHdr.channel]->UpdateState(CMD_GET_NCSI_PASSTHRU_STATS);

    if (sm_ret) {
        if (sm_ret == INIT_REQRD) {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTF_INIT_REQRD);
            SDK_TRACE_ERR("cmd: %x failed as channel is not initialized", cmd);

            goto error_out;
        }
        else {
            resp.rsp.code = htons(NCSI_RSP_COMMAND_FAILED);
            resp.rsp.reason = htons(NCSI_REASON_INTERNAL_ERR);
            SDK_TRACE_ERR("cmd: %x failed due to internal err in state machine",
                    cmd);

            goto error_out;
        }
    }

    //FIXME: Need to use some macro instead of hard coded 9 here for oob port
    hndlr->ReadMacStats(port, p_stats);

    passthru_tx_pkts = mgmt_p_stats->frames_tx_all - hndlr->stats.tx_total_cnt;
    memcpy(&resp.tx_pkts, memrev((uint8_t *) &passthru_tx_pkts, sizeof(uint64_t)), sizeof(uint64_t));
    resp.tx_dropped = htonl((uint32_t)mgmt_p_stats->frames_tx_bad);
    resp.tx_us_err = 0;
    resp.rx_pkts = htonl((uint32_t)mgmt_p_stats->frames_rx_all);
    resp.rx_dropped = htonl((uint32_t)mgmt_p_stats->frames_rx_bad_all);
    resp.rx_us_err = htonl((uint32_t)mgmt_p_stats->frames_rx_undersized);
    resp.rx_os_err = htonl((uint32_t)mgmt_p_stats->frames_rx_oversized);

    resp.tx_channel_err = 0; //FIXME
    resp.tx_os_err = 0; //FIXME
    resp.rx_channel_err = 0;//FIXME

    SDK_TRACE_DEBUG("Response for GetNcsiPassthruStats:");
    SDK_TRACE_DEBUG("tx_pkts (in BE format)         :%ld",resp.tx_pkts       );
    SDK_TRACE_DEBUG("tx_dropped      :%d",ntohl(resp.tx_dropped    ));
    SDK_TRACE_DEBUG("tx_channel_err  :%d",ntohl(resp.tx_channel_err));
    SDK_TRACE_DEBUG("tx_us_err       :%d",ntohl(resp.tx_us_err     ));
    SDK_TRACE_DEBUG("tx_os_err       :%d",ntohl(resp.tx_os_err     ));
    SDK_TRACE_DEBUG("rx_pkts         :%d",ntohl(resp.rx_pkts       ));
    SDK_TRACE_DEBUG("rx_dropped      :%d",ntohl(resp.rx_dropped    ));
    SDK_TRACE_DEBUG("rx_channel_err  :%d",ntohl(resp.rx_channel_err));
    SDK_TRACE_DEBUG("rx_us_err       :%d",ntohl(resp.rx_us_err     ));
    SDK_TRACE_DEBUG("rx_os_err       :%d",ntohl(resp.rx_os_err     ));

error_out:
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_NCSI_PASSTHRU_STATS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_PASSTHRU_STATS_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetPackageStatus(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetPkgStatusRespPkt resp;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now ignoring the state machine for this cmd

    /* send all 0s in response as we don't suppport HW arb in package */
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PACKAGE_STATUS);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_PKG_STATUS_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

void CmdHndler::GetPackageUUID(void *obj, const void *cmd_pkt, ssize_t cmd_sz)
{
    ssize_t ret;
    struct GetPkgUUIDRespPkt resp;
    std::string serial_num;
    CmdHndler *hndlr = (CmdHndler *)obj;
    const struct NcsiFixedCmdPkt *cmd = (NcsiFixedCmdPkt *)cmd_pkt;

    memset(&resp, 0, sizeof(resp));
    memcpy(&resp.rsp.NcsiHdr, &cmd->cmd.NcsiHdr, sizeof(resp.rsp.NcsiHdr));

    NCSI_CMD_BEGIN_BANNER();
    SDK_TRACE_INFO("ncsi_channel: 0x%x", cmd->cmd.NcsiHdr.channel);

    //FIXME: As of now ignoring the state machine for this cmd

    //TODO: Implement the logic here
    sdk::platform::readFruKey(SERIALNUMBER_KEY, serial_num);
    strncpy((char*)resp.uuid, serial_num.c_str(), sizeof(resp.uuid));
    resp.rsp.NcsiHdr.type = ncsi_cmd_resp_opcode(CMD_GET_PACKAGE_UUID);
    resp.rsp.NcsiHdr.length = htons(NCSI_GET_PKG_UUID_RSP_PAYLOAD_LEN);

    ret = hndlr->SendNcsiCmdResponse(&resp, sizeof(resp));

    NCSI_CMD_END_BANNER();

    return;
}

int CmdHndler::ValidateCmdPkt(const void *pkt, ssize_t sz)
{
    const uint8_t *buf = (uint8_t *)pkt;

    if ( !buf || !sz ) {
        SDK_TRACE_ERR("Zero sized packets cannot be validated");
        return -1;
    }

    if (sz < MIN_802_3_FRAME_SZ) {
        SDK_TRACE_ERR("NCSI packet size (%d) is less than min frame size "
                "for 802.3", sz);
        stats.rx_drop_cnt++;
    }

    if (buf[NCSI_HDR_REV_OFFSET] != SUPPORTED_NCSI_REV) {
        SDK_TRACE_ERR("NCSI Header Rev %d not supported. Expected Rev is %d",
                buf[NCSI_HDR_REV_OFFSET], SUPPORTED_NCSI_REV);
        stats.rx_drop_cnt++;
    }

    stats.rx_total_cnt++;
    stats.valid_cmd_rx_cnt++;

    return 0;
}

int CmdHndler::HandleCmd(const void* pkt, ssize_t sz)
{
    int ret = 0;
    const uint8_t *buf = (uint8_t *)pkt;
    void *rsp = NULL;
    ssize_t rsp_sz = 0;
    uint8_t opcode;

    ret = ValidateCmdPkt(pkt, sz);

    if (ret) {
        SDK_TRACE_ERR("Received malformed or invalid NCSI command packet: ret: %d "
                , ret);
        return ret;
    }

    opcode = buf[NCSI_CMD_OPCODE_OFFSET];

    if (CmdTable[opcode]) {
        SDK_TRACE_DEBUG("Handling Ncsi command: 0x%x ", opcode);
        CmdTable[opcode](this, pkt, sz);

        if (rsp && rsp_sz) {
            SDK_TRACE_INFO("Sending Ncsi Response for cmd: 0x%x ", opcode);

        }
    }
    else {
        SDK_TRACE_ERR("Ncsi command 0x%x is not supported",
                buf[NCSI_CMD_OPCODE_OFFSET]);
        //TODO: Send the NCSI response saying unsupported command
        stats.unsup_cmd_rx_cnt++;
    }

    return 0;
}

} // namespace ncsi
} // namespace platform
} // namespace sdk

