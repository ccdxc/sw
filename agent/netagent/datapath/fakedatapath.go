// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package datapath

import (
	"fmt"

	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/agent/netagent/netutils"
	"github.com/prometheus/common/log"
)

// FakeDatapath has the fake datapath for testing purposes
type FakeDatapath struct {
	currVethIdx int // current Veth number
}

// NewFakeDatapath returns a new fake datapath
func NewFakeDatapath() (*FakeDatapath, error) {
	// create new fake datapath
	fdp := FakeDatapath{
		currVethIdx: 1,
	}

	return &fdp, nil
}

func (fdp *FakeDatapath) CreateEndpoint(ep *netagent.EndpointState) (*netagent.IntfInfo, error) {
	// container and switch interface names
	cintf := fmt.Sprintf("cvport-%s", ep.EndpointInfo.EndpointUUID[:8])
	sintf := fmt.Sprintf("svport-%s", ep.EndpointInfo.EndpointUUID[:8])

	// if the veth interfaces already exists, delete it
	netutils.DeleteVethPair(cintf, sintf)

	// create Veth pairs
	err := netutils.CreateVethPair(cintf, sintf)
	if err != nil {
		log.Errorf("Error creating veth pair: %s/%s. Err: %v", cintf, sintf, err)
		return nil, err
	}

	// create an interface info
	intfInfo := netagent.IntfInfo{
		ContainerIntfName: cintf,
		SwitchIntfName:    sintf,
	}

	// increment current veth idx
	fdp.currVethIdx++

	return &intfInfo, nil
}

func (fdp *FakeDatapath) DeleteEndpoint(ep *netagent.EndpointState) error {
	// container and switch interface names
	cintf := fmt.Sprintf("cvport-%s", ep.EndpointInfo.EndpointUUID[:8])
	sintf := fmt.Sprintf("svport-%s", ep.EndpointInfo.EndpointUUID[:8])

	// delete veth pair
	return netutils.DeleteVethPair(cintf, sintf)
}
