// Package integration is a layer on etcd's integration package.
package integration

import (
	"strings"
	"testing"

	"github.com/coreos/etcd/clientv3"
	"github.com/coreos/etcd/integration"
)

// ClusterV3 encapsulates etcd's ClusterV3.
type ClusterV3 struct {
	c *integration.ClusterV3
}

// NewClusterV3 creates and starts a new etcd server and client.
func NewClusterV3(t *testing.T) *ClusterV3 {
	cfg := &integration.ClusterConfig{
		Size: 1,
	}
	return &ClusterV3{
		c: integration.NewClusterV3(t, cfg),
	}
}

// Client returns the client object.
func (c *ClusterV3) Client() *clientv3.Client {
	return c.c.Client(0)
}

// ClientURL returns the client URL of the server.
func (c *ClusterV3) ClientURL() string {
	return strings.Join(c.c.Client(0).Endpoints(), ",")
}

// Terminate cleans up the state created by the test.
func (c *ClusterV3) Terminate(t *testing.T) {
	c.c.Terminate(t)
}
