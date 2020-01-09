package defs

import (
	"context"
	"fmt"
	"net/url"
	"sync"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

// DefaultPrefix is the naming prefix for objects we create in vcenter
const DefaultPrefix = "#Pen-"

// DefaultPGPrefix is the prefix given to PG objects created by Venice
var DefaultPGPrefix = fmt.Sprintf("%sPG-", DefaultPrefix)

// DefaultDVSPrefix is the prefix given to DVS objects created by Venice
var DefaultDVSPrefix = fmt.Sprintf("%sDVS-", DefaultPrefix)

const (
	// VCEvent indicates a vc event
	VCEvent = Probe2StoreMsgType("VCEvent")
)

// Probe2StoreMsgType defines the probe to store message types
type Probe2StoreMsgType string

// Probe2StoreMsg specifies a store operation
type Probe2StoreMsg struct {
	MsgType Probe2StoreMsgType
	// Based on ProbeMsg type, receiver should know
	// what to cast val to.
	Val interface{}
}

// VCObject defines the vSphere object types we are interested in
type VCObject string

const (
	// VirtualMachine identifies the VCenter VM object type
	VirtualMachine = VCObject("VirtualMachine")
	// HostSystem identifies the VCenter Host object type
	HostSystem = VCObject("HostSystem")
	// VmwareDistributedVirtualSwitch identifies the VCenter Host object type
	VmwareDistributedVirtualSwitch = VCObject("VmwareDistributedVirtualSwitch")
	// DistributedVirtualSwitch identifies the VCenter Host object type
	// In VCSim, the type is DistributedVirtualSwitch not vmwareDistributedVirtualSwitch
	// DistributedVirtualSwitch = VCObject("DistributedVirtualSwitch")

	// DistributedVirtualPortgroup identifies the VCenter PG object type
	DistributedVirtualPortgroup = VCObject("DistributedVirtualPortgroup")
	// Datacenter identifies the VCenter Host object type
	Datacenter = VCObject("Datacenter")
)

// VCOp defines the object operations
type VCOp string

const (
	// VCOpSet indicates a create or modify
	VCOpSet = VCOp("set")
	// VCOpDelete indicates a delete
	VCOpDelete = VCOp("delete")
)

// VCProp defines the vSphere property type
type VCProp string

const (
	// HostPropConfig identifies host's config property
	HostPropConfig = VCProp("host-config")
	// VMPropConfig identifies the VM's config property
	VMPropConfig = VCProp("config")
	// VMPropName identifies the VM's name property
	VMPropName = VCProp("name")
	// VMPropRT identifies the VM's runtime property
	VMPropRT = VCProp("runtime")
	// VMPropTag identifies the VM's tag property
	VMPropTag = VCProp("tag")
	// VMPropCustom identifies the VM's customValue property
	VMPropCustom = VCProp("customValue")
	// VMPropOverallStatus identifies the VM's overallStatus property
	VMPropOverallStatus = VCProp("overallStatus")
)

// VCEventMsg specifies a vc event
type VCEventMsg struct {
	VcObject   VCObject
	Key        string // vSphere key for the object
	Changes    []types.PropertyChange
	UpdateType types.ObjectUpdateKind
	DcID       string
	DcName     string
	Originator string // Identifier for the VC that originated the update
}

// TagEntry is an item of a TagMsg
type TagEntry struct {
	// Tag display name in vcenter
	Name string
	// category display name in vcenter
	Category string
}

// TagMsg specifies the tag probe to store object
type TagMsg struct {
	Tags []TagEntry
}

// State is shared state between VCHub objects
type State struct {
	VcURL      *url.URL
	VcID       string
	Ctx        context.Context
	Log        log.Logger
	StateMgr   *statemgr.Statemgr
	OrchConfig *orchestration.Orchestrator
	Wg         *sync.WaitGroup
	// If supplied, we only process events if the DC name matches this
	// TODO: for testing locally only, remove eventually
	ForceDCname string
}
