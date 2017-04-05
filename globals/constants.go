package globals

const (
	// Config directory
	ConfigDir = "/etc/pensando"

	// Ports
	CMDRESTPort       = "9001"
	CMDGRPCPort       = "9002"
	KVStorePeerPort   = "5001"
	KVStoreClientPort = "5002"

	// KV Store keys
	ClusterKey = "/cluster"
	NodesKey   = "/nodes"
)
