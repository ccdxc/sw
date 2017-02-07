package mdns

import "github.com/pensando/sw/api"

// DiscoveredNode represents node in the network discovered with MDNS. Can include this node.
type DiscoveredNode struct {
	api.TypeMeta   `json:",inline"`
	api.ObjectMeta `json:"meta"`

	Data map[string]string
}

// DiscoveredNodeList is list of DiscoveredNodes discovered with mDNS in the network
type DiscoveredNodeList struct {
	api.TypeMeta `json:",inline"`
	api.ListMeta `json:"meta"`

	Items []DiscoveredNode
}

// DiscoveredNodeEventType defines possible types of events for a watch.
type DiscoveredNodeEventType string

const (
	// CreatedOrUpdated is an event to indicate an object is created/updated
	CreatedOrUpdated DiscoveredNodeEventType = "CreatedOrUpdated"
	// Deleted is an event to indicate an object is deleted.
	Deleted DiscoveredNodeEventType = "Deleted"
)

// DiscoveredNodeEvent contains information about a single event on Peer DiscoveredNode
type DiscoveredNodeEvent struct {
	// Type of the watch event.
	Type DiscoveredNodeEventType

	// DiscoveredNode that created/updated/deleted the service on domain
	DiscoveredNode DiscoveredNode
}

// Browser is Interface to browse the network for services using mDNS
type Browser interface {
	// Return the current node list as known by the background job
	List() (*DiscoveredNodeList, error)
}

// Server is used to export this node's info. Automatically starts publishing records on creation of server.
type Server interface {
	// Publish new TXT records to network
	Publish(data map[string]string)

	// Stop the server
	Stop()
}
