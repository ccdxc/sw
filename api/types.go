package api

import (
	"net"
)

// TODO(sanjayt): Move StatusResults to enum in .proto file
var (
	// StatusResultOk indicates that the operation succeeded (http code 200).
	StatusResultOk StatusResult = StatusResult{"Ok"}

	// StatusResultExpired indicates that the data being requested has expired
	// (http code 410).
	StatusResultExpired StatusResult = StatusResult{"Expired"}

	// StatusResultInternalError indicates that the server had an internal error
	// processing the request (http code 500).
	StatusResultInternalError StatusResult = StatusResult{"Internal Error"}
)

// ClusterSpec contains the configuration of the cluster.
type ClusterSpec struct {
	// QuorumNodes contains the list of hostnames for nodes configured to be quorum
	// nodes in the cluster.
	QuorumNodes []string `json:"quorumNodes,omitempty"`

	// VirtualIP is the IP address for managing the cluster. It will be hosted by
	// the winner of election between quorum nodes.
	VirtualIP net.IP `json:"virtualIP,omitempty"`

	// NTPServers contains the list of NTP servers for the cluster.
	NTPServers []string `json:"ntpServers,omitempty"`

	// DNSSubDomain is the DNS subdomain for the default tenant.
	DNSSubDomain string `json:"dnsSubDomain,omitempty"`
}

// ClusterStatus contains the current state of the cluster.
type ClusterStatus struct {
	// Leader contains the node name of the cluster leader.
	Leader string `json:"leader,omitempty"`
}

// Cluster is representation of a multi node cluster.
type Cluster struct {
	TypeMeta   `json:",inline"`
	ObjectMeta `json:"metadata,omitempty"`

	// Spec contains the configuration of the cluster.
	Spec ClusterSpec `json:"spec,omitempty"`

	// Status contains the current state of the cluster.
	Status ClusterStatus `json:"status,omitempty"`
}

// NodeSpec contains the configuration of the node.
type NodeSpec struct {
	// FIXME: To be defined.
}

// NodeStatus contains the current state of the node.
type NodeStatus struct {
	// FIXME: To be defined.
}

// Node is representation of a single node in the cluster.
type Node struct {
	TypeMeta   `json:",inline"`
	ObjectMeta `json:"metadata,omitempty"`

	// Spec contains the configuration of the node.
	Spec NodeSpec `json:"spec,omitempty"`

	// Status contains the current state of the node.
	Status NodeStatus `json:"status,omitempty"`
}

// NodeList is representation of all nodes in the cluster.
type NodeList struct {
	TypeMeta `json:",inline"`
	ListMeta `json:"metadata,omitempty"`

	// Items is the list of nodes in the cluster.
	Items []Node `json:"items,omitempty"`
}
