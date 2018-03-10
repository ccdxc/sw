// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package globals

// global constants
const (

	// Default Tenant name
	DefaultTenant = "default"

	// Default Namespace
	DefaultNamespace = "default"

	// Config directory
	ConfigDir       = "/etc/pensando"
	EtcdServiceFile = "pen-etcd.service"

	// Data directory
	EtcdDataDir = "/var/lib/etcd"

	// API gw Port
	APIGwRESTPort = "9000"

	// CMD
	CMDRESTPort                    = "9001"
	CMDGRPCUnauthPort              = "9002"
	CMDCertAPIPort                 = CMDGRPCUnauthPort
	CMDClusterMgmtPort             = CMDGRPCUnauthPort
	CMDSmartNICRegistrationAPIPort = CMDGRPCUnauthPort
	CMDGRPCAuthPort                = "9009"
	CMDResolverPort                = CMDGRPCAuthPort
	CMDSmartNICUpdatesPort         = CMDGRPCAuthPort

	// API Server
	APIServerPort     = "9003"
	APIServerRESTPort = "9103"

	// VC Hub
	VCHubAPIPort = "9004"

	// NPM
	NpmRPCPort  = "9005"
	NpmRESTPort = "9006"

	// Agent
	AgentRESTPort = "9007"

	// Cluster Key Managements Services
	CKMAPILocalPort = "9008"
	CKMEndpointName = "ckm"

	// NMD
	NmdRESTPort = "9008"

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
	ElasticsearchRPCPort  = "9300"

	// Kibana
	KibanaRESTPort = "5601"

	// Influx
	InfluxHTTPPort   = "8086"
	InfluxAdminPort  = "8083"
	CollectorAPIPort = "10777"
	CitadelRPCPort   = "7000"
	CitadelHTTPPort  = "7086"

	// KV Store keys
	RootPrefix = "/venice"
	ClusterKey = "/venice/cmd/cluster"
	NodesKey   = "/venice/cmd/nodes"

	// Events manager
	EvtsMgrRPCPort = "9010"

	// TPM
	TpmRPCPort = "9020"

	// Spyglass search service
	SpyglassRPCPort = "9011"
)
