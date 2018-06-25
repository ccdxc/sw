package types

import (
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/troubleshooting/protos"
	tsproto "github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/emstore"
)

const (
	halMaxMirrorSession = 8
)

// IPAddrDetails maintains ip details.
type IPAddrDetails struct {
	IP        []byte
	IsIpv4    bool
	IsSubnet  bool
	PrefixLen uint32
}

// AppPortDetails maintains ip-proto, l4port#
type AppPortDetails struct {
	Ipproto int32
	L4port  int32
}

// FlowMonitorIPRuleDetails contains src, dest selection value, AppPort selection value.
type FlowMonitorIPRuleDetails struct {
	SrcIPObj     *IPAddrDetails
	SrcIPString  string
	DestIPObj    *IPAddrDetails
	DestIPString string
	AppPortObj   *AppPortDetails
}

// FlowMonitorMACRuleDetails contains src, dest selection value, AppPort selection value.
type FlowMonitorMACRuleDetails struct {
	SrcMAC     uint64
	DestMAC    uint64
	AppPortObj *AppPortDetails
}

// MirrorDB maintains list of mirrorSessions, flow, drop rule objects.
type MirrorDB struct {
	MirrorSessionDB        map[string]*tsproto.MirrorSession
	MirrorSessionNameToID  map[string]uint64
	MirrorSessionIDToObj   map[uint64]state.MirrorSessionObj
	FlowMonitorRuleToID    map[state.FlowMonitorRuleSpec]uint64
	FlowMonitorRuleIDToObj map[uint64]state.FlowMonitorObj
	DropRuleToID           map[state.CopiedDropReasons]uint64
	DropRuleIDToObj        map[uint64]state.DropMonitorObj
	AllocatedMirrorIds     map[uint64]bool
}

// TsAgent is the troubleshooting agent instance
type TsAgent struct {
	sync.Mutex
	Store    emstore.Emstore
	NodeUUID string
	Ctrlerif CtrlerAPI
	Datapath TsDatapathAPI
	DB       MirrorDB
}

// CtrlerAPI is all APIs provided by TS controller module to TsAgent. Implemented in ctrlerif/ctrlerif.go
type CtrlerAPI interface {
}

// CtrlerIntf is all APIs provided by TS agent. Invoked when controller intf is wathching on TS objects.
// Implemented in troubleshooting/state/
type CtrlerIntf interface {
	GetAgentID() string
	RegisterCtrlerIf(ctrlerif CtrlerAPI) error
	CreatePacketCaptureSession(pcSession *tsproto.MirrorSession) error
	ListPacketCaptureSession() []*tsproto.MirrorSession
	GetPacketCaptureSession(pcSession *tsproto.MirrorSession) *tsproto.MirrorSession
	UpdatePacketCaptureSession(pcSession *tsproto.MirrorSession) error
	DeletePacketCaptureSession(pcSession *tsproto.MirrorSession) error
	//EnablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error
	//DisablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error
}

// TsDatapathAPI is all APIs provided by datapath/hal module. Implemented in datapath/hal.go (and in mockhal.go)
type TsDatapathAPI interface {
	SetAgent(ag TsDatapathIntf) error
	CreatePacketCaptureSession(string, uint64, *MirrorDB, *halproto.MirrorSessionRequestMsg,
		[]*halproto.FlowMonitorRuleRequestMsg,
		[]*halproto.DropMonitorRuleRequestMsg) error
	UpdatePacketCaptureSession(string, uint64, *MirrorDB, *halproto.MirrorSessionRequestMsg,
		[]*halproto.FlowMonitorRuleRequestMsg,
		[]*halproto.DropMonitorRuleRequestMsg) ([]uint64, []uint64, error)
	DeletePacketCaptureSession(string, *halproto.MirrorSessionDeleteRequestMsg) error
	UpdateFlowMonitorRule([]*halproto.FlowMonitorRuleRequestMsg) error
	DeleteFlowMonitorRule([]*halproto.FlowMonitorRuleDeleteRequestMsg) error
	UpdateDropMonitorRule([]*halproto.DropMonitorRuleRequestMsg) error
	DeleteDropMonitorRule([]*halproto.DropMonitorRuleDeleteRequestMsg) error
}

// TsDatapathIntf is all APIs provided by troubleshooting agent to datapath
type TsDatapathIntf interface {
	GetAgentID() string
}
