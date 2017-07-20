/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package vchub

import (
	"reflect"
	"runtime/debug"
	"testing"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/vmware/govmomi/vim25/soap"
	"golang.org/x/net/context"
	"google.golang.org/grpc"

	infraapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/pensando/sw/orch/vchub/sim"
	"github.com/pensando/sw/orch/vchub/store"
	vcp "github.com/pensando/sw/orch/vchub/vcprobe"
	kv "github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/utils/kvstore/memkv"
	kvs "github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/vic/pkg/vsphere/simulator/esx"
)

const (
	serverAddr  = "127.0.0.1:" + globals.VCHubAPIPort
	testNic1Mac = "6a:00:02:e7:a8:40"
	testNic2Mac = "6a:00:02:e7:aa:54"
	testIf1Mac  = "6e:00:02:e7:dd:40"
	testIf2Mac  = "6e:00:02:e7:dc:54"
	testIf1ID   = "52fd7958-f4da-78bb-1590-856861348cee:4001"
	testIf2ID   = "53256758-eecc-79bb-1590-899861348cfd:4004"
	waitTO      = 2 * time.Second
)

type TestSuite struct {
	cc          *grpc.ClientConn
	vcHubClient api.VCHubApiClient
	cluster     *integration.ClusterV3
	testNics    []*api.SmartNIC
	testIfs     []*api.NwIF
	testStore   *memkv.MemKv
}

func (ts *TestSuite) setup(t *testing.T, fake bool) {
	startVCHServer()
	// setup store and server
	if fake {
		s, err := store.Init("", kvs.KVStoreTypeMemkv)
		if err != nil {
			t.Fatalf("store.Init returned %v", err)
		}

		m, ok := s.(*memkv.MemKv)
		if !ok {
			t.Fatalf("Bad store type")
		}
		ts.testStore = m
	} else {
		ts.cluster = integration.NewClusterV3(t)
		sURL := ts.cluster.ClientURL()
		store.Init(sURL, kvs.KVStoreTypeEtcd)
	}

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(serverAddr, opts...)
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	ts.cc = conn
	ts.vcHubClient = api.NewVCHubApiClient(conn)
	ts.testNics = make([]*api.SmartNIC, 2)
	ts.testNics[0] = &api.SmartNIC{
		ObjectKind:       "SmartNIC",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "ServerA.vmnic0",
			ResourceVersion: "3",
		},
		Status: &api.SmartNIC_Status{
			HostIP:     "10.193.231.234",
			MacAddress: testNic1Mac,
			Switch:     "Pensando-dvs-1",
		},
	}

	ts.testNics[1] = &api.SmartNIC{
		ObjectKind:       "SmartNIC",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "ServerB.vmnic0",
			ResourceVersion: "4",
		},
		Status: &api.SmartNIC_Status{
			HostIP:     "10.193.231.251",
			MacAddress: testNic2Mac,
			Switch:     "Pensando-dvs-1",
		},
	}

	ts.testIfs = make([]*api.NwIF, 2)
	ts.testIfs[0] = &api.NwIF{
		ObjectKind:       "NwIF",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "warrior1.eth0",
			ResourceVersion: "4",
			UUID:            "52fd7958-f4da-78bb-1590-856861348cee:4001",
		},
		Config: &api.NwIF_Config{
			LocalVLAN: 321,
		},
		Status: &api.NwIF_Status{
			MacAddress:  testIf1Mac,
			PortGroup:   "Pensando-default",
			Switch:      "Pensando-dvs-1",
			SmartNIC_ID: testNic1Mac,
		},
		Attributes: map[string]string{
			"VMName": "warrior1",
			"Org":    "Winner",
		},
	}
	ts.testIfs[1] = &api.NwIF{
		ObjectKind:       "NwIF",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "cav1.eth0",
			ResourceVersion: "4",
			UUID:            "53256758-eecc-79bb-1590-899861348cfd:4004",
		},
		Config: &api.NwIF_Config{
			LocalVLAN: 322,
		},
		Status: &api.NwIF_Status{
			MacAddress:  testIf2Mac,
			PortGroup:   "Pensando-default",
			Switch:      "Pensando-dvs-1",
			SmartNIC_ID: testNic2Mac,
		},
		Attributes: map[string]string{
			"VMName": "cav1",
			"Org":    "RunnerUp",
		},
	}

	sim.Setup()
}

func (ts *TestSuite) teardown(t *testing.T) {
	ts.cc.Close()
	if ts.cluster != nil {
		ts.cluster.Terminate(t)
	}
	stopVCHServer()
	sim.TearDown()
	time.Sleep(200 * time.Millisecond)
}

func (ts *TestSuite) w4Channel(t *testing.T, prefix string, active bool) {
	for i := 0; i < 30; i++ {
		if suite.testStore.IsWatchActive(prefix) == active {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if suite.testStore.IsWatchActive(prefix) != active {
		debug.PrintStack()
		t.Fatalf("Watch channel not in expected state after 3 sec")
	}
}

var suite *TestSuite

func verifySmartNICList(t *testing.T, expected map[string]*api.SmartNIC) {
	filter := &api.Filter{}
	nicList, err := suite.vcHubClient.ListSmartNICs(context.Background(), filter)
	if err != nil {
		t.Errorf("Error listing nics %v", err)
	}
	nics := nicList.GetItems()

	if len(nics) != len(expected) {
		t.Errorf("Expected %d items, got %+v", len(expected), nics)
	}

	for _, n1 := range nics {
		n2 := expected[n1.Status.MacAddress]
		// ignore resource version for now
		n1.ObjectMeta.ResourceVersion = n2.ObjectMeta.ResourceVersion
		if !reflect.DeepEqual(n1, n2) {
			t.Errorf("Expected %+v, got %+v", n2, n1)
		}
		delete(expected, n1.Status.MacAddress)
	}
}

func TestListSmartNICs(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	nicMap := make(map[string]*api.SmartNIC)
	// verify empty list
	verifySmartNICList(t, nicMap) // empties the map!

	nicMap[testNic1Mac] = suite.testNics[0]
	nicMap[testNic2Mac] = suite.testNics[1]

	for k, v := range nicMap {
		err = store.SmartNICCreate(context.Background(), k, v)
		if err != nil {
			t.Errorf("SmartNICCreate failed %v", err)
		}
	}

	verifySmartNICList(t, nicMap) // empties the map!

	// Delete one object and verify
	err = store.SmartNICDelete(context.Background(), testNic1Mac)
	nicMap[testNic2Mac] = suite.testNics[1]
	verifySmartNICList(t, nicMap) // empties the map!
}

func verifyWatch(t *testing.T, sndMap, rcvMap interface{}, wc <-chan int, loopCount int) {

	for i := 0; i < loopCount; i++ {
		select {

		case <-wc:

		case <-time.After(waitTO):
			t.Errorf("Timed out waiting for watch event")

		}
	}

	if !reflect.DeepEqual(rcvMap, sndMap) {
		t.Errorf("Expected %+v, got %+v", sndMap, rcvMap)
	}
}

func watchSmartNICs(rcvCtx context.Context, t *testing.T, rcvMap map[string]*api.SmartNIC, eventMap map[api.WatchEvent_EventType]int, ackCh chan<- int) {

	defer close(ackCh)
	ws := &api.WatchSpec{}
	stream, err := suite.vcHubClient.WatchSmartNICs(rcvCtx, ws)
	if err != nil {
		t.Fatalf("Error %v", err)
	}

	for {
		count := 0
		e, err := stream.Recv()

		if err != nil {
			return
		}

		event := e.GetE()
		nics := e.GetSmartnics()
		if event.Event == api.WatchEvent_Delete {
			for _, nic := range nics {
				delete(rcvMap, nic.Status.MacAddress)
			}
		} else {
			for _, nic := range nics {
				rcvMap[nic.Status.MacAddress] = nic
			}
		}

		eventMap[event.Event] = eventMap[event.Event] + 1
		count++
		ackCh <- count
	}

}

func TestWatchSmartNICs(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	rcvMap := make(map[string]*api.SmartNIC)
	sndMap := make(map[string]*api.SmartNIC)

	// start watch on an empty database
	rcvCtx, rcvCancel := context.WithCancel(context.Background())
	eventMap := make(map[api.WatchEvent_EventType]int)
	ackCh := make(chan int)
	go watchSmartNICs(rcvCtx, t, rcvMap, eventMap, ackCh)

	// Create an object and verify
	err := store.SmartNICCreate(context.Background(), suite.testNics[0].Status.MacAddress, suite.testNics[0])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
	}

	sndMap[suite.testNics[0].Status.MacAddress] = suite.testNics[0]
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Create another object and verify
	err = store.SmartNICCreate(context.Background(), suite.testNics[1].Status.MacAddress, suite.testNics[1])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
	}

	sndMap[suite.testNics[1].Status.MacAddress] = suite.testNics[1]
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Update an object and verify
	suite.testNics[1].ObjectMeta.Name = "Butterfly.vmnic0"
	err = store.SmartNICUpdate(context.Background(), suite.testNics[1].Status.MacAddress, suite.testNics[1])
	if err != nil {
		t.Errorf("SmartNICUpdate failed %v", err)
	}

	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Delete an object and verify
	err = store.SmartNICDelete(context.Background(), suite.testNics[1].Status.MacAddress)
	if err != nil {
		t.Errorf("SmartNICDelete failed %v", err)
	}

	delete(sndMap, suite.testNics[1].Status.MacAddress)
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)
	rcvCancel()
	// Verify the event count
	if eventMap[api.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[api.WatchEvent_Create])
	}
	if eventMap[api.WatchEvent_Delete] != 1 {
		t.Errorf("Expected 1 delete event, got :%d", eventMap[api.WatchEvent_Delete])
	}
	if eventMap[api.WatchEvent_Update] != 1 {
		t.Errorf("Expected 1 update event, got :%d", eventMap[api.WatchEvent_Update])
	}

	delete(rcvMap, suite.testNics[0].Status.MacAddress)
	delete(rcvMap, suite.testNics[1].Status.MacAddress)
	for k := range eventMap {
		eventMap[k] = 0
	}
	// Update, Create, Delete, Create
	suite.testNics[0].Status.HostIP = "10.193.246.201"
	err = store.SmartNICUpdate(context.Background(), suite.testNics[0].Status.MacAddress, suite.testNics[0])
	if err != nil {
		t.Errorf("SmartNICUpdate failed %v", err)
	}
	err = store.SmartNICCreate(context.Background(), suite.testNics[1].Status.MacAddress, suite.testNics[1])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
	}
	err = store.SmartNICDelete(context.Background(), suite.testNics[0].Status.MacAddress)
	if err != nil {
		t.Errorf("SmartNICDelete failed %v", err)
	}
	err = store.SmartNICCreate(context.Background(), suite.testNics[0].Status.MacAddress, suite.testNics[0])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
	}
	sndMap[suite.testNics[0].Status.MacAddress] = suite.testNics[0]
	sndMap[suite.testNics[1].Status.MacAddress] = suite.testNics[1]

	// start watch on the updated database
	rcvCtx, rcvCancel = context.WithCancel(context.Background())
	ackCh = make(chan int, 4)
	go watchSmartNICs(rcvCtx, t, rcvMap, eventMap, ackCh)
	verifyWatch(t, sndMap, rcvMap, ackCh, 2)

	// Verify the event count
	if eventMap[api.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[api.WatchEvent_Create])
	}
}

func TestSmartNICInspect(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, true)
	defer suite.teardown(t)

	// verify initial values of stats
	empty := &api.Empty{}
	stats, err := suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount != 0 || stats.StoreWatchErrCount != 0 || stats.StoreWatchBadEventCount != 0 || stats.WatchCloseCount != 0 || stats.GrpcSendErrCount != 0 {
		t.Errorf("Expected zero stats. Got %+v", stats)
	}

	ws := &api.WatchSpec{}
	wCtx, wCancel := context.WithCancel(context.Background())
	// Generate an error from store.
	suite.testStore.SetErrorState(true)
	stream, err := suite.vcHubClient.WatchSmartNICs(wCtx, ws)
	if stream != nil {
		_, err = stream.Recv()
		if err == nil {
			t.Errorf("Expected error, got success")
		}
	}
	wCancel()
	suite.testStore.SetErrorState(false)

	// Generate errors from store.
	wCtx, wCancel = context.WithCancel(context.Background())
	stream, err = suite.vcHubClient.WatchSmartNICs(wCtx, ws)
	if err != nil {
		t.Fatalf("Watch failed -- %v", err)
	}

	doneCh := make(chan bool)
	go func() {
		for {
			_, err = stream.Recv()
			if err != nil {
				close(doneCh)
				return
			}
		}
	}()

	suite.w4Channel(t, "/vchub/smartnics/", true)

	// Inject a WatcherError
	errEv := &kv.WatchEvent{Type: kv.WatcherError}
	suite.testStore.InjectWatchEvent("/vchub/smartnics/", errEv, 1)
	suite.w4Channel(t, "/vchub/smartnics/", false)
	suite.w4Channel(t, "/vchub/smartnics/", true)

	// Inject a Badevent
	badEv := &kv.WatchEvent{Type: "BAD"}
	suite.testStore.InjectWatchEvent("/vchub/smartnics/", badEv, 1)
	suite.w4Channel(t, "/vchub/smartnics/", false)
	suite.w4Channel(t, "/vchub/smartnics/", true)

	// Inject an alien obj
	alienObj := suite.testIfs[1]
	alienEv := &kv.WatchEvent{Type: kv.Created, Object: alienObj}
	suite.testStore.InjectWatchEvent("/vchub/smartnics/", alienEv, 1)

	// Cause a close
	suite.testStore.CloseWatch("/vchub/smartnics/")
	<-doneCh
	time.Sleep(1 * time.Second)

	// Generate send error
	wCtx, wCancel = context.WithCancel(context.Background())
	stream, err = suite.vcHubClient.WatchSmartNICs(wCtx, ws)
	if err != nil {
		t.Fatalf("Watch failed -- %v", err)
	}
	doneCh = make(chan bool)
	go func() {
		stream.Recv()
		close(doneCh)
	}()
	suite.w4Channel(t, "/vchub/smartnics/", true)

	wCancel()
	<-doneCh
	time.Sleep(1 * time.Second)

	// Inject a valid obj
	validObj := suite.testNics[0]
	validEv := &kv.WatchEvent{Type: kv.Created, Object: validObj}
	suite.testStore.InjectWatchEvent("/vchub/smartnics/", validEv, 1)
	suite.w4Channel(t, "/vchub/smartnics/", false)
	time.Sleep(1 * time.Second)

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(serverAddr, opts...)
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	suite.cc = conn
	suite.vcHubClient = api.NewVCHubApiClient(conn)
	empty = &api.Empty{}
	stats, err = suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount == 0 || stats.StoreWatchErrCount == 0 || stats.StoreWatchBadEventCount == 0 || stats.WatchCloseCount == 0 {
		t.Errorf("Expected non-zero stats. Got %+v", stats)
	}

}

func verifyNwIFList(t *testing.T, expected map[string]*api.NwIF) {
	filter := &api.Filter{}
	ifList, err := suite.vcHubClient.ListNwIFs(context.Background(), filter)
	if err != nil {
		t.Errorf("Error listing ifs %v", err)
	}
	ifs := ifList.GetItems()

	if len(ifs) != len(expected) {
		t.Errorf("Expected %d items, got %+v", len(expected), ifs)
	}

	for _, n1 := range ifs {
		n2 := expected[n1.ObjectMeta.UUID]
		if !reflect.DeepEqual(n1, n2) {
			t.Errorf("Expected %+v, got %+v", n2, n1)
		}
		delete(expected, n1.ObjectMeta.UUID)
	}
}

func TestListNwIFs(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	ifMap := make(map[string]*api.NwIF)

	// Verify empty list
	verifyNwIFList(t, ifMap)

	ifMap[testIf1ID] = suite.testIfs[0]
	ifMap[testIf2ID] = suite.testIfs[1]

	for k, v := range ifMap {
		err = store.NwIFCreate(context.Background(), k, v)
		if err != nil {
			t.Errorf("NwIFCreate failed %v", err)
		}
	}

	verifyNwIFList(t, ifMap) // empties the map!

	// Delete one object and verify
	err = store.NwIFDelete(context.Background(), testIf2ID)
	ifMap[testIf1ID] = suite.testIfs[0]
	verifyNwIFList(t, ifMap)
}

func watchNwIFs(rcvCtx context.Context, t *testing.T, rcvMap map[string]*api.NwIF, eventMap map[api.WatchEvent_EventType]int, ackCh chan<- int) {

	defer close(ackCh)
	ws := &api.WatchSpec{}
	stream, err := suite.vcHubClient.WatchNwIFs(rcvCtx, ws)
	if err != nil {
		t.Fatalf("Error %v", err)
	}

	for {
		count := 0
		e, err := stream.Recv()

		if err != nil {
			return
		}

		event := e.GetE()
		ifs := e.GetNwifs()
		if event.Event == api.WatchEvent_Delete {
			for _, nif := range ifs {
				delete(rcvMap, nif.ObjectMeta.UUID)
			}
		} else {
			for _, nif := range ifs {
				rcvMap[nif.ObjectMeta.UUID] = nif
			}
		}

		eventMap[event.Event] = eventMap[event.Event] + 1
		count++
		ackCh <- count
	}

}

func TestWatchNwIFs(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	rcvMap := make(map[string]*api.NwIF)
	sndMap := make(map[string]*api.NwIF)

	// start watch on an empty database
	rcvCtx, rcvCancel := context.WithCancel(context.Background())
	eventMap := make(map[api.WatchEvent_EventType]int)
	ackCh := make(chan int)
	go watchNwIFs(rcvCtx, t, rcvMap, eventMap, ackCh)

	// Create an object and verify
	err := store.NwIFCreate(context.Background(), testIf1ID, suite.testIfs[0])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
	}

	sndMap[testIf1ID] = suite.testIfs[0]
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Create another object and verify
	err = store.NwIFCreate(context.Background(), testIf2ID, suite.testIfs[1])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
	}

	sndMap[testIf2ID] = suite.testIfs[1]
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Update an object and verify
	suite.testIfs[1].ObjectMeta.Name = "spur.eth0"
	err = store.NwIFUpdate(context.Background(), testIf2ID, suite.testIfs[1])
	if err != nil {
		t.Errorf("NwIFUpdate failed %v", err)
	}

	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Delete an object and verify
	err = store.NwIFDelete(context.Background(), testIf2ID)
	if err != nil {
		t.Errorf("NwIFDelete failed %v", err)
	}

	delete(sndMap, testIf2ID)
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)

	// Another update
	suite.testIfs[0].Attributes["titles"] = "2"
	err = store.NwIFUpdate(context.Background(), testIf1ID, suite.testIfs[0])
	if err != nil {
		t.Errorf("NwIFUpdate failed %v", err)
	}
	verifyWatch(t, sndMap, rcvMap, ackCh, 1)
	rcvCancel()
	// Verify the event count
	if eventMap[api.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[api.WatchEvent_Create])
	}
	if eventMap[api.WatchEvent_Delete] != 1 {
		t.Errorf("Expected 1 delete event, got :%d", eventMap[api.WatchEvent_Delete])
	}
	if eventMap[api.WatchEvent_Update] != 2 {
		t.Errorf("Expected 1 update event, got :%d", eventMap[api.WatchEvent_Update])
	}

	delete(rcvMap, testIf1ID)
	delete(rcvMap, testIf2ID)
	for k := range eventMap {
		eventMap[k] = 0
	}
	// Create, Delete, Create
	err = store.NwIFCreate(context.Background(), testIf2ID, suite.testIfs[1])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
	}
	err = store.NwIFDelete(context.Background(), testIf1ID)
	if err != nil {
		t.Errorf("NwIFDelete failed %v", err)
	}
	err = store.NwIFCreate(context.Background(), testIf1ID, suite.testIfs[0])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
	}
	sndMap[testIf1ID] = suite.testIfs[0]
	sndMap[testIf2ID] = suite.testIfs[1]

	// start watch on the updated database
	rcvCtx, rcvCancel = context.WithCancel(context.Background())
	ackCh = make(chan int, 4)
	go watchNwIFs(rcvCtx, t, rcvMap, eventMap, ackCh)
	verifyWatch(t, sndMap, rcvMap, ackCh, 2)

	// Verify the event count
	if eventMap[api.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[api.WatchEvent_Create])
	}
}

func TestNwIFInspect(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, true)
	defer suite.teardown(t)

	// verify initial values of stats
	empty := &api.Empty{}
	stats, err := suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount != 0 || stats.StoreWatchErrCount != 0 || stats.StoreWatchBadEventCount != 0 || stats.WatchCloseCount != 0 || stats.GrpcSendErrCount != 0 {
		t.Errorf("Expected zero stats. Got %+v", stats)
	}

	ws := &api.WatchSpec{}
	wCtx, wCancel := context.WithCancel(context.Background())
	// Generate an error from store.
	suite.testStore.SetErrorState(true)
	stream, err := suite.vcHubClient.WatchNwIFs(wCtx, ws)
	if stream != nil {
		_, err = stream.Recv()
		if err == nil {
			t.Errorf("Expected error, got success")
		}
	}
	wCancel()
	suite.testStore.SetErrorState(false)

	// Generate errors from store.
	wCtx, wCancel = context.WithCancel(context.Background())
	stream, err = suite.vcHubClient.WatchNwIFs(wCtx, ws)
	if err != nil {
		t.Fatalf("Watch failed -- %v", err)
	}

	doneCh := make(chan bool)
	go func() {
		for {
			_, err = stream.Recv()
			if err != nil {
				close(doneCh)
				return
			}
		}
	}()

	suite.w4Channel(t, "/vchub/nwifs/", true)

	// Inject a WatcherError
	errEv := &kv.WatchEvent{Type: kv.WatcherError}
	suite.testStore.InjectWatchEvent("/vchub/nwifs/", errEv, 1)
	suite.w4Channel(t, "/vchub/nwifs/", false)
	suite.w4Channel(t, "/vchub/nwifs/", true)

	// Inject a Badevent
	badEv := &kv.WatchEvent{Type: "BAD"}
	suite.testStore.InjectWatchEvent("/vchub/nwifs/", badEv, 1)
	suite.w4Channel(t, "/vchub/nwifs/", false)
	suite.w4Channel(t, "/vchub/nwifs/", true)

	// Inject an alien obj
	alienObj := suite.testNics[1]
	alienEv := &kv.WatchEvent{Type: kv.Created, Object: alienObj}
	suite.testStore.InjectWatchEvent("/vchub/nwifs/", alienEv, 1)

	// Cause a close
	suite.testStore.CloseWatch("/vchub/nwifs/")
	<-doneCh
	time.Sleep(1 * time.Second)

	// Generate send error
	wCtx, wCancel = context.WithCancel(context.Background())
	stream, err = suite.vcHubClient.WatchNwIFs(wCtx, ws)
	if err != nil {
		t.Fatalf("Watch failed -- %v", err)
	}
	doneCh = make(chan bool)
	go func() {
		stream.Recv()
		close(doneCh)
	}()
	suite.w4Channel(t, "/vchub/nwifs/", true)

	wCancel()
	<-doneCh
	time.Sleep(1 * time.Second)

	// Inject a valid obj
	validObj := suite.testIfs[0]
	validEv := &kv.WatchEvent{Type: kv.Created, Object: validObj}
	suite.testStore.InjectWatchEvent("/vchub/nwifs/", validEv, 1)
	suite.w4Channel(t, "/vchub/nwifs/", false)

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(serverAddr, opts...)
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	suite.cc = conn
	suite.vcHubClient = api.NewVCHubApiClient(conn)
	stats, err = suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount == 0 || stats.StoreWatchErrCount == 0 || stats.StoreWatchBadEventCount == 0 || stats.WatchCloseCount == 0 {
		t.Errorf("Expected non-zero stats. Got %+v", stats)
	}

}

func getExpectedNICs(t *testing.T) map[string]*api.SmartNIC {
	nicMap := make(map[string]*api.SmartNIC)
	hosts := esx.GetHostList()
	if hosts == nil || len(hosts) < 1 {
		t.Errorf("Error -- no esx hosts")
		return nicMap
	}
	for _, e := range hosts {
		for _, n := range e.Config.Network.Pnic {
			nicMap[n.Mac] = &api.SmartNIC{
				ObjectKind:       "SmartNIC",
				ObjectAPIVersion: "v1",
				ObjectMeta:       &infraapi.ObjectMeta{},
				Status: &api.SmartNIC_Status{
					MacAddress: n.Mac,
				},
			}
		}
	}

	return nicMap
}

func TestVCP(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	// Start a vc simulator
	vc1, err := sim.Simulate("127.0.0.1:8989", 3, 3)
	if err != nil {
		t.Errorf("Error %v simulating vCenter", err)
		return
	}
	//u, err := soap.ParseURL("administrator@vsphere.local:N0isystem$@vcenter-vm1")
	//vc := "https://user:pass@127.0.0.1:8990/sdk"
	//vc := "administrator@vsphere.local:N0isystem$@192.168.70.194"
	u1, err := soap.ParseURL(vc1)
	if err != nil {
		t.Errorf("Error %v parsing url %s", err, vc1)
		return
	}

	hostCh := make(chan defs.HostMsg, 96)
	snicStore := store.NewSNICStore()
	go snicStore.Run(context.Background(), hostCh)
	v1 := vcp.NewVCProbe(u1, hostCh)
	err = v1.Start()
	if err != nil {
		t.Errorf("Error %v from vcp.Start", err)
		return
	}

	time.Sleep(1 * time.Second)
	go v1.Run()
	time.Sleep(1 * time.Second)
	nicMap := getExpectedNICs(t)
	verifySmartNICList(t, nicMap)

	hosts := esx.GetHostList()
	esx.AddPnicToHost(hosts[0], "vmnic2", "0c:c4:7a:70:68:68")
	esx.AddPnicToHost(hosts[1], "vmnic3", "0c:c4:7a:70:86:86")
	time.Sleep(1200 * time.Millisecond)
	nicMap = getExpectedNICs(t)
	verifySmartNICList(t, nicMap)
	esx.DelPnicFromHost(hosts[1], "vmnic3")
	time.Sleep(1200 * time.Millisecond)
	nicMap = getExpectedNICs(t)
	verifySmartNICList(t, nicMap)

	// Start another vc simulator
	vc2, err := sim.Simulate("127.0.0.1:8990", 4, 3)
	if err != nil {
		t.Errorf("Error %v simulating vCenter", err)
		return
	}
	time.Sleep(1 * time.Second)

	u2, err := soap.ParseURL(vc2)
	if err != nil {
		t.Errorf("Error %v parsing url %s", err, vc2)
		return
	}
	v2 := vcp.NewVCProbe(u2, hostCh)
	err = v2.Start()
	if err != nil {
		t.Errorf("Error %v from vcp.Start", err)
		return
	}

	time.Sleep(1 * time.Second)
	go v2.Run()
	time.Sleep(1 * time.Second)
	nicMap = getExpectedNICs(t)
	verifySmartNICList(t, nicMap)

	v1.Stop()
	v2.Stop()
	close(hostCh)
}
