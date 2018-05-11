// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// Test streaming of captured packets to controller
package rpcserver

import (
	"context"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const testServerURL = "localhost:0" // dynamically find a free port

// Dummy Writer
type Writer interface {
	WriteMirrorSession(ms *monitoring.MirrorSession) error
	Close() error
}

// APISrvWriter is the writer instance - dummy
type APISrvWriter struct {
	// dummy
}

func (wr *APISrvWriter) WriteMirrorSession(ms *monitoring.MirrorSession) error {
	return nil
}

// Close stops the client and releases resources
func (wr *APISrvWriter) Close() error {
	return nil
}

// createRPCServerClient creates rpc client and server
func createRPCServerClient(t *testing.T) (*statemgr.Statemgr, *RPCServer, *rpckit.RPCClient) {
	// create state manager
	stateMgr, err := statemgr.NewStatemgr(&APISrvWriter{})
	if err != nil {
		t.Fatalf("Could not create state manager. Err: %v", err)
	}

	// start the rpc server
	rpcServer, err := NewRPCServer(testServerURL, stateMgr)
	if err != nil {
		t.Fatalf("Error creating RPC server. Err: %v", err)
	}
	Assert(t, (rpcServer != nil), "Failed to create rpc server")
	// get the port that got assigned
	testServerURL := rpcServer.GetListenURL()

	// create a grpc client
	rpcClient, err := rpckit.NewRPCClient("testServer", testServerURL)
	if err != nil {
		t.Fatalf("Error connecting to grpc server. Err: %v", err)
	}

	return stateMgr, rpcServer, rpcClient
}

var testMirrorSessions = []monitoring.MirrorSession{
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 1",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
			},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
				{
					Type: "ERSPAN",
					ExportCfg: api.ExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"TCP/1234"},
					},
				},
			},
		},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 2",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{"ALL_PKTS"},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 2,
					},
				},
			},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Dst: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
}

// TestSecurityGroupRPC tests security group rpcs
func TestMirrorSessionRpc(t *testing.T) {
	// create rpc server and client
	stateMgr, rpcServer, rpcClient := createRPCServerClient(t)
	Assert(t, ((stateMgr != nil) && (rpcServer != nil) && (rpcClient != nil)), "Err creating rpc server")
	defer rpcClient.Close()
	defer rpcServer.Stop()

	// create Mirror API client
	msRPCClient := tsproto.NewMirrorSessionApiClient(rpcClient.ClientConn)

	// create one session that start right-away and therefore sent to the agent
	// statemgr expects a pointer to object, send pointer to each MirrorSession, not its copy
	ms := &testMirrorSessions[0]
	Assert(t, (ms.Spec.StartConditions.ScheduleTime == nil), "Test case bug - cannot schedule session[0]")
	_ = stateMgr.CreateMirrorSession(ms)

	// start watch - server-side streaming
	ometa := api.ObjectMeta{Name: "default", Tenant: "default"}
	rxStream, err := msRPCClient.WatchMirrorSessions(context.Background(), &ometa)
	evtList, err := rxStream.Recv()
	//Assert(t, (err == nil && (len(evtList.MirrorSessionEvents) == len(testMirrorSessions))), "All Mirror session not received by the client")
	Assert(t, (err == nil && (len(evtList.MirrorSessionEvents) == 1)), "All Mirror session not received by the client")

	ms = &testMirrorSessions[1]
	Assert(t, (ms.Spec.StartConditions.ScheduleTime != nil), "Test case bug - must schedule session[1]")
	if ms.Spec.StartConditions.ScheduleTime != nil {
		// fix the time
		schAfter := time.Duration(uint64(ms.Spec.StartConditions.ScheduleTime.Seconds) * uint64(time.Second))
		schTime := time.Now().Add(schAfter)
		ts, _ := types.TimestampProto(schTime)
		ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
	}
	_ = stateMgr.CreateMirrorSession(ms)

	// start client-side streaming to receive status/packets
	stream, _ := msRPCClient.GetMirrorSessionsStatus(context.Background())
	mspList := tsproto.MirrorSessionStatusList{
		SmartNIC: "TestNaples1",
		StatusList: []*tsproto.MirrorSession{
			&tsproto.MirrorSession{
				TypeMeta:   testMirrorSessions[0].TypeMeta,
				ObjectMeta: testMirrorSessions[0].ObjectMeta,
				Status: tsproto.MirrorSessionStatus{
					State:     tsproto.MirrorSessionState_RUNNING.String(),
					SessionId: 0x55,
				},
			},
		},
	}
	stream.Send(&mspList)
	stream.CloseAndRecv()
	// pickup session 1 that started running (sheduled->running)
	evtList, err = rxStream.Recv()
	Assert(t, (err == nil && (len(evtList.MirrorSessionEvents) == 1)), "Mirror session[1] not received by the client")

	// delete  mirror sessions
	for i := range testMirrorSessions {
		// statemgr expects a pointer to object, send pointer to each MirrorSession, not its copy
		ms := &testMirrorSessions[i]
		stateMgr.DeleteMirrorSession(ms)
		evtList, err = rxStream.Recv()
		Assert(t, (err == nil && (len(evtList.MirrorSessionEvents) == 1)), "All Mirror session deletes not received by the client")
	}
	stateMgr.Stop()
}
