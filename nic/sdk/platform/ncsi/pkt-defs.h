/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#ifndef __PKT_DEFS_H__
#define __PKT_DEFS_H__
#include <linux/types.h>
#include <linux/if_ether.h>

struct NcsiPktHdr {
    struct ethhdr eth_hdr;
	uint8_t       mc_id;
	uint8_t       version;
	uint8_t       rsvd;
	uint8_t       id;
	uint8_t       type;
	uint8_t       channel;
	__be16        length;
	__be32        rsvd1[2];
} __attribute__((packed));

struct NcsiCmdPktHdr {
	struct NcsiPktHdr NcsiHdr;
} __attribute__((packed));

struct NcsiFixedResp {
	struct NcsiPktHdr NcsiHdr;
	__be16            code;
	__be16            reason;
} __attribute__((packed));

/* 2 bytes response code and 2 bytes reason code */
#define NCSI_FIXED_RSP_PAYLOAD_LEN      4

/* NCSI common command packet */
struct NcsiFixedCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be32               csum;
	uint8_t              pad[26];
} __attribute__((packed));

struct NcsiRspPkt {
	struct NcsiFixedResp rsp;
	__be32                csum;
	uint8_t               pad[22];
} __attribute__((packed));

/* Packet definitions for NCSI command packets */

/* Select Package */
struct SelectPackageCmdPkt {
	struct NcsiCmdPktHdr cmd;
	uint8_t              rsvd[3];
	uint8_t              hw_arbitration;
	__be32               csum;
	uint8_t              pad[22];
} ;

/* Disable Channel */
struct DisableChanCmdPkt {
	struct NcsiCmdPktHdr cmd;
	uint8_t              rsvd[3];
	uint8_t              ald;
	__be32               csum;
	uint8_t              pad[22];
};

/* Reset Channel */
struct ResetChanCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be32               rsvd;
	__be32               csum;
	uint8_t              pad[22];
};

/* Set Link */
struct SetLinkCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be32               mode;
	__be32               oem_mode;
	__be32               csum;
	uint8_t              pad[18];
};

/* Set VLAN Filter */
struct SetVlanFilterCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be16               rsvd;
	__be16               vlan;
	__be16               rsvd1;
	uint8_t              index;
	uint8_t              enable;
	__be32               csum;
	uint8_t              pad[18];
} __attribute__((packed));

/* Enable VLAN */
struct EnVlanCmdPkt {
	struct NcsiCmdPktHdr cmd;
	uint8_t              rsvd[3];
	uint8_t              mode;
	__be32               csum;
	uint8_t              pad[22];
} __attribute__((packed));

/* Set MAC Address */
struct SetMacAddrCmdPkt {
	struct NcsiCmdPktHdr cmd;
	uint8_t              mac[6];
	uint8_t              index;
	uint8_t              at_e;
	__be32               csum;
	uint8_t              pad[18];
} __attribute__((packed));

/* Enable Broadcast Filter */
struct EnBcastFilterCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be32               mode;
	__be32               csum;
	uint8_t              pad[22];
} __attribute__((packed));

/* Enable Global Multicast Filter */
struct EnGlobalMcastFilterCmdPkt {
	struct NcsiCmdPktHdr cmd;
	__be32               mode;
	__be32               csum;
	uint8_t              pad[22];
} __attribute__((packed));

/* Packet definitions for NCSI response packets */

/* Get Link Status */

struct GetLinkStatusRespPkt {
	struct NcsiFixedResp rsp;
	__be32                status;
	__be32                other;
	__be32                oem_status;
	__be32                csum;
	uint8_t               pad[10];
} __attribute__((packed));
#define NCSI_GET_LINK_STATUS_RSP_PAYLOAD_LEN    16

/* Get Version ID */
struct GetVersionIdRespPkt {
	struct NcsiFixedResp rsp;
	__be32                ncsi_version;
	uint8_t               rsvd[3];
	uint8_t               alpha2;
	uint8_t               fw_name[12];
	__be32                fw_version;
	__be16                pci_ids[4];
	__be32                mf_id;
	__be32                csum;
} __attribute__((packed));

#define NCSI_GET_VER_ID_RSP_PAYLOAD_LEN (sizeof(struct GetVersionIdRespPkt) - sizeof(struct NcsiFixedResp))

/* Get Capabilities */
struct GetCapRespPkt {
	struct NcsiFixedResp rsp;
	__be32                cap;
	__be32                bc_cap;
	__be32                mc_cap;
	__be32                buf_cap;
	__be32                aen_cap;
	uint8_t               vlan_cnt;
	uint8_t               mixed_cnt;
	uint8_t               mc_cnt;
	uint8_t               uc_cnt;
	uint8_t               rsvd[2];
	uint8_t               vlan_mode;
	uint8_t               channel_cnt;
	__be32                csum;
} __attribute__((packed));

#define NCSI_GET_CAP_RSP_PAYLOAD_LEN (sizeof(struct GetCapRespPkt) - sizeof(struct NcsiFixedResp))


/* Get Parameters */
struct GetParamRespPkt {
	struct NcsiFixedResp rsp;
	uint8_t               mac_cnt;
	uint8_t               rsvd[2];
	uint8_t               mac_enable;
	uint8_t               vlan_cnt;
	uint8_t               rsvd1;
	__be16                vlan_enable;
	__be32                link_mode;
	__be32                bc_mode;
	__be32                valid_modes;
	uint8_t               vlan_mode;
	uint8_t               fc_mode;
	uint8_t               rsvd2[2];
	__be32                aen_mode;
	uint8_t               mac[6];
	__be16                vlan;
	__be32                csum;
} __attribute__((packed));


#define NCSI_GET_PARAM_RSP_PAYLOAD_LEN (sizeof(struct GetParamRespPkt) - sizeof(struct NcsiFixedResp))

/* Get Controller Packet Statistics */
struct GetNicStatsRespPkt {
	struct NcsiFixedResp rsp;
	__be32                cnt_hi;
	__be32                cnt_lo;
	__be32                rx_bytes;
	__be32                tx_bytes;
	__be32                rx_uc_pkts;
	__be32                rx_mc_pkts;
	__be32                rx_bc_pkts;
	__be32                tx_uc_pkts;
	__be32                tx_mc_pkts;
	__be32                tx_bc_pkts;
	__be32                fcs_err;
	__be32                align_err;
	__be32                false_carrier;
	__be32                runt_pkts;
	__be32                jabber_pkts;
	__be32                rx_pause_xon;
	__be32                rx_pause_xoff;
	__be32                tx_pause_xon;
	__be32                tx_pause_xoff;
	__be32                tx_s_collision;
	__be32                tx_m_collision;
	__be32                l_collision;
	__be32                e_collision;
	__be32                rx_ctl_frames;
	__be32                rx_64_frames;
	__be32                rx_127_frames;
	__be32                rx_255_frames;
	__be32                rx_511_frames;
	__be32                rx_1023_frames;
	__be32                rx_1522_frames;
	__be32                rx_9022_frames;
	__be32                tx_64_frames;
	__be32                tx_127_frames;
	__be32                tx_255_frames;
	__be32                tx_511_frames;
	__be32                tx_1023_frames;
	__be32                tx_1522_frames;
	__be32                tx_9022_frames;
	__be32                rx_valid_bytes;
	__be32                rx_runt_pkts;
	__be32                rx_jabber_pkts;
	__be32                csum;
} __attribute__((packed));


#define NCSI_GET_NIC_STATS_RSP_PAYLOAD_LEN (sizeof(struct GetNicStatsRespPkt) - sizeof(struct NcsiFixedResp))

/* Get NCSI Statistics */
struct GetNCSIStatsRespPkt {
    struct NcsiFixedResp rsp;
    __be32                rx_cmds;
    __be32                dropped_cmds;
    __be32                cmd_type_errs;
    __be32                cmd_csum_errs;
    __be32                rx_pkts;
    __be32                tx_pkts;
    __be32                tx_aen_pkts;
    __be32                csum;
} __attribute__((packed));

#define NCSI_GET_NCSI_STATS_RSP_PAYLOAD_LEN (sizeof(struct GetNCSIStatsRespPkt) - sizeof(struct NcsiFixedResp))

/* Get NCSI Pass-through Statistics */
struct GetPassThruStatsRespPkt {
	struct NcsiFixedResp rsp;
	__be32                tx_pkts;
	__be32                tx_dropped;
	__be32                tx_channel_err;
	__be32                tx_us_err;
	__be32                rx_pkts;
	__be32                rx_dropped;
	__be32                rx_channel_err;
	__be32                rx_us_err;
	__be32                rx_os_err;
	__be32                csum;
} __attribute__((packed));

#define NCSI_GET_PASSTHRU_STATS_RSP_PAYLOAD_LEN (sizeof(struct GetPassThruStatsRespPkt) - sizeof(struct NcsiFixedResp))

/* Get package status */
struct GetPkgStatusRespPkt {
	struct NcsiFixedResp rsp;
	__be32                status;
	__be32                csum;
	uint8_t               pad[18];
} __attribute__((packed));

#define NCSI_GET_PKG_STATUS_RSP_PAYLOAD_LEN     8

/* Get package UUID */
struct GetPkgUUIDRespPkt {
	struct NcsiFixedResp rsp;
	uint8_t               uuid[16];
	__be32                csum;
	uint8_t               pad[6];
} __attribute__((packed));

#define NCSI_GET_PKG_UUID_RSP_PAYLOAD_LEN     20

/* NCSI Commands Opcodes */
typedef enum {
    CMD_CLEAR_INIT_STATE         = 0x00,
    CMD_SELECT_PACKAGE           = 0x01,
    CMD_DESELECT_PACKAGE         = 0x02,
    CMD_EN_CHAN                  = 0x03,
    CMD_DIS_CHAN                 = 0x04,
    CMD_RESET_CHAN               = 0x05,
    CMD_EN_CHAN_NW_TX            = 0x06,
    CMD_DIS_CHAN_NW_TX           = 0x07,
    CMD_SET_LINK                 = 0x09,
    CMD_GET_LINK_STATUS          = 0x0A,
    CMD_SET_VLAN_FILTER          = 0x0B,
    CMD_EN_VLAN                  = 0x0C,
    CMD_DIS_VLAN                 = 0x0D,
    CMD_SET_MAC_ADDR             = 0x0E,
    CMD_EN_BCAST_FILTER          = 0x10,
    CMD_DIS_BCAST_FILTER         = 0x11,
    CMD_EN_GLOBAL_MCAST_FILTER   = 0x12,
    CMD_DIS_GLOBAL_MCAST_FILTER  = 0x13,
    CMD_GET_VER_ID               = 0x15,
    CMD_GET_CAP                  = 0x16,
    CMD_GET_PARAMS               = 0x17,
    CMD_GET_NIC_STATS            = 0x18,
    CMD_GET_NCSI_STATS           = 0x19,
    CMD_GET_NCSI_PASSTHRU_STATS  = 0x1A,
    CMD_GET_PACKAGE_STATUS       = 0x1B,
    CMD_GET_PACKAGE_UUID         = 0x52
} NcsiCmd;

#define ncsi_cmd_resp_opcode(cmd) (cmd + 0x80)

#endif //__PKT_DEFS_H__

