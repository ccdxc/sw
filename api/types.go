package api

import (
	"net"
)

// TypeMeta contains the metadata about kind and version for all API objects.
type TypeMeta struct {
	// Kind represents the type of the API object.
	Kind string `json:"kind,omitempty"`

	// APIVersion defines the version of the API object.
	APIVersion string `json:"apiVersion,omitempty"`
}

// ListMeta contains the metadata for list of objects.
type ListMeta struct {
	// Resource version of object store at the time of list generation.
	ResourceVersion string `json:"resourceVersion,omitempty"`
}

// ObjectMeta contains metadata that all objects stored in kvstore must have.
type ObjectMeta struct {
	// Name of the object, unique within a Namespace for scoped objects.
	Name string `json:"name,omitempty"`

	// Namespace of the object, for scoped objects.
	Namespace string `json:"namespace,omitempty"`

	// Resource version in the object store. This can only be set by the server.
	ResourceVersion string `json:"resourceVersion,omitempty"`

	// UUID is the unique identifier for the object. This can only be set by the server.
	UUID string `json:"uuid,omitempty"`

	// TODO: Add timestamps, labels etc.
}

// ObjectRef contains identifying information about an object.
type ObjectRef struct {
	// Kind represents the type of the API object.
	Kind string `json:"kind,omitempty"`

	// Namespace of the object, for scoped objects.
	Namespace string `json:"namespace,omitempty"`

	// Name of the object, unique within a Namespace for scoped objects.
	Name string `json:"name,omitempty"`
}

// StatusResult contains possible statuses for a NIC.
type StatusResult string

const (
	// StatusResultOk indicates that the operation succeeded (http code 200).
	StatusResultOk StatusResult = "Ok"

	// StatusResultExpired indicates that the data being requested has expired
	// (http code 410).
	StatusResultExpired StatusResult = "Expired"

	// StatusResultInternalError indicates that the server had an internal error
	// processing the request (http code 500).
	StatusResultInternalError StatusResult = "Internal Error"
)

// Status is returned for calls that dont return objects.
type Status struct {
	TypeMeta `json:",inline"`

	// Result contains the status of the operation, success or failure.
	Result StatusResult `json:"reason,omitempty"`

	// Message contains human readable form of the error.
	Message string `json:"message,omitempty"`

	// Code is the HTTP status code.
	Code int32 `json:"code,omitempty"`

	// Reference to the object (optional) for which this status is being sent.
	Ref *ObjectRef `json:"ref,omitempty"`
}

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
