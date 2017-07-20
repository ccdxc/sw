package defs

// VCOp defines the object operations
type VCOp string

const (
	// VCOpSet indicates a create or modify
	VCOpSet = VCOp("set")
	// VCOpDelete indicates a delete
	VCOpDelete = VCOp("delete")
)

// HostMsg specifies a hostsystem object operation
type HostMsg struct {
	Op         VCOp
	Key        string // vSphere key for the hostsystem object
	Value      *ESXHost
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
	Mac     string // Mac address of the PNIC as seen by the server
	DvsUUID string // Uuid of the DVS to which this PNIC is attached
}

// DvsInstance contains information about the proxy dvs on the host
type DvsInstance struct {
	Name    string   // Name of the dvs
	Uplinks []string // Uplink names(e.g. vmnic1) on the host that are part of this dvs
}
