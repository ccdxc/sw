package vcprobe

import (
	"fmt"
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestMessages(t *testing.T) {
	vcID := "user:pass@127.0.0.1:8990/sdk"
	expectedMsgs := map[defs.VCObject][]defs.Probe2StoreMsg{
		// The changes property is not checked currently
		defs.VirtualMachine: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "vm-19",
				Originator: vcID,
			},
			defs.Probe2StoreMsg{
				VcObject:   defs.VirtualMachine,
				Key:        "vm-21",
				Originator: vcID,
			},
		},
		defs.HostSystem: []defs.Probe2StoreMsg{
			defs.Probe2StoreMsg{
				VcObject:   defs.HostSystem,
				Key:        "host-14",
				Originator: vcID,
			},
		},
	}
	config := log.GetDefaultConfig("vcprobe_test")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "Failed to create DC")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "failed to create host")
	_, err = dc.AddVM("vm1", "host1")
	AssertOk(t, err, "failed to create vm")
	_, err = dc.AddVM("vm2", "host1")
	AssertOk(t, err, "failed to create vm")

	u := s.GetURL()

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)

	vcp := NewVCProbe(vcID, u, storeCh, probeCh, nil, logger)
	vcp.Start()
	defer vcp.Stop()

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

func TestReconnect(t *testing.T) {
	vcID := "user:pass@127.0.0.1:8990"
	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "failed dc create")
	dc.AddHost("host1")
	dc.AddVM("vm1", "host1")

	fmt.Printf("starting on %s\n", s.GetURL())

	u := s.GetURL()
	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)

	config := log.GetDefaultConfig("vcprobe_test")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	vcp := NewVCProbe(vcID, u, storeCh, probeCh, nil, logger)
	err = vcp.Start()
	AssertOk(t, err, "Failed to start probe")
	defer vcp.Stop()

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
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.FailNow()
	}

	// Reset values and break connection
	eventMap = make(map[defs.VCObject][]defs.Probe2StoreMsg)

	go func() {
		for {
			select {
			case <-doneCh:
				return
			case m := <-storeCh:
				eventMap[m.VcObject] = append(eventMap[m.VcObject], m)

				if len(eventMap[defs.HostSystem]) >= 1 &&
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()

	s.Server.CloseClientConnections()

	select {
	case <-doneCh:
	case <-time.After(10 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.FailNow()
	}
}

func TestLoginRetry(t *testing.T) {
	t.Skip("Watch currently does not process events for objects that have been created after the watch has started")

	// If the probe is started before the vcenter instance is up, it should
	// keep retrying to create the client

	vcID := "user:pass@127.0.0.1:8990"

	u, err := soap.ParseURL(vcID)
	AssertOk(t, err, "Failed to parse url")
	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)
	config := log.GetDefaultConfig("vcprobe_test")
	config.LogToStdout = true
	logger := log.SetConfig(config)

	vcp := NewVCProbe(vcID, u, storeCh, probeCh, nil, logger)
	err = vcp.Start()
	AssertOk(t, err, "Failed to start probe")
	defer vcp.Stop()

	time.Sleep(time.Second)

	fmt.Printf("starting sim %s\n", u)

	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()

	dc, err := s.AddDC("test-dc-1")
	AssertOk(t, err, "failed dc create")
	dc.AddHost("host1")
	dc.AddVM("vm1", "host1")

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
					len(eventMap[defs.VirtualMachine]) >= 1 {
					doneCh <- true
				}
			}
		}
	}()
	select {
	case <-doneCh:
	case <-time.After(3 * time.Second):
		doneCh <- false
		t.Logf("Failed to receive all messages.")
		t.FailNow()
	}
}
