// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package globals

// global constants
const (
	// Config directory
	ConfigDir       = "/etc/pensando"
	EtcdServiceFile = "pen-etcd.service"

	// Data directory
	EtcdDataDir = "/var/lib/etcd"

	// API gw Port
	APIGwRESTPort = "9000"

	// CMD
	CMDRESTPort    = "9001"
	CMDGRPCPort    = "9002"
	CMDCertAPIPort = "9009"

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

	// Kibana
	KibanaRESTPort = "5601"

	// Influx
	InfluxHTTPPort   = "8086"
	InfluxAdminPort  = "8083"
	CollectorAPIPort = "10777"

	// KV Store keys
	RootPrefix = "/venice"
	ClusterKey = "/venice/cmd/cluster"
	NodesKey   = "/venice/cmd/nodes"
)
