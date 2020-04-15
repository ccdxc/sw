// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// Test streaming of captured packets to controller
package rpcserver

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/nic/agent/protos/tsproto"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
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

func (wr *APISrvWriter) WriteTechSupportRequest(ms *monitoring.TechSupportRequest) error {
	return nil
}

// Close stops the client and releases resources
func (wr *APISrvWriter) Close() error {
	return nil
}

// GetAPIClient gets API client
func (wr *APISrvWriter) GetAPIClient() (apiclient.Services, error) {
	return nil, nil
}

// createRPCServerClient creates rpc client and server
func createRPCServerClient(t *testing.T) (*statemgr.Statemgr, *RPCServer, *rpckit.RPCClient) {
	// create state manager
	stateMgr, err := statemgr.NewStatemgr(&APISrvWriter{}, nil)
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
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
				{
					Type: "ERSPAN_TYPE_3",
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"1234"},
					},
				},
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"TCP/1234"},
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
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Dst: &monitoring.MatchSelector{
						IPAddresses: []string{"192.168.100.2"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP"},
					},
				},
			},
		},
	},
}

// TestMirrorSessionRpc tests mirror session rpcs
func TestMirrorSessionRpc(t *testing.T) {
	// create rpc server and client
	stateMgr, rpcServer, rpcClient := createRPCServerClient(t)
	Assert(t, ((stateMgr != nil) && (rpcServer != nil) && (rpcClient != nil)), "Err creating rpc server")
	defer rpcClient.Close()
	defer rpcServer.Stop()

	// create Mirror API client
	msRPCClient := netproto.NewMirrorSessionApiV1Client(rpcClient.ClientConn)

	// create one session that starts right-away and therefore sent to the agent
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
	//pktFilters := evtList.MirrorSessionEvents[0].MirrorSession.Spec.PacketFilters
	//Assert(t, len(pktFilters) == 1, "invalid number of packet filters %v", len(pktFilters))
	//Assert(t, pktFilters[0] == ms.Spec.PacketFilters[0], "expected %v, got %v",
	//	ms.Spec.PacketFilters[0], pktFilters[0])

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

func mustReadTechSupportEvent(t *testing.T, stream tsproto.TechSupportApi_WatchTechSupportRequestsClient, numExpectedNotifications int) {
	ctx, cancel := context.WithTimeout(stream.Context(), 3*time.Second)
	defer cancel()
	recvFn := func(ctx context.Context) (interface{}, error) {
		evtList, err := stream.Recv()
		AssertOk(t, err, "Error reading event from stream")
		Assert(t, len(evtList.Events) == numExpectedNotifications,
			fmt.Sprintf("Wrong number of notifications. Have: %d, Want: %d", len(evtList.Events), numExpectedNotifications))
		return nil, nil
	}
	_, err := utils.ExecuteWithContext(ctx, recvFn)
	AssertOk(t, err, "Timeout reading event from stream")
}

func newTechSupportNodeStatusUpdate(nodeName, nodeKind, reqName, instanceID string, status tsproto.TechSupportRequestStatus_ActionStatus) *tsproto.UpdateTechSupportResultParameters {
	request := &tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: "TechSupportRequest",
		},
		ObjectMeta: api.ObjectMeta{
			Name: reqName,
		},
		Spec: tsproto.TechSupportRequestSpec{
			InstanceID: instanceID,
		},
		Status: tsproto.TechSupportRequestStatus{
			Status: status,
		},
	}
	return &tsproto.UpdateTechSupportResultParameters{
		NodeName: nodeName,
		NodeKind: nodeKind,
		Request:  request,
	}
}

func checkNodeResult(t *testing.T, nodeName, nodeKind string, request *monitoring.TechSupportRequest, expectedResult monitoring.TechSupportJobStatus) {
	var nodeResults map[string]*monitoring.TechSupportNodeResult
	switch nodeKind {
	case statemgr.KindSmartNICNode:
		nodeResults = request.Status.DSCResults
	case statemgr.KindControllerNode:
		nodeResults = request.Status.ControllerNodeResults
	default:
		Assert(t, false, fmt.Sprintf("Unknown NodeKind: %s", nodeKind))
	}
	Assert(t, nodeResults != nil && nodeResults[nodeName] != nil, fmt.Sprintf("No result for node %s in request %+v", nodeName, request))

	result := nodeResults[nodeName].Status
	Assert(t, result == expectedResult.String(), fmt.Sprintf("Unexpected result in TechSupportRequest, node: %s, have: %s, want: %s", nodeName, result, expectedResult.String()))
}

func makeTechSupportSmartNICNodeObject(name string, labels map[string]string) *cluster.DistributedServiceCard {
	return &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{
			Kind: statemgr.KindSmartNICNode,
		},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Labels: labels,
		},
	}
}

func makeTechSupportControllerNodeObject(name string, labels map[string]string) *cluster.Node {
	return &cluster.Node{
		TypeMeta: api.TypeMeta{
			Kind: statemgr.KindControllerNode,
		},
		ObjectMeta: api.ObjectMeta{
			Name:   name,
			Labels: labels,
		},
	}
}

func makeNodeSelectorSpec(reqs []*labels.Requirement) *monitoring.TechSupportRequestSpec_NodeSelectorSpec {
	return &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
		Labels: &labels.Selector{
			Requirements: reqs,
		},
	}
}

// TestTechSupportRPCs tests TechSupport notification delivery and status update mechanisms
func TestTechSupportRPCs(t *testing.T) {
	// create rpc server and client
	stateMgr, rpcServer, rpcClient := createRPCServerClient(t)
	Assert(t, ((stateMgr != nil) && (rpcServer != nil) && (rpcClient != nil)), "Err creating rpc server")
	defer stateMgr.Stop()
	defer rpcClient.Close()
	defer rpcServer.Stop()

	const (
		agentName1 = "test-agent1"
		agentName2 = "test-agent2"
		reqName1   = "Test TechSupportRequest 1"
		reqName2   = "Test TechSupportRequest 2"
	)

	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	makeTechSupportControllerNodeObject(agentName1, map[string]string{"color": "green"})

	tsr1 := &monitoring.TechSupportRequest{
		ObjectMeta: api.ObjectMeta{
			Name: reqName1,
		},
		TypeMeta: api.TypeMeta{
			Kind: "TechSupportRequest",
		},
		Spec: monitoring.TechSupportRequestSpec{
			Verbosity: 1,
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{agentName1},
			},
		},
	}

	// create TS API client
	tsRPCClient := tsproto.NewTechSupportApiClient(rpcClient.ClientConn)

	ag1 := &cluster.Node{
		ObjectMeta: api.ObjectMeta{
			Name: agentName1,
		},
		TypeMeta: api.TypeMeta{
			Kind: statemgr.KindControllerNode,
			//Kind: statemgr.KindSmartNICNode,
		},
	}
	evt0 := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: ag1,
	}
	stateMgr.TechSupportWatcher <- evt0

	ag2 := &cluster.Node{
		ObjectMeta: api.ObjectMeta{
			Name: agentName2,
		},
		TypeMeta: api.TypeMeta{
			Kind: statemgr.KindControllerNode,
		},
	}
	evt0 = kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: ag2,
	}
	stateMgr.TechSupportWatcher <- evt0

	// create one request before the agent starts watching
	evt1 := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: tsr1,
	}
	stateMgr.TechSupportWatcher <- evt1

	// Error cases
	invalidWatchParams := []*tsproto.WatchTechSupportRequestsParameters{
		{NodeName: "", NodeKind: statemgr.KindControllerNode},
		{NodeName: "fakeNode", NodeKind: ""},
		{NodeName: "fakeNode", NodeKind: "fakeKind"},
	}
	for _, p := range invalidWatchParams {
		stream, err := tsRPCClient.WatchTechSupportRequests(ctx, p)
		AssertOk(t, err, "Error invoking WatchTechSupportRequests RPC")
		// should get error when trying to read
		_, err = stream.Recv()
		Assert(t, err != nil, fmt.Sprintf("WatchTechSupportRequestsParameters did not fail with invalid params %+v", p))
	}

	// agent-1 starts watching and immediately receives notification for tsr1
	wp1 := &tsproto.WatchTechSupportRequestsParameters{
		NodeName: agentName1,
		NodeKind: statemgr.KindSmartNICNode,
	}
	rxStream1, err := tsRPCClient.WatchTechSupportRequests(ctx, wp1)
	AssertOk(t, err, "Error starting TechSupport watch for agent %s", agentName1)
	mustReadTechSupportEvent(t, rxStream1, 1)

	// agent-2 starts watching and initially does not receive any notification
	wp2 := &tsproto.WatchTechSupportRequestsParameters{
		NodeName: agentName2,
		NodeKind: statemgr.KindControllerNode,
	}
	rxStream2, err := tsRPCClient.WatchTechSupportRequests(ctx, wp2)
	AssertOk(t, err, "Error starting TechSupport watch for agent %s", agentName2)

	// now create tsr2. Both agent-1 and agent-2 get a notification
	tsr2 := &monitoring.TechSupportRequest{
		ObjectMeta: api.ObjectMeta{
			Name: reqName2,
		},
		TypeMeta: api.TypeMeta{
			Kind: "TechSupportRequest",
		},
		Spec: monitoring.TechSupportRequestSpec{
			Verbosity: 1,
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{agentName1, agentName2},
			},
		},
	}
	evt2 := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: tsr2,
	}
	stateMgr.TechSupportWatcher <- evt2

	mustReadTechSupportEvent(t, rxStream1, 1)
	mustReadTechSupportEvent(t, rxStream2, 1)

	// get instance IDs
	tsr1ObjState, err := stateMgr.GetTechSupportObjectState(tsr1)
	AssertOk(t, err, "Error getting state for tsr1")
	instID1 := tsr1ObjState.(*statemgr.TechSupportRequestState).TechSupportRequest.Status.InstanceID

	tsr2ObjState, err := stateMgr.GetTechSupportObjectState(tsr2)
	AssertOk(t, err, "Error getting state for tsr2")
	instID2 := tsr2ObjState.(*statemgr.TechSupportRequestState).TechSupportRequest.Status.InstanceID

	// post status updates
	updAg1Req1 := newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName1, instID1, tsproto.TechSupportRequestStatus_Failed)
	updAg1Req2 := newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName2, instID2, tsproto.TechSupportRequestStatus_Completed)
	updAg2Req1 := newTechSupportNodeStatusUpdate(wp2.NodeName, wp2.NodeKind, reqName1, instID1, tsproto.TechSupportRequestStatus_Completed)

	for _, update := range []*tsproto.UpdateTechSupportResultParameters{updAg1Req1, updAg1Req2, updAg2Req1} {
		result, err := tsRPCClient.UpdateTechSupportResult(ctx, update)
		AssertOk(t, err, "Error posting TechSupport result update")
		Assert(t, result.Status == "ok", "Unexpected Error updating TechSupport result")
	}

	// verify updates
	tsr1ObjState, err = stateMgr.GetTechSupportObjectState(tsr1)
	AssertOk(t, err, "Error getting state for tsr1")
	updTSR1 := tsr1ObjState.(*statemgr.TechSupportRequestState).TechSupportRequest

	tsr2ObjState, err = stateMgr.GetTechSupportObjectState(tsr2)
	AssertOk(t, err, "Error getting state for tsr2")
	updTSR2 := tsr2ObjState.(*statemgr.TechSupportRequestState).TechSupportRequest

	checkNodeResult(t, wp1.NodeName, wp1.NodeKind, updTSR1, monitoring.TechSupportJobStatus_Failed)
	checkNodeResult(t, wp1.NodeName, wp1.NodeKind, updTSR2, monitoring.TechSupportJobStatus_Completed)
	checkNodeResult(t, wp2.NodeName, wp2.NodeKind, updTSR1, monitoring.TechSupportJobStatus_Completed)

	// post status updates with wrong instance ID -- they should be ignored
	updAg1Req1 = newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName1, "ININSTID", tsproto.TechSupportRequestStatus_Failed)
	updAg1Req2 = newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName2, "", tsproto.TechSupportRequestStatus_Completed)
	updAg2Req1 = newTechSupportNodeStatusUpdate(wp2.NodeName, wp2.NodeKind, reqName1, "ININSTID", tsproto.TechSupportRequestStatus_Completed)

	checkNodeResult(t, wp1.NodeName, wp1.NodeKind, updTSR1, monitoring.TechSupportJobStatus_Failed)
	checkNodeResult(t, wp1.NodeName, wp1.NodeKind, updTSR2, monitoring.TechSupportJobStatus_Completed)
	checkNodeResult(t, wp2.NodeName, wp2.NodeKind, updTSR1, monitoring.TechSupportJobStatus_Completed)

	for _, update := range []*tsproto.UpdateTechSupportResultParameters{updAg1Req1, updAg1Req2, updAg2Req1} {
		result, err := tsRPCClient.UpdateTechSupportResult(ctx, update)
		// stale updates are ignored
		AssertOk(t, err, "Error posting TechSupport result update")
		Assert(t, result.Status == "ok", "Unexpected Error updating TechSupport result")
	}

	// delete  techsupport objects
	stateMgr.PurgeDeletedTechSupportObjects([]*monitoring.TechSupportRequest{})

	mustReadTechSupportEvent(t, rxStream1, 1)
	mustReadTechSupportEvent(t, rxStream1, 1)
	mustReadTechSupportEvent(t, rxStream2, 1)

	// post status updates for deleted objects, -- they should be ignored
	newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName1, "ININSTID", tsproto.TechSupportRequestStatus_Failed)
	newTechSupportNodeStatusUpdate(wp1.NodeName, wp1.NodeKind, reqName2, "", tsproto.TechSupportRequestStatus_Completed)
	newTechSupportNodeStatusUpdate(wp2.NodeName, wp2.NodeKind, reqName1, "ININSTID", tsproto.TechSupportRequestStatus_Completed)

	for _, update := range []*tsproto.UpdateTechSupportResultParameters{updAg1Req1, updAg1Req2, updAg2Req1} {
		_, err := tsRPCClient.UpdateTechSupportResult(ctx, update)
		Assert(t, err != nil, "Unexpected success posting updates for non-existant requests")
	}
}

// TestTechSupportNodeSelector tests the TechSupport node selection logic
func TestTechSupportNodeSelector(t *testing.T) {
	stateMgr, err := statemgr.NewStatemgr(&APISrvWriter{}, nil)
	AssertOk(t, err, "Error instantiating StateMgr")
	defer stateMgr.Stop()
	s := NewTechSupportRPCServer(stateMgr)

	nodeName := "name-test-node"

	type nameBasedTestCase struct {
		selector    *monitoring.TechSupportRequestSpec_NodeSelectorSpec
		nodeName    string
		nodeKind    string
		expectMatch bool
	}

	nbtcs := []*nameBasedTestCase{
		{ // empty selector should not match anything
			selector:    &monitoring.TechSupportRequestSpec_NodeSelectorSpec{},
			nodeName:    nodeName,
			nodeKind:    statemgr.KindSmartNICNode,
			expectMatch: false,
		},
		{ // name-based selector should match both kinds
			selector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{nodeName},
			},
			nodeName:    nodeName,
			nodeKind:    statemgr.KindSmartNICNode,
			expectMatch: true,
		},
		{ // name-based selector should match both kinds
			selector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names: []string{nodeName},
			},
			nodeName:    nodeName,
			nodeKind:    statemgr.KindControllerNode,
			expectMatch: true,
		},
	}

	for i, tc := range nbtcs {
		Assert(t, s.nodeSelectorMatch(tc.nodeName, tc.nodeKind, tc.selector) == tc.expectMatch,
			fmt.Sprintf("Unexpected match result, testcase index: %d", i))
	}

	// Pre-populate nodes with labels
	nodes := map[string]kvstore.WatchEvent{
		"no-labels":  {Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("no-labels", nil)},
		"blue-node":  {Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("blue-node", map[string]string{"color": "blue"})},
		"green-node": {Type: kvstore.Created, Object: makeTechSupportControllerNodeObject("green-node", map[string]string{"color": "green"})},
		"red-node":   {Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("red-node", map[string]string{"color": "red"})},
		"green-star": {Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("green-star", map[string]string{"color": "green", "star": "true"})},
	}

	for _, n := range nodes {
		stateMgr.TechSupportWatcher <- n
	}

	AssertEventually(t, func() (bool, interface{}) {
		smartNICNodes := stateMgr.ListTechSupportObjectState(statemgr.KindSmartNICNode)
		ctrlrNodes := stateMgr.ListTechSupportObjectState(statemgr.KindControllerNode)
		return len(smartNICNodes)+len(ctrlrNodes) == len(nodes), nil
	}, "StateMgr did not process all notifications")

	type labelBasedTestCase struct {
		selector  *monitoring.TechSupportRequestSpec_NodeSelectorSpec
		nodeMatch map[string]bool
	}

	lbtcs := []*labelBasedTestCase{
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("color", labels.Operator_in, []string{"blue", "green"}),
			}),
			nodeMatch: map[string]bool{"blue-node": true, "green-node": true, "green-star": true},
		},
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("color", labels.Operator_in, []string{"blue", "green"}),
				labels.MustGetRequirement("star", labels.Operator_equals, []string{"true"}),
			}),
			nodeMatch: map[string]bool{"green-star": true},
		},
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("color", labels.Operator_in, []string{"red"}),
				labels.MustGetRequirement("star", labels.Operator_equals, []string{"true"}),
			}),
			nodeMatch: map[string]bool{},
		},
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("star", labels.Operator_notEquals, []string{"true"}),
			}),
			nodeMatch: map[string]bool{"blue-node": true, "green-node": true, "red-node": true},
		},
	}

	for i, tc := range lbtcs {
		for n, e := range nodes {
			kind := e.Object.GetObjectKind()
			match := s.nodeSelectorMatch(n, kind, tc.selector)
			Assert(t, match == tc.nodeMatch[n], fmt.Sprintf("Unexpected match result, testcase index: %d, node: %s", i, n))
		}
	}

	// update labels
	updates := []kvstore.WatchEvent{
		{Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("blue-node", map[string]string{"color": "black"})},
		{Type: kvstore.Created, Object: makeTechSupportSmartNICNodeObject("green-star", map[string]string{"color": "green", "star": "false"})},
	}
	for _, u := range updates {
		stateMgr.TechSupportWatcher <- u
	}

	AssertEventually(t, func() (bool, interface{}) {
		blueNode, err := stateMgr.FindTechSupportObject("blue-node", "", statemgr.KindSmartNICNode)
		AssertOk(t, err, "Error getting object state")
		greenStar, err := stateMgr.FindTechSupportObject("green-star", "", statemgr.KindSmartNICNode)
		AssertOk(t, err, "Error getting object state")
		return blueNode.GetObjectMeta().Labels["color"] == "black" &&
			greenStar.GetObjectMeta().Labels["star"] == "false", nil
	}, "StateMgr did not process all notifications")

	ulbtcs := []*labelBasedTestCase{
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("color", labels.Operator_in, []string{"blue", "green"}),
			}),
			nodeMatch: map[string]bool{"green-node": true, "green-star": true},
		},
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("color", labels.Operator_equals, []string{"black"}),
			}),
			nodeMatch: map[string]bool{"blue-node": true},
		},
		{
			selector: makeNodeSelectorSpec([]*labels.Requirement{
				labels.MustGetRequirement("star", labels.Operator_equals, []string{"true"}),
			}),
			nodeMatch: map[string]bool{},
		},
	}

	for i, tc := range ulbtcs {
		for n, e := range nodes {
			kind := e.Object.GetObjectKind()
			match := s.nodeSelectorMatch(n, kind, tc.selector)
			Assert(t, match == tc.nodeMatch[n], fmt.Sprintf("Unexpected match result, testcase index: %d, node: %s", i, n))
		}
	}
}
