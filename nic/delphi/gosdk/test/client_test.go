package clientTest

import (
	"log"
	"testing"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/hub"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

type service struct {
	mountDone chan struct{}
	gotNotify chan *InterfaceSpec
	name      string
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
	s.gotNotify <- obj
}

func (s *service) OnInterfaceSpecDelete(obj *InterfaceSpec) {
	log.Printf("%s Updated! %s\n", s.Name(), obj.GetMacAddress())
	s.gotNotify <- obj
}

func (s *service) OnInterfaceSpecUpdate(obj *InterfaceSpec) {
	log.Printf("%s Deleted!\n", s.Name())
	s.gotNotify <- obj
}

func TestA(t *testing.T) {
	h := hub.NewHub()

	log.Printf("Starting hub")
	h.Start()

	s1 := &service{
		mountDone: make(chan struct{}),
		gotNotify: make(chan *InterfaceSpec),
		name:      "test1",
	}
	c1, err := gosdk.NewClient(s1)
	if err != nil {
		t.Errorf("NewClient(): %s", err)
	}
	err = c1.Dial()
	if err != nil {
		t.Errorf("Dial(): %s", err)
	}

	_ = <-s1.mountDone
	log.Printf("### Client 1 Mount done")

	s2 := &service{
		mountDone: make(chan struct{}),
		gotNotify: make(chan *InterfaceSpec),
		name:      "test2",
	}
	c2, err := gosdk.NewClient(s2)
	InterfaceSpecMount(c2, delphi.MountMode_ReadMode)
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
	msg := spec.GetMessage()
	log.Printf("### SPEC: %+v", spec)
	log.Printf("### MESSAGE: %s", msg.String())

	log.Printf("### Client 1 Set Object done")

	for i := 0; i < 2; i++ {
		select {
		case _ = <-s1.gotNotify:
			log.Printf("### Client 1 Got Notify")
		case obj := <-s2.gotNotify:
			log.Printf("### Client 2 Got Notify")
			if obj.GetKey().GetIfidx() != 1 {
				t.Fail()
			}
		}
	}

	log.Printf("Client1 Data:\n")
	c1.DumpSubtrees()
	log.Printf("Client2 Data:\n")
	c2.DumpSubtrees()

	spec.Delete()

	_ = <-s1.gotNotify
	log.Printf("### Client 1 Got Notify")
	_ = <-s2.gotNotify
	log.Printf("### Client 2 Got Notify")

	c1.Close()
	c2.Close()

	log.Printf("Client1 Data:\n")
	c1.DumpSubtrees()
	log.Printf("Client2 Data:\n")
	c2.DumpSubtrees()

	log.Printf("Stopping hub")
	h.Stop()
}

func TestB(t *testing.T) {
	msg := &InterfaceSpec_{
		Meta: nil,
		Key: &IntfIndex_{
			Ifidx: 1,
		},
		MacAddress: "test",
	}

	if msg.GetKey().GetIfidx() != 1 {
		t.Fail()
	}

	if msg.GetMeta() != nil {
		t.Fail()
	}

	if msg.GetMacAddress() != "test" {
		t.Fail()
	}
}
