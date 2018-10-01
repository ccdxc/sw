package services

import (
	"context"
	"fmt"
	"testing"

	rolloutproto "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
)

type rolloutTestHelper struct {
	t                 *testing.T
	numServiceCall    int
	numServiceSuccess int
	numServiceFail    int
	numVeniceCall     int
	numVeniceSuccess  int
	numVeniceFail     int
}

func (h *rolloutTestHelper) WriteServiceStatus(ctx context.Context, s *rolloutproto.ServiceRolloutStatusUpdate) {
	h.numServiceCall++
	for _, status := range s.Status.OpStatus {
		if status.OpStatus != "success" {
			h.numServiceFail++
		} else {
			h.numServiceSuccess++
		}
	}
}

func (h *rolloutTestHelper) WriteStatus(ctx context.Context, s *rolloutproto.VeniceRolloutStatusUpdate) {
	h.numVeniceCall++
	for _, status := range s.Status.OpStatus {
		if status.OpStatus != "success" {
			h.numVeniceFail++
		} else {
			h.numVeniceSuccess++
		}
	}
}

func TestRolloutMgr(t *testing.T) {
	r := newRolloutMgr()

	numupgradeServicesCalled, numgetUpgradeOrderCalled, numVenicePrecheckCalled, numVeniceRunVersionCalled := 0, 0, 0, 0
	r.upgradeServices = func(srvList []string) error { numupgradeServicesCalled++; return nil }
	r.getUpgradeOrder = func() []string { numgetUpgradeOrderCalled++; return []string{} }
	r.venicePreCheck = func(version string) error { numVenicePrecheckCalled++; return nil }
	r.veniceRunVersion = func(version string) error { numVeniceRunVersionCalled++; return nil }

	r.Start()
	defer r.Stop()

	h := rolloutTestHelper{t: t}
	r.RegisterServiceStatusWriter(&h)
	r.RegisterVeniceStatusWriter(&h)

	sro := rolloutproto.ServiceRollout{
		Spec: rolloutproto.ServiceRolloutSpec{
			Ops: []*rolloutproto.ServiceOpSpec{
				&rolloutproto.ServiceOpSpec{
					Op:      rolloutproto.ServiceOp_ServiceRunVersion,
					Version: "dummy",
				},
			},
		},
	}
	r.CreateServiceRollout(&sro)
	if numupgradeServicesCalled != 1 {
		t.Fatalf("Unexpected numupgradeServicesCalled got %d", numupgradeServicesCalled)
	}
	if numgetUpgradeOrderCalled != 1 {
		t.Fatalf("Unexpected numgetUpgradeOrderCalled got %d", numgetUpgradeOrderCalled)
	}
	// since its same version - no invocations to upgrade should happen
	r.UpdateServiceRollout(&sro)
	if numupgradeServicesCalled != 1 {
		t.Fatalf("Unexpected numupgradeServicesCalled got %d", numupgradeServicesCalled)
	}
	if numgetUpgradeOrderCalled != 1 {
		t.Fatalf("Unexpected numgetUpgradeOrderCalled got %d", numgetUpgradeOrderCalled)
	}
	// check again
	r.UpdateServiceRollout(&sro)
	if numupgradeServicesCalled != 1 {
		t.Fatalf("Unexpected numupgradeServicesCalled got %d", numupgradeServicesCalled)
	}
	if numgetUpgradeOrderCalled != 1 {
		t.Fatalf("Unexpected numgetUpgradeOrderCalled got %d", numgetUpgradeOrderCalled)
	}
	// on a version change - upgrade should be invoked
	sro.Spec.Ops[0].Version = "dummy2"
	r.UpdateServiceRollout(&sro)
	if numupgradeServicesCalled != 2 {
		t.Fatalf("Unexpected numupgradeServicesCalled got %d", numupgradeServicesCalled)
	}
	if numgetUpgradeOrderCalled != 2 {
		t.Fatalf("Unexpected numgetUpgradeOrderCalled got %d", numgetUpgradeOrderCalled)
	}
	if h.numServiceCall != 2 || h.numServiceSuccess != 2 || h.numServiceFail != 0 {
		t.Fatalf("Unexpected count for serviceCall/Success/Fail got %#v", h)
	}

	sro.Spec.Ops[0].Version = "dummy3"
	r.upgradeServices = func(srvList []string) error { numupgradeServicesCalled++; return fmt.Errorf("dummyErr") }
	r.UpdateServiceRollout(&sro)
	if numupgradeServicesCalled != 3 {
		t.Fatalf("Unexpected numupgradeServicesCalled got %d", numupgradeServicesCalled)
	}
	if numgetUpgradeOrderCalled != 3 {
		t.Fatalf("Unexpected numgetUpgradeOrderCalled got %d", numgetUpgradeOrderCalled)
	}

	if h.numServiceCall != 3 || h.numServiceSuccess != 2 || h.numServiceFail != 1 {
		t.Fatalf("Unexpected count for serviceCall/Success/Fail got %#v", h)
	}

	r.DeleteServiceRollout(&sro)

	vro := rolloutproto.VeniceRollout{
		Spec: rolloutproto.VeniceRolloutSpec{
			Ops: []*rolloutproto.VeniceOpSpec{
				&rolloutproto.VeniceOpSpec{
					Op:      rolloutproto.VeniceOp_VenicePreCheck,
					Version: "dummy",
				},
			},
		},
	}
	r.CreateVeniceRollout(&vro)
	if numVenicePrecheckCalled != 1 {
		t.Fatalf("Unexpected numVenicePrecheckCalled got %d", numVenicePrecheckCalled)
	}

	vro.Spec.Ops[0].Op = rolloutproto.VeniceOp_VeniceRunVersion
	r.UpdateVeniceRollout(&vro)
	if numVenicePrecheckCalled != 1 {
		t.Fatalf("Unexpected numVenicePrecheckCalled got %d", numVenicePrecheckCalled)
	}
	if numVeniceRunVersionCalled != 1 {
		t.Fatalf("Unexpected numVeniceRunVersionCalled got %d", numVeniceRunVersionCalled)
	}
	r.DeleteVeniceRollout(&vro)

	r.UnregisterServiceStatusWriter(&h)
	r.UnregisterVeniceStatusWriter(&h)

}
