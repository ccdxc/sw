package defs

import (
	"github.com/vmware/govmomi/vim25/types"
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

// VCObject defines the vSphere object types we are interested in
type VCObject string

const (
	// VirtualMachine identifies the VCenter VM object type
	VirtualMachine = VCObject("VirtualMachine")
	// HostSystem identifies the VCenter Host object type
	HostSystem = VCObject("HostSystem")
)

// Probe2StoreMsg specifies a store operation
type Probe2StoreMsg struct {
	VcObject   VCObject
	Key        string // vSphere key for the object
	Changes    []types.PropertyChange
	Originator string // Identifier for the VC that originated the update
}

// Store2ProbeMsgType defines the store to probe message types
type Store2ProbeMsgType string

const (
	// CreatePG indicates a PG create
	CreatePG = Store2ProbeMsgType("createPG")
	// DeletePG indicates a PG delete
	DeletePG = Store2ProbeMsgType("deletePG")
	// ProbeMsgResync indicates the probe should resync it's inventory
	ProbeMsgResync = Store2ProbeMsgType("resync")
)

// Store2ProbeMsg specifies a store to probe message
type Store2ProbeMsg struct {
	MsgType Store2ProbeMsgType
	// Based on ProbeMsg type, receiver should know
	// what to cast val to.
	Val interface{}
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
