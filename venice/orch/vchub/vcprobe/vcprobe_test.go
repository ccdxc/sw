package vcprobe

import (
	"testing"
	"time"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/orch/vchub/sim"
)

const (
	testNic1Mac = "6a:00:02:e7:a8:40"
)

func TestSOAP(t *testing.T) {
	s := sim.New()
	defer s.Destroy()

	vc1, err := s.Run("127.0.0.1:8990", []string{testNic1Mac}, 2)
	if err != nil {
		t.Fatalf("Error %v simulating vCenter", err)
	}

	u, err := soap.ParseURL(vc1)
	if err != nil {
		t.Fatal(err)
	}

	storeCh := make(chan defs.StoreMsg, 24)
	time.Sleep(100 * time.Millisecond) // let simulator start
	vcp := NewVCProbe(u, storeCh)
	vcp.Start()
	vcp.Run()

	counts := make(map[defs.VCProp]int)
	stopped := false
	for m := range storeCh {
		c := counts[m.Property]
		counts[m.Property] = c + 1

		if counts[defs.HostPropConfig] >= 1 &&
			counts[defs.VMPropConfig] >= 2 {
			if !stopped {
				stopped = true
				vcp.Stop()
				close(storeCh)
			}
		}

	}
}
