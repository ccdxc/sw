package types

import (
	"encoding/json"
	"net/http"
	"sync"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/emstore"
)

const (
	halMaxMirrorSession = 8
	// FlowMonitorRuleIDType is the resource type used by netflow export and mirror session to get a unique id
	FlowMonitorRuleIDType = "FlowMonitorRuleID"
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

// DropRuleSpec specifies drop reason
type DropRuleSpec struct {
	DropReasons halproto.DropReasons
}

// DropMonitorObj maintains drop rule related id allocation and is storable in emdb
type DropMonitorObj struct {
	api.TypeMeta
	api.ObjectMeta
	Spec             DropRuleSpec
	RuleID           uint64
	MirrorSessionIDs []uint64
}

// Marshal provides data marshalling before storing in emdb
func (m *DropMonitorObj) Marshal() (dAtA []byte, err error) {
	return json.Marshal(*m)
}

// Unmarshal provides data marshalling before storing in emdb
func (m *DropMonitorObj) Unmarshal(dAtA []byte) error {
	return json.Unmarshal(dAtA, m)
}

// FlowMonitorRuleSpec specifies flow monitor rule spec
type FlowMonitorRuleSpec struct {
	SourceIP      string
	DestIP        string
	SourceMac     uint64
	DestMac       uint64
	EtherType     uint32
	Protocol      uint32
	SourceL4Port  uint32
	DestL4Port    uint32
	SourceGroupID uint64
	DestGroupID   uint64
	VrfID         uint64
}

// FlowMonitorObj maintains flow rule related id allocation and is storable in emdb
type FlowMonitorObj struct {
	api.TypeMeta
	api.ObjectMeta
	Spec             FlowMonitorRuleSpec
	RuleID           uint64
	MirrorSessionIDs []uint64
}

// Marshal provides data marshalling before storing in emdb
func (m *FlowMonitorObj) Marshal() (dAtA []byte, err error) {
	return json.Marshal(*m)
}

// Unmarshal provides data marshalling before storing in emdb
func (m *FlowMonitorObj) Unmarshal(dAtA []byte) error {
	return json.Unmarshal(dAtA, m)
}

// MirrorSessionObj maintains mirrorsession related id allocation and is storable in emdb
type MirrorSessionObj struct {
	api.TypeMeta
	api.ObjectMeta
	MirrorID           uint64
	Created            bool
	Handle             uint64
	FlowMonitorRuleIDs []uint64
	DropMonitorRuleIDs []uint64
}

// Marshal provides data marshalling before storing in emdb
func (m *MirrorSessionObj) Marshal() (dAtA []byte, err error) {
	return json.Marshal(*m)
}

// Unmarshal provides data marshalling before storing in emdb
func (m *MirrorSessionObj) Unmarshal(dAtA []byte) error {
	return json.Unmarshal(dAtA, m)
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
	MirrorSessionDB        map[string]*netproto.MirrorSession
	MirrorSessionNameToID  map[string]uint64
	MirrorSessionIDToObj   map[uint64]MirrorSessionObj
	FlowMonitorRuleToID    map[FlowMonitorRuleSpec]uint64
	FlowMonitorRuleIDToObj map[uint64]FlowMonitorObj
	DropRuleToID           map[halproto.DropReasons]uint64
	DropRuleIDToObj        map[uint64]DropMonitorObj
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
	CreateMirrorSession(pcSession *netproto.MirrorSession) error
	ListMirrorSession() []*netproto.MirrorSession
	GetMirrorSession(pcSession *netproto.MirrorSession) *netproto.MirrorSession
	UpdateMirrorSession(pcSession *netproto.MirrorSession) error
	DeleteMirrorSession(pcSession *netproto.MirrorSession) error
	//EnablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error
	//DisablePacketCaptureSession(pcSession *tsproto.PacketCaptureSession) error

	// TechSupportRequest APIs
	//CreateTechSupportRequest(pcSession *tsproto.TechSupportRequest) error
	//ListTechSupportRequest() []*tsproto.TechSupportRequest
	//GetTechSupportRequest(pcSession *tsproto.TechSupportRequest) *tsproto.TechSupportRequest
	//UpdateTechSupportRequest(pcSession *tsproto.TechSupportRequest) error
	//DeleteTechSupportRequest(pcSession *tsproto.TechSupportRequest) error
	Debug(w http.ResponseWriter, r *http.Request)
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
