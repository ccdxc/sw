// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"fmt"
	"math/rand"
	"reflect"
	"sync/atomic"
	"time"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/globals"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

const (
	// kindControllerNode  is the kind of Node objects in string form
	kindControllerNode = string(cluster.KindNode)
	// kindSmartNICNode is the kind of SmartNIC objects in string form
	kindSmartNICNode = string(cluster.KindDistributedServiceCard)
	// kindTechSupportRequest is the kind of SmartNIC objects in string form
	kindTechSupportRequest = string(monitoring.KindTechSupportRequest)
)

type techSupportAgent struct {
	// the name of the node. Must match the name of the Node/SmartNIC object
	name string
	// the kind of the node: Node (controller) or SmartNIC
	kind string
	// the names of the requests that this agent is supposed to match -- used for verification
	matchingRequests map[string]*monitoring.TechSupportRequest
	// the notifications that have been received
	notifications []*tsproto.TechSupportRequestEvent
	// a flag used to know if agent is watching or not -- access using atomics
	watching int32 // 0 --> not watching, 1 --> watching
	// rpc client used to watch and send updates
	rpcClient *rpckit.RPCClient
	// API client used to watch and send updates
	tsAPIClient tsproto.TechSupportApiClient
	// cancel function
	cancelFn context.CancelFunc
	// the address where the TSM API is available
	tsmAPIAddr string
}

func (ag *techSupportAgent) watch(inCtx context.Context, c *C) {
	AssertEventually(c, func() (bool, interface{}) {
		rpcClient, err := rpckit.NewRPCClient(ag.name, ag.tsmAPIAddr, rpckit.WithRemoteServerName(globals.Tsm))
		if err != nil {
			return false, fmt.Errorf("Error creating TSM client %s: %v", ag.name, err)
		}
		ag.rpcClient = rpcClient
		return true, nil
	}, fmt.Sprintf("Error creating TSM client %s", ag.name), "1s", "15s")

	// start watch for techsupport requests
	client := tsproto.NewTechSupportApiClient(ag.rpcClient.ClientConn)
	ag.tsAPIClient = client
	watchParams := &tsproto.WatchTechSupportRequestsParameters{
		NodeName: ag.name,
		NodeKind: ag.kind,
	}
	ctx, cancelFn := context.WithCancel(inCtx)
	ag.cancelFn = cancelFn
	stream, err := client.WatchTechSupportRequests(ctx, watchParams)
	AssertOk(c, err, "Error sending WatchTechSupportRequests request")
	atomic.StoreInt32(&ag.watching, 1)
	log.Infof("Agent %s started watch", ag.name)
	for {
		evtList, err := stream.Recv()
		if err != nil {
			break
		}
		ag.notifications = append(ag.notifications, evtList.Events...)
	}
	log.Infof("Agent %s stopped watch", ag.name)
	atomic.StoreInt32(&ag.watching, 0)
}

func (ag *techSupportAgent) isWatching() bool {
	return atomic.LoadInt32(&ag.watching) != 0
}

func (ag *techSupportAgent) sendUpdate(ctx context.Context, c *C, reqName, instanceID string, status tsproto.TechSupportRequestStatus_ActionStatus, expectSuccess bool) {
	update := &tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: kindTechSupportRequest,
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
	startTime := api.Timestamp{}
	err := startTime.Parse("now() - 1m")
	AssertOk(c, err, "Error parsing timestamp")
	update.Status.StartTime = &startTime

	if status == tsproto.TechSupportRequestStatus_Failed || status == tsproto.TechSupportRequestStatus_Completed {
		endTime := api.Timestamp{}
		err := endTime.Parse("now()")
		AssertOk(c, err, "Error parsing timestamp")
		update.Status.EndTime = &endTime
	}
	updParams := &tsproto.UpdateTechSupportResultParameters{
		NodeName: ag.name,
		NodeKind: ag.kind,
		Request:  update,
	}
	_, err = ag.tsAPIClient.UpdateTechSupportResult(ctx, updParams)
	ok := (expectSuccess && err == nil) || (!expectSuccess && err != nil)
	Assert(c, ok, "Unexpected RPC error: %v, expectSuccess: %v", err, expectSuccess)
}

func (ag *techSupportAgent) stop() {
	if ag.cancelFn != nil {
		ag.cancelFn()
	}
	if ag.rpcClient != nil {
		ag.rpcClient.Close()
		ag.rpcClient = nil
	}
	ag.tsAPIClient = nil
}

// utility functions
func (it *veniceIntegSuite) createTechSupportRequest(ctx context.Context, tsr *monitoring.TechSupportRequest) error {
	_, err := it.restClient.MonitoringV1().TechSupportRequest().Create(ctx, tsr)
	if err != nil {
		log.Errorf("Error creating TechSupportRequest %+v: %v", tsr, err)
		return err
	}
	return err
}

func (it *veniceIntegSuite) deleteTechSupportRequest(ctx context.Context, o *api.ObjectMeta) error {
	_, err := it.restClient.MonitoringV1().TechSupportRequest().Delete(ctx, o)
	if err != nil {
		log.Errorf("Error deleting TechSupportRequest %+v: %v", o, err)
	}
	return err
}

func (it *veniceIntegSuite) createSmartNICNode(ctx context.Context, node *cluster.DistributedServiceCard) error {
	_, err := it.apisrvClient.ClusterV1().DistributedServiceCard().Create(ctx, node)
	if err != nil {
		log.Errorf("Error creating SmartNIC node %+v: %v", node, err)
		return err
	}
	return err
}

func (it *veniceIntegSuite) createControllerNode(ctx context.Context, node *cluster.Node) error {
	_, err := it.apisrvClient.ClusterV1().Node().Create(ctx, node)
	if err != nil {
		log.Errorf("Error creating Controller node %+v: %v", node, err)
		return err
	}
	return err
}

func (it *veniceIntegSuite) deleteSmartNICNode(ctx context.Context, meta *api.ObjectMeta) error {
	n, err := it.apisrvClient.ClusterV1().DistributedServiceCard().Get(ctx, meta)
	if err != nil {
		log.Errorf("Error getting SmartNIC node %+v: %v", meta, err)
		return err
	}
	n.Spec.Admit = false
	n.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_PENDING.String()
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, n)
	if err != nil {
		log.Errorf("Error updating DistributedServiceCard node %+v: %v", meta, err)
		return err
	}
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Delete(ctx, meta)
	if err != nil {
		log.Errorf("Error deleting DistributedServiceCard node %+v: %v", n, err)
		return err
	}
	return err
}

func (it *veniceIntegSuite) deleteControllerNode(ctx context.Context, node *api.ObjectMeta) error {
	_, err := it.apisrvClient.ClusterV1().Node().Delete(ctx, node)
	if err != nil {
		log.Errorf("Error deleting Controller node %+v: %v", node, err)
		return err
	}
	return err
}

func newTechSupportRequest(name string, nodeNames []string, label *labels.Selector) *monitoring.TechSupportRequest {
	return &monitoring.TechSupportRequest{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		TypeMeta: api.TypeMeta{
			Kind:       kindTechSupportRequest,
			APIVersion: "v1",
		},
		Spec: monitoring.TechSupportRequestSpec{
			Verbosity: rand.Int31n(7),
			NodeSelector: &monitoring.TechSupportRequestSpec_NodeSelectorSpec{
				Names:  nodeNames,
				Labels: label,
			},
			CollectionSelector: label,
		},
	}
}

func newTechSupportAgent(name, kind string, matchingRequests []*monitoring.TechSupportRequest) *techSupportAgent {
	tsa := &techSupportAgent{
		name:             name,
		kind:             kind,
		matchingRequests: make(map[string]*monitoring.TechSupportRequest),
		tsmAPIAddr:       integTestTsmURL,
	}
	for _, r := range matchingRequests {
		tsa.matchingRequests[r.ObjectMeta.Name] = r
	}
	return tsa
}

func newSmartNICNode(name string, label map[string]string) *cluster.DistributedServiceCard {
	nic := policygen.CreateSmartNIC(name,
		cluster.DistributedServiceCardStatus_ADMITTED.String(),
		"esx-1",
		&cluster.DSCCondition{
			Type:   cluster.DSCCondition_HEALTHY.String(),
			Status: cluster.ConditionStatus_FALSE.String(),
		})
	nic.ObjectMeta.Labels = label
	nic.Spec.ID = name
	return nic
}

func newControllerNode(name string, label map[string]string) *cluster.Node {
	node := &cluster.Node{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		TypeMeta: api.TypeMeta{
			Kind: string(cluster.KindNode),
		},
	}
	node.ObjectMeta.Labels = label
	return node
}

func newLabelSelector(reqs []*labels.Requirement) *labels.Selector {
	return &labels.Selector{
		Requirements: reqs,
	}
}

// Check that all agents have established the watch
func checkAgentWatches(c *C, agents map[string]*techSupportAgent) {
	AssertEventually(c, func() (bool, interface{}) {
		for _, agent := range agents {
			if !agent.isWatching() {
				return false, fmt.Sprintf("Agent %s is not watching yet", agent.name)
			}
		}
		return true, nil
	}, "Not all techsupport agents are watching", "500ms", "30s")
}

// check that all agents have received numNotifications for each matching request
func checkAgentNotifications(c *C, agents map[string]*techSupportAgent, numNotifications int) {
	// Make sure all agents have received the notification
	AssertEventually(c, func() (bool, interface{}) {
		for _, agent := range agents {
			have := len(agent.notifications)
			want := numNotifications * len(agent.matchingRequests)
			if have != want {
				return false, fmt.Sprintf("Agent %v has wrong notification count. Have: %d, Want: %d, %+v", agent.name, have, want, agent.notifications)
			}
		}
		return true, nil
	}, "Error verifying notifications", "500ms", "30s")

	// now verify that the notification matches the request
	for _, agent := range agents {
		for _, n := range agent.notifications {
			notReq := n.Request
			refReq := agent.matchingRequests[notReq.ObjectMeta.Name]
			Assert(c, refReq != nil, fmt.Sprintf("Agent %s received notification for unknown request %s", agent.name, notReq.Name))
			Assert(c, reflect.DeepEqual(notReq.Spec.Labels, refReq.Spec.CollectionSelector),
				fmt.Sprintf("Request in notification has wrong labels, agent: %s, have: %+v, want: %+v", agent.name, notReq.Spec.Labels, refReq.Spec.CollectionSelector))
			Assert(c, notReq.Spec.Verbosity == refReq.Spec.Verbosity,
				fmt.Sprintf("Request in notification has wrong verbosity, agent: %s, have: %d, want: %d", agent.name, notReq.Spec.Verbosity, refReq.Spec.Verbosity))
		}
	}
}

func clearNotifications(c *C, agents map[string]*techSupportAgent) {
	for _, agent := range agents {
		agent.notifications = nil
	}
}

func getTechSupportNodeResult(req *monitoring.TechSupportRequest, nodeName string) *monitoring.TechSupportNodeResult {
	if req.Status.DSCResults != nil && req.Status.DSCResults[nodeName] != nil {
		return req.Status.DSCResults[nodeName]
	} else if req.Status.ControllerNodeResults != nil && req.Status.ControllerNodeResults[nodeName] != nil {
		return req.Status.ControllerNodeResults[nodeName]
	}
	return nil
}

func (it *veniceIntegSuite) checkTechSupportNodeStatus(ctx context.Context, c *C, reqName string, agentNames []string, status monitoring.TechSupportJobStatus) {
	reqMeta := &api.ObjectMeta{
		Name: reqName,
	}
	AssertEventually(c, func() (bool, interface{}) {
		req, err := it.apisrvClient.MonitoringV1().TechSupportRequest().Get(ctx, reqMeta)
		if err != nil {
			fmt.Printf("Error getting TechSupportRequest %s: %v", reqName, err)
			return false, err
		}
		lenResults := len(req.Status.DSCResults) + len(req.Status.ControllerNodeResults)
		if lenResults != len(agentNames) {
			fmt.Printf("Number of results does not match. Have: %d, want: %d", lenResults, len(agentNames))
			return false, fmt.Errorf("Number of results does not match. Have: %d, want: %d", lenResults, len(agentNames))
		}
		for _, an := range agentNames {
			result := getTechSupportNodeResult(req, an)
			if result == nil {
				fmt.Printf("Result not found for agent %s", an)
				return false, fmt.Errorf("Result not found for agent %s", an)
			}
			if result.Status != status.String() {
				fmt.Printf("Unexpected status in result for agent %s. Have: %s, want: %s", an, result.Status, status.String())
				return false, fmt.Errorf("Unexpected status in result for agent %s. Have: %s, want: %s", an, result.Status, status.String())
			}
			startTime, err := result.StartTime.Time()
			AssertOk(c, err, fmt.Sprintf("Error decoding StartTime %+v", result.StartTime))
			Assert(c, !startTime.IsZero(), fmt.Sprintf("Start time not set in result for agent %s. result: %+v", an, result))
			if status == monitoring.TechSupportJobStatus_Completed || status == monitoring.TechSupportJobStatus_Failed {
				endTime, err := result.StartTime.Time()
				AssertOk(c, err, fmt.Sprintf("Error decoding EndTime %+v", result.EndTime))
				Assert(c, !endTime.IsZero(), fmt.Sprintf("End time not set in result for agent %s. result: %+v", an, result))
			}
		}
		return true, nil
	}, "TechSupportRequest Status check failed", "500ms", "60s")
}

func (it *veniceIntegSuite) getTechSupportRequest(ctx context.Context, c *C, reqName string) *monitoring.TechSupportRequest {
	reqMeta := &api.ObjectMeta{
		Name: reqName,
	}
	req, err := it.apisrvClient.MonitoringV1().TechSupportRequest().Get(ctx, reqMeta)
	AssertOk(c, err, "Error getting TechSupportRequest")
	return req
}

func (it *veniceIntegSuite) getTechSupportRequestStatus(ctx context.Context, c *C, reqName string) string {
	return it.getTechSupportRequest(ctx, c, reqName).Status.Status
}

func (it *veniceIntegSuite) getTechSupportRequestInstanceID(ctx context.Context, c *C, reqName string) string {
	return it.getTechSupportRequest(ctx, c, reqName).Status.InstanceID
}

// waitForTechSupportControllerUpdate returns when the TechSupportController has received an specific object update
func (it *veniceIntegSuite) waitForTechSupportControllerUpdate(c *C, refObj statemgr.TechSupportObject) {
	AssertEventually(c, func() (bool, interface{}) {
		state, err := it.tsCtrler.StateMgr.GetTechSupportObjectState(refObj)
		if err != nil {
			return false, fmt.Errorf("Object %+v not found in TechSupport controller db", refObj.GetObjectMeta())
		}
		if state.GetObjectMeta().GetResourceVersion() != refObj.GetObjectMeta().GetResourceVersion() {
			return false, fmt.Errorf("Version mismatch, Have: %v, Want: %v", state.GetObjectMeta().GetResourceVersion(), refObj.GetObjectMeta().GetResourceVersion())
		}
		return true, nil
	}, fmt.Sprintf("TechSupport controller did not receive expected update: %+v", refObj), "500ms", "30s")
}

func (it *veniceIntegSuite) TestTechSupportRequestCreateDelete(c *C) {
	tsrs := map[string]*monitoring.TechSupportRequest{
		"TSR1": newTechSupportRequest("TSR1", []string{"00ae.cd88.0011"}, nil),
		"TSR2": newTechSupportRequest("TSR2", []string{"00ae.cd88.0011", "00ae.cd88.0012", "00ae.cd88.0014"}, nil),
		"TSR3": newTechSupportRequest("TSR3", []string{"00ae.cd88.0014"}, nil),
	}

	agents := map[string]*techSupportAgent{
		"node-1": newTechSupportAgent("00ae.cd88.0011", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["TSR1"], tsrs["TSR2"]}),
		"node-2": newTechSupportAgent("00ae.cd88.0012", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["TSR2"]}),
		"node-3": newTechSupportAgent("00ae.cd88.0013", kindSmartNICNode, []*monitoring.TechSupportRequest{}),
	}

	ctx, err := authntestutils.NewLoggedInContext(context.Background(), it.apiGwAddr, it.userCred)
	AssertOk(c, err, "Error creating logged in context")
	n1 := newSmartNICNode("00ae.cd88.0011", map[string]string{"group": "orange", "type": "hr"})
	err = it.createSmartNICNode(ctx, n1)
	AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n1))
	defer it.deleteSmartNICNode(ctx, &n1.ObjectMeta)

	n2 := newSmartNICNode("00ae.cd88.0012", map[string]string{"group": "orange", "type": "hr"})
	err = it.createSmartNICNode(ctx, n2)
	AssertOk(c, err, fmt.Sprintf("Error creating Controller node %+v", n2))
	defer it.deleteSmartNICNode(ctx, &n2.ObjectMeta)

	n3 := newSmartNICNode("00ae.cd88.0013", map[string]string{"group": "orange", "type": "hr"})
	err = it.createSmartNICNode(ctx, n3)
	AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n3))
	defer it.deleteSmartNICNode(ctx, &n3.ObjectMeta)

	n4 := newSmartNICNode("00ae.cd88.0014", map[string]string{"group": "orange", "type": "hr"})
	err = it.createSmartNICNode(ctx, n4)
	AssertOk(c, err, fmt.Sprintf("Error creating Controller node %+v", n4))
	defer it.deleteSmartNICNode(ctx, &n4.ObjectMeta)

	for _, agent := range agents {
		go agent.watch(ctx, c)
		defer agent.stop()
	}
	checkAgentWatches(c, agents)

	// "late-agent" represents a node that shows up after the requests have been posted and it does not get the notification as it was not part of the
	lateAgent := newTechSupportAgent("00ae.cd88.0014", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["TSR2"], tsrs["TSR3"]})
	go lateAgent.watch(ctx, c)
	defer lateAgent.stop()
	lateAgentMap := map[string]*techSupportAgent{"00ae.cd88.0014": lateAgent}
	checkAgentWatches(c, lateAgentMap)

	for _, r := range tsrs {
		err := it.createTechSupportRequest(ctx, r)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
	}

	checkAgentNotifications(c, agents, 1)
	checkAgentNotifications(c, lateAgentMap, 1)

	for rn := range tsrs {
		rs := it.getTechSupportRequestStatus(ctx, c, rn)
		Assert(c, rs == monitoring.TechSupportJobStatus_Scheduled.String(), fmt.Sprintf("Wrong status for request %s, have: %s, want: Scheduled", rn, rs))
	}

	for rn, req := range tsrs {
		rs := it.getTechSupportRequestStatus(ctx, c, rn)
		Assert(c, rs == monitoring.TechSupportJobStatus_Scheduled.String(), fmt.Sprintf("Wrong status for request %s, have: %s, want: Scheduled", rn, rs))
		err := it.deleteTechSupportRequest(ctx, &req.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting TechSupportRequest %+v", req.ObjectMeta))
	}

	checkAgentNotifications(c, lateAgentMap, 2)
}

func (it *veniceIntegSuite) TestTechSupportLabelBasedSelection(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), it.apiGwAddr, it.userCred)
	AssertOk(c, err, "Error creating logged in context")

	smartNICNodes := map[string]*cluster.DistributedServiceCard{
		"00ae.cd02.0011": newSmartNICNode("00ae.cd02.0011", map[string]string{"group": "red", "type": "prod"}),
		"00ae.cd02.0012": newSmartNICNode("00ae.cd02.0012", map[string]string{"group": "blue", "type": "prod"}),
		"00ae.cd02.0013": newSmartNICNode("00ae.cd02.0013", map[string]string{"group": "blue", "type": "dev"}),
		"00ae.cd02.0014": newSmartNICNode("00ae.cd02.0014", map[string]string{"star": "yes"}),
	}

	tsrs := map[string]*monitoring.TechSupportRequest{
		"LblProd": newTechSupportRequest("LblProd", []string{},
			newLabelSelector([]*labels.Requirement{labels.MustGetRequirement("type", labels.Operator_equals, []string{"prod"})})),

		"LblStar": newTechSupportRequest("LblStar", []string{},
			newLabelSelector([]*labels.Requirement{labels.MustGetRequirement("star", labels.Operator_equals, []string{"no"})})),

		"NIC1OrBlue": newTechSupportRequest("NIC1OrBlue", []string{"00ae.cd02.0011"},
			newLabelSelector([]*labels.Requirement{labels.MustGetRequirement("group", labels.Operator_in, []string{"blue", "green"})})),
	}

	agents := map[string]*techSupportAgent{
		"00ae.cd02.0011": newTechSupportAgent("00ae.cd02.0011", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["LblProd"], tsrs["NIC1OrBlue"]}),
		"00ae.cd02.0012": newTechSupportAgent("00ae.cd02.0012", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["LblProd"], tsrs["NIC1OrBlue"]}),
		"00ae.cd02.0013": newTechSupportAgent("00ae.cd02.0013", kindSmartNICNode, []*monitoring.TechSupportRequest{tsrs["NIC1OrBlue"]}),
		"00ae.cd02.0014": newTechSupportAgent("00ae.cd02.0014", kindSmartNICNode, []*monitoring.TechSupportRequest{}),
	}

	for _, n := range smartNICNodes {
		err := it.createSmartNICNode(ctx, n)
		AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n))
		defer it.deleteSmartNICNode(ctx, &n.ObjectMeta)
	}

	for _, agent := range agents {
		go agent.watch(ctx, c)
		defer agent.stop()
	}

	for _, r := range tsrs {
		err := it.createTechSupportRequest(ctx, r)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
	}

	checkAgentNotifications(c, agents, 1)

	for _, r := range tsrs {
		err := it.deleteTechSupportRequest(ctx, &r.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
	}

	checkAgentNotifications(c, agents, 2)

	// Update labels on nodes and check that they are honored
	smartNIC2 := smartNICNodes["00ae.cd02.0012"]
	delete(smartNIC2.ObjectMeta.Labels, "group")
	smartNIC2.ObjectMeta.ResourceVersion = ""
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, smartNIC2)
	AssertOk(c, err, "Error updating labels for node 00ae.cd02.0012")
	agent2 := agents["00ae.cd02.0012"]
	delete(agent2.matchingRequests, "NIC1OrBlue")

	smartNIC4 := smartNICNodes["00ae.cd02.0014"]
	smartNIC4.ObjectMeta.Labels = map[string]string{"star": "no"}
	smartNIC4.ObjectMeta.ResourceVersion = ""
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(ctx, smartNIC4)
	AssertOk(c, err, "Error updating labels for node 00ae.cd02.0014")
	agent4 := agents["00ae.cd02.0014"]
	agent4.matchingRequests["LblStar"] = tsrs["LblStar"]

	clearNotifications(c, agents)

	for _, r := range tsrs {
		err := it.createTechSupportRequest(ctx, r)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
	}

	checkAgentNotifications(c, agents, 1)

	for _, r := range tsrs {
		err := it.deleteTechSupportRequest(ctx, &r.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
	}

	checkAgentNotifications(c, agents, 2)
}

func (it *veniceIntegSuite) TestTechSupportStatusUpdates(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), it.apiGwAddr, it.userCred)
	AssertOk(c, err, "Error creating logged in context")

	numAgents := 10
	reqName := "TSR"

	agentNames := []string{}
	for i := 0; i < numAgents; i++ {
		agentNames = append(agentNames, fmt.Sprintf("00ae.cd03.002%d", i))
	}

	tsr := newTechSupportRequest(reqName, agentNames, nil)

	agents := make(map[string]*techSupportAgent)
	for i := 0; i < numAgents; i++ {
		agentName := agentNames[i]

		n1 := newSmartNICNode(agentName, map[string]string{"group": "red", "type": "prod"})
		err = it.createSmartNICNode(ctx, n1)
		AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n1))
		defer it.deleteSmartNICNode(ctx, &n1.ObjectMeta)

		agent := newTechSupportAgent(agentName, kindSmartNICNode, []*monitoring.TechSupportRequest{tsr})
		agents[agentName] = agent
		go agent.watch(ctx, c)
		defer agent.stop()
	}

	err = it.createTechSupportRequest(ctx, tsr)
	AssertOk(c, err, "Error creating TechSupportRequest")

	checkAgentWatches(c, agents)
	instID := it.getTechSupportRequestInstanceID(ctx, c, reqName)

	Assert(c, it.getTechSupportRequestStatus(ctx, c, reqName) == monitoring.TechSupportJobStatus_Scheduled.String(), "Req status should be \"Scheduled\"")

	for _, agent := range agents {
		agent.sendUpdate(ctx, c, reqName, instID, tsproto.TechSupportRequestStatus_InProgress, true)
	}

	it.checkTechSupportNodeStatus(ctx, c, reqName, agentNames, monitoring.TechSupportJobStatus_Running)
	Assert(c, it.getTechSupportRequestStatus(ctx, c, reqName) == monitoring.TechSupportJobStatus_Running.String(), "Req status should be \"Running\"")

	for _, agent := range agents {
		agent.sendUpdate(ctx, c, reqName, instID, tsproto.TechSupportRequestStatus_Completed, true)
	}

	it.checkTechSupportNodeStatus(ctx, c, reqName, agentNames, monitoring.TechSupportJobStatus_Completed)
	Assert(c, it.getTechSupportRequestStatus(ctx, c, reqName) == monitoring.TechSupportJobStatus_Completed.String(), "Req status should be \"Completed\"")

	err = it.deleteTechSupportRequest(ctx, &tsr.ObjectMeta)
	AssertOk(c, err, "Error deleting TechSupportRequest")
	time.Sleep(time.Second)

	// Negative test-case: send update for deleted request
	for _, agent := range agents {
		agent.sendUpdate(ctx, c, reqName, instID, tsproto.TechSupportRequestStatus_Failed, false)
	}
}

func (it *veniceIntegSuite) TestTechSupportControllerRestart(c *C) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), it.apiGwAddr, it.userCred)
	AssertOk(c, err, "Error creating logged in context")

	// delete pre-existing requests, if any
	oldRequests, err := it.apisrvClient.MonitoringV1().TechSupportRequest().List(ctx, &api.ListWatchOptions{})
	AssertOk(c, err, "Error getting TechSupportRequest list")
	for _, r := range oldRequests {
		it.deleteTechSupportRequest(ctx, &r.ObjectMeta)
	}

	// count pre-existing nodes, if any
	oldNodes, err := it.apisrvClient.ClusterV1().DistributedServiceCard().List(ctx, &api.ListWatchOptions{})
	AssertOk(c, err, "Error getting nodes list")
	numOldNodes := len(oldNodes)

	// create some initial nodes
	numNodes := 1 + rand.Intn(20)
	nodes := []*cluster.DistributedServiceCard{}
	for i := 0; i < numNodes; i++ {
		n := newSmartNICNode(fmt.Sprintf("00ae.cd99.%04d", i), nil)
		err := it.createSmartNICNode(ctx, n)
		AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n))
		nodes = append(nodes, n)
		defer it.deleteSmartNICNode(ctx, &n.ObjectMeta)
	}

	// create some initial techsupport requests
	numRequests := 5 + rand.Intn(20)
	requests := []*monitoring.TechSupportRequest{}
	for i := 0; i < numRequests; i++ {
		r := newTechSupportRequest(fmt.Sprintf("00ae.cd99.%04d", i), []string{"00ae.cd99.0000"}, nil)
		err := it.createTechSupportRequest(ctx, r)
		AssertOk(c, err, fmt.Sprintf("Error creating TechSupportRequest %+v", r))
		requests = append(requests, r)
		defer it.deleteTechSupportRequest(ctx, &r.ObjectMeta)
	}

	// simulate restart by creating a new TSM controller instance
	tsCtrler, err := tsm.NewTsCtrler("localhost:0", "localhost:0", globals.APIServer, it.resolverClient)
	AssertOk(c, err, "Error creating new instance of TechSupportController")

	// New controller instance should have received notifications for existing requests
	sm := tsCtrler.StateMgr
	AssertEventually(c, func() (bool, interface{}) {
		ctRequests := sm.ListTechSupportObjectState(kindTechSupportRequest)
		if len(ctRequests) != numRequests {
			return false, nil
		}
		return true, nil
	}, fmt.Sprintf("New controller instance does not have correct number of techsupport requests. Have: %d, want: %d",
		len(sm.ListTechSupportObjectState(kindTechSupportRequest)), numRequests), "1s", "15s")

	// New controller instance should have received notifications for existing nodes
	AssertEventually(c, func() (bool, interface{}) {
		ctNodes := sm.ListTechSupportObjectState(kindSmartNICNode)
		if len(ctNodes) != numNodes+numOldNodes {
			return false, nil
		}
		return true, nil
	}, fmt.Sprintf("New controller instance does not have correct number of nodes. Have: %d, want: %d",
		len(sm.ListTechSupportObjectState(kindSmartNICNode)), numNodes+numOldNodes), "1s", "15s")

	tsmAPIAddr := tsCtrler.RPCServer.GetListenURL()
	agent := newTechSupportAgent("00ae.cd99.0000", kindSmartNICNode, requests)
	agent.tsmAPIAddr = tsmAPIAddr
	go agent.watch(ctx, c)
	defer agent.stop()

	agentMap := map[string]*techSupportAgent{"00ae.cd99.0000": agent}
	checkAgentWatches(c, agentMap)
	checkAgentNotifications(c, agentMap, 1)
}
