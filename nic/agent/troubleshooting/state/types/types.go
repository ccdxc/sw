package types

import (
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	tsproto "github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/utils/emstore"
)

// MirrorSessionStatus maintains status of mirrorSession
type MirrorSessionStatus struct {
	MirrorID uint32 // upto maximum of halMaxMirrorSession. Id assigned to mirror session.
	Created  bool   // True when mirror session is active
	Handle   uint64 // handle returned by HAL when mirror session is created in pipeline.
}

// FlowMonitorRule rule object fields that HAL uses
type FlowMonitorRule struct {
	SourceIP          [16]byte
	DestIP            [16]byte
	SourceMac         uint64
	DestMac           uint64
	EtherType         uint32
	Protocol          uint32
	SourceL4Port      uint32
	DestL4Port        uint32
	SourceGroupID     uint64
	DestGroupID       uint64
	MirrorSessionName string
}

// DropMonitorRule drop rule fields that HAL uses
type DropMonitorRule struct {
	DropReasons       halproto.DropReasons
	DropReasonAll     bool
	MirrorSessionName string
}

// MonitorRuleStatus maintains status of drop/flow rule
type MonitorRuleStatus struct {
	RuleID  uint64 // Id allocated to flow rule or drop rule
	Applied bool   // True when rule is applied / sent to HAL.
	Handle  uint64 // Handle returned by HAL. If valid, then rule is applied in HAL
}

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
	SrcIPObj   *IPAddrDetails
	DestIPObj  *IPAddrDetails
	AppPortObj *AppPortDetails
}

// FlowMonitorMACRuleDetails contains src, dest selection value, AppPort selection value.
type FlowMonitorMACRuleDetails struct {
	SrcMAC     uint64
	DestMAC    uint64
	AppPortObj *AppPortDetails
}

// MirrorDB maintains list of mirrorSessions, flow,drop rules.
type MirrorDB struct {
	// Span related DS
	//mirrorDB map[int]*tsproto.MirrorSession // maps Mirror-session-ID to mirror config
	MirrorSessionNameToID              map[string]uint32
	PktMirrorSessions                  map[string]*MirrorSessionStatus
	FlowMonitorRuleIDToFlowMonitorRule map[uint64]FlowMonitorRule
	FlowMonitorRules                   map[FlowMonitorRule]*MonitorRuleStatus
	DropRuleIDToDropRule               map[uint64]DropMonitorRule
	DropMonitorRules                   map[DropMonitorRule]*MonitorRuleStatus
	AllocatedMirrorIds                 uint32
	AllocatedDropRuleIds               uint64
	AllocatedFlowMonitorRuleIds        uint64
}

// TsAgent is the network agent instance
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
	UpdatePacketCaptureSession(pcSession *tsproto.MirrorSession) error
	DeletePacketCaptureSession(pcSession *tsproto.MirrorSession) error
	//EnablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error
	//DisablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error
}

// TsDatapathAPI is all APIs provided by datapath/hal module. Implemented in datapath/hal.go (and in mockhal.go)
type TsDatapathAPI interface {
	SetAgent(ag TsDatapathIntf) error
	CreatePacketCaptureSession(string, *MirrorDB, *halproto.MirrorSessionRequestMsg, []*halproto.FlowMonitorRuleRequestMsg, []*halproto.DropMonitorRuleRequestMsg) error
	UpdatePacketCaptureSession(pcs *tsproto.MirrorSession) error
	DeletePacketCaptureSession(pcs *tsproto.MirrorSession) error
	//EnablePacketCaptureSession(pcs *tsproto.PacketCaptureSession) error
	//DisablePacketCaptureSession(pcs *tsproto.PacketCaptureSession) error
}

// TsDatapathIntf is all APIs provided by troubleshooting agent to datapath
type TsDatapathIntf interface {
	GetAgentID() string
}
