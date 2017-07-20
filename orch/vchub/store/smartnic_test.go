/*
 *
 * Copyright 2017, Pensando Systems
 * All rights reserved.
 *
 */

package store

import (
	"errors"
	"testing"
	"time"

	"golang.org/x/net/context"

	infraapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/orch/vchub/api"
	"github.com/pensando/sw/orch/vchub/defs"
	"github.com/pensando/sw/utils/kvstore/memkv"
	kvs "github.com/pensando/sw/utils/kvstore/store"
)

const (
	testNic1Mac = "6a:00:02:e7:a8:40"
	testNic2Mac = "6a:00:02:e7:aa:54"
	testIf1Mac  = "6e:00:02:e7:dd:40"
	testIf2Mac  = "6e:00:02:e7:dc:54"
	testIf1ID   = "52fd7958-f4da-78bb-1590-856861348cee:4001"
	testIf2ID   = "53256758-eecc-79bb-1590-899861348cfd:4004"
	waitTO      = 2 * time.Second
	testVCId    = "192.168.0.1:8989"
)

type TestSuite struct {
	testNics  []*api.SmartNIC
	testIfs   []*api.NwIF
	testStore *memkv.MemKv
}

func (ts *TestSuite) setup(t *testing.T) {
	s, err := Init("", kvs.KVStoreTypeMemkv)
	if err != nil {
		t.Fatalf("store.Init returned %v", err)
	}

	m, ok := s.(*memkv.MemKv)
	if !ok {
		t.Fatalf("Bad store type")
	}
	ts.testStore = m

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
}

var suite *TestSuite

func w4Resp(wc <-chan error) error {
	select {

	case e := <-wc:
		return e

	case <-time.After(waitTO):
		return errors.New("Timed out waiting for response")

	}
}

func TestSmartNICAPI(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t)

	// simple create
	err = SmartNICCreate(context.Background(), testNic1Mac, suite.testNics[0])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
		return
	}

	nicList, err := SmartNICList(context.Background())
	if err != nil {
		t.Errorf("SmartNICList failed %v", err)
		return
	}

	nics := nicList.GetItems()
	if len(nics) != 1 {
		t.Errorf("Expected 1 nic. Got %+v", nics)
		return
	}

	// redundant create
	err = SmartNICCreate(context.Background(), testNic1Mac, suite.testNics[0])
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
		return
	}

	// update with err
	suite.testStore.SetErrorState(true)
	suite.testNics[0].ObjectMeta.Name = "Butterfly.vmnic0"
	respCh := make(chan error)
	go func() {
		respCh <- SmartNICCreate(context.Background(), testNic1Mac, suite.testNics[0])
		close(respCh)
	}()

	time.Sleep(200 * time.Millisecond)
	suite.testStore.SetErrorState(false)
	err = w4Resp(respCh)
	if err != nil {
		t.Errorf("SmartNICCreate failed %v", err)
	}

	// read and verify that it was updated
	n := &api.SmartNIC{}
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, n)
	if err != nil {
		t.Errorf("Get failed %v", err)
	}

	if n.ObjectMeta.Name != "Butterfly.vmnic0" {
		t.Errorf("Expected Butterfly.vmnic0, got %v", n.ObjectMeta.Name)
	}

	// update api
	n.Status.Switch = "Foo"
	err = SmartNICUpdate(context.Background(), testNic1Mac, n)
	if err != nil {
		t.Errorf("SmartNICUpdate failed %v", err)
	}

	// read and verify that it was updated
	nn := &api.SmartNIC{}
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, nn)
	if err != nil {
		t.Errorf("Get failed %v", err)
	}

	if nn.Status.Switch != "Foo" {
		t.Errorf("Expected Foo, got %v", n.Status.Switch)
	}

	// delete
	err = SmartNICDelete(context.Background(), testNic1Mac)
	if err != nil {
		t.Errorf("SmartNICDelete failed %v", err)
	}

	// delete again
	err = SmartNICDelete(context.Background(), testNic1Mac)
	if err == nil {
		t.Errorf("SmartNICDelete succeeded while expecting failure")
	}

}

func TestSmartNICStore(t *testing.T) {
	//var err error

	suite = &TestSuite{}
	suite.setup(t)

	s := NewSNICStore()
	hostCh := make(chan defs.HostMsg, 16)
	defer close(hostCh)
	go s.Run(context.Background(), hostCh)

	time.Sleep(100 * time.Millisecond) // let store start

	globalHostKey := testVCId + ":" + "host-1"

	// inject a delete
	m1 := defs.HostMsg{
		Op:         defs.VCOpDelete,
		Key:        globalHostKey,
		Originator: testVCId,
	}

	hostCh <- m1

	// inject a set host
	h1 := &defs.ESXHost{
		DvsMap:  make(map[string]*defs.DvsInstance),
		PenNICs: make(map[string]*defs.NICInfo),
	}
	h1.DvsMap["00:11:zz:dd"] = &defs.DvsInstance{
		Name:    "Pen-dvs1",
		Uplinks: []string{"vmnic0", "vmnic1"},
	}

	h1.PenNICs["vmnic0"] = &defs.NICInfo{Mac: testNic1Mac, DvsUUID: "00:11:zz:dd"}
	h1.PenNICs["vmnic1"] = &defs.NICInfo{Mac: testNic2Mac, DvsUUID: "00:11:zz:dd"}

	m2 := defs.HostMsg{
		Op:         defs.VCOpSet,
		Key:        globalHostKey,
		Value:      h1,
		Originator: testVCId,
	}

	hostCh <- m2
	time.Sleep(100 * time.Millisecond) // let store process the msg

	nicList, err := SmartNICList(context.Background())
	if err != nil {
		t.Errorf("SmartNICList failed %v", err)
		return
	}

	nics := nicList.GetItems()
	if len(nics) != 2 {
		t.Errorf("Expected 2 nics. Got %+v", nics)
		return
	}

	// inject set host with only one nic
	h2 := &defs.ESXHost{
		DvsMap:  make(map[string]*defs.DvsInstance),
		PenNICs: make(map[string]*defs.NICInfo),
	}
	h2.DvsMap["00:11:zz:dd"] = &defs.DvsInstance{
		Name:    "Pen-dvs1",
		Uplinks: []string{"vmnic1"},
	}
	h2.PenNICs["vmnic1"] = &defs.NICInfo{Mac: testNic2Mac, DvsUUID: "00:11:zz:dd"}
	m2.Value = h2
	hostCh <- m2
	time.Sleep(500 * time.Millisecond) // let store process the msg

	nicList, err = SmartNICList(context.Background())
	if err != nil {
		t.Errorf("SmartNICList failed %v", err)
		return
	}

	nics = nicList.GetItems()
	if len(nics) != 1 {
		t.Errorf("Expected 1 nic. Got %+v", nics)
		t.Errorf("m2val: %+v", m2.Value)
		return
	}

	// inject a redundant message
	hostCh <- m2
	time.Sleep(100 * time.Millisecond) // let store process the msg

	// inject set host with the other nic
	delete(h1.PenNICs, "vmnic1")
	h1.DvsMap["00:11:zz:dd"] = &defs.DvsInstance{
		Name:    "Pen-dvs1",
		Uplinks: []string{"vmnic0"},
	}
	m2.Value = h1
	hostCh <- m2
	time.Sleep(100 * time.Millisecond) // let store process the msg

	nn := &api.SmartNIC{}
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, nn)
	if err != nil {
		t.Errorf("Get failed %v", err)
		return
	}
	if nn.Status.MacAddress != testNic1Mac {
		t.Errorf("Expected %s, got %v", testNic1Mac, nn.Status.MacAddress)
	}

	// inject a delete
	hostCh <- m1
	time.Sleep(100 * time.Millisecond) // let store process the msg
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, nn)
	if err == nil {
		t.Errorf("Get succeeded while expecting failure")
	}
}
