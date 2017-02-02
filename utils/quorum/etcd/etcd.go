package etcd

import (
	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/utils/quorum"
)

// etcdQuorum implements a Quorum using etcd.
type etcdQuorum struct {
	client *clientv3.Client
	config *quorum.Config
}

// NewQuorum creates a new etcdQuorum.
func NewQuorum(c *quorum.Config) (quorum.Interface, error) {
	// TODO
	return nil, nil
}

// List returns the current quorum members.
func (e *etcdQuorum) List() ([]quorum.Member, error) {
	// TODO
	return nil, nil
}

// Add adds a new member to the quorum.
func (e *etcdQuorum) Add(member *quorum.Member) error {
	// TODO
	return nil
}

// Remove removes an existing quorum member.
func (e *etcdQuorum) Remove(id uint64) error {
	// TODO
	return nil
}
