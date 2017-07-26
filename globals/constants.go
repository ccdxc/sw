package globals

// global constants
const (
	// Config directory
	ConfigDir       = "/etc/pensando"
	EtcdServiceFile = "pen-etcd.service"

	// Data directory
	EtcdDataDir = "/var/lib/etcd"

	// Ports
	APIGwRESTPort = "9000"

	// CMD
	CMDRESTPort = "9001"
	CMDGRPCPort = "9002"

	// VC Hub
	VCHubAPIPort = "9003"

	// KVStore
	KVStorePeerPort   = "5001"
	KVStoreClientPort = "5002"

	// Kubernetes
	KubeAPIServerPort   = "8080"
	KubeletCadvisorPort = "4194"
	KubeletHealthzPort  = "10248"
	KubeletServerPort   = "10250"
	KubeletReadOnlyPort = "10255"

	// Elasticsearch
	ElasticsearchRESTPort = "9200"

	// KV Store keys
	ClusterKey = "/venice/cmd/cluster"
	NodesKey   = "/venice/cmd/nodes"
)
