// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/protos/generated/nimbus"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func createPolicies(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		err := createPolicy(stateMgr, "testSp"+strconv.Itoa(i), "1")
		if err != nil {
			return err
		}
		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Sg not found", "1ms", "1s")

	}
	return nil
}

func createPoliciesWithApps(t *testing.T, stateMgr *Statemgr, start, end int, apps []string) error {

	for i := start; i < end; i++ {
		err := createPolicyWithApps(stateMgr, "testSp"+strconv.Itoa(i), apps, "1")
		if err != nil {
			return err
		}

		allAppsPresent := true
		for _, app := range apps {
			_, err := stateMgr.FindApp("default", app)
			if err != nil {
				allAppsPresent = false
				break
			}
		}

		if allAppsPresent {
			AssertEventually(t, func() (bool, interface{}) {

				_, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
				if err == nil {
					return true, nil
				}
				return false, nil
			}, "Policy not found", "1ms", "1s")
		} else {
			AssertEventually(t, func() (bool, interface{}) {

				_, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
				if err == nil {
					return false, nil
				}
				return true, nil
			}, "Policy found when not expected", "1ms", "1s")
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

func createApps(t *testing.T, stateMgr *Statemgr, apps []string) error {

	for i, app := range apps {
		err := createApp(stateMgr, app, strconv.Itoa(i+100))
		if err != nil {
			return err
		}
		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindApp("default", app)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "App not found", "1ms", "1s")
	}
	return nil
}

func updateApps(t *testing.T, stateMgr *Statemgr, apps []string) error {

	for i, app := range apps {
		err := updateApp(stateMgr, app, strconv.Itoa(i+100))
		if err != nil {
			return err
		}
		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindApp("default", app)
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "App not found", "1ms", "1s")
	}
	return nil
}

func updatePolicies(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		policy, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
		if err != nil {
			return err
		}

		version, _ := strconv.Atoi(policy.NetworkSecurityPolicy.GenerationID)
		newGENID := strconv.Itoa(version + 1)
		err = updatePolicy(stateMgr, "testSp"+strconv.Itoa(i), newGENID)
		if err != nil {
			return err
		}

		AssertEventually(t, func() (bool, interface{}) {

			policy, err = stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
			if err != nil {
				fmt.Printf("Did not find policy....\n")
				return false, nil
			}
			if policy.NetworkSecurityPolicy.GenerationID != newGENID {
				fmt.Printf("Gen ID mismatch...\n")
				return false, nil
			}
			return true, nil
		}, "Sg updated", "1ms", "1s")

	}
	return nil
}

func deletePolicies(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {

		err := deletePolicy(stateMgr, "testSp"+strconv.Itoa(i), "")
		if err != nil {
			return err
		}
		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindSgpolicy("default", "testSp"+strconv.Itoa(i))
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Sg still found", "1ms", "1s")
	}
	return nil
}

func deleteApps(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := 100 + start; i < 100+end; i++ {
		name := "testApp" + strconv.Itoa(i)
		err := deleteApp(stateMgr, name)
		if err != nil {
			return err
		}

		AssertEventually(t, func() (bool, interface{}) {
			app, err := stateMgr.FindApp("default", "testApp"+strconv.Itoa(i))
			if err != nil {
				fmt.Printf("Did not find the appp afer delete %v...\n", name)
				return true, nil
			}
			fmt.Printf("Trying to find policis...\n")
			policies, _ := stateMgr.ctrler.NetworkSecurityPolicy().List(context.Background(), &api.ListWatchOptions{})
			fmt.Printf("Found  policis...%v\n", policies)
			for _, policy := range policies {
				//policy.Spec.References
				resp := make(map[string]apiintf.ReferenceObj)
				policy.References(app.App.GetObjectMeta().Name, app.App.GetObjectMeta().Namespace, resp)
				fmt.Printf("Found  references for pol %v \n", resp)
				for _, ref := range resp {
					if ref.RefKind == "App" {
						for _, key := range ref.Refs {
							splits := strings.Split(key, "/")
							if splits[len(splits)-1] == "testApp"+strconv.Itoa(i) {
								//Not deleted as there is reference
								return true, nil
							}
						}

					}
				}
			}
			fmt.Printf("Deleted App %v \n", name)
			return false, nil
		}, "App still found", "1ms", "1s")
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

func createSGs(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		_, err := createSg(stateMgr, "default", "testSg"+strconv.Itoa(i), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		if err != nil {
			return err
		}

		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindSecurityGroup("default", "testSg"+strconv.Itoa(i))
			if err == nil {
				return true, nil
			}
			return false, nil
		}, "Sg still found", "1ms", "1s")

	}
	return nil
}

func deleteSGs(t *testing.T, stateMgr *Statemgr, start, end int) error {

	for i := start; i < end; i++ {
		_, err := deleteSg(stateMgr, "default", "testSg"+strconv.Itoa(i), labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		if err != nil {
			return err
		}
		AssertEventually(t, func() (bool, interface{}) {

			_, err := stateMgr.FindSecurityGroup("default", "testSg"+strconv.Itoa(i))
			if err != nil {
				return true, nil
			}
			return false, nil
		}, "Sg still found", "1ms", "1s")
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

type NetworkSecurityPolicyReactor interface {
	CreateNetworkSecurityPolicy(networksecuritypolicyObj *netproto.NetworkSecurityPolicy) error // creates an NetworkSecurityPolicy
	FindNetworkSecurityPolicy(meta api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error)     // finds an NetworkSecurityPolicy
	ListNetworkSecurityPolicy() []*netproto.NetworkSecurityPolicy                               // lists all NetworkSecurityPolicys
	UpdateNetworkSecurityPolicy(networksecuritypolicyObj *netproto.NetworkSecurityPolicy) error // updates an NetworkSecurityPolicy
	DeleteNetworkSecurityPolicy(networksecuritypolicyObj, ns, name string) error                // deletes an NetworkSecurityPolicy
	GetWatchOptions(cts context.Context, kind string) api.ListWatchOptions
}

type AppReactor interface {
	CreateApp(appObj *netproto.App) error               // creates an App
	FindApp(meta api.ObjectMeta) (*netproto.App, error) // finds an App
	ListApp() []*netproto.App                           // lists all Apps
	UpdateApp(appObj *netproto.App) error               // updates an App
	DeleteApp(appObj, ns, name string) error            // deletes an App
	GetWatchOptions(cts context.Context, kind string) api.ListWatchOptions
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

func (*testAgent) GetWatchOptions(cts context.Context, kind string) api.ListWatchOptions {
	return api.ListWatchOptions{}
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
	ag.securityPolicies = make(map[string]*netproto.NetworkSecurityPolicy)
	ag.apps = make(map[string]*netproto.App)
	ag.evtMap = make(map[string]*event)

	time.Sleep(100 * time.Millisecond)

	return ag
}

func newStateMgrWithServer(t *testing.T, grpcServer *rpckit.RPCServer) (*Statemgr, error) {

	// create nimbus server
	msrv := nimbus.NewMbusServer("npm-test", grpcServer)

	// create network state manager
	stateMgr, err := NewStatemgr(nil, globals.APIServer, nil, msrv, log.GetNewLogger(log.GetDefaultConfig("npm-test")))
	if err != nil {
		log.Errorf("Could not create network manager. Err: %v", err)
		return nil, err
	}

	err = createTenant(t, stateMgr, "default")
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

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)

	Assert(t, ag != nil, "Error Starting state manager")

	// create sg
	sg, err := createSg(stateMgr, "default", "testSg", labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
	AssertOk(t, err, "Error creating security group")

	// verify we can find the sg
	AssertEventually(t, func() (bool, interface{}) {

		sgs, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err != nil {
			return false, nil
		}
		AssertEquals(t, sgs.SecurityGroup.Spec.WorkloadSelector.String(), sg.Spec.WorkloadSelector.String(), "Security group params did not match")
		return true, nil
	}, "Sg not found", "1ms", "1s")

	// delete the security group
	err = stateMgr.ctrler.SecurityGroup().Delete(sg)
	AssertOk(t, err, "Error deleting security group")

	AssertEventually(t, func() (bool, interface{}) {

		_, err := stateMgr.FindSecurityGroup("default", "testSg")
		if err != nil {
			return true, nil
		}
		return false, nil
	}, "Sg still found", "1ms", "1s")

}

func TestAggWatchWithSgList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

}

func TestAggWatchWithSgAndPoliciesList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"NetworkSecurityPolicy"}, ag)

	time.Sleep(100 * time.Millisecond)

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 3 {
			return true, nil
		}

		return false, fmt.Errorf("did not receive the expected security policies")
	}, fmt.Sprintf("Security policies not received by the agent. Expected : %v, Got : %v. Agent - %v", 3, len(ag.securityPolicies), ag), "100ms", "5s")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Security Policies not deleted")
	}, fmt.Sprintf("Security policies not deleted for agent. %v", ag), "100ms", "5s")
}

func TestAggWatchWithSgAndPoliciesListOneAfterOther(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"NetworkSecurityPolicy"}, ag)

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 3 {
			return true, nil
		}

		return false, fmt.Errorf("did not receive the expected security policies")
	}, fmt.Sprintf("Security policies not received by the agent. Expected : %v, Got : %v. Agent - %v", 3, len(ag.securityPolicies), ag), "100ms", "5s")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("security policies not deleted")
	}, fmt.Sprintf("Security policies not delted from Agent - %v", ag), "100ms", "5s")
}

func TestAggWatchWithSgAndUpdatePoliciesList(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	go ag.client.WatchAggregate(context.Background(), []string{"NetworkSecurityPolicy"}, ag)

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	//Now just update the policies

	ag.evtMap["NetworkSecurityPolicy"].update = 0
	err = updatePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error updating security policies")

	time.Sleep(20 * time.Millisecond)
	fmt.Printf("Update %v\n", ag.evtMap["NetworkSecurityPolicy"])
	Assert(t, ag.evtMap["NetworkSecurityPolicy"].update == 3, "Update successful")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_UpdateEvent)
	Assert(t, synced, "Config Sync")
}

func TestAggWatchWithSgAndUpdatePoliciesListWithDisconnect(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	err = createPolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	//Make sure agent has not received any objects.
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	cancelCtx, cancel := context.WithCancel(context.Background())
	go ag.client.WatchAggregate(cancelCtx, []string{"NetworkSecurityPolicy"}, ag)

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 3 {
			return true, nil
		}

		return false, fmt.Errorf("did not receive the expected security policies")
	}, fmt.Sprintf("Security groups not received by the agent. Expected : %v, Got : %v. Agent - %v", 3, len(ag.securityPolicies), ag), "100ms", "5s")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	//lets cancel
	cancel()
	time.Sleep(100 * time.Millisecond)
	//Now just update the policies
	err = updatePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error updating security policies")

	//Forget everything
	ag.securityPolicies = make(map[string]*netproto.NetworkSecurityPolicy)
	ag.evtMap = make(map[string]*event)

	go ag.client.WatchAggregate(context.Background(), []string{"NetworkSecurityPolicy"}, ag)
	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) != 3 {
			return false, fmt.Sprintf("expected : %d, got : %v. Agent Object : %v", 3, len(ag.securityPolicies), ag)
		}
		return true, nil
	}, fmt.Sprintf("Expected number of security groups or policies not found. %v", ag), "100ms", "1s")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_CreateEvent)
	Assert(t, synced, "Config Sync")

	time.Sleep(2 * time.Second)
	err = propogationCompleteForPolicies(stateMgr, 0, 3)
	AssertOk(t, err, "Error in policy propogation.")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Security policies not deleted")
	}, fmt.Sprintf("Security policies not deleted for agent. %v", ag), "100ms", "5s")

	synced = stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "NetworkSecurityPolicy",
		api.EventType_DeleteEvent)
	Assert(t, synced, "Config Sync")

}

func TestAggWatchWithAppAndPolicyDep(t *testing.T) {
	// create network state manager

	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	apps := generateApps(0, 10)
	err = createApps(t, stateMgr, apps)
	AssertOk(t, err, "Error creating Apps")

	err = updateApps(t, stateMgr, apps)
	AssertOk(t, err, "Error creating Apps")

	err = createPoliciesWithApps(t, stateMgr, 0, 3, apps)
	AssertOk(t, err, "Error creating security policies")

	go ag.client.WatchAggregate(context.Background(), []string{"App", "NetworkSecurityPolicy"}, ag)

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) != 3 {
			return false, fmt.Sprintf("expected : %d, got : %v. Agent Object : %v", 3, len(ag.securityPolicies), ag)
		}
		if len(ag.apps) != 10 {
			return false, fmt.Sprintf("expected : %d, got : %v. Agent Object : %v", 10, len(ag.apps), ag)
		}
		return true, nil
	}, fmt.Sprintf("Expected number of security groups or policies or apps not found. %v", ag), "100ms", "5s")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")
	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	err = deleteApps(t, stateMgr, 0, 10)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Security policies not deleted")
	}, fmt.Sprintf("Security policies not deleted for agent. %v", ag), "100ms", "5s")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.apps) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Apps not deleted")
	}, fmt.Sprintf("Apps not deleted for agent. %v", ag), "100ms", "5s")

	synced := stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "App",
		api.EventType_CreateEvent)
	Assert(t, synced, "Config Sync")

	synced = stateMgr.topics.AggregateTopic.WatcherInConfigSync(snic.Status.PrimaryMAC, "App",
		api.EventType_UpdateEvent)
	Assert(t, synced, "Config Sync")
}

func TestAggWatchWithAppAndPolicyDepOutOfOrder(t *testing.T) {
	// create network state manager

	t.Skip("Skipping this as obj resolved is not enabled in unit tests")
	url := "localhost:55555"

	grpcServer, err := rpckit.NewRPCServer("netctrler", url, rpckit.WithTLSProvider(nil))
	AssertOk(t, err, "Error Starting rpc server")

	stateMgr, err := newStateMgrWithServer(t, grpcServer)
	AssertOk(t, err, "Error Starting state manager")
	grpcServer.Start()
	defer grpcServer.Stop()

	snic, err := addSnic(stateMgr)
	ag := newTestAgent(snic.Status.PrimaryMAC, url)
	Assert(t, ag != nil, "Error Starting state manager")

	time.Sleep(100 * time.Millisecond)

	go ag.client.WatchAggregate(context.Background(), []string{"App", "NetworkSecurityPolicy"}, ag)
	Assert(t, len(ag.apps) == 0, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	err = createSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security groups")

	apps := generateApps(0, 3)
	err = createPoliciesWithApps(t, stateMgr, 0, 3, apps)
	AssertOk(t, err, "Error creating security policies")

	time.Sleep(100 * time.Millisecond)
	Assert(t, len(ag.apps) == 0, "received apps group by agent")
	Assert(t, len(ag.securityPolicies) == 0, "received sg group by agent")

	err = createApps(t, stateMgr, apps)
	AssertOk(t, err, "Error creating Apps")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.apps) != 3 {
			return false, fmt.Sprintf("expected : %d, got : %v. Agent Object : %v", 3, len(ag.apps), ag)
		}
		if len(ag.securityPolicies) != 3 {
			return false, fmt.Sprintf("expected : %d, got : %v. Agent Object : %v", 3, len(ag.securityPolicies), ag)
		}
		return true, nil
	}, fmt.Sprintf("Expected number of security groups, or apps or policies not found. %v", ag), "100ms", "1s")

	err = deleteSGs(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error creating security policies")

	err = deleteApps(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		return false, fmt.Errorf("Security Groups not deleted")
	}, fmt.Sprintf("Security groups not deleted for agent. %v", ag), "100ms", "5s")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.apps) == 3 {
			return true, nil
		}

		return false, fmt.Errorf("Apps not not received")
	}, fmt.Sprintf("Apps not received by agent. %v", ag), "100ms", "5s")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 3 {
			return true, nil
		}

		return false, fmt.Errorf("Security policies not received by agent")
	}, fmt.Sprintf("Security policies not received by the agent. %v", ag), "100ms", "5s")

	err = deletePolicies(t, stateMgr, 0, 3)
	AssertOk(t, err, "Error deleting security policies")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.securityPolicies) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Security policies not deleted")
	}, fmt.Sprintf("Security policies not deleted for agent. %v", ag), "100ms", "5s")

	AssertEventually(t, func() (bool, interface{}) {
		if len(ag.apps) == 0 {
			return true, nil
		}

		return false, fmt.Errorf("Apps not deleted")
	}, fmt.Sprintf("Apps not deleted for agent. %v", ag), "100ms", "5s")
}
