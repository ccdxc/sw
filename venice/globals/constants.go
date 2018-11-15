// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package globals

// global constants
const (

	// Default Tenant name
	DefaultTenant = "default"

	// Default Namespace
	DefaultNamespace = "default"

	// Localhost name
	// The Go resolver by default will contact the DNS servers listed in /etc/resolver.conf,
	// by passing /etc/hosts and ignoring /etc/nsswitch.conf
	// If localhost resolution fails because the remote server does not respond in time or
	// responds with some non-local result, Venice cluster fails to form.
	// To avoid this kind of issues we use "127.0.0.1" instead of "localhost"
	Localhost = "127.0.0.1"

	// Well-known user names
	KubernetesAPIServerUserName = "kube-apiserver" // the name used by K8s API server to authenticate to other entities

	// AdminRole is admin role name that has all privileges for a tenant
	AdminRole = "AdminRole"

	// Config directories
	ConfigDir                            = "/etc/pensando"
	CmdConfigDir                         = ConfigDir + "/" + Cmd
	CommonConfigDir                      = ConfigDir + "/shared/common"
	EtcdConfigDir                        = ConfigDir + "/" + Etcd
	NtpConfigDir                         = ConfigDir + "/" + Ntp
	KubernetesConfigDir                  = ConfigDir + "/kubernetes"
	KubernetesAPIServerConfigDir         = KubernetesConfigDir + "/" + KubeAPIServer
	KubernetesControllerManagerConfigDir = KubernetesConfigDir + "/" + KubeControllerManager
	KubernetesSchedulerConfigDir         = KubernetesConfigDir + "/" + KubeScheduler
	KubeletConfigDir                     = KubernetesConfigDir + "/" + Kubelet

	EtcdServiceFile = "pen-etcd.service"

	// Data directory
	EtcdDataDir = "/var/lib/etcd"

	// PKI directories
	//
	// Top-level PKI directory
	PKIDir            = "/var/lib/pensando/pki"
	CmdPKIDir         = PKIDir + "/" + Cmd
	EtcdPKIDir        = PKIDir + "/" + Etcd                 // Credentials for etcd instances to mutually authenticate each other
	EtcdClientAuthDir = PKIDir + "/shared/etcd-client-auth" // Credentials for etcd clients, shared between CMD and K8s APiServer
	// Kubernetes components
	KubernetesPKIDir                  = PKIDir + "/kubernetes"
	KubernetesAPIServerPKIDir         = KubernetesPKIDir + "/" + KubeAPIServer
	KubernetesControllerManagerPKIDir = KubernetesPKIDir + "/" + KubeControllerManager
	KubernetesSchedulerPKIDir         = KubernetesPKIDir + "/" + KubeScheduler
	KubeletPKIDir                     = KubernetesPKIDir + "/" + Kubelet
	KubernetesServiceAccountsPKIDir   = KubernetesPKIDir + "/service-accounts"
	// Kubernetes clients
	KubernetesAPIServerClientPKIDir = KubernetesPKIDir + "/apiserver-client"
	// Elastic instance-to-instance communication
	ElasticNodeAuthDir = PKIDir + "/" + ElasticSearch + "-node"
	// Elastic server-to-client communication
	ElasticHTTPSAuthDir = PKIDir + "/" + ElasticSearch + "-https"
	// Elastic clients
	ElasticClientAuthDir = PKIDir + "/shared/elastic-client-auth"
	// Filebeat does not run as root, so it has a hard time navigating the /var/lib/pki directory structure.
	// Since it doesn't need anything else from pki, it's easier to mount Elastic credentials directly in the Filebeat directory
	FilebeatElasticClientAuthDir = "/usr/share/filebeat/auth"
	VosHTTPSAuthDir              = PKIDir + "/" + Vos + "/certs"

	// LogDir direcory
	LogDir = "/var/log/pensando"

	// PenCtlLogDir direcory
	PenCtlLogDir = "/var/log"

	// Events directory
	EventsDir = "/var/lib/pensando/events"

	// Cores directory
	CoresDir = "/data/core"

	// Update directory
	UpdateDir = "/update"

	//Reverse Proxy Router Port
	RevProxyPort = "8888"

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
	CMDAuthCertAPIPort             = CMDGRPCAuthPort

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
	// Nic registration interval in seconds
	NicRegIntvl = 1
	// Nic update interval in seconds
	NicUpdIntvl = 30

	// KVStore
	KVStorePeerPort   = "5001"
	KVStoreClientPort = "5002"

	// Kubernetes
	KubeAPIServerPort   = "6443"
	KubeletCadvisorPort = "4194"
	KubeletHealthzPort  = "10248"
	KubeletServerPort   = "10250"
	KubeletReadOnlyPort = "10255"

	// Elasticsearch
	ElasticsearchRESTPort = "9200"
	ElasticsearchRPCPort  = "9300"

	// Kibana
	KibanaRESTPort = "5601"

	// Citadel
	InfluxHTTPPort      = "8086"
	InfluxAdminPort     = "8083"
	CollectorRPCPort    = "10777"
	AggregatorAPIPort   = "10778"
	CitadelRPCPort      = "7000"
	CitadelHTTPPort     = "7086"
	CitadelQueryRPCPort = "7087"

	// KV Store keys
	RootPrefix       = "/venice"        // Root for all venice objects
	ConfigRootPrefix = "/venice/config" // Root for config objects
	ClusterKey       = "/venice/config/cluster/cluster"
	NodesKey         = "/venice/config/cluster/nodes"
	StagingBasePath  = "/venice/staged"

	// API Infra constants
	ConfigURIPrefix  = "configs"
	StagingURIPrefix = "staging"

	// Events manager
	EvtsMgrRPCPort = "9010"

	// TPM
	TpmRPCPort = "9020"

	// Trouble shooting (TSM) Controller
	TsmRPCPort  = "9030"
	TsmRESTPort = "9031"

	// Spyglass search service
	SpyglassRPCPort    = "9011"
	SpyglassMaxResults = 8192

	// Events proxy
	EvtsProxyRPCPort = "9012"

	// objstore port
	VosPort = "19001"

	// tmagent port
	TmAGENTRestPort = "9013"

	// Rollout
	RolloutRPCPort = "9015"
)
