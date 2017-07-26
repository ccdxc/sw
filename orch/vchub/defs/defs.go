package defs

// VCOp defines the object operations
type VCOp string

// VCProp defines the vSphere property type
type VCProp string

const (
	// VCOpSet indicates a create or modify
	VCOpSet = VCOp("set")
	// VCOpDelete indicates a delete
	VCOpDelete = VCOp("delete")
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
)

// StoreMsg specifies a store operation
type StoreMsg struct {
	Op         VCOp
	Property   VCProp // Type of property passed
	Key        string // vSphere key for the hostsystem object
	Value      interface{}
	Originator string // Identifier for the VC that originated the update
}

// ESXHost defines an ESX object with our fields of interest
type ESXHost struct {
	HostIP  string
	DvsMap  map[string]*DvsInstance // proxy switches indexed by SwitchUuid
	PenNICs map[string]*NICInfo     // Pensando NICs seen on the host, indexed by name
}

// NICInfo contains info about a pNIC
type NICInfo struct {
	Name    string // Device name for the PNIC
	Mac     string // Mac address of the PNIC as seen by the server
	DvsUUID string // Uuid of the DVS to which this PNIC is attached
}

// DvsInstance contains information about the proxy dvs on the host
type DvsInstance struct {
	Name    string   // Name of the dvs
	Uplinks []string // Uplink names(e.g. vmnic1) on the host that are part of this dvs
}

// VirtualNIC contains information about a VNIC
type VirtualNIC struct {
	Name         string // Name of the vNIC
	MacAddress   string
	PortgroupKey string // Identifier for the backing port-group on dvs
	PortKey      string // PortId on the switch
	SwitchUUID   string // Attached switch
}

// VMConfig defines a VirtualMachine config object with our fields of interest
type VMConfig struct {
	Vnics map[string]*VirtualNIC // indexed by vnic mac
}

// VMRuntime defines a VirtualMachine runtime info with our fields of interest
type VMRuntime struct {
	HostKey    string // key to host where VM is placed
	PowerState string
}
