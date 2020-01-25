// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleProfile(t *testing.T) {
	t.Parallel()
	profile := netproto.Profile{
		TypeMeta: api.TypeMeta{Kind: "Profile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testProfile",
		},
		Spec: netproto.ProfileSpec{
			FwdMode:    "TRANSPARENT",
			PolicyMode: "BASENET",
		},
	}

	err := HandleProfile(infraAPI, systemClient, types.Create, profile)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleProfile(infraAPI, systemClient, types.Update, profile)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleProfile(infraAPI, systemClient, types.Delete, profile)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleProfile(infraAPI, systemClient, 42, profile)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleProfileInfraFailures(t *testing.T) {
	t.Parallel()
	profile := netproto.Profile{
		TypeMeta: api.TypeMeta{Kind: "Profile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testProfile",
		},
		Spec: netproto.ProfileSpec{
			FwdMode:    "TRANSPARENT",
			PolicyMode: "BASENET",
		},
	}

	i := newBadInfraAPI()
	err := HandleProfile(i, systemClient, types.Create, profile)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleProfile(i, systemClient, types.Update, profile)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleProfile(i, systemClient, types.Delete, profile)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
