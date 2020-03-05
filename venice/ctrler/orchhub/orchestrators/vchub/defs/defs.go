package defs

import (
	"context"
	"fmt"
	"net/url"
	"strings"
	"sync"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/utils/log"
)

// VCTagCategory is the category name in vcenter for all tags we write to VC
const VCTagCategory = "Pensando Systems"

// VCTagCategoryDescription is the description for VCTagCategory
const VCTagCategoryDescription = "Category for Pensando System tags."

// VCTagVlanPrefix is the tag prefix to apply to PGs to show their wire vlan value
const VCTagVlanPrefix = "VLAN: "

// VCTagVlanDescription is the description for VCTagVlan
const VCTagVlanDescription = "VLAN that will be used when packets leave the DSCs."

// VCTagManagedDefault is the tag to apply to objects we manage if clusterID is not present
const VCTagManagedDefault = "Pensando Managed"

// VCTagManaged is the tag to apply to objects we manage
const vcTagManaged = "Pensando Managed - %s"

// VCTagManagedDescription is the description for VCTagManaged
const VCTagManagedDescription = "This object is managed by Pensando and should not be renamed or modified."

// CreateVCTagManagedTag generates tag name for VCTagManaged
func CreateVCTagManagedTag(clusterID string) string {
	return fmt.Sprintf(vcTagManaged, clusterID)
}

// CreateClusterID generates clusterID
func CreateClusterID(clusterObj cluster.Cluster) string {
	return clusterObj.Name + strings.Split(clusterObj.UUID, "-")[0]
}

// DefaultPrefix is the naming prefix for objects we create in vcenter
const DefaultPrefix = "#Pen-"

// DefaultPGPrefix is the prefix given to PG objects created by Venice
var DefaultPGPrefix = fmt.Sprintf("%sPG-", DefaultPrefix)

// DefaultDVSPrefix is the prefix given to DVS objects created by Venice
var DefaultDVSPrefix = fmt.Sprintf("%sDVS-", DefaultPrefix)

const (
	// VCEvent indicates a vc event
	VCEvent = Probe2StoreMsgType("VCEvent")
	// VCNotification indicates a notification (event) received from vCenter
	VCNotification = Probe2StoreMsgType("VCNotification")
	// VCConnectionStatus indicates probe has re-established connection
	VCConnectionStatus = Probe2StoreMsgType("VCConnectionStatus")
)

// VmkWorkloadPrefix - used when creating name for dummy workload for vmkernel networking
const VmkWorkloadPrefix = "VmkWorkload"

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
	HostPropConfig = VCProp("config")
	// HostPropName identifies the Host's name property
	HostPropName = VCProp("name")
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
	Key        string // vCenter object key
	Changes    []types.PropertyChange
	UpdateType types.ObjectUpdateKind
	DcID       string
	DcName     string
	Originator string // Identifier for the VC that originated the update
}

// VCNotificationType indicates notification types
type VCNotificationType string

const (
	// VMotionStart indicates start of a VM migration/relocation
	VMotionStart = VCNotificationType("VMotion Start")
	// VMotionFailed indicates failure/cancellation of a VM migration/relocation
	VMotionFailed = VCNotificationType("VMotion Failed")
	// VMotionDone indicates completion of a VM migration/relocation
	VMotionDone = VCNotificationType("VMotion Done")
)

// VCNotificationMsg defines notifications from VC Event manager
type VCNotificationMsg struct {
	Type VCNotificationType
	Msg  interface{}
}

// VMotionStartMsg indicates start of VM migration
type VMotionStartMsg struct {
	VMKey        string
	HotMigration bool
	DcID         string
	// DstHostKey/Name
	DstHostKey  string
	DstHostName string
	// DstDcID/Name is applicable for inter-DC migration
	DstDcName string
	DstDcID   string
}

// VMotionFailedMsg indicates that VMtion operation failed/stopped
type VMotionFailedMsg struct {
	DcID       string
	VMKey      string
	Reason     string
	DstHostKey string
}

// VMotionDoneMsg indicates completion of VM migration
type VMotionDoneMsg struct {
	DcID       string
	VMKey      string
	SrcHostKey string
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
	ClusterID  string
	Ctx        context.Context
	Log        log.Logger
	StateMgr   *statemgr.Statemgr
	OrchConfig *orchestration.Orchestrator
	Wg         *sync.WaitGroup
	// If supplied, we only process events if the DC name matches any of this
	// TODO: for testing locally only, remove eventually
	ForceDCNames map[string]bool
}

const (
	// MaxVmotionTimeout is the maximum time allowed for vmotion to complete
	MaxVmotionTimeout = "100s"
)
