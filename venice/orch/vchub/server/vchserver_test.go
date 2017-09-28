/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package server

import (
	"fmt"
	"reflect"
	"runtime/debug"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/vmware/govmomi/vim25/soap"
	"github.com/vmware/govmomi/vim25/types"
	"golang.org/x/net/context"
	"google.golang.org/grpc"

	infraapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/orch/vchub/sim"
	"github.com/pensando/sw/venice/orch/vchub/store"
	vcp "github.com/pensando/sw/venice/orch/vchub/vcprobe"
	kv "github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/kvstore/etcd/integration"
	"github.com/pensando/sw/venice/utils/kvstore/memkv"
	kvs "github.com/pensando/sw/venice/utils/kvstore/store"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/vic/pkg/vsphere/simulator"
	"github.com/pensando/vic/pkg/vsphere/simulator/esx"
)

const (
	vchTestPort  = "19003"
	serverAddr   = "127.0.0.1:" + vchTestPort
	testNic1Mac  = "6a:00:02:e7:a8:40"
	testNic2Mac  = "6a:00:02:e7:aa:54"
	testIf1Mac   = "6e:00:02:e7:dd:40"
	testIf2Mac   = "6e:00:02:e7:dc:54"
	testIf3Mac   = "6e:00:02:e7:ee:64"
	testPG       = "dvportgroup-30"
	testIf1ID    = "52fd7958-f4da-78bb-1590-856861348cee:4001"
	testIf2ID    = "53256758-eecc-79bb-1590-899861348cfd:4004"
	waitTO       = 10 * time.Second
	pollInterval = 10 * time.Millisecond
	vchURL       = ":" + vchTestPort
)

type TestSuite struct {
	vch         *VchServer
	cc          *grpc.ClientConn
	vcHubClient orch.OrchApiClient
	cluster     *integration.ClusterV3
	testNics    []*orch.SmartNIC
	testIfs     []*orch.NwIF
	testStore   *memkv.MemKv
}

func (ts *TestSuite) setup(t *testing.T, fake bool) {
	vch, err := NewVCHServer(vchURL)
	if err != nil {
		t.Errorf("VCHServer start failed %v", err)
	}
	ts.vch = vch
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
	ts.vcHubClient = orch.NewOrchApiClient(conn)
	ts.testNics = make([]*orch.SmartNIC, 2)
	ts.testNics[0] = &orch.SmartNIC{
		ObjectKind:       "SmartNIC",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "ServerA.vmnic0",
			ResourceVersion: "3",
		},
		Status: &orch.SmartNIC_Status{
			HostIP:     "10.193.231.234",
			MacAddress: testNic1Mac,
			Switch:     "Pensando-dvs-1",
		},
	}

	ts.testNics[1] = &orch.SmartNIC{
		ObjectKind:       "SmartNIC",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "ServerB.vmnic0",
			ResourceVersion: "4",
		},
		Status: &orch.SmartNIC_Status{
			HostIP:     "10.193.231.251",
			MacAddress: testNic2Mac,
			Switch:     "Pensando-dvs-1",
		},
	}

	ts.testIfs = make([]*orch.NwIF, 2)
	ts.testIfs[0] = &orch.NwIF{
		ObjectKind:       "NwIF",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "warrior1.eth0",
			ResourceVersion: "4",
			UUID:            "52fd7958-f4da-78bb-1590-856861348cee:4001",
		},
		Config: &orch.NwIF_Config{
			LocalVLAN: 321,
		},
		Status: &orch.NwIF_Status{
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
	ts.testIfs[1] = &orch.NwIF{
		ObjectKind:       "NwIF",
		ObjectAPIVersion: "v1",
		ObjectMeta: &infraapi.ObjectMeta{
			Name:            "cav1.eth0",
			ResourceVersion: "4",
			UUID:            "53256758-eecc-79bb-1590-899861348cfd:4004",
		},
		Config: &orch.NwIF_Config{
			LocalVLAN: 322,
		},
		Status: &orch.NwIF_Status{
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
	ts.vch.StopServer()
	store.Close()
	if ts.cluster != nil {
		time.Sleep(200 * time.Millisecond)
		ts.cluster.Terminate(t)
	}

	sim.TearDown()
	time.Sleep(500 * time.Millisecond)
}

func (ts *TestSuite) w4Channel(t *testing.T, prefix string, active bool) {
	for i := 0; i < 100; i++ {
		if suite.testStore.IsWatchActive(prefix) == active {
			break
		}
		time.Sleep(100 * time.Millisecond)
	}
	if suite.testStore.IsWatchActive(prefix) != active {
		debug.PrintStack()
		t.Errorf("Watch channel not in %v state after 10 sec", active)
	}
}

var suite *TestSuite

func verifySmartNICList(t *testing.T, expected map[string]*orch.SmartNIC) {
	err := checkSmartNICList(expected)
	if err != nil {
		t.Errorf("%v", err)
	}
}
func checkSmartNICList(expected map[string]*orch.SmartNIC) error {
	filter := &orch.Filter{}
	nicList, err := suite.vcHubClient.ListSmartNICs(context.Background(), filter)
	if err != nil {
		return fmt.Errorf("Error listing nics %v", err)
	}
	nics := nicList.GetItems()

	if len(nics) != len(expected) {
		return fmt.Errorf("Expected %d items, got %+v", len(expected), nics)
	}

	for _, n1 := range nics {
		n2 := expected[n1.Status.MacAddress]
		if n2 != nil {
			// ignore resource version for now
			n1.ObjectMeta.ResourceVersion = n2.ObjectMeta.ResourceVersion
			n1.Status.Switch = n2.Status.Switch
			if !reflect.DeepEqual(n1, n2) {
				return fmt.Errorf("Expected %+v, got %+v", n2, n1)
			}
		}
		delete(expected, n1.Status.MacAddress)
	}

	return nil
}

func TestListSmartNICs(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	nicMap := make(map[string]*orch.SmartNIC)
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

func watchSmartNICs(rcvCtx context.Context, t *testing.T, rcvMap map[string]*orch.SmartNIC, eventMap map[orch.WatchEvent_EventType]int, ackCh chan<- int) {

	defer close(ackCh)
	ws := &orch.WatchSpec{}
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
		if event.Event == orch.WatchEvent_Delete {
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

	rcvMap := make(map[string]*orch.SmartNIC)
	sndMap := make(map[string]*orch.SmartNIC)

	// start watch on an empty database
	rcvCtx, rcvCancel := context.WithCancel(context.Background())
	eventMap := make(map[orch.WatchEvent_EventType]int)
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
	if eventMap[orch.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[orch.WatchEvent_Create])
	}
	if eventMap[orch.WatchEvent_Delete] != 1 {
		t.Errorf("Expected 1 delete event, got :%d", eventMap[orch.WatchEvent_Delete])
	}
	if eventMap[orch.WatchEvent_Update] != 1 {
		t.Errorf("Expected 1 update event, got :%d", eventMap[orch.WatchEvent_Update])
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
	rcvCancel()

	// Verify the event count
	if eventMap[orch.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[orch.WatchEvent_Create])
	}
}

func TestSmartNICInspect(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, true)
	defer suite.teardown(t)

	// verify initial values of stats
	empty := &orch.Empty{}
	stats, err := suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount != 0 || stats.StoreWatchErrCount != 0 || stats.StoreWatchBadEventCount != 0 || stats.WatchCloseCount != 0 || stats.GrpcSendErrCount != 0 {
		t.Errorf("Expected zero stats. Got %+v", stats)
	}

	ws := &orch.WatchSpec{}
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
	suite.w4Channel(t, "/vchub/smartnics/", false)
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
	time.Sleep(100 * time.Millisecond)
	suite.w4Channel(t, "/vchub/smartnics/", false)

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

	// Inject a valid obj
	validObj := suite.testNics[0]
	validEv := &kv.WatchEvent{Type: kv.Created, Object: validObj}
	suite.testStore.InjectWatchEvent("/vchub/smartnics/", validEv, 1)
	suite.w4Channel(t, "/vchub/smartnics/", false)

	// setup client
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(serverAddr, opts...)
	if err != nil {
		log.Fatalf("fail to dial: %v", err)
	}
	suite.cc = conn
	suite.vcHubClient = orch.NewOrchApiClient(conn)
	empty = &orch.Empty{}
	stats, err = suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount == 0 || stats.StoreWatchErrCount == 0 || stats.StoreWatchBadEventCount == 0 || stats.WatchCloseCount == 0 {
		t.Errorf("Expected non-zero stats. Got %+v", stats)
	}

}

func verifyNwIFList(t *testing.T, expected map[string]*orch.NwIF) {
	err := checkNwIFList(expected)
	if err != nil {
		t.Errorf("%v", err)
	}
}

func checkNwIFList(expected map[string]*orch.NwIF) error {
	filter := &orch.Filter{}
	ifList, err := suite.vcHubClient.ListNwIFs(context.Background(), filter)
	if err != nil {
		return fmt.Errorf("Error listing ifs %v", err)
	}
	ifs := ifList.GetItems()

	if len(ifs) < len(expected) {
		return fmt.Errorf("Expected %d items, got %d", len(expected), len(ifs))
	}

	for _, n1 := range ifs {
		n2 := expected[n1.ObjectMeta.UUID]
		if n2 == nil {
			continue
		}
		n1.ObjectMeta.ResourceVersion = n2.ObjectMeta.ResourceVersion
		if !reflect.DeepEqual(n1, n2) {
			return fmt.Errorf("Expected %+v, got %+v", n2, n1)
		}
		delete(expected, n1.ObjectMeta.UUID)
	}

	if len(expected) != 0 {
		return fmt.Errorf("%d items not found", len(expected))
	}
	return nil
}

func TestListNwIFs(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	ifMap := make(map[string]*orch.NwIF)

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

func watchNwIFs(rcvCtx context.Context, t *testing.T, rcvMap map[string]*orch.NwIF, eventMap map[orch.WatchEvent_EventType]int, ackCh chan<- int) {

	defer close(ackCh)
	ws := &orch.WatchSpec{}
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
		if event.Event == orch.WatchEvent_Delete {
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

	rcvMap := make(map[string]*orch.NwIF)
	sndMap := make(map[string]*orch.NwIF)

	// start watch on an empty database
	rcvCtx, rcvCancel := context.WithCancel(context.Background())
	eventMap := make(map[orch.WatchEvent_EventType]int)
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
	if eventMap[orch.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[orch.WatchEvent_Create])
	}
	if eventMap[orch.WatchEvent_Delete] != 1 {
		t.Errorf("Expected 1 delete event, got :%d", eventMap[orch.WatchEvent_Delete])
	}
	if eventMap[orch.WatchEvent_Update] != 2 {
		t.Errorf("Expected 1 update event, got :%d", eventMap[orch.WatchEvent_Update])
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
	rcvCancel()

	// Verify the event count
	if eventMap[orch.WatchEvent_Create] != 2 {
		t.Errorf("Expected 2 create events, got :%d", eventMap[orch.WatchEvent_Create])
	}
}

func TestNwIFInspect(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, true)
	defer suite.teardown(t)

	// verify initial values of stats
	empty := &orch.Empty{}
	stats, err := suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount != 0 || stats.StoreWatchErrCount != 0 || stats.StoreWatchBadEventCount != 0 || stats.WatchCloseCount != 0 || stats.GrpcSendErrCount != 0 {
		t.Errorf("Expected zero stats. Got %+v", stats)
	}

	ws := &orch.WatchSpec{}
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
	suite.w4Channel(t, "/vchub/nwifs/", false)
	suite.testStore.SetErrorState(false)

	// Generate errors from store.
	wCtx, wCancel = context.WithCancel(context.Background())

	doneCh := make(chan bool)
	watcherFunc := func(errStr string) {
		for {
			stream, err := suite.vcHubClient.WatchNwIFs(wCtx, ws)
			if err != nil {
				t.Fatalf("Watch failed -- %v", err)
			}
			for {
				_, err = stream.Recv()
				if err != nil && strings.Contains(err.Error(), errStr) {
					close(doneCh)
					return
				} else if err != nil {
					log.Infof("Unexpected ERR %s ", err.Error())
					break
				}
			}
		}
	}
	go watcherFunc("Watch session closed")

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
	time.Sleep(100 * time.Millisecond)
	suite.w4Channel(t, "/vchub/nwifs/", false)

	// Generate send error
	wCtx, wCancel = context.WithCancel(context.Background())
	doneCh = make(chan bool)
	go watcherFunc("canceled")
	suite.w4Channel(t, "/vchub/nwifs/", true)

	wCancel()
	<-doneCh

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
	suite.vcHubClient = orch.NewOrchApiClient(conn)
	stats, err = suite.vcHubClient.Inspect(context.Background(), empty)
	if err != nil {
		t.Errorf("Error %v from inspect", err)
	}

	if stats.StoreWatchFailCount == 0 || stats.StoreWatchErrCount == 0 || stats.StoreWatchBadEventCount == 0 || stats.WatchCloseCount == 0 {
		t.Errorf("Expected non-zero stats. Got %+v", stats)
	}

}

func getExpectedNICs(t *testing.T) map[string]*orch.SmartNIC {
	nicMap := make(map[string]*orch.SmartNIC)
	hosts := esx.GetHostList()
	if hosts == nil || len(hosts) < 1 {
		t.Errorf("Error -- no esx hosts")
		return nicMap
	}
	for _, e := range hosts {
		//spew.Dump(e)
		//spew.Dump(e.Reference())
		for _, n := range e.Config.Network.Pnic {
			nicMap[n.Mac] = &orch.SmartNIC{
				ObjectKind:       "SmartNIC",
				ObjectAPIVersion: "v1",
				ObjectMeta:       &infraapi.ObjectMeta{},
				Status: &orch.SmartNIC_Status{
					MacAddress: n.Mac,
				},
			}
		}
	}

	return nicMap
}

func verifyVCPSnics(t *testing.T, poll, timeOut string) {
	AssertEventually(t, func() (bool, []interface{}) {
		nicMap := getExpectedNICs(t)
		return checkSmartNICList(nicMap) == nil, nil
	}, "verifyVCPSnics", poll, timeOut)
}

func TestVCPSnic(t *testing.T) {
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

	storeCh := make(chan defs.StoreMsg, 96)
	vchStore := store.NewVCHStore(context.Background())
	vchStore.Run(storeCh)
	v1 := vcp.NewVCProbe(u1, storeCh)
	time.Sleep(100 * time.Millisecond) // let simulator start
	err = v1.Start()
	if err != nil {
		t.Errorf("Error %v from vcp.Start", err)
		return
	}

	v1.Run()
	verifyVCPSnics(t, "50ms", "5s")

	hosts := esx.GetHostList()
	simulator.AddPnicToHost(hosts[0], "vmnic2", "0c:c4:7a:70:68:68")
	simulator.AddPnicToHost(hosts[1], "vmnic3", "0c:c4:7a:70:86:86")
	verifyVCPSnics(t, "50ms", "10s")

	simulator.DelPnicFromHost(hosts[1], "vmnic3")
	verifyVCPSnics(t, "50ms", "10s")

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
	v2 := vcp.NewVCProbe(u2, storeCh)
	err = v2.Start()
	if err != nil {
		t.Errorf("Error %v from vcp.Start", err)
		return
	}

	v2.Run()
	verifyVCPSnics(t, "50ms", "5s")

	v1.Stop()
	v2.Stop()
	close(storeCh)
	vchStore.WaitForExit()
	simulator.DelPnicFromHost(hosts[0], "vmnic2")
}

func getPNICMac(href *types.ManagedObjectReference) string {
	if href == nil {
		return ""
	}

	h := simulator.Map.Get(*href)
	if h == nil {
		return ""
	}

	hs := h.(*simulator.HostSystem)

	if hs.Config == nil || hs.Config.Network == nil {
		return ""
	}

	if len(hs.Config.Network.Pnic) > 0 {
		return hs.Config.Network.Pnic[0].Mac
	}
	return ""
}

func getExpectedNwIFs(t *testing.T) map[string]*orch.NwIF {
	ifMap := make(map[string]*orch.NwIF)
	vms := simulator.GetVMList()
	if vms == nil || len(vms) < 1 {
		t.Errorf("Error -- no VMs")
		return ifMap
	}
	for _, v := range vms {
		vmRef := v.Reference()
		vmKey := "127.0.0.1:8989:" + vmRef.Value
		pnicMac := getPNICMac(v.Runtime.Host)
		if pnicMac == "" {
			continue
		}
		for _, d := range v.Config.Hardware.Device {
			veth := vcp.GetVeth(d)
			if veth == nil {
				continue
			}

			b := veth.Backing.(*types.VirtualEthernetCardDistributedVirtualPortBackingInfo)
			ifKey := vmKey + "::" + veth.MacAddress
			ifMap[ifKey] = &orch.NwIF{
				ObjectKind:       "NwIF",
				ObjectAPIVersion: "v1",
				ObjectMeta: &infraapi.ObjectMeta{
					UUID:   ifKey,
					Tenant: "default",
					Name:   v.Name,
				},
				Config: &orch.NwIF_Config{},
				Status: &orch.NwIF_Status{
					MacAddress:  veth.MacAddress,
					PortGroup:   b.Port.PortgroupKey,
					Switch:      b.Port.SwitchUuid,
					SmartNIC_ID: pnicMac,
					WlName:      v.Name,
					WlUUID:      vmKey,
					Network:     b.Port.PortgroupKey,
					IpAddress:   store.GetIPFromMac(veth.MacAddress),
				},
			}
		}
	}

	return ifMap
}

func wait4NwIFEvent(doneCh chan bool, ev orch.WatchEvent_EventType, mac, pg string) {
	defer close(doneCh)
	ws := &orch.WatchSpec{}
	stream, err := suite.vcHubClient.WatchNwIFs(context.Background(), ws)
	if err != nil {
		log.Fatalf("Error %v", err)
	}

	for {
		e, err := stream.Recv()

		if err != nil {
			return
		}

		event := e.GetE()
		ifs := e.GetNwifs()
		if event.Event == ev {
			for _, nwif := range ifs {
				if nwif.Status.MacAddress == mac && nwif.Status.PortGroup == pg {
					log.Info("||||Got create for %+v", nwif)
					doneCh <- true
				}
			}
		}
	}
}

func TestVCPNwIF(t *testing.T) {
	suite = &TestSuite{}
	suite.setup(t, false)
	defer suite.teardown(t)

	// Start a vc simulator
	vc1, err := sim.Simulate("127.0.0.1:8989", 3, 3)
	if err != nil {
		t.Errorf("Error %v simulating vCenter", err)
		return
	}
	u1, err := soap.ParseURL(vc1)
	if err != nil {
		t.Errorf("Error %v parsing url %s", err, vc1)
		return
	}

	storeCh := make(chan defs.StoreMsg, 96)
	vchStore := store.NewVCHStore(context.Background())
	vchStore.Run(storeCh)
	v1 := vcp.NewVCProbe(u1, storeCh)
	time.Sleep(100 * time.Millisecond)
	err = v1.Start()
	if err != nil {
		t.Errorf("Error %v from vcp.Start", err)
		return
	}

	v1.Run()
	AssertEventually(t, func() (bool, []interface{}) {
		ifMap := getExpectedNwIFs(t)
		return checkNwIFList(ifMap) == nil, nil
	}, "Default NwIFs", "50ms", "5s")

	// Add a new NwIF and verify
	doneCh := make(chan bool)
	go wait4NwIFEvent(doneCh, orch.WatchEvent_Create, testIf3Mac, testPG)
	veth, _ := sim.AddNwIF(testIf3Mac, testPG)
	select {

	case <-doneCh:

	case <-time.After(waitTO):
		t.Errorf("Timed out waiting for watch event")

	}

	// Delete the new NwIF and verify
	doneCh = make(chan bool)
	go wait4NwIFEvent(doneCh, orch.WatchEvent_Delete, testIf3Mac, testPG)
	err = sim.DeleteNwIF(veth)
	if err != nil {
		t.Errorf("Failed to delete veth %s", veth)
	}
	select {

	case <-doneCh:

	case <-time.After(waitTO):
		t.Errorf("Timed out waiting for delete watch event")

	}

	v1.Stop()
	close(storeCh)
	vchStore.WaitForExit()
}
