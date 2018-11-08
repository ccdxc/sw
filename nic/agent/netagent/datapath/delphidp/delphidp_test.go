// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/goproto"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestDelphiMount(t *testing.T) {
	// start the fake delphi hub
	hub := gosdk.NewFakeHub()
	hub.Start()

	// create delphi datapath
	dp, err := NewDelphiDatapath()
	AssertOk(t, err, "Error creating delphi datapath")

	// verify we receive mount complete callback
	AssertEventually(t, func() (bool, interface{}) {
		return dp.IsMountComplete(), dp
	}, "Mounting delphi failed")

	// stop the client and hub
	dp.delphiClient.Close()
	hub.Stop()
	time.Sleep(time.Millisecond)
}

func TestDelphiInterface(t *testing.T) {
	// start the fake delphi hub
	hub := gosdk.NewFakeHub()
	hub.Start()

	// create delphi datapath
	dp, err := NewDelphiDatapath()
	AssertOk(t, err, "Error creating delphi datapath")

	// verify we receive mount complete callback
	AssertEventually(t, func() (bool, interface{}) {
		return dp.IsMountComplete(), dp
	}, "Mounting delphi failed")

	// interface
	intf := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEnic",
		},
		Spec: netproto.InterfaceSpec{
			Type: "ENIC",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 100,
		},
	}

	// lif
	lif := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testLif",
		},
		Spec: netproto.InterfaceSpec{
			Type: "LIF",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 200,
		},
	}

	// create an interface
	err = dp.CreateInterface(intf, lif, nil, nil)
	AssertOk(t, err, "Error creating interface in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		iflist := dp.delphiClient.List("InterfaceSpec")
		log.Infof("Got if : %+v", iflist[0])
		return (len(iflist) == 1), iflist
	}, "invalid number of interfaces")

	// verify interface parameters match
	ifKey := &goproto.InterfaceKeyHandle{
		InterfaceId: intf.Status.InterfaceID,
	}
	ifSpec := goproto.GetInterfaceSpec(dp.delphiClient, ifKey)
	Assert(t, ifSpec != nil, "Error getting interface spec", ifSpec)
	Assert(t, ifSpec.IfType == goproto.IntfType_IF_TYPE_ENIC, "invalid interface type")

	// delete the interface
	err = dp.DeleteInterface(intf, nil)
	AssertOk(t, err, "Error deleting interface")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := dp.delphiClient.List("InterfaceSpec")
		return (len(iflist) == 0), iflist
	}, "interface still found after deleting")
	ifSpec = goproto.GetInterfaceSpec(dp.delphiClient, ifKey)
	Assert(t, ifSpec == nil, "Interface still found after deleting")

	// create an interface status and verify we get the callback
	intfStatus := &goproto.InterfaceStatus{
		KeyOrHandle: ifKey,
	}
	dp.delphiClient.SetObject(intfStatus)
	AssertEventually(t, func() (bool, interface{}) {
		return (dp.eventStats["OnInterfaceStatusUpdate"] >= 1), dp.eventStats
	}, "interface status reactor wasnt called")

	// delete the status and verify we get delete callback
	dp.delphiClient.DeleteObject(intfStatus)
	AssertEventually(t, func() (bool, interface{}) {
		return (dp.eventStats["OnInterfaceStatusDelete"] >= 1), dp.eventStats
	}, "interface status delete reactor wasnt called")

	dp.delphiClient.Close()
	hub.Stop()
	time.Sleep(time.Millisecond)
}

func TestDelphiUplinkInterface(t *testing.T) {
	// start the fake delphi hub
	hub := gosdk.NewFakeHub()
	hub.Start()

	// create delphi datapath
	dp, err := NewDelphiDatapath()
	AssertOk(t, err, "Error creating delphi datapath")

	// verify we receive mount complete callback
	AssertEventually(t, func() (bool, interface{}) {
		return dp.IsMountComplete(), dp
	}, "Mounting delphi failed")

	// interface
	intf := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testUplink",
		},
		Spec: netproto.InterfaceSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: 100,
		},
	}

	// lif
	port := &netproto.Port{
		TypeMeta: api.TypeMeta{Kind: "Port"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testUplink",
		},
		Spec: netproto.PortSpec{
			Type: "UPLINK_ETH",
		},
		Status: netproto.PortStatus{
			PortID: 200,
		},
	}

	// create an interface
	err = dp.CreateInterface(intf, nil, port, nil)
	AssertOk(t, err, "Error creating interface in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		iflist := dp.delphiClient.List("InterfaceSpec")
		log.Infof("Got if : %+v", iflist[0])
		return (len(iflist) == 1), iflist
	}, "invalid number of interfaces")

	// verify interface parameters match
	ifKey := &goproto.InterfaceKeyHandle{
		InterfaceId: intf.Status.InterfaceID,
	}
	ifSpec := goproto.GetInterfaceSpec(dp.delphiClient, ifKey)
	Assert(t, ifSpec != nil, "Error getting interface spec", ifSpec)
	Assert(t, ifSpec.IfType == goproto.IntfType_IF_TYPE_UPLINK, "invalid interface type")

	// delete the interface
	err = dp.DeleteInterface(intf, nil)
	AssertOk(t, err, "Error deleting interface")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := dp.delphiClient.List("InterfaceSpec")
		return (len(iflist) == 0), iflist
	}, "interface still found after deleting")
	ifSpec = goproto.GetInterfaceSpec(dp.delphiClient, ifKey)
	Assert(t, ifSpec == nil, "Interface still found after deleting")

	dp.delphiClient.Close()
	hub.Stop()
	time.Sleep(time.Millisecond)
}

func TestDelphiDatapath(t *testing.T) {
	// start the fake delphi hub
	hub := gosdk.NewFakeHub()
	hub.Start()

	// create delphi datapath
	dp, err := NewDelphiDatapath()
	AssertOk(t, err, "Error creating delphi datapath")

	// verify we receive mount complete callback
	AssertEventually(t, func() (bool, interface{}) {
		return dp.IsMountComplete(), dp
	}, "Mounting delphi failed")

	// call all methods
	dp.SetAgent(nil)
	dp.CreateLocalEndpoint(nil, nil, nil, 0, 0, nil)
	dp.UpdateLocalEndpoint(nil, nil, nil)
	dp.DeleteLocalEndpoint(nil, nil, 0)
	dp.CreateRemoteEndpoint(nil, nil, nil, 0, nil)
	dp.UpdateRemoteEndpoint(nil, nil, nil)
	dp.DeleteRemoteEndpoint(nil, nil)
	dp.CreateNetwork(nil, nil, nil)
	dp.UpdateNetwork(nil, nil)
	dp.DeleteNetwork(nil, nil, nil)
	dp.CreateSecurityGroup(nil)
	dp.UpdateSecurityGroup(nil)
	dp.DeleteSecurityGroup(nil)
	dp.CreateVrf(0, "")
	dp.DeleteVrf(0)
	dp.UpdateVrf(0)
	dp.CreateNatPool(nil, nil)
	dp.UpdateNatPool(nil, nil)
	dp.DeleteNatPool(nil, nil)
	dp.CreateNatPolicy(nil, nil, nil)
	dp.UpdateNatPolicy(nil, nil, nil)
	dp.DeleteNatPolicy(nil, nil)
	dp.CreateRoute(nil, nil)
	dp.UpdateRoute(nil, nil)
	dp.DeleteRoute(nil, nil)
	dp.CreateNatBinding(nil, nil, 0, nil)
	dp.UpdateNatBinding(nil, nil)
	dp.DeleteNatBinding(nil, nil)
	dp.CreateIPSecPolicy(nil, nil, nil)
	dp.UpdateIPSecPolicy(nil, nil)
	dp.DeleteIPSecPolicy(nil, nil)
	dp.CreateIPSecSAEncrypt(nil, nil, nil)
	dp.UpdateIPSecSAEncrypt(nil, nil)
	dp.DeleteIPSecSAEncrypt(nil, nil)
	dp.CreateIPSecSADecrypt(nil, nil, nil)
	dp.UpdateIPSecSADecrypt(nil, nil)
	dp.DeleteIPSecSADecrypt(nil, nil)
	dp.CreateSGPolicy(nil, 0, nil)
	dp.UpdateSGPolicy(nil, 0)
	dp.DeleteSGPolicy(nil, 0)
	dp.CreateTunnel(nil, nil)
	dp.UpdateTunnel(nil, nil)
	dp.DeleteTunnel(nil, nil)
	dp.CreateTCPProxyPolicy(nil, nil)
	dp.UpdateTCPProxyPolicy(nil, nil)
	dp.DeleteTCPProxyPolicy(nil, nil)
	dp.CreatePort(nil)
	dp.ListInterfaces()

	dp.delphiClient.Close()
	hub.Stop()
	time.Sleep(time.Millisecond)
}
