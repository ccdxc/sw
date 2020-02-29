package objects

import (
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
)

//  iterator functions
type HostIteratorFn func(*HostCollection) error
type VeniceNodeIteratorFn func(*VeniceNodeCollection) error
type NaplesIteratorFn func(*NaplesCollection) error
type VeniceContainerRunCommandFn func(string) (string, string, int)

type CollectionCommon struct {
	Client  objClient.ObjClient
	Testbed *testbed.TestBed
	err     error
}

func (c *CollectionCommon) SetError(err error) {
	c.err = err
}

func (c *CollectionCommon) HasError() bool {
	return c.err != nil
}

func (c *CollectionCommon) Error() error {
	return c.err
}

//ConnectionOptions options for fuz connections
type ConnectionOptions struct {
	NumConns          int
	Port              string
	Proto             string
	Duration          string
	ReconnectAttempts int32
	Cps               int //Connections per second
}
