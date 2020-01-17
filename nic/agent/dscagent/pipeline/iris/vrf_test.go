// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleVrfCustomer(t *testing.T) {
	t.Parallel()
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testVrf",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
	}

	err := HandleVrf(infraAPI, vrfClient, types.Create, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleVrf(infraAPI, vrfClient, types.Update, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleVrf(infraAPI, vrfClient, types.Delete, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleVrf(infraAPI, vrfClient, 42, vrf)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleVrfInfra(t *testing.T) {
	t.Parallel()
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testVrf",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := HandleVrf(infraAPI, vrfClient, types.Create, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleVrf(infraAPI, vrfClient, types.Update, vrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleVrf(infraAPI, vrfClient, types.Delete, vrf)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleVrfCustomerInfraFailures(t *testing.T) {
	t.Parallel()
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testVrf",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
	}
	i := newBadInfraAPI()

	err := HandleVrf(i, vrfClient, types.Create, vrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleVrf(i, vrfClient, types.Update, vrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleVrf(i, vrfClient, types.Delete, vrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleVrf(i, vrfClient, 42, vrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
