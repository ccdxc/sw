package gosdk

import (
	"log"
	"testing"
	"time"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type service struct {
	mountDone       chan struct{}
	gotCreateNotify chan struct{}
	gotUpdateNotify chan struct{}
	gotDeleteNotify chan struct{}
	name            string
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s", s.name)
	s.mountDone <- struct{}{}
}

func (s *service) TxnBegin()  {}
func (s *service) TxnCommit() {}
func (s *service) TxnAbort()  {}
func (s *service) Name() string {
	return s.name
}

func (s *service) OnTestInterfaceSpecCreate(obj *TestInterfaceSpec) {
	log.Printf("%s Created! %s\n", s.Name(), obj.GetMacAddress())
	s.gotCreateNotify <- struct{}{}
}

func (s *service) OnTestInterfaceSpecDelete(obj *TestInterfaceSpec) {
	log.Printf("%s Updated! %s\n", s.Name(), obj.GetMacAddress())
	s.gotDeleteNotify <- struct{}{}
}

func (s *service) OnTestInterfaceSpecUpdate(old, obj *TestInterfaceSpec) {
	log.Printf("%s Deleted!\n", s.Name())
	s.gotUpdateNotify <- struct{}{}
}

func TestClientBasic(t *testing.T) {
	h := NewFakeHub()
	h.Start()

	s1 := &service{
		mountDone:       make(chan struct{}),
		gotCreateNotify: make(chan struct{}),
		gotUpdateNotify: make(chan struct{}),
		gotDeleteNotify: make(chan struct{}),
		name:            "test1",
	}

	c1, err := NewClient(s1)
	if err != nil {
		t.Errorf("NewClient(): %s", err)
	}
	c1.MountKind("TestInterfaceSpec", delphi.MountMode_ReadMode)

	go c1.Run()

	AssertEventually(t, func() (bool, interface{}) {
		return c1.IsConnected(), c1
	}, "client did not connect")

	c1.WatchMount(s1)

	_ = <-s1.mountDone
	log.Printf("### Client 1 Mount done #1")

	_ = <-s1.mountDone
	log.Printf("### Client 1 Mount done #2")

	s2 := &service{
		mountDone:       make(chan struct{}),
		gotCreateNotify: make(chan struct{}),
		gotUpdateNotify: make(chan struct{}),
		gotDeleteNotify: make(chan struct{}),
		name:            "test2",
	}
	c2, err := NewClient(s2)
	c2.MountKind("TestInterfaceSpec", delphi.MountMode_ReadMode)
	c2.MountKindKey("Interface", "Intf:20", delphi.MountMode_ReadMode)

	go c2.Run()

	TestInterfaceSpecWatch(c1, s1)
	TestInterfaceSpecWatch(c2, s2)

	_ = <-s2.mountDone
	log.Printf("### Client 2 Mount done")
	AssertEventually(t, func() (bool, interface{}) {
		return c2.IsConnected(), c2
	}, "client did not connect")

	spec := &TestInterfaceSpec{
		Key: &IntfIndex{
			Ifidx: 1,
		},
		MacAddress: "TestMacAddress",
	}
	c1.SetObject(spec)
	log.Printf("### SPEC: %+v", spec)
	log.Printf("### Client 1 Set Object done")

	for i := 0; i < 2; i++ {
		select {
		case _ = <-s1.gotUpdateNotify:
			log.Printf("### Client 1 Got Update Notify")
		case _ = <-s2.gotCreateNotify:
			log.Printf("### Client 2 Got Create Notify")
		}
	}

	log.Printf("Client1 Data:\n")
	c1.DumpSubtrees()
	log.Printf("Client2 Data:\n")
	c2.DumpSubtrees()

	spec2 := GetTestInterfaceSpec(c2, &IntfIndex{Ifidx: 1})
	if spec2.GetMacAddress() != spec.GetMacAddress() {
		t.Errorf(`spec2.GetMacAddress() != spec.GetMacAddress()`)
	}

	objs := c1.List("TestInterfaceSpec")
	if len(objs) != 1 {
		t.Errorf(`len(objs) != 1`)
	}
	if objs[0].GetDelphiMessage().String() != spec.GetDelphiMessage().String() {
		t.Errorf(`objs[0] != spec`)
	}

	c2.DeleteObject(spec)

	_ = <-s1.gotDeleteNotify
	log.Printf("### Client 1 Got Delete Notify")
	_ = <-s2.gotDeleteNotify
	log.Printf("### Client 2 Got Delete Notify")

	c1.Close()
	c2.Close()

	log.Printf("Client1 Data:\n")
	c1.DumpSubtrees()
	log.Printf("Client2 Data:\n")
	c2.DumpSubtrees()

	h.Stop()
	time.Sleep(time.Millisecond * 10) // wait a little for hub to fully stop
}

// this tests if hub starts after client, client will retry for it..
func TestClientRetry(t *testing.T) {
	s1 := &service{
		mountDone:       make(chan struct{}),
		gotCreateNotify: make(chan struct{}),
		gotUpdateNotify: make(chan struct{}),
		gotDeleteNotify: make(chan struct{}),
		name:            "test1",
	}
	c1, err := NewClient(s1)
	AssertOk(t, err, "Error creating delphi client")

	// try to connect in the background
	go c1.Run()

	time.Sleep(time.Millisecond * 10)

	// start the delphi hub
	h := NewFakeHub()
	h.Start()
	defer h.Stop()

	AssertEventually(t, func() (bool, interface{}) {
		return c1.IsConnected(), c1
	}, "client did not connect", "1s", "30s")
	c1.Close()
	time.Sleep(time.Millisecond * 10)
}
