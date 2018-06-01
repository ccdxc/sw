// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package globals

// global constants
const (

	// Default Tenant name
	DefaultTenant = "default"

	// Default Namespace
	DefaultNamespace = "default"

	// Config directories
	ConfigDir           = "/etc/pensando"
	CmdConfigDir        = ConfigDir + "/" + Cmd
	CommonConfigDir     = ConfigDir + "/shared/common"
	EtcdConfigDir       = ConfigDir + "/" + Etcd
	KubernetesConfigDir = ConfigDir + "/kubernetes"
	NtpConfigDir        = ConfigDir + "/" + Ntp

	EtcdServiceFile = "pen-etcd.service"

	// Data directory
	EtcdDataDir = "/var/lib/etcd"

	// Events directoty
	EventsDir = "/var/lib/pensando/events"

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
	ClusterKey = "/venice/cluster/cluster"
	NodesKey   = "/venice/cluster/nodes"

	// Events manager
	EvtsMgrRPCPort = "9010"

	// TPM
	TpmRPCPort = "9020"

	// Trouble shooting (TSM) Controller
	TsmRPCPort  = "9030"
	TsmRESTPort = "9031"

	// Spyglass search service
	SpyglassRPCPort = "9011"

	// Events proxy
	EvtsProxyRPCPort = "9012"
)
