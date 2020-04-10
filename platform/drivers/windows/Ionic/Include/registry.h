#pragma once

#define IONIC_REG_SRIOV			0
#define IONIC_REG_VMQ			1
#define IONIC_REG_RSS			2
#define IONIC_REG_VLANID		3
#define IONIC_REG_PRIVLAN		4
#define IONIC_REG_JUMBO			5
#define IONIC_REG_LSOV1			6
#define IONIC_REG_LSOV2V4		7
#define IONIC_REG_LSOV2V6		8
#define IONIC_REG_IPCSV4		9
#define IONIC_REG_TCPCSV4		10
#define IONIC_REG_TCPCSV6		11
#define IONIC_REG_UDPCSV4		12
#define IONIC_REG_UDPCSV6		13
#define IONIC_REG_RXPOOL		14
#define IONIC_REG_TXMODE		15
#define IONIC_REG_NUMVFS		16
#define IONIC_REG_VMQVLAN		17
#define IONIC_REG_RSSQUEUES		18
#define IONIC_REG_TXBUFFERS		19
#define IONIC_REG_RXBUFFERS		20
#define IONIC_REG_PME			21
#define IONIC_REG_AUTONEG		22
#define IONIC_REG_SPEED			23
#define IONIC_REG_FEC			24
#define IONIC_REG_PAUSE			25
#define IONIC_REG_PAUSETYPE		26

#define IONIC_REG_ENTRY_COUNT	27 // Include the 0th entry

#ifndef DEFINITIONS_ONLY
struct registry_entry ionic_registry[] = {
		{L"*SRIOV",						0,		1,		1,		0}, // Current set to 0 since SRIOV is disabled now
		{L"*VMQ",						0,		1,		1,		0}, // Current set to 0 since VMQ is disabled now
		{L"*RSS",						0,		1,		1,		1},
		{L"VlanID",						0,		ETH_VLAN_ID_MAX,	0,		0},
		{L"*PriorityVLANTag",			0,		3,		3,		3},
		{L"*JumboPacket",				IONIC_MIN_MTU,	IONIC_MAX_MTU,	IONIC_DEFAULT_MTU,	IONIC_DEFAULT_MTU},
		{L"*LsoV1IPv4",					0,		1,		0,		0},
		{L"*LsoV2IPv4",					0,		1,		1,		1},
		{L"*LsoV2IPv6",					0,		1,		1,		1},
		{L"*IPChecksumOffloadIPv4",		0,		3,		3,		3},
		{L"*TCPChecksumOffloadIPv4",	0,		3,		3,		3},
		{L"*TCPChecksumOffloadIPv6",	0,		3,		3,		3},
		{L"*UDPChecksumOffloadIPv4",	0,		3,		3,		3},
		{L"*UDPChecksumOffloadIPv6",	0,		3,		3,		3},
		{L"RxPoolSize",					IONIC_MIN_RX_POOL_FACTOR,	IONIC_MAX_RX_POOL_FACTOR,	IONIC_DEFAULT_RX_POOL_FACTOR,	IONIC_DEFAULT_RX_POOL_FACTOR},
		{L"TxFlushMode",				1,		3,		1,		1},
		{L"*NUMVFs",					0,		256,	0,		0},
		{L"*VMQVlanFiltering",			0,		1,		0,		0},
		{L"*NumRSSQueues",				1,		32,		4,		4},
		{L"*TransmitBuffers",			IONIC_MIN_TXRX_DESC, IONIC_MAX_TX_DESC, IONIC_DEF_TXRX_DESC, IONIC_DEF_TXRX_DESC},
		{L"*ReceiveBuffers",			IONIC_MIN_TXRX_DESC, IONIC_MAX_RX_DESC, IONIC_DEF_TXRX_DESC, IONIC_DEF_TXRX_DESC},
		{L"*EnablePME",					0,		1,		0,		0},
		{L"AutoNegotiate",				0,		1,		1,		IONIC_REG_UNDEFINED},
		{L"Speed",						IONIC_SPEED_1G,		IONIC_SPEED_100G,		0,		IONIC_REG_UNDEFINED},
		{L"FEC",						PORT_FEC_TYPE_NONE,		PORT_FEC_TYPE_RS,	0,		IONIC_REG_UNDEFINED},
		{L"Pause",						0,		IONIC_PAUSE_F_TX | IONIC_PAUSE_F_RX,		0,		IONIC_REG_UNDEFINED},
		{L"PauseType",					0,		2,		0,		IONIC_REG_UNDEFINED},
		};
#endif
