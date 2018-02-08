// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cmdif

import (
	"net/http"
	"testing"

	context "golang.org/x/net/context"

	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

type mockAgent struct {
	name string
	sync.Mutex
	nic        *cmd.SmartNIC
	nicAdded   map[string]*cmd.SmartNIC
	nicUpdated map[string]*cmd.SmartNIC
	nicDeleted map[string]*cmd.SmartNIC
}

func createMockAgent(name string) *mockAgent {
	return &mockAgent{
		name:       name,
		nicAdded:   make(map[string]*cmd.SmartNIC),
		nicUpdated: make(map[string]*cmd.SmartNIC),
		nicDeleted: make(map[string]*cmd.SmartNIC),
	}
}
func (ag *mockAgent) RegisterCMD(cmd state.CmdAPI) error {
	return nil
}

func (ag *mockAgent) NaplesConfigHandler(req *http.Request) (interface{}, error) {
	return nil, nil
}

func (ag *mockAgent) GetAgentID() string {
	return "mockAgent_" + ag.name
}

func (ag *mockAgent) CreateSmartNIC(n *cmd.SmartNIC) error {
	ag.Lock()
	defer ag.Unlock()
	ag.nicAdded[objectKey(n.ObjectMeta)] = n
	return nil
}

func (ag *mockAgent) UpdateSmartNIC(n *cmd.SmartNIC) error {
	ag.Lock()
	defer ag.Unlock()
	ag.nicUpdated[objectKey(n.ObjectMeta)] = n
	return nil
}

func (ag *mockAgent) DeleteSmartNIC(n *cmd.SmartNIC) error {
	ag.Lock()
	defer ag.Unlock()
	ag.nicDeleted[objectKey(n.ObjectMeta)] = n
	return nil
}

func (ag *mockAgent) GetSmartNIC() (*cmd.SmartNIC, error) {
	ag.Lock()
	defer ag.Unlock()
	return ag.nic, nil
}

func (ag *mockAgent) SetSmartNIC(nic *cmd.SmartNIC) error {
	ag.Lock()
	defer ag.Unlock()
	ag.nic = nic
	return nil
}

type mockRPCServer struct {
	grpcServer *rpckit.RPCServer
	nicdb      map[string]*cmd.SmartNIC
	stop       chan bool
}

const testSrvURL = "localhost:8585"

func createRPCServer(t *testing.T) *mockRPCServer {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer("cmd", testSrvURL)
	if err != nil {
		t.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// create fake rpc server
	srv := mockRPCServer{
		grpcServer: grpcServer,
		nicdb:      make(map[string]*cmd.SmartNIC),
		stop:       make(chan bool),
	}

	// Register self as rpc handler for both NIC registration and watch/updates.
	// In reality CMD uses two different servers because watches and updates APIs are
	// exposed over TLS, whereas NIC registration is not.
	grpc.RegisterSmartNICRegistrationServer(grpcServer.GrpcServer, &srv)
	grpc.RegisterSmartNICUpdatesServer(grpcServer.GrpcServer, &srv)
	grpcServer.Start()

	return &srv
}

func (srv *mockRPCServer) RegisterNIC(ctx context.Context, req *grpc.RegisterNICRequest) (*grpc.RegisterNICResponse, error) {
	if req.Nic.Name == "invalid-mac" {
		return nil, errors.New("Invalid request")
	}
	return &grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_ADMITTED.String()}, nil
}

func (srv *mockRPCServer) UpdateNIC(ctx context.Context, req *grpc.UpdateNICRequest) (*grpc.UpdateNICResponse, error) {
	if req.Nic.Name == "invalid-mac" {
		return nil, errors.New("Invalid request")
	}
	return &grpc.UpdateNICResponse{Nic: &req.Nic}, nil
}

func (srv *mockRPCServer) WatchNICs(meta *api.ObjectMeta, stream grpc.SmartNICUpdates_WatchNICsServer) error {
	// walk local db and send stream resp
	for _, nic := range srv.nicdb {
		// watch event
		watchEvt := grpc.SmartNICEvent{
			EventType: api.EventType_CreateEvent,
			Nic:       *nic,
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

	<-srv.stop

	log.Infof("### Exiting server side watch")
	return nil
}

func (srv *mockRPCServer) Stop() {
	close(srv.stop)
	srv.grpcServer.Stop()
}

func TestCmdClient(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})
	// create a mock rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)
	defer srv.Stop()

	// create a nic
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "1111.1111.1111",
		},
	}
	srv.nicdb["1111.1111.1111"] = &nic

	// create a mock agent
	ag := createMockAgent(t.Name())

	// create cmd client
	cl, err := NewCmdClient(ag, testSrvURL, testSrvURL, nil)
	AssertOk(t, err, "Error creating cmd client")
	log.Infof("Cmd client name: %s", cl.getAgentName())
	defer cl.Stop()

	cl.nmd.SetSmartNIC(&nic)

	// verify register rpc call from client to server
	_, err = cl.RegisterSmartNICReq(&nic)
	AssertOk(t, err, "Error making smartNIC register request")

	// verify update rpc call
	nic.Status = cmd.SmartNICStatus{
		Conditions: []*cmd.SmartNICCondition{
			{
				Type:   cmd.SmartNICCondition_HEALTHY.String(),
				Status: cmd.ConditionStatus_TRUE.String(),
			},
		},
	}
	_, err = cl.UpdateSmartNICReq(&nic)
	AssertOk(t, err, "Error making smartNIC update request")
}

func TestCmdClientWatch(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	// create a fake rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)
	defer srv.Stop()

	// create a nic
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "1111.1111.1111",
		},
	}
	srv.nicdb["1111.1111.1111"] = &nic

	// create mock agent
	ag := createMockAgent(t.Name())

	// create CMD client
	cl, err := NewCmdClient(ag, testSrvURL, testSrvURL, nil)
	AssertOk(t, err, "Error creating CMD client")
	Assert(t, (cl != nil), "Error creating CMD client")
	defer cl.Stop()

	cl.nmd.SetSmartNIC(&nic)

	// verify client got the nic
	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicAdded[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC add not found in agent")

	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicUpdated[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC update not found in agent")

	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicDeleted[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC delete not found in agent")
}

func TestCmdClientErrorHandling(t *testing.T) {
	tsdb.Init(&tsdb.DummyTransmitter{}, tsdb.Options{})

	// create a mock rpc server
	srv := createRPCServer(t)
	Assert(t, (srv != nil), "Error creating rpc server", srv)
	defer srv.Stop()

	// create a nic
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "1111.1111.1111",
		},
	}
	srv.nicdb["1111.1111.1111"] = &nic

	// create a mock agent
	ag := createMockAgent(t.Name())

	// create cmd client
	cl, err := NewCmdClient(ag, testSrvURL, testSrvURL, nil)
	AssertOk(t, err, "Error creating cmd client")
	log.Infof("Cmd client name: %s", cl.getAgentName())
	defer cl.Stop()

	cl.nmd.SetSmartNIC(&nic)

	// Test register nic failure
	nic.Name = "invalid-mac"
	_, err = cl.RegisterSmartNICReq(&nic)
	Assert(t, err != nil, "Register NIC should have failed")

	// Test update nic failure
	_, err = cl.UpdateSmartNICReq(&nic)
	Assert(t, err != nil, "Update NIC should have failed")

	nic.Name = "1111.1111.1111"
	// verify register rpc call from client to server
	_, err = cl.RegisterSmartNICReq(&nic)
	AssertOk(t, err, "Error making smartNIC register request")

	// close rpcClient, to force error
	cl.closeUpdatesRPC()

	nic.Name = "2222.2222.2222"
	srv.nicdb["2222.2222.2222"] = &nic
	cl.nmd.SetSmartNIC(&nic)

	go func() {
		cl.runSmartNICWatcher(cl.watchCtx)
	}()

	// verify client got the nic
	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicAdded[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC add not found in agent")

	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicUpdated[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC update not found in agent")

	AssertEventually(t, func() (bool, interface{}) {
		ag.Lock()
		defer ag.Unlock()
		n := ag.nicDeleted[objectKey(nic.ObjectMeta)]
		return (n != nil && n.Name == nic.Name), nil
	}, "NIC delete not found in agent")
}
