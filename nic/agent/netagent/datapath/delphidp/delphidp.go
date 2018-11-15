// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"sync"

	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/goproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	genproto "github.com/pensando/sw/nic/delphi/proto/goproto"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// DelphiDatapath contains internal state of delphi datapath
type DelphiDatapath struct {
	sync.Mutex
	name          string
	agent         types.DatapathIntf
	delphiClient  clientApi.Client
	sysmgrClient  *sysmgr.Client
	mountComplete bool
	eventStats    map[string]int
}

// NewDelphiDatapath creates a new delphi datapath service
func NewDelphiDatapath() (*DelphiDatapath, error) {
	// create delphi datapath service
	dp := DelphiDatapath{
		name:       "netagent",
		eventStats: make(map[string]int),
	}

	// create delphi client
	cl, err := gosdk.NewClient(&dp)
	if err != nil {
		log.Fatalf("Error creating delphi client. Err: %v", err)
	}

	// create sysmgr client
	dp.sysmgrClient = sysmgr.NewClient(cl, "netagent")

	dp.delphiClient = cl

	// mount objects
	goproto.InterfaceSpecMount(cl, delphi.MountMode_ReadWriteMode)
	goproto.InterfaceStatusMount(cl, delphi.MountMode_ReadMode)
	goproto.LifSpecMount(cl, delphi.MountMode_ReadWriteMode)
	goproto.LifStatusMount(cl, delphi.MountMode_ReadMode)

	// setup watches
	goproto.InterfaceStatusWatch(cl, &dp)

	// connect to delphi hub
	go cl.Run()

	return &dp, nil
}

// OnMountComplete gets called after all the objectes are mounted
func (dp *DelphiDatapath) OnMountComplete() {
	log.Infof("On mount complete got called")

	// let sysmgr know init completed
	dp.sysmgrClient.InitDone()

	// dummy code to test cgo
	genproto.NewExampleMetricsIterator()

	dp.Lock()
	defer dp.Unlock()
	dp.mountComplete = true
}

// Name returns the name of the service
func (dp *DelphiDatapath) Name() string {
	return dp.name
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

// IsMountComplete returns true if delphi on mount complete callback is completed
func (dp *DelphiDatapath) IsMountComplete() bool {
	dp.Lock()
	defer dp.Unlock()
	return dp.mountComplete
}

// SetAgent sets the agent
func (dp *DelphiDatapath) SetAgent(ag types.DatapathIntf) error {
	dp.agent = ag
	return nil
}

// CreateLocalEndpoint creates a local endpoint in datapath
func (dp *DelphiDatapath) CreateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, lifID, enicID uint64, ns *netproto.Namespace) (*types.IntfInfo, error) {
	return nil, nil
}

// UpdateLocalEndpoint updates a local endpoint in datapath
func (dp *DelphiDatapath) UpdateLocalEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error {
	return nil
}

// DeleteLocalEndpoint deletes a local endpoint in datapath
func (dp *DelphiDatapath) DeleteLocalEndpoint(ep *netproto.Endpoint, nw *netproto.Network, enicID uint64) error {
	return nil
}

// CreateRemoteEndpoint creates a remote endpoint in datapath
func (dp *DelphiDatapath) CreateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup, uplinkID uint64, ns *netproto.Namespace) error {
	return nil
}

// UpdateRemoteEndpoint updates a remote endpoint in datapath
func (dp *DelphiDatapath) UpdateRemoteEndpoint(ep *netproto.Endpoint, nt *netproto.Network, sgs []*netproto.SecurityGroup) error {
	return nil
}

// DeleteRemoteEndpoint deletes a remote endpoint in datapath
func (dp *DelphiDatapath) DeleteRemoteEndpoint(ep *netproto.Endpoint, nw *netproto.Network) error {
	return nil
}

// CreateNetwork creates a network
func (dp *DelphiDatapath) CreateNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// UpdateNetwork updates a network in datapath
func (dp *DelphiDatapath) UpdateNetwork(nw *netproto.Network, ns *netproto.Namespace) error {
	return nil
}

// DeleteNetwork deletes a network from datapath
func (dp *DelphiDatapath) DeleteNetwork(nw *netproto.Network, uplinks []*netproto.Interface, ns *netproto.Namespace) error {
	return nil
}

// CreateSecurityGroup creates a security group
func (dp *DelphiDatapath) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// UpdateSecurityGroup updates a security group
func (dp *DelphiDatapath) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	return nil
}

// DeleteSecurityGroup deletes a security group
func (dp *DelphiDatapath) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
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
func (dp *DelphiDatapath) CreateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {
	return nil
}

// UpdateNatPool updates a nat pool in the datapath
func (dp *DelphiDatapath) UpdateNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {
	return nil
}

// DeleteNatPool deletes a nat pool in the datapath
func (dp *DelphiDatapath) DeleteNatPool(np *netproto.NatPool, ns *netproto.Namespace) error {
	return nil
}

// CreateNatPolicy creates a nat policy in the datapath
func (dp *DelphiDatapath) CreateNatPolicy(np *netproto.NatPolicy, npLUT map[string]*types.NatPoolRef, ns *netproto.Namespace) error {
	return nil
}

// UpdateNatPolicy updates a nat policy in the datapath
func (dp *DelphiDatapath) UpdateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, ns *netproto.Namespace) error {
	return nil
}

// DeleteNatPolicy deletes a nat policy in the datapath
func (dp *DelphiDatapath) DeleteNatPolicy(np *netproto.NatPolicy, ns *netproto.Namespace) error {
	return nil
}

// CreateRoute creates a route
func (dp *DelphiDatapath) CreateRoute(rt *netproto.Route, ns *netproto.Namespace) error {
	return nil
}

// UpdateRoute updates a route
func (dp *DelphiDatapath) UpdateRoute(rt *netproto.Route, ns *netproto.Namespace) error {
	return nil
}

// DeleteRoute deletes a route
func (dp *DelphiDatapath) DeleteRoute(rt *netproto.Route, ns *netproto.Namespace) error {
	return nil
}

// CreateNatBinding creates a nat policy in the datapath
func (dp *DelphiDatapath) CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, ns *netproto.Namespace) (*netproto.NatBinding, error) {
	return nil, nil
}

// UpdateNatBinding updates a nat policy in the datapath
func (dp *DelphiDatapath) UpdateNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {
	return nil
}

// DeleteNatBinding deletes a nat policy in the datapath
func (dp *DelphiDatapath) DeleteNatBinding(np *netproto.NatBinding, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecPolicy creates a IPSec policy in the datapath
func (dp *DelphiDatapath) CreateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace, ipSecLUT map[string]*types.IPSecRuleRef) error {
	return nil
}

// UpdateIPSecPolicy updates a IPSec policy in the datapath
func (dp *DelphiDatapath) UpdateIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecPolicy deletes a IPSec policy in the datapath
func (dp *DelphiDatapath) DeleteIPSecPolicy(np *netproto.IPSecPolicy, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSAEncrypt creates a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) CreateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns, tep *netproto.Namespace) error {
	return nil
}

// UpdateIPSecSAEncrypt updates a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) UpdateIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSAEncrypt deletes a IPSecSA encrypt rule in the datapath
func (dp *DelphiDatapath) DeleteIPSecSAEncrypt(np *netproto.IPSecSAEncrypt, ns *netproto.Namespace) error {
	return nil
}

// CreateIPSecSADecrypt creates a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) CreateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns, tep *netproto.Namespace) error {
	return nil
}

// UpdateIPSecSADecrypt updates a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) UpdateIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// DeleteIPSecSADecrypt deletes a IPSecSA decrypt rule in the datapath
func (dp *DelphiDatapath) DeleteIPSecSADecrypt(np *netproto.IPSecSADecrypt, ns *netproto.Namespace) error {
	return nil
}

// CreateSGPolicy creates a security group policy in the datapath
func (dp *DelphiDatapath) CreateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64, sgs []*netproto.SecurityGroup) error {
	return nil
}

// UpdateSGPolicy updates a security group policy in the datapath
func (dp *DelphiDatapath) UpdateSGPolicy(sgp *netproto.SGPolicy, vrfID uint64) error {
	return nil
}

// DeleteSGPolicy deletes sg policy
func (dp *DelphiDatapath) DeleteSGPolicy(sgp *netproto.SGPolicy, vrfID uint64) error {
	return nil
}

// CreateTunnel deletes a security group policy in the datapath
func (dp *DelphiDatapath) CreateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {
	return nil
}

// UpdateTunnel creates a tunnel in the datapath
func (dp *DelphiDatapath) UpdateTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {
	return nil
}

// DeleteTunnel updates a tunnel in the datapath
func (dp *DelphiDatapath) DeleteTunnel(tun *netproto.Tunnel, ns *netproto.Namespace) error {
	return nil
}

// CreateTCPProxyPolicy deletes a tunnel in the datapath
func (dp *DelphiDatapath) CreateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error {
	return nil
}

// UpdateTCPProxyPolicy creates a tcp proxy policy in the datapath
func (dp *DelphiDatapath) UpdateTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error {
	return nil
}

// DeleteTCPProxyPolicy updates a tcp proxy policy in the datapath
func (dp *DelphiDatapath) DeleteTCPProxyPolicy(tcp *netproto.TCPProxyPolicy, ns *netproto.Namespace) error {
	return nil
}

// CreatePort creates port
func (dp *DelphiDatapath) CreatePort(port *netproto.Port) (*netproto.Port, error) {
	return port, nil
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
func (dp *DelphiDatapath) CreateSecurityProfile(profile *netproto.SecurityProfile, vrf *netproto.Namespace) error {
	return nil
}

// UpdateSecurityProfile updates security profile
func (dp *DelphiDatapath) UpdateSecurityProfile(profile *netproto.SecurityProfile) error {
	return nil
}

// DeleteSecurityProfile deletes security profile
func (dp *DelphiDatapath) DeleteSecurityProfile(profile *netproto.SecurityProfile, vrf *netproto.Namespace) error {
	return nil
}

// GetUUID gets the FRU information for the NAPLES from HAL.
func (dp *DelphiDatapath) GetUUID() (string, error) {
	return "", nil
}
