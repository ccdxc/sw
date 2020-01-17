// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestSecurityProfileCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	AssertOk(t, err, "Error creating security profile")
	sgp, err := ag.FindSecurityProfile(profile.ObjectMeta)
	AssertOk(t, err, "Security Profile was not found in DB")
	Assert(t, sgp.Name == "testSecurityProfile", "SecurityProfile names did not match", sgp)

	// verify duplicate tenant creations succeed
	err = ag.CreateSecurityProfile(&profile)
	AssertOk(t, err, "Error creating duplicate security profile")

	// verify list api works.
	sgpList := ag.ListSecurityProfile()
	Assert(t, len(sgpList) == 1, "Incorrect number of sg policies")

	// delete the security profile and verify its gone from db
	err = ag.DeleteSecurityProfile(profile.Tenant, profile.Namespace, profile.Name)
	AssertOk(t, err, "Error deleting security profile")
	_, err = ag.FindSecurityProfile(profile.ObjectMeta)
	Assert(t, err != nil, "Security Profile was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteSecurityProfile(profile.Tenant, profile.Namespace, profile.Name)
	Assert(t, err != nil, "deleting non-existing security profile succeeded", ag)
}

func TestSecurityProfileCreateDeleteImplicitDefaultVrf(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	AssertOk(t, err, "Error creating security profile")
	sgp, err := ag.FindSecurityProfile(profile.ObjectMeta)
	AssertOk(t, err, "Security Profile was not found in DB")
	Assert(t, sgp.Name == "testSecurityProfile", "SecurityProfile names did not match", sgp)

	// verify duplicate tenant creations succeed
	err = ag.CreateSecurityProfile(&profile)
	AssertOk(t, err, "Error creating duplicate security profile")

	// verify list api works.
	sgpList := ag.ListSecurityProfile()
	Assert(t, len(sgpList) == 1, "Incorrect number of sg policies")

	// delete the security profile and verify its gone from db
	err = ag.DeleteSecurityProfile(profile.Tenant, profile.Namespace, profile.Name)
	AssertOk(t, err, "Error deleting security profile")
	_, err = ag.FindSecurityProfile(profile.ObjectMeta)
	Assert(t, err != nil, "Security Profile was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteSecurityProfile(profile.Tenant, profile.Namespace, profile.Name)
	Assert(t, err != nil, "deleting non-existing security profile succeeded", ag)
}

func TestSecurityProfileUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfileUpdate",
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	AssertOk(t, err, "Error creating security profile")
	sgp, err := ag.FindSecurityProfile(profile.ObjectMeta)
	AssertOk(t, err, "SG policy was not found in DB")
	Assert(t, sgp.Name == "testSecurityProfileUpdate", "sec profile names did not match", sgp)

	updProfile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfileUpdate",
		},
		Spec: netproto.SecurityProfileSpec{
			AttachVrfs: []string{"default"},
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "20s",
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

	err = ag.UpdateSecurityProfile(&updProfile)
	AssertOk(t, err, "Error updating security profile")
}

//--------------------- Corner Case Tests ---------------------//

func TestSecurityProfileMissingAttachmentNamespaces(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			AttachVrfs: []string{"non-existent-namespace"},
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidSessionIdleTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "bad timeout",
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidTCPTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "bad timeout",
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidTCPDropTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "bad timeout",
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

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidTCPConnectionSetupTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "bad timeout",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidTCPCloseTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "1s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "bad timeout",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidDropTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "1s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "bad timeout",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidUDPTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "15s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "bad timeout",
				UDPDrop:            "1s",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileUDPDropTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "bad timeout",
				ICMP:               "100ms",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileInvalidICMPTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "15s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "1s",
				ICMP:               "bad timeout",
				ICMPDrop:           "1h10m15s",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}

func TestSecurityProfileICMPDropTimeOut(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// security profile
	profile := netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testSecurityProfile",
		},
		Spec: netproto.SecurityProfileSpec{
			Timeouts: &netproto.Timeouts{
				SessionIdle:        "10s",
				TCP:                "1m",
				TCPDrop:            "5s",
				TCPConnectionSetup: "300ms",
				TCPClose:           "1h",
				Drop:               "30s",
				UDP:                "5s",
				UDPDrop:            "5s",
				ICMP:               "100ms",
				ICMPDrop:           "bad timeout",
			},
		},
	}

	// create security profile
	err := ag.CreateSecurityProfile(&profile)
	Assert(t, err != nil, "Security Profile creation with invalid session idle time out must fail")
}
