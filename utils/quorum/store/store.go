package store

import (
	"fmt"

	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/quorum/etcd"
)

// exported constants
const (
	KVStoreTypeEtcd = "etcd"
)

// New creates a new KV Store Quorum based on provided configuration.
func New(c *quorum.Config) (quorum.Interface, error) {
	switch c.Type {
	case KVStoreTypeEtcd:
		return etcd.NewQuorum(c)
	default:
		return nil, fmt.Errorf("Unknown kv store type: %v", c.Type)
	}
}

// Start starts a member of KV Store Quorum using exisitng configuration.
// This is typically called after a restart of node or process
func Start(c *quorum.Config) (quorum.Interface, error) {
	switch c.Type {
	case KVStoreTypeEtcd:
		return etcd.StartQuorum(c)
	default:
		return nil, fmt.Errorf("Unknown kv store type: %v", c.Type)
	}
}
