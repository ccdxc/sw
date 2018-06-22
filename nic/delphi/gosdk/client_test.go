package gosdk

import (
	"log"
	"testing"

	"github.com/pensando/sw/nic/delphi/proto/delphi"
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

func (s *service) OnInterfaceSpecCreate(obj *InterfaceSpec) {
	log.Printf("%s Created! %s\n", s.Name(), obj.GetMacAddress())
	s.gotCreateNotify <- struct{}{}
}

func (s *service) OnInterfaceSpecDelete(obj *InterfaceSpec) {
	log.Printf("%s Updated! %s\n", s.Name(), obj.GetMacAddress())
	s.gotDeleteNotify <- struct{}{}
}

func (s *service) OnInterfaceSpecUpdate(obj *InterfaceSpec) {
	log.Printf("%s Deleted!\n", s.Name())
	s.gotUpdateNotify <- struct{}{}
}

func TestClientBasic(t *testing.T) {
	h := NewHub()
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
	err = c1.Dial()
	if err != nil {
		t.Errorf("Dial(): %s", err)
	}

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
	c2.MountKind("Interface", delphi.MountMode_ReadMode)
	err = c2.Dial()
	if err != nil {
		t.Errorf("Dial(): %s", err)
	}

	InterfaceSpecWatch(c1, s1)
	InterfaceSpecWatch(c2, s2)

	_ = <-s2.mountDone
	log.Printf("### Client 2 Mount done")

	spec := NewInterfaceSpec(c1)
	spec.GetKey().SetIfidx(1)
	spec.SetMacAddress("TestMacAddress")
	log.Printf("### SPEC: %+v", spec)

	//c1.SetObject(spec)
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

	key := NewIntfIndex(c2)
	key.SetIfidx(1)
	spec2 := GetInterfaceSpec(c2, key)
	if spec2.GetMacAddress() != spec.GetMacAddress() {
		t.Errorf(`spec2.GetMacAddress() != spec.GetMacAddress()`)
	}

	spec.Delete()

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
}
