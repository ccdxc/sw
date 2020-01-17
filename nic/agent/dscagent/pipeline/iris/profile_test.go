// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleSecurityProfile(t *testing.T) {
	t.Parallel()
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			AttachVrfs: []string{"default"},
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	defaultVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: 65,
		},
	}

	err := HandleSecurityProfile(infraAPI, nwSecClient, vrfClient, types.Create, profile, defaultVrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleSecurityProfile(infraAPI, nwSecClient, vrfClient, types.Update, profile, defaultVrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleSecurityProfile(infraAPI, nwSecClient, vrfClient, types.Delete, profile, defaultVrf)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleSecurityProfile(infraAPI, nwSecClient, vrfClient, 42, profile, defaultVrf)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleSecurityProfileInfraFailures(t *testing.T) {
	t.Parallel()
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			AttachVrfs: []string{"default"},
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	defaultVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: 65,
		},
	}

	i := newBadInfraAPI()
	err := HandleSecurityProfile(i, nwSecClient, vrfClient, types.Create, profile, defaultVrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleSecurityProfile(i, nwSecClient, vrfClient, types.Update, profile, defaultVrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleSecurityProfile(i, nwSecClient, vrfClient, types.Delete, profile, defaultVrf)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
