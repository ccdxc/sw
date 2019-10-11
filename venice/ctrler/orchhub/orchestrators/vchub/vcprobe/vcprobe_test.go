package vcprobe

import (
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testNic1Mac = "6a00.02e7.a840"
)

func TestSOAP(t *testing.T) {
	vcID := "127.0.0.1:8990"
	expectedMsgs := map[defs.VCObject][]defs.Probe2StoreMsg{
		// The changes property is not checked currently
		defs.VirtualMachine: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "virtualmachine-41",
				Originator: vcID,
			},
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "virtualmachine-45",
				Originator: vcID,
			},
		},
		defs.HostSystem: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.HostSystem,
				Key:        "hostsystem-21",
				Originator: vcID,
			},
		},
	}
	hosts := []string{testNic1Mac} // A default cluster host will be created in addition to this list
	vms := 2                       // VMs to be created per resource pool

	logger := log.SetConfig(log.GetDefaultConfig("vcprobe_test"))
	s := sim.New()
	defer s.Destroy()

	vc1, err := s.Run("127.0.0.1:8990", hosts, vms)
	if err != nil {
		t.Fatalf("Error %v simulating vCenter", err)
	}

	u, err := soap.ParseURL(vc1)
	if err != nil {
		t.Fatal(err)
	}

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)
	time.Sleep(100 * time.Millisecond) // let simulator start

	vcp := NewVCProbe(vcID, u, storeCh, probeCh, nil, logger)
	vcp.Start()
	defer vcp.Stop()
	vcp.Run()

	eventMap := make(map[defs.VCObject][]defs.Probe2StoreMsg)
	doneCh := make(chan bool)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 2 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		sim.PrintInventory()
		doneCh <- false
		t.Logf("Failed to receive all messages. ")
		t.Logf("Expected: ")
		for k, v := range expectedMsgs {
			t.Logf("%d %s but got %d %s", len(v), k, len(eventMap[k]), k)
		}
		t.FailNow()
	}
	for objType, events := range expectedMsgs {
		recvEvents := eventMap[objType]
		for _, expE := range events {
			foundMatch := false
			for _, recvE := range recvEvents {
				if expE.Key == recvE.Key &&
					expE.Originator == recvE.Originator {
					foundMatch = true
					break
				}
			}
			Assert(t, foundMatch, "could not find matching event for %v", expE)
		}
	}
}
