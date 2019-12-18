// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// DelphiDatapath contains internal state of delphi datapath
type DelphiDatapath struct {
	sync.Mutex
	agent        types.DatapathIntf
	delphiClient clientApi.Client
	eventStats   map[string]int
}

// NewDelphiDatapath creates a new delphi datapath service
func NewDelphiDatapath(delphiClient clientApi.Client) (*DelphiDatapath, error) {
	// create delphi datapath service
	dp := DelphiDatapath{
		eventStats:   make(map[string]int),
		delphiClient: delphiClient,
	}

	// mount objects
	halproto.InterfaceSpecMount(dp.delphiClient, delphi.MountMode_ReadWriteMode)
	halproto.InterfaceStatusMount(dp.delphiClient, delphi.MountMode_ReadMode)
	halproto.LifSpecMount(dp.delphiClient, delphi.MountMode_ReadWriteMode)
	halproto.LifStatusMount(dp.delphiClient, delphi.MountMode_ReadMode)

	// setup watches
	halproto.InterfaceStatusWatch(dp.delphiClient, &dp)

	return &dp, nil
}

func (dp *DelphiDatapath) incrEventStats(evtName string) {
	dp.Lock()
	defer dp.Unlock()
	if _, ok := dp.eventStats[evtName]; ok {
		dp.eventStats[evtName]++
	} else {
		dp.eventStats[evtName] = 1
	}
}

// SetAgent sets the agent
func (dp *DelphiDatapath) SetAgent(ag types.DatapathIntf) error {
	dp.agent = ag
	return nil
}

// CreateVrf creates a vrf
func (dp *DelphiDatapath) CreateVrf(vrfID uint64, vrfType string) error {
	return nil
}

// DeleteVrf deletes a vrf
func (dp *DelphiDatapath) DeleteVrf(vrfID uint64) error {
	return nil
}

// UpdateVrf updates a vrf
func (dp *DelphiDatapath) UpdateVrf(vrfID uint64) error {
	return nil
}

// CreateTunnel deletes a security group policy in the datapath
func (dp *DelphiDatapath) CreateTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	return nil
}

// UpdateTunnel creates a tunnel in the datapath
func (dp *DelphiDatapath) UpdateTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	return nil
}

// DeleteTunnel updates a tunnel in the datapath
func (dp *DelphiDatapath) DeleteTunnel(tun *netproto.Tunnel, vrf *netproto.Vrf) error {
	return nil
}

// CreatePort creates port
func (dp *DelphiDatapath) CreatePort(ports ...*netproto.Port) error {
	return nil
}

// UpdatePort updates port
func (dp *DelphiDatapath) UpdatePort(port *netproto.Port) (*netproto.Port, error) {
	return port, nil
}

// DeletePort creates port
func (dp *DelphiDatapath) DeletePort(port *netproto.Port) error {
	return nil
}

// CreateSecurityProfile creates security profile
func (dp *DelphiDatapath) CreateSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	return nil
}

// UpdateSecurityProfile updates security profile
func (dp *DelphiDatapath) UpdateSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	return nil
}

// DeleteSecurityProfile deletes security profile
func (dp *DelphiDatapath) DeleteSecurityProfile(profile *netproto.SecurityProfile, attachmentVrfs []*netproto.Vrf) error {
	return nil
}

// GetUUID gets the FRU information for the NAPLES from HAL.
func (dp *DelphiDatapath) GetUUID() (string, error) {
	return "", nil
}
