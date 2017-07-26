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
	"github.com/pensando/sw/orch"
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
	dvs1Id      = "00:11:zz:dd"
	dvs2Id      = "00:11:xx:ee"
)

type TestSuite struct {
	testNics  []*orch.SmartNIC
	testIfs   []*orch.NwIF
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
	n := &orch.SmartNIC{}
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
	nn := &orch.SmartNIC{}
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

	suite = &TestSuite{}
	suite.setup(t)

	s := NewVCHStore(context.Background())
	storeCh := make(chan defs.StoreMsg, 16)
	defer close(storeCh)
	go s.Run(storeCh)

	time.Sleep(100 * time.Millisecond) // let store start

	globalHostKey := testVCId + ":" + "host-1"

	// inject a delete
	m1 := defs.StoreMsg{
		Op:         defs.VCOpDelete,
		Property:   defs.HostPropConfig,
		Key:        globalHostKey,
		Originator: testVCId,
	}

	storeCh <- m1

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

	m2 := defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.HostPropConfig,
		Key:        globalHostKey,
		Value:      h1,
		Originator: testVCId,
	}

	storeCh <- m2
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
	storeCh <- m2
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
	storeCh <- m2
	time.Sleep(100 * time.Millisecond) // let store process the msg

	// inject set host with the other nic
	delete(h1.PenNICs, "vmnic1")
	h1.DvsMap["00:11:zz:dd"] = &defs.DvsInstance{
		Name:    "Pen-dvs1",
		Uplinks: []string{"vmnic0"},
	}
	m2.Value = h1
	storeCh <- m2
	time.Sleep(100 * time.Millisecond) // let store process the msg

	nn := &orch.SmartNIC{}
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, nn)
	if err != nil {
		t.Errorf("Get failed %v", err)
		return
	}
	if nn.Status.MacAddress != testNic1Mac {
		t.Errorf("Expected %s, got %v", testNic1Mac, nn.Status.MacAddress)
	}

	// inject a delete
	storeCh <- m1
	time.Sleep(100 * time.Millisecond) // let store process the msg
	err = kvStore.Get(context.Background(), smartNICPath+testNic1Mac, nn)
	if err == nil {
		t.Errorf("Get succeeded while expecting failure")
	}
}

func TestNwIFAPI(t *testing.T) {
	var err error

	suite = &TestSuite{}
	suite.setup(t)

	// simple create
	err = NwIFCreate(context.Background(), testIf1Mac, suite.testIfs[0])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
		return
	}

	ifList, err := NwIFList(context.Background())
	if err != nil {
		t.Errorf("NwIFList failed %v", err)
		return
	}

	ifs := ifList.GetItems()
	if len(ifs) != 1 {
		t.Errorf("Expected 1 if. Got %+v", ifs)
		return
	}

	// redundant create
	err = NwIFCreate(context.Background(), testIf1Mac, suite.testIfs[0])
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
		return
	}

	// update with err
	suite.testStore.SetErrorState(true)
	suite.testIfs[0].ObjectMeta.Name = "Butterfly.eth0"
	respCh := make(chan error)
	go func() {
		respCh <- NwIFCreate(context.Background(), testIf1Mac, suite.testIfs[0])
		close(respCh)
	}()

	time.Sleep(200 * time.Millisecond)
	suite.testStore.SetErrorState(false)
	err = w4Resp(respCh)
	if err != nil {
		t.Errorf("NwIFCreate failed %v", err)
	}

	// read and verify that it was updated
	n := &orch.NwIF{}
	err = kvStore.Get(context.Background(), nwifPath+testIf1Mac, n)
	if err != nil {
		t.Errorf("Get failed %v", err)
	}

	if n.ObjectMeta.Name != "Butterfly.eth0" {
		t.Errorf("Expected Butterfly.eth0, got %v", n.ObjectMeta.Name)
	}

	// update api
	n.Status.PortGroup = "Foooo"
	err = NwIFUpdate(context.Background(), testIf1Mac, n)
	if err != nil {
		t.Errorf("NwIFUpdate failed %v", err)
	}

	// read and verify that it was updated
	nn := &orch.NwIF{}
	err = kvStore.Get(context.Background(), nwifPath+testIf1Mac, nn)
	if err != nil {
		t.Errorf("Get failed %v", err)
	}

	if nn.Status.PortGroup != "Foooo" {
		t.Errorf("Expected Foooo, got %v", n.Status.PortGroup)
	}

	// delete
	err = NwIFDelete(context.Background(), testIf1Mac)
	if err != nil {
		t.Errorf("NwIFDelete failed %v", err)
	}

	// delete again
	err = NwIFDelete(context.Background(), testIf1Mac)
	if err == nil {
		t.Errorf("NwIFDelete succeeded while expecting failure")
	}

}

func TestNwIFStore(t *testing.T) {

	suite = &TestSuite{}
	suite.setup(t)

	s := NewVCHStore(context.Background())
	storeCh := make(chan defs.StoreMsg, 16)
	defer close(storeCh)
	go s.Run(storeCh)

	time.Sleep(100 * time.Millisecond) // let store start

	globalHostKey := testVCId + ":" + "host-101"
	globalVMKey := testVCId + ":" + "vm-1"

	// inject a delete
	m1 := defs.StoreMsg{
		Op:         defs.VCOpDelete,
		Property:   defs.VMPropConfig,
		Key:        globalVMKey,
		Originator: testVCId,
	}

	storeCh <- m1

	// inject a set host
	h1 := &defs.ESXHost{
		DvsMap:  make(map[string]*defs.DvsInstance),
		PenNICs: make(map[string]*defs.NICInfo),
	}
	h1.DvsMap[dvs1Id] = &defs.DvsInstance{
		Name:    "Pen-dvs1",
		Uplinks: []string{"vmnic0"},
	}

	h1.DvsMap[dvs2Id] = &defs.DvsInstance{
		Name:    "Pen-dvs2",
		Uplinks: []string{"vmnic1"},
	}

	h1.PenNICs["vmnic0"] = &defs.NICInfo{Mac: testNic1Mac, DvsUUID: dvs1Id}
	h1.PenNICs["vmnic1"] = &defs.NICInfo{Mac: testNic2Mac, DvsUUID: dvs2Id}

	m2 := defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.HostPropConfig,
		Key:        globalHostKey,
		Value:      h1,
		Originator: testVCId,
	}

	storeCh <- m2
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

	// inject set VM config
	vnic1 := &defs.VirtualNIC{
		Name:       "eth0",
		MacAddress: testIf1Mac,
		PortKey:    "32",
		SwitchUUID: dvs1Id,
	}

	vm1 := &defs.VMConfig{Vnics: make(map[string]*defs.VirtualNIC)}
	vm1.Vnics[testIf1Mac] = vnic1
	m2 = defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.VMPropConfig,
		Key:        globalVMKey,
		Value:      vm1,
		Originator: testVCId,
	}
	storeCh <- m2
	time.Sleep(200 * time.Millisecond) // let store process the msg

	ifList, err := NwIFList(context.Background())
	if err != nil {
		t.Errorf("NwIFList failed %v", err)
		return
	}

	ifs := ifList.GetItems()
	if len(ifs) != 1 {
		t.Errorf("Expected 1 nwif. Got %+v", ifs)
		t.Errorf("m2val: %+v", m2.Value)
		return
	}

	// inject a vm config set with two vnics
	vnic2 := &defs.VirtualNIC{
		Name:       "eth1",
		MacAddress: testIf2Mac,
		PortKey:    "32",
		SwitchUUID: dvs2Id,
	}
	vm1 = &defs.VMConfig{Vnics: make(map[string]*defs.VirtualNIC)}
	vm1.Vnics[testIf1Mac] = vnic1
	vm1.Vnics[testIf2Mac] = vnic2

	m2 = defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.VMPropConfig,
		Key:        globalVMKey,
		Value:      vm1,
		Originator: testVCId,
	}
	storeCh <- m2

	time.Sleep(200 * time.Millisecond) // let store process the msg
	ifList, err = NwIFList(context.Background())
	if err != nil {
		t.Errorf("NwIFList failed %v", err)
		return
	}

	ifs = ifList.GetItems()
	if len(ifs) != 2 {
		t.Errorf("Expected 2 nwif. Got %+v", ifs)
		t.Errorf("m2val: %+v", m2.Value)
		return
	}

	// Inject set runtime
	rt1 := &defs.VMRuntime{
		HostKey: testVCId + ":" + "host-101",
	}

	m3 := defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.VMPropRT,
		Key:        globalVMKey,
		Value:      rt1,
		Originator: testVCId,
	}
	storeCh <- m3
	time.Sleep(200 * time.Millisecond) // let store process the msg

	// verify smartnic binding
	if1 := &orch.NwIF{}
	err = kvStore.Get(context.Background(), nwifPath+testIf1Mac, if1)
	if err != nil {
		t.Errorf("Get failed %v", err)
		return
	}

	if if1.Status.SmartNIC_ID != testNic1Mac {
		t.Errorf("Expected %s, got %s", testNic1Mac, if1.Status.SmartNIC_ID)
	}
	err = kvStore.Get(context.Background(), nwifPath+testIf2Mac, if1)
	if err != nil {
		t.Errorf("Get failed %v", err)
		return
	}
	if if1.Status.SmartNIC_ID != testNic2Mac {
		t.Errorf("Expected %s, got %s", testNic2Mac, if1.Status.SmartNIC_ID)
	}

	// inject redundant messages
	storeCh <- m2
	storeCh <- m3

	// inject a change runtime
	// Inject set runtime
	rt2 := &defs.VMRuntime{
		HostKey: testVCId + ":" + "host-102",
	}

	m4 := defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.VMPropRT,
		Key:        globalVMKey,
		Value:      rt2,
		Originator: testVCId,
	}
	storeCh <- m4
	time.Sleep(200 * time.Millisecond) // let store process the msg

	// verify Status.SmartNIC_ID changed to blank
	ifList, err = NwIFList(context.Background())
	if err != nil {
		t.Errorf("NwIFList failed %v", err)
		return
	}

	ifs = ifList.GetItems()
	if len(ifs) != 2 {
		t.Errorf("Expected 2 nwif. Got %+v", ifs)
		return
	}

	for _, nwif := range ifs {
		if nwif.Status.SmartNIC_ID != "" {
			t.Errorf("Expected blank Status.SmartNIC_ID. Got %+v", nwif.Status)
		}
	}

	// inject with only 1 vnic
	vm1 = &defs.VMConfig{Vnics: make(map[string]*defs.VirtualNIC)}
	vm1.Vnics[testIf2Mac] = vnic2
	m2 = defs.StoreMsg{
		Op:         defs.VCOpSet,
		Property:   defs.VMPropConfig,
		Key:        globalVMKey,
		Value:      vm1,
		Originator: testVCId,
	}
	storeCh <- m2
	time.Sleep(200 * time.Millisecond) // let store process the msg
	err = kvStore.Get(context.Background(), nwifPath+testIf1Mac, if1)
	if err == nil {
		t.Errorf("Read of %s succeeded while expecting failure", testIf1Mac)
		return
	}

	// inject a delete
	m2 = defs.StoreMsg{
		Op:         defs.VCOpDelete,
		Property:   defs.VMPropConfig,
		Key:        globalVMKey,
		Originator: testVCId,
	}
	storeCh <- m2
	time.Sleep(200 * time.Millisecond) // let store process the msg
	err = kvStore.Get(context.Background(), nwifPath+testIf2Mac, if1)
	if err == nil {
		t.Errorf("Read of %s succeeded while expecting failure", testIf2Mac)
		return
	}
}
