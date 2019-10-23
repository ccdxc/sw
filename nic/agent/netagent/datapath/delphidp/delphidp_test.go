// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package delphidp

import (
	"testing"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/datapath/delphidp/halproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
type delphidpTestSuite struct {
	hub      gosdk.Hub
	datapath *DelphiDatapath
}

// OnMountComplete gets called after all the objectes are mounted
func (ds *delphidpTestSuite) OnMountComplete() {
	log.Infof("On mount complete got called")
}

// Name returns the name of the service
func (ds *delphidpTestSuite) Name() string {
	return "Delphi Unit Tests"
}

// Hook up gocheck into the "go test" runner.
func TestDelphiDatapath(t *testing.T) {
	// integ test suite
	var sts = &delphidpTestSuite{}

	var _ = Suite(sts)
	TestingT(t)
}

func (ds *delphidpTestSuite) SetUpSuite(c *C) {
	// start the fake delphi hub
	ds.hub = gosdk.NewFakeHub()
	ds.hub.Start()

	// start the fake delphi client
	cl, err := gosdk.NewClient(ds)
	if err != nil {
		log.Fatalf("Error creating delphi client. Err: %v", err)
	}

	// create delphi datapath
	dp, err := NewDelphiDatapath(cl)
	AssertOk(c, err, "Error creating delphi datapath")
	ds.datapath = dp

	go cl.Run()
}

func (ds *delphidpTestSuite) TearDownSuite(c *C) {
	ds.datapath.delphiClient.Close()
	ds.hub.Stop()
	time.Sleep(time.Millisecond)
}

func (ds *delphidpTestSuite) TestDelphiInterface(t *C) {
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

	// create an interface
	err := ds.datapath.CreateInterface(intf)
	AssertOk(t, err, "Error creating interface in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("InterfaceSpec")
		log.Infof("Got if : %+v", iflist[0])
		return (len(iflist) == 1), iflist
	}, "invalid number of interfaces")

	// verify interface parameters match
	ifKey := &halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: intf.Status.InterfaceID,
		},
	}
	ifSpec := halproto.GetInterfaceSpec(ds.datapath.delphiClient, ifKey)
	Assert(t, ifSpec != nil, "Error getting interface spec", ifSpec)
	Assert(t, ifSpec.Type == halproto.IfType_IF_TYPE_ENIC, "invalid interface type")

	// delete the interface
	err = ds.datapath.DeleteInterface(intf)
	AssertOk(t, err, "Error deleting interface")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("InterfaceSpec")
		return (len(iflist) == 0), iflist
	}, "interface still found after deleting")
	ifSpec = halproto.GetInterfaceSpec(ds.datapath.delphiClient, ifKey)
	Assert(t, ifSpec == nil, "Interface still found after deleting")

	// create an interface status and verify we get the callback
	intfStatus := &halproto.InterfaceStatus{
		KeyOrHandle: ifKey,
	}
	ds.datapath.delphiClient.SetObject(intfStatus)
	AssertEventually(t, func() (bool, interface{}) {
		return (ds.datapath.eventStats["OnInterfaceStatusUpdate"] >= 1), ds.datapath.eventStats
	}, "interface status reactor wasnt called")

	// delete the status and verify we get delete callback
	ds.datapath.delphiClient.DeleteObject(intfStatus)
	AssertEventually(t, func() (bool, interface{}) {
		return (ds.datapath.eventStats["OnInterfaceStatusDelete"] >= 1), ds.datapath.eventStats
	}, "interface status delete reactor wasnt called")
}

func (ds *delphidpTestSuite) TestDelphiUplinkInterface(t *C) {
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
			IFUplinkStatus: &netproto.InterfaceUplinkStatus{
				PortID: uint32(port.Status.PortID),
			},
		},
	}

	// create an interface
	err := ds.datapath.CreateInterface(intf)
	AssertOk(t, err, "Error creating interface in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("InterfaceSpec")
		log.Infof("Got if : %+v", iflist[0])
		return (len(iflist) == 1), iflist
	}, "invalid number of interfaces")

	// verify interface parameters match
	ifKey := &halproto.InterfaceKeyHandle{
		KeyOrHandle: &halproto.InterfaceKeyHandle_InterfaceId{
			InterfaceId: intf.Status.InterfaceID,
		},
	}
	ifSpec := halproto.GetInterfaceSpec(ds.datapath.delphiClient, ifKey)
	Assert(t, ifSpec != nil, "Error getting interface spec", ifSpec)
	Assert(t, ifSpec.Type == halproto.IfType_IF_TYPE_UPLINK, "invalid interface type")

	// delete the interface
	err = ds.datapath.DeleteInterface(intf)
	AssertOk(t, err, "Error deleting interface")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("InterfaceSpec")
		return (len(iflist) == 0), iflist
	}, "interface still found after deleting")
	ifSpec = halproto.GetInterfaceSpec(ds.datapath.delphiClient, ifKey)
	Assert(t, ifSpec == nil, "Interface still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiRemoteEndpoint(t *C) {
	// vrf
	vrf := &netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNamespace",
			Name:      "testTenant",
		},
	}

	// network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// endpoint
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			MacAddress:   "4242.4242.4242",
			IPv4Address:  "10.1.1.1/24",
			IPv4Gateway:  "20.1.1.254",
		},
	}

	// create an endpoint
	err := ds.datapath.CreateRemoteEndpoint(&epinfo, &nt, nil, 0, vrf)
	AssertOk(t, err, "Error creating endpoint in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		eplist := ds.datapath.delphiClient.List("EndpointSpec")
		log.Infof("Got endpoint : %+v", eplist[0])
		return (len(eplist) == 1), eplist
	}, "invalid number of endpoints")

	err = ds.datapath.UpdateRemoteEndpoint(&epinfo, &nt, nil)
	AssertOk(t, err, "Error updating endpoint in delphi")

	// delete the interface
	err = ds.datapath.DeleteRemoteEndpoint(&epinfo, &nt)
	AssertOk(t, err, "Error deleting endpoint")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("EndpointSpec")
		return (len(iflist) == 0), iflist
	}, "endpoint still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiLocalEndpoint(t *C) {
	// vrf
	vrf := &netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNamespace",
			Name:      "testTenant",
		},
	}

	// network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// endpoint
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "testEndpoint",
			Namespace: "default",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
			WorkloadUUID: "testWorkloadUUID",
			NetworkName:  "default",
			MacAddress:   "4242.4242.4242",
			IPv4Address:  "10.1.1.1/24",
			IPv4Gateway:  "20.1.1.254",
		},
	}

	// create an endpoint
	_, err := ds.datapath.CreateLocalEndpoint(&epinfo, &nt, nil, 0, 0, vrf)
	AssertOk(t, err, "Error creating endpoint in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		eplist := ds.datapath.delphiClient.List("EndpointSpec")
		log.Infof("Got endpoint : %+v", eplist[0])
		return (len(eplist) == 1), eplist
	}, "invalid number of endpoints")

	err = ds.datapath.UpdateLocalEndpoint(&epinfo, &nt, nil)
	AssertOk(t, err, "Error updating endpoint in delphi")

	// delete the interface
	err = ds.datapath.DeleteLocalEndpoint(&epinfo, &nt, 0)
	AssertOk(t, err, "Error deleting endpoint")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		iflist := ds.datapath.delphiClient.List("EndpointSpec")
		return (len(iflist) == 0), iflist
	}, "endpoint still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiNetwork(t *C) {
	// vrf
	vrf := &netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNamespace",
			Name:      "testTenant",
		},
	}
	// uplink
	intf := netproto.Interface{
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

	// network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "default",
			Namespace: "default",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
			RouterMAC:   "0001.0203.0405",
		},
		Status: netproto.NetworkStatus{
			NetworkID: 100,
		},
	}

	// create a network
	err := ds.datapath.CreateNetwork(&nt, []*netproto.Interface{&intf}, vrf)
	AssertOk(t, err, "Error creating network in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		ntlist := ds.datapath.delphiClient.List("NetworkSpec")
		log.Infof("Got network : %+v", ntlist[0])
		return (len(ntlist) == 1), ntlist
	}, "invalid number of networks")

	err = ds.datapath.UpdateNetwork(&nt, []*netproto.Interface{&intf}, vrf)
	AssertOk(t, err, "Error updating network in delphi")

	// delete the network
	err = ds.datapath.DeleteNetwork(&nt, nil, vrf)
	AssertOk(t, err, "Error deleting network")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		ntlist := ds.datapath.delphiClient.List("NetworkSpec")
		return (len(ntlist) == 0), ntlist
	}, "network still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiSecurityGroup(t *C) {
	// security group
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "test-sg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
		},
	}

	// create a network
	err := ds.datapath.CreateSecurityGroup(&sg)
	AssertOk(t, err, "Error creating sg in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		sglist := ds.datapath.delphiClient.List("SecurityGroupSpec")
		log.Infof("Got sg : %+v", sglist[0])
		return (len(sglist) == 1), sglist
	}, "invalid number of sgs")

	err = ds.datapath.UpdateSecurityGroup(&sg)
	AssertOk(t, err, "Error updating sg in delphi")

	// delete the network
	err = ds.datapath.DeleteSecurityGroup(&sg)
	AssertOk(t, err, "Error deleting sg")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		sglist := ds.datapath.delphiClient.List("SecurityGroupSpec")
		return (len(sglist) == 0), sglist
	}, "sg still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiSgPolicy(t *C) {
	// sg policy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "icmp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
				{
					Action: "DENY",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0/24"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80-81",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1/24"},
					},
				},
				{
					Action: "REJECT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
				},
				{
					Action: "LOG",
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
				},
				{
					Action: "LOG",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80-90",
								Protocol: "tcp",
							},
						},
					},
				},
				{
					Action: "PERMIT",
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1"},
					},
				},
			},
		},
	}

	// create a sg policy
	err := ds.datapath.CreateNetworkSecurityPolicy(&sgPolicy, 100, nil, nil)
	AssertOk(t, err, "Error creating sgpolicy in delphi")

	AssertEventually(t, func() (bool, interface{}) {
		sgplist := ds.datapath.delphiClient.List("SecurityPolicySpec")
		log.Infof("Got sg : %+v", sgplist[0])
		return (len(sgplist) == 1), sgplist
	}, "invalid number of sg policies")

	err = ds.datapath.UpdateNetworkSecurityPolicy(&sgPolicy, 100, nil)
	AssertOk(t, err, "Error updating sg policy in delphi")

	// delete the network
	err = ds.datapath.DeleteNetworkSecurityPolicy(&sgPolicy, 100)
	AssertOk(t, err, "Error deleting sg policy")

	// verify interface is acutually gone
	AssertEventually(t, func() (bool, interface{}) {
		sgplist := ds.datapath.delphiClient.List("SecurityPolicySpec")
		return (len(sgplist) == 0), sgplist
	}, "sg policy still found after deleting")
}

func (ds *delphidpTestSuite) TestDelphiDatapathStubs(c *C) {

	// call all methods
	ds.datapath.SetAgent(nil)
	ds.datapath.CreateVrf(0, "")
	ds.datapath.DeleteVrf(0)
	ds.datapath.UpdateVrf(0)
	ds.datapath.CreateNatPool(nil, nil)
	ds.datapath.UpdateNatPool(nil, nil)
	ds.datapath.DeleteNatPool(nil, nil)
	ds.datapath.CreateNatPolicy(nil, nil, nil)
	ds.datapath.UpdateNatPolicy(nil, nil, nil)
	ds.datapath.DeleteNatPolicy(nil, nil)
	ds.datapath.CreateRoute(nil, nil)
	ds.datapath.UpdateRoute(nil, nil)
	ds.datapath.DeleteRoute(nil, nil)
	ds.datapath.CreateNatBinding(nil, nil, 0, nil)
	ds.datapath.UpdateNatBinding(nil, nil, 0, nil)
	ds.datapath.DeleteNatBinding(nil, nil)
	ds.datapath.CreateIPSecPolicy(nil, nil, nil)
	ds.datapath.UpdateIPSecPolicy(nil, nil, nil)
	ds.datapath.DeleteIPSecPolicy(nil, nil)
	ds.datapath.CreateIPSecSAEncrypt(nil, nil, nil)
	ds.datapath.UpdateIPSecSAEncrypt(nil, nil, nil)
	ds.datapath.DeleteIPSecSAEncrypt(nil, nil)
	ds.datapath.CreateIPSecSADecrypt(nil, nil, nil)
	ds.datapath.UpdateIPSecSADecrypt(nil, nil, nil)
	ds.datapath.DeleteIPSecSADecrypt(nil, nil)
	ds.datapath.CreateTunnel(nil, nil)
	ds.datapath.UpdateTunnel(nil, nil)
	ds.datapath.DeleteTunnel(nil, nil)
	ds.datapath.CreateTCPProxyPolicy(nil, nil)
	ds.datapath.UpdateTCPProxyPolicy(nil, nil)
	ds.datapath.DeleteTCPProxyPolicy(nil, nil)
	ds.datapath.CreatePort(nil)
	ds.datapath.ListInterfaces()
}
