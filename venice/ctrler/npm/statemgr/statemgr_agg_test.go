// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func createPolicies(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		err := createPolicy(stateMgr, "testSp"+strconv.Itoa(i), "1")
		if err != nil {
			return err
		}
		_, err = stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return err
		}
	}
	return nil
}

func createPoliciesWithApps(stateMgr *Statemgr, start, end int, apps []string) error {

	for i := start; i < end; i++ {
		err := createPolicyWithApps(stateMgr, "testSp"+strconv.Itoa(i), apps, "1")
		if err != nil {
			return err
		}
		_, err = stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return err
		}
	}
	return nil
}

func generateApps(start, end int) []string {

	apps := []string{}
	for i := 100 + start; i < 100+end; i++ {
		name := "testApp" + strconv.Itoa(i)
		apps = append(apps, name)
	}
	return apps
}

func createApps(stateMgr *Statemgr, apps []string) error {

	for i, app := range apps {
		err := createApp(stateMgr, app, strconv.Itoa(i+100))
		if err != nil {
			return err
		}
		_, err = stateMgr.FindApp("default", app)
		if err != nil {
			return err
		}
	}
	return nil
}

func updatePolicies(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		policy, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return err
		}
		version, _ := strconv.Atoi(policy.NetworkSecurityPolicy.GenerationID)
		err = updatePolicy(stateMgr, "testSp"+strconv.Itoa(i), strconv.Itoa(version+1))
		if err != nil {
			return err
		}
	}
	return nil
}

func deletePolicies(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {

		err := deletePolicy(stateMgr, "testSp"+strconv.Itoa(i), "")
		if err != nil {
			return err
		}
		_, err = stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err == nil {
			return errors.New("Error delting policy")
		}
	}
	return nil
}

func deleteApps(stateMgr *Statemgr, start, end int) error {

	for i := 100 + start; i < 100+end; i++ {
		name := "testApp" + strconv.Itoa(i)
		err := deleteApp(stateMgr, name)
		if err != nil {
			return err
		}
		_, err = stateMgr.FindApp("default", name)
		if err == nil {
			return fmt.Errorf("Policy not delted")
		}
	}
	return nil
}

func propogationCompleteForPolicies(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		policy, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return err
		}

		for nodeID, version := range policy.NodeVersions {
			if version != policy.NetworkSecurityPolicy.GenerationID {
				return fmt.Errorf("Node version mimsatch for %v c : %v  e: %v", nodeID, version, policy.NetworkSecurityPolicy.GenerationID)
			}
		}

	}
	return nil
}

func createSGs(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		_, err := createSg(stateMgr, "default", "testSp"+strconv.Itoa(i), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		if err != nil {
			return err
		}
		_, err = stateMgr.FindSecurityGroup("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return fmt.Errorf("Not created security group : %v", err.Error())
		}
	}
	return nil
}

func deleteSGs(stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		_, err := deleteSg(stateMgr, "default", "testSp"+strconv.Itoa(i), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		if err != nil {
			return err
		}
		_, err = stateMgr.FindSecurityGroup("default", "testSp"+strconv.Itoa(i))
		if err == nil {
			return fmt.Errorf("Not delete security group")
		}
	}
	return nil
}

func newNimbusClient(uuid, url string) (*nimbus.NimbusClient, error) {

	rpckit.SetDefaultClientFactory(rpckit.NewClientFactory(uuid))

	rpcClient, err := rpckit.NewRPCClient("test-1234-12323", url,
		rpckit.WithTLSProvider(nil), rpckit.WithRemoteServerName("netctrler"))

	if err != nil {
		log.Errorf("Error connecting to grpc server. Err: %v", err)
		return nil, err
	}

	client, err := nimbus.NewNimbusClient("test-1234-12323", url, rpcClient)
	if err != nil {
		log.Errorf("Error creating nimbus client. Err: %v", err)
		return nil, err
	}

	return client, nil
}

type SecurityGroupReactor interface {
	CreateSecurityGroup(securitygroupObj *netproto.SecurityGroup) error     // creates an SecurityGroup
	FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) // finds an SecurityGroup
	ListSecurityGroup() []*netproto.SecurityGroup                           // lists all SecurityGroups
	UpdateSecurityGroup(securitygroupObj *netproto.SecurityGroup) error     // updates an SecurityGroup
	DeleteSecurityGroup(securitygroupObj, ns, name string) error            // deletes an SecurityGroup
	GetWatchOptions(cts context.Context, kind string) api.ObjectMeta
}

type NetworkSecurityPolicyReactor interface {
	CreateNetworkSecurityPolicy(networksecuritypolicyObj *netproto.NetworkSecurityPolicy) error // creates an NetworkSecurityPolicy
	FindNetworkSecurityPolicy(meta api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error)     // finds an NetworkSecurityPolicy
	ListNetworkSecurityPolicy() []*netproto.NetworkSecurityPolicy                               // lists all NetworkSecurityPolicys
	UpdateNetworkSecurityPolicy(networksecuritypolicyObj *netproto.NetworkSecurityPolicy) error // updates an NetworkSecurityPolicy
	DeleteNetworkSecurityPolicy(networksecuritypolicyObj, ns, name string) error                // deletes an NetworkSecurityPolicy
	GetWatchOptions(cts context.Context, kind string) api.ObjectMeta
}

type AppReactor interface {
	CreateApp(appObj *netproto.App) error               // creates an App
	FindApp(meta api.ObjectMeta) (*netproto.App, error) // finds an App
	ListApp() []*netproto.App                           // lists all Apps
	UpdateApp(appObj *netproto.App) error               // updates an App
	DeleteApp(appObj, ns, name string) error            // deletes an App
	GetWatchOptions(cts context.Context, kind string) api.ObjectMeta
}

const (
	CREATE = 1
	UPDATE = 2
	DELETE = 3
)

type event struct {
	create uint32
	update uint32
	delete uint32
}

type testAgent struct {
	sync.Mutex
	client           *nimbus.NimbusClient
	securityGroups   map[string]*netproto.SecurityGroup
	securityPolicies map[string]*netproto.NetworkSecurityPolicy
	apps             map[string]*netproto.App
	evtMap           map[string]*event
}

func (t *testAgent) updateEvent(kind string, eventType int) {
	evt, ok := t.evtMap[kind]
	if !ok {
		evt = &event{}
		t.evtMap[kind] = evt
	}
	switch eventType {
	case CREATE:
		evt.create++
	case UPDATE:
		evt.update++
	case DELETE:
		evt.delete++
	}
}

func (t *testAgent) CreateNetworkSecurityPolicy(obj *netproto.NetworkSecurityPolicy) error {
	t.Lock()
	defer t.Unlock()
	t.securityPolicies[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), CREATE)
	return nil
}

func (t *testAgent) FindNetworkSecurityPolicy(meta api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error) {
	t.Lock()
	defer t.Unlock()
	if sg, ok := t.securityPolicies[meta.GetKey()]; ok {
		return sg, nil
	}
	return nil, fmt.Errorf("Not found")
}

func (*testAgent) ListNetworkSecurityPolicy() []*netproto.NetworkSecurityPolicy {
	return nil
}

func (t *testAgent) UpdateNetworkSecurityPolicy(obj *netproto.NetworkSecurityPolicy) error {
	t.Lock()
	defer t.Unlock()
	t.securityPolicies[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), UPDATE)
	return nil
}

func (t *testAgent) DeleteNetworkSecurityPolicy(tenant, ns, name string) error {
	t.Lock()
	defer t.Unlock()
	sg := &netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: ns,
			Name:      name,
		},
	}
	if sg, ok := t.securityPolicies[sg.GetKey()]; ok {
		t.updateEvent(sg.GetKind(), DELETE)
		delete(t.securityPolicies, sg.GetKey())
		return nil
	}

	return fmt.Errorf("Not found")
}

func (*testAgent) GetWatchOptions(cts context.Context, kind string) api.ObjectMeta {
	return api.ObjectMeta{}
}

func (t *testAgent) CreateSecurityGroup(obj *netproto.SecurityGroup) error {
	t.Lock()
	defer t.Unlock()
	t.securityGroups[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), CREATE)
	return nil
}

func (t *testAgent) FindSecurityGroup(meta api.ObjectMeta) (*netproto.SecurityGroup, error) {
	t.Lock()
	defer t.Unlock()
	if sg, ok := t.securityGroups[meta.GetKey()]; ok {
		return sg, nil
	}
	return nil, fmt.Errorf("Not found")
}

func (*testAgent) ListSecurityGroup() []*netproto.SecurityGroup {
	return nil
}

func (t *testAgent) UpdateSecurityGroup(obj *netproto.SecurityGroup) error {
	t.Lock()
	defer t.Unlock()
	t.securityGroups[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), UPDATE)
	return nil
}

func (t *testAgent) DeleteSecurityGroup(tenant, ns, name string) error {
	t.Lock()
	defer t.Unlock()
	sg := &netproto.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: ns,
			Name:      name,
		},
	}
	if sg, ok := t.securityGroups[sg.GetKey()]; ok {
		t.updateEvent(sg.GetKind(), DELETE)
		delete(t.securityGroups, sg.GetKey())
		return nil
	}

	return fmt.Errorf("Not found")
}

func (t *testAgent) CreateApp(obj *netproto.App) error {
	t.Lock()
	defer t.Unlock()
	t.apps[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), CREATE)
	return nil
}

func (t *testAgent) FindApp(meta api.ObjectMeta) (*netproto.App, error) {
	t.Lock()
	defer t.Unlock()
	if sg, ok := t.apps[meta.GetKey()]; ok {
		return sg, nil
	}
	return nil, fmt.Errorf("Not found")
}

func (*testAgent) ListApp() []*netproto.App {
	return nil
}

func (t *testAgent) UpdateApp(obj *netproto.App) error {
	t.Lock()
	defer t.Unlock()
	t.apps[obj.GetKey()] = obj
	t.updateEvent(obj.GetKind(), UPDATE)
	return nil
}

func (t *testAgent) DeleteApp(tenant, ns, name string) error {
	t.Lock()
	defer t.Unlock()
	sg := &netproto.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: ns,
			Name:      name,
		},
	}
	if sg, ok := t.apps[sg.GetKey()]; ok {
		t.updateEvent(sg.GetKind(), DELETE)
		delete(t.apps, sg.GetKey())
		return nil
	}

	return fmt.Errorf("Not found")
}

func newTestAgent(uuid, url string) *testAgent {
	client, err := newNimbusClient(uuid, url)
	if err != nil {
		return nil
	}

	ag := &testAgent{client: client}
	ag.securityGroups = make(map[string]*netproto.SecurityGroup)
	ag.securityPolicies = make(map[string]*netproto.NetworkSecurityPolicy)
	ag.apps = make(map[string]*netproto.App)
	ag.evtMap = make(map[string]*event)

	time.Sleep(100 * time.Millisecond)

	return ag
}

func newStateMgrWithServer(grpcServer *rpckit.RPCServer) (*Statemgr, error) {

	// create nimbus server
	msrv := nimbus.NewMbusServer("npm-test", grpcServer)

	// create network state manager
	stateMgr, err := NewStatemgr(nil, globals.APIServer, nil, msrv, log.GetNewLogger(log.GetDefaultConfig("npm-test")))
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	err = createTenant(stateMgr, "default")
	if err != nil {
		return nil, err
	}

	return stateMgr, nil

}

func stopStateMgr(statemgr *Statemgr) {
}

func addSnic(stateMgr *Statemgr) (*cluster.DistributedServiceCard, error) {

	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: "0001.0203.0405",
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID: "test-snic",
		},
		Status: cluster.DistributedServiceCardStatus{
			PrimaryMAC:     "0001.0203.0405",
			AdmissionPhase: "admitted",
			Conditions:     []cluster.DSCCondition{cluster.DSCCondition{Type: "healthy", Status: "true"}},
		},
	}

	// create the smartNic
	err := stateMgr.ctrler.DistributedServiceCard().Create(&snic)

	return &snic, err

}
func TestSgCreateDeleteWithAgent(t *testing.T) {
	// create network state manager

	url := "localhost:55555"
	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)

	Assert(t, ag != nil, "Error Starting state manager")

	go ag.client.WatchSecurityGroups(context.Background(), ag)
	time.Sleep(50 * time.Millisecond)

	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Could not find the security group")
	AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 1, "received sg group by agent")

	// delete the security group
	err = stateMgr.ctrler.SecurityGroup().Delete(&sgs.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	// verify the sg is gone
	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	Assert(t, (err != nil), "Security group still found after deleting")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
}

func TestAggWatchWithSg(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup"}, ag)

	time.Sleep(100 * time.Millisecond)
	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
	AssertOk(t, err, "Could not find the security group")
	AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 1, "received sg group by agent")

	// delete the security group
	err = stateMgr.ctrler.SecurityGroup().Delete(&sgs.SecurityGroup.SecurityGroup)
	AssertOk(t, err, "Error deleting security group")

	// verify the sg is gone
	_, err = stateMgr.FindSecurityGroup("default", "testSg")
	Assert(t, (err != nil), "Security group still found after deleting")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
}

func TestAggWatchWithSgList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
}

func TestAggWatchWithSgAndPoliciesList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup", "NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")
}

func TestAggWatchWithSgAndPoliciesListOneAfterOther(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"NetworkSecurityPolicy"}, ag)
	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")
}

func TestAggWatchWithSgAndUpdatePoliciesList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup", "NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	//Now just update the policies

	ag.evtMap["NetworkSecurityPolicy"].update = 0
	err = updatePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error updating security policies")

	time.Sleep(20 * time.Millisecond)
	fmt.Printf("Update %v\n", ag.evtMap["NetworkSecurityPolicy"])
	Assert(t, ag.evtMap["NetworkSecurityPolicy"].update == 3, "Update successful")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_UpdateEvent)
	Assert(t, synced, "Config Sync")
}

func TestAggWatchWithSgAndUpdatePoliciesListWithDisconnect(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	cancelCtx, cancel := context.WithCancel(context.Background())
	go ag.client.WatchAggregate(cancelCtx, []string{"SecurityGroup", "NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	//lets cancel
	cancel()
	time.Sleep(100 * time.Millisecond)
	//Now just update the policies
	err = updatePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error updating security policies")

	//Forget everything
	ag.securityGroups = make(map[string]*netproto.SecurityGroup)
	ag.securityPolicies = make(map[string]*netproto.NetworkSecurityPolicy)
	ag.evtMap = make(map[string]*event)

	go ag.client.WatchAggregate(context.Background(), []string{"SecurityGroup", "NetworkSecurityPolicy"}, ag)
	time.Sleep(100 * time.Millisecond)

	time.Sleep(20 * time.Millisecond)
	Assert(t, ag.evtMap["NetworkSecurityPolicy"].create == 3, "Create successful")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_CreateEvent)
	Assert(t, synced, "Config Sync")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	synced = stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_DeleteEvent)
	Assert(t, synced, "Config Sync")

}

func TestAggWatchWithAppAndPolicyDep(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	apps := generateApps(0, 3)
	err = createApps(stateMgr, apps)
	AssertOk(t, err, "Error creating Apps")

	err = createPoliciesWithApps(stateMgr, 0, 3, apps)
	AssertOk(t, err, "Error creating security policies")

	go ag.client.WatchAggregate(context.Background(), []string{"App", "SecurityGroup", "NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.apps) == 3, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	err = deleteApps(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")
	Assert(t, len(ag.apps) == 0, "sg not deleted")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "App",
		api.EventType_CreateEvent)
	Assert(t, synced, "Config Sync")

	synced = stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "App",
		api.EventType_UpdateEvent)
	Assert(t, synced, "Config Sync")
}

func TestAggWatchWithAppAndPolicyDepOutOfOrder(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	go ag.client.WatchAggregate(context.Background(), []string{"App", "SecurityGroup", "NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.apps) == 0, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	err = createSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	apps := generateApps(0, 3)
	err = createPoliciesWithApps(stateMgr, 0, 3, apps)
	AssertOk(t, err, "Error creating security policies")

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.apps) == 0, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	err = createApps(stateMgr, apps)
	AssertOk(t, err, "Error creating Apps")

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 3, "received sg group by agent")
	Assert(t, len(ag.apps) == 3, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	err = deleteSGs(stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	err = deleteApps(stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 0, "received sg group by agent")
	Assert(t, len(ag.apps) == 3, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 3, "received sg group by agent")

	err = deletePolicies(stateMgr, 0, 3)

	AssertOk(t, err, "Error deleting security policies")
	time.Sleep(100 * time.Millisecond)

	Assert(t, len(ag.securityGroups) == 0, "sg not deleted")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")
	Assert(t, len(ag.apps) == 0, "sg not deleted")
}
