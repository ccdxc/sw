// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"testing"

	context "golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

type fakeAgent struct {
	name       string
	netAdded   map[string]*netproto.Network
	netUpdated map[string]*netproto.Network
	netDeleted map[string]*netproto.Network

	epAdded   map[string]*netproto.Endpoint
	epUpdated map[string]*netproto.Endpoint
	epDeleted map[string]*netproto.Endpoint

	sgAdded   map[string]*netproto.SecurityGroup
	sgUpdated map[string]*netproto.SecurityGroup
	sgDeleted map[string]*netproto.SecurityGroup
}

func createFakeAgent(name string) *fakeAgent {
	return &fakeAgent{
		name:       name,
		netAdded:   make(map[string]*netproto.Network),
		netUpdated: make(map[string]*netproto.Network),
		netDeleted: make(map[string]*netproto.Network),
		epAdded:    make(map[string]*netproto.Endpoint),
		epUpdated:  make(map[string]*netproto.Endpoint),
		epDeleted:  make(map[string]*netproto.Endpoint),
		sgAdded:    make(map[string]*netproto.SecurityGroup),
		sgUpdated:  make(map[string]*netproto.SecurityGroup),
		sgDeleted:  make(map[string]*netproto.SecurityGroup),
	}
}
func (ag *fakeAgent) RegisterCtrlerIf(ctrlerif state.CtrlerAPI) error {
	return nil
}

func (ag *fakeAgent) GetAgentID() string {
	return "fakeAgent_" + ag.name
}
func (ag *fakeAgent) CreateNetwork(nt *netproto.Network) error {
	ag.netAdded[objectKey(nt.ObjectMeta)] = nt
	return nil
}

func (ag *fakeAgent) UpdateNetwork(nt *netproto.Network) error {
	ag.netUpdated[objectKey(nt.ObjectMeta)] = nt
	return nil
}

func (ag *fakeAgent) DeleteNetwork(nt *netproto.Network) error {
	ag.netDeleted[objectKey(nt.ObjectMeta)] = nt
	return nil
}

func (ag *fakeAgent) ListNetwork() []*netproto.Network {
	var netlist []*netproto.Network

	// walk all networks
	for _, nw := range ag.netAdded {
		netlist = append(netlist, nw)
	}

	return netlist
}

func (ag *fakeAgent) FindNetwork(meta api.ObjectMeta) (*netproto.Network, error) {
	return nil, nil
}

func (ag *fakeAgent) CreateEndpoint(ep *netproto.Endpoint) (*state.IntfInfo, error) {
	ag.epAdded[objectKey(ep.ObjectMeta)] = ep
	return nil, nil
}

func (ag *fakeAgent) UpdateEndpoint(ep *netproto.Endpoint) error {
	ag.epUpdated[objectKey(ep.ObjectMeta)] = ep
	return nil
}

func (ag *fakeAgent) DeleteEndpoint(ep *netproto.Endpoint) error {
	ag.epDeleted[objectKey(ep.ObjectMeta)] = ep
	return nil
}

func (ag *fakeAgent) ListEndpoint() []*netproto.Endpoint {
	var eplist []*netproto.Endpoint

	// walk all endpoints
	for _, ep := range ag.epAdded {
		eplist = append(eplist, ep)
	}

	return eplist
}

func (ag *fakeAgent) CreateSecurityGroup(sg *netproto.SecurityGroup) error {
	ag.sgAdded[objectKey(sg.ObjectMeta)] = sg
	return nil
}

func (ag *fakeAgent) UpdateSecurityGroup(sg *netproto.SecurityGroup) error {
	ag.sgUpdated[objectKey(sg.ObjectMeta)] = sg
	return nil
}

func (ag *fakeAgent) DeleteSecurityGroup(sg *netproto.SecurityGroup) error {
	ag.sgDeleted[objectKey(sg.ObjectMeta)] = sg
	return nil
}

func (ag *fakeAgent) ListSecurityGroup() []*netproto.SecurityGroup {
	var sglist []*netproto.SecurityGroup

	// walk all sgs
	for _, sg := range ag.sgAdded {
		sglist = append(sglist, sg)
	}

	return sglist
}

// CreateTenant creates a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) CreateTenant(tn *netproto.Tenant) error {
	return nil
}

// DeleteTenant deletes a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) DeleteTenant(tn *netproto.Tenant) error {
	return nil
}

// ListTenant lists tenants. Stubbed out to satisfy the interface
func (ag *fakeAgent) ListTenant() []*netproto.Tenant {
	return nil
}

// UpdateTenant updates a tenant. Stubbed out to satisfy the interface
func (ag *fakeAgent) UpdateTenant(tn *netproto.Tenant) error {
	return nil
}

type fakeRPCServer struct {
	grpcServer *rpckit.RPCServer
	netdp      map[string]*netproto.Network
	epdb       map[string]*netproto.Endpoint
	sgdb       map[string]*netproto.SecurityGroup
}

func createRPCServer(t *testing.T) *fakeRPCServer {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer("netctrler", ":0")
	if err != nil {
		t.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// create fake rpc server
	srv := fakeRPCServer{
		grpcServer: grpcServer,
		netdp:      make(map[string]*netproto.Network),
		epdb:       make(map[string]*netproto.Endpoint),
		sgdb:       make(map[string]*netproto.SecurityGroup),
	}

	// register self as rpc handler
	netproto.RegisterNetworkApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterEndpointApiServer(grpcServer.GrpcServer, &srv)
	netproto.RegisterSecurityApiServer(grpcServer.GrpcServer, &srv)
	grpcServer.Start()

	return &srv
}

func (srv *fakeRPCServer) GetNetwork(context.Context, *api.ObjectMeta) (*netproto.Network, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListNetworks(context.Context, *api.ObjectMeta) (*netproto.NetworkList, error) {
	return nil, nil
}

func (srv *fakeRPCServer) WatchNetworks(meta *api.ObjectMeta, stream netproto.NetworkApi_WatchNetworksServer) error {
	// walk local db and send stream resp
	for _, net := range srv.netdp {
		// watch event
		watchEvt := netproto.NetworkEvent{
			EventType: api.EventType_CreateEvent,
			Network:   *net,
		}
		watchEvtList := netproto.NetworkEventList{NetworkEvents: []*netproto.NetworkEvent{&watchEvt}}

		// send create event
		err := stream.Send(&watchEvtList)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		watchEvtList = netproto.NetworkEventList{NetworkEvents: []*netproto.NetworkEvent{&watchEvt}}
		err = stream.Send(&watchEvtList)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		watchEvtList = netproto.NetworkEventList{NetworkEvents: []*netproto.NetworkEvent{&watchEvt}}
		err = stream.Send(&watchEvtList)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) CreateEndpoint(ctx context.Context, ep *netproto.Endpoint) (*netproto.Endpoint, error) {
	return ep, nil
}

func (srv *fakeRPCServer) GetEndpoint(context.Context, *api.ObjectMeta) (*netproto.Endpoint, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListEndpoints(context.Context, *api.ObjectMeta) (*netproto.EndpointList, error) {
	return nil, nil
}

func (srv *fakeRPCServer) DeleteEndpoint(ctx context.Context, ep *netproto.Endpoint) (*netproto.Endpoint, error) {
	return ep, nil
}

func (srv *fakeRPCServer) WatchEndpoints(meta *api.ObjectMeta, stream netproto.EndpointApi_WatchEndpointsServer) error {
	// walk local db and send stream resp
	for _, ep := range srv.epdb {
		// watch event
		watchEvt := netproto.EndpointEvent{
			EventType: api.EventType_CreateEvent,
			Endpoint:  *ep,
		}

		// send create event
		err := stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func (srv *fakeRPCServer) GetSecurityGroup(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SecurityGroup, error) {
	return nil, nil
}

func (srv *fakeRPCServer) ListSecurityGroups(context.Context, *api.ObjectMeta) (*netproto.SecurityGroupList, error) {
	return nil, nil
}

func (srv *fakeRPCServer) WatchSecurityGroups(sel *api.ObjectMeta, stream netproto.SecurityApi_WatchSecurityGroupsServer) error {
	// walk local db and send stream resp
	for _, sg := range srv.sgdb {
		// watch event
		watchEvt := netproto.SecurityGroupEvent{
			EventType:     api.EventType_CreateEvent,
			SecurityGroup: *sg,
		}

		// send create event
		err := stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func TestNpmclient(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})
	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")

	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}

	// verify we can make an rpc call from client to server
	_, err = cl.EndpointCreateReq(&epinfo)
	AssertOk(t, err, "Error making endpoint create request")
	_, err = cl.EndpointDeleteReq(&epinfo)
	AssertOk(t, err, "Error making endpoint delete request")

	// stop the server
	srv.grpcServer.Stop()

	// verify rpc call returns error
	// verify we can make an rpc call from client to server
	_, err = cl.EndpointCreateReq(&epinfo)
	Assert(t, (err != nil), "endpoint create request suceeded while expecting it to fail")
	_, err = cl.EndpointDeleteReq(&epinfo)
	Assert(t, (err != nil), "endpoint delete request suceeded while expecting it to fail")

	// stop the client
	cl.Stop()
}

func TestNpmClientWatch(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})
	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create a network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testNetwork",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.1.1.0/24",
			IPv4Gateway: "10.1.1.254",
		},
	}
	srv.netdp["testNetwork"] = &nt

	// create an endpoint
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID: "testEndpointUUID",
		},
		Status: netproto.EndpointStatus{},
	}
	srv.epdb["testEndpoint"] = &epinfo

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// create http REST server
	restSrv, err := restapi.NewRestServer(ag, ":0")
	AssertOk(t, err, "Error creating the rest server")

	// verify client got the network & ep
	AssertEventually(t, func() (bool, []interface{}) {
		nw := ag.netAdded[objectKey(nt.ObjectMeta)]
		return (nw != nil && nw.Name == nt.Name), nil
	}, "Network add not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		nw := ag.netUpdated[objectKey(nt.ObjectMeta)]
		return (nw != nil && nw.Name == nt.Name), nil
	}, "Network update not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		nw := ag.netDeleted[objectKey(nt.ObjectMeta)]
		return (nw != nil && nw.Name == nt.Name), nil
	}, "Network delete not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		ep, ok := ag.epAdded[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Endpoint add not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		ep, ok := ag.epUpdated[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Endpoint update not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		ep, ok := ag.epDeleted[objectKey(epinfo.ObjectMeta)]
		return (ok && ep.Name == epinfo.Name), nil
	}, "Endpoint delete not found in agent")

	// verify the network REST api
	var netList []*netproto.Network
	restSrvURL := restSrv.GetListenURL()
	err = netutils.HTTPGet("http://"+restSrvURL+"/api/networks/", &netList)
	AssertOk(t, err, "Error getting networks from REST server")
	Assert(t, len(netList) == 1, "Incorrect number of networks")

	// verify endpoint REST api
	var epList []*netproto.Endpoint
	err = netutils.HTTPGet("http://"+restSrvURL+"/api/endpoints/", &epList)
	AssertOk(t, err, "Error getting endpoints from REST server")
	Assert(t, len(epList) == 1, "Incorrect number of endpoints")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
	restSrv.Stop()
}

func TestSecurityGroupWatch(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})
	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create an sg
	sg := netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsg",
		},
		Spec: netproto.SecurityGroupSpec{
			SecurityProfile: "unknown",
			Rules:           []netproto.SecurityRule{},
		},
	}
	srv.sgdb["testsg"] = &sg

	// create a fake agent
	ag := createFakeAgent(t.Name())

	// create npm client
	cl, err := NewNpmClient(ag, srv.grpcServer.GetListenURL(), nil)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security group
	AssertEventually(t, func() (bool, []interface{}) {
		sgs, ok := ag.sgAdded[objectKey(sg.ObjectMeta)]
		return (ok && sgs.Name == sg.Name), nil
	}, "Security group add not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		sgs, ok := ag.sgUpdated[objectKey(sg.ObjectMeta)]
		return (ok && sgs.Name == sg.Name), nil
	}, "Security group update not found in agent")
	AssertEventually(t, func() (bool, []interface{}) {
		sgs, ok := ag.sgDeleted[objectKey(sg.ObjectMeta)]
		return (ok && sgs.Name == sg.Name), nil
	}, "Security group delete not found in agent")

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
}
