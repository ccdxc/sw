// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package ctrlerif

import (
	"testing"
	"time"

	context "golang.org/x/net/context"

	"github.com/Sirupsen/logrus"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/utils/rpckit"
	. "github.com/pensando/sw/utils/testutils"
)

type fakeAgent struct {
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

func createFakeAgent() *fakeAgent {
	return &fakeAgent{
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
func (ag *fakeAgent) RegisterCtrlerIf(ctrlerif netagent.CtrlerAPI) error {
	return nil
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

func (ag *fakeAgent) CreateEndpoint(ep *netproto.Endpoint) (*netagent.IntfInfo, error) {
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

type fakeRPCServer struct {
	grpcServer *rpckit.RPCServer
	netdp      map[string]*netproto.Network
	epdb       map[string]*netproto.Endpoint
	sgdb       map[string]*netproto.SecurityGroup
}

const testSrvURL = "localhost:8585"

func createRPCServer(t *testing.T) *fakeRPCServer {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer("netctrler", testSrvURL, "", "", "")
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

		// send create event
		err := stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
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
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
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
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send update event
		watchEvt.EventType = api.EventType_UpdateEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}

		// send delete event
		watchEvt.EventType = api.EventType_DeleteEvent
		err = stream.Send(&watchEvt)
		if err != nil {
			logrus.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	return nil
}

func TestNpmclient(t *testing.T) {
	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)

	// create a fake agent
	ag := createFakeAgent()

	// create npm client
	cl, err := NewNpmClient(ag, testSrvURL)
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
	ag := createFakeAgent()

	// create npm client
	cl, err := NewNpmClient(ag, testSrvURL)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the network
	time.Sleep(time.Millisecond * 10)
	Assert(t, ag.netAdded[objectKey(nt.ObjectMeta)].Name == nt.Name, "Network not found in agent", ag)
	Assert(t, ag.netUpdated[objectKey(nt.ObjectMeta)].Name == nt.Name, "Network not found in agent", ag)
	Assert(t, ag.netDeleted[objectKey(nt.ObjectMeta)].Name == nt.Name, "Network not found in agent", ag)
	Assert(t, ag.epAdded[objectKey(epinfo.ObjectMeta)].Name == epinfo.Name, "Endpoint not found in agent", ag)
	Assert(t, ag.epUpdated[objectKey(epinfo.ObjectMeta)].Name == epinfo.Name, "Endpoint not found in agent", ag)
	Assert(t, ag.epDeleted[objectKey(epinfo.ObjectMeta)].Name == epinfo.Name, "Endpoint not found in agent", ag)

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
	time.Sleep(time.Millisecond * 100)
}

func TestSecurityGroupWatch(t *testing.T) {
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
	ag := createFakeAgent()

	// create npm client
	cl, err := NewNpmClient(ag, testSrvURL)
	AssertOk(t, err, "Error creating npm client")
	Assert(t, (cl != nil), "Error creating npm client")

	// verify client got the security group
	time.Sleep(time.Millisecond * 10)
	Assert(t, ag.sgAdded[objectKey(sg.ObjectMeta)].Name == sg.Name, "Security group not found in agent", ag)
	Assert(t, ag.sgUpdated[objectKey(sg.ObjectMeta)].Name == sg.Name, "Security group not found in agent", ag)
	Assert(t, ag.sgDeleted[objectKey(sg.ObjectMeta)].Name == sg.Name, "Security group not found in agent", ag)

	// stop the server and client
	cl.Stop()
	srv.grpcServer.Stop()
	time.Sleep(time.Millisecond * 100)
}
