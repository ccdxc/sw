// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
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

// CreateNatPool creates a nat pool in the datapath
func (dp *DelphiDatapath) CreateNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	return nil
}

// UpdateNatPool updates a nat pool in the datapath
func (dp *DelphiDatapath) UpdateNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	return nil
}

// DeleteNatPool deletes a nat pool in the datapath
func (dp *DelphiDatapath) DeleteNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	return nil
}

// CreateNatPolicy creates a nat policy in the datapath
func (dp *DelphiDatapath) CreateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, vrf *netproto.Vrf) error {
	return nil
}

// UpdateNatPolicy updates a nat policy in the datapath
func (dp *DelphiDatapath) UpdateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, vrf *netproto.Vrf) error {
	return nil
}

// DeleteNatPolicy deletes a nat policy in the datapath
func (dp *DelphiDatapath) DeleteNatPolicy(np *netproto.NatPolicy, vrf *netproto.Vrf) error {
	return nil
}

// CreateRoute creates a route
func (dp *DelphiDatapath) CreateRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	return nil
}

// UpdateRoute updates a route
func (dp *DelphiDatapath) UpdateRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	return nil
}

// DeleteRoute deletes a route
func (dp *DelphiDatapath) DeleteRoute(rt *netproto.Route, vrf *netproto.Vrf) error {
	return nil
}

// CreateNatBinding creates a nat policy in the datapath
func (dp *DelphiDatapath) CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, vrf *netproto.Vrf) (*netproto.NatBinding, error) {
	return nil, nil
}

// UpdateNatBinding updates a nat policy in the datapath
func (dp *DelphiDatapath) UpdateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, vrf *netproto.Vrf) error {
	return nil
}

// DeleteNatBinding deletes a nat policy in the datapath
func (dp *DelphiDatapath) DeleteNatBinding(np *netproto.NatBinding, vrf *netproto.Vrf) error {
	return nil
}

// CreateIPSecPolicy creates a IPSec policy in the datapath
func (dp *DelphiDatapath) CreateIPSecPolicy(ipSec *netproto.IPSecPolicy, vrf *netproto.Vrf, ipSecLUT map[string]*types.IPSecRuleRef) error {
	return nil
}

// UpdateIPSecPolicy updates a IPSec policy in the datapath
func (dp *DelphiDatapath) UpdateIPSecPolicy(ipSec *netproto.IPSecPolicy, vrf *netproto.Vrf, ipSecLUT map[string]*types.IPSecRuleRef) error {
	return nil
}

// DeleteIPSecPolicy deletes a IPSec policy in the datapath
func (dp *DelphiDatapath) DeleteIPSecPolicy(np *netproto.IPSecPolicy, vrf *netproto.Vrf) error {
	return nil
}

// CreateIPSecSAEncrypt creates a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, vrf, tepVrf *netproto.Vrf) error {
	return nil
}

// UpdateIPSecSAEncrypt updates a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) UpdateIPSecSAEncrypt(sa *netproto.IPSecSAEncrypt, vrf, tepVrf *netproto.Vrf) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) DeleteIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, vrf *netproto.Vrf) error {
	return nil
}

// CreateIPSecSADecrypt creates a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) CreateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, vrf, tepVrf *netproto.Vrf) error {
	return nil
}

// UpdateIPSecSADecrypt updates a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) UpdateIPSecSADecrypt(sa *netproto.IPSecSADecrypt, vrf, tepVrf *netproto.Vrf) error {
	return nil
}

// DeleteIPSecSADecrypt deletes a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) DeleteIPSecSADecrypt(np *netproto.IPSecSADecrypt, vrf *netproto.Vrf) error {
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

// CreateTCPProxyPolicy deletes a tunnel in the datapath
func (dp *DelphiDatapath) CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
	return nil
}

// UpdateTCPProxyPolicy creates a tcp proxy policy in the datapath
func (dp *DelphiDatapath) UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
	return nil
}

// DeleteTCPProxyPolicy updates a tcp proxy policy in the datapath
func (dp *DelphiDatapath) DeleteTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, vrf *netproto.Vrf) error {
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
