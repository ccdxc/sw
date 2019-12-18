package _internal

import (
	"github.com/pensando/sw/nic/agent/netagent/state"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestIPSecSADecryptCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
			TepVrf:             "default",
		},
	}

	dupSADecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
			TepVrf:             "default",
		},
	}
	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	foundIPSecSA, err := ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA  ecnrypt was not found in DB")
	Assert(t, foundIPSecSA.Name == "testIPSecSADecrypt", "IPSecSA encrypt names did not match", foundIPSecSA)

	// verify duplicate sa encrypt creations succeed
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating duplicate IPSec SA Decrypt rule")

	// verify duplicate sa encrypt with changed spec doesn't succeed.
	err = ag.CreateIPSecSADecrypt(&dupSADecrypt)
	Assert(t, err != nil, "Error creating duplicate IPSec SA Decrypt rule")

	// verify list api works.
	npList := ag.ListIPSecSADecrypt()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec SA Decrypt rules")

	// delete the ipsec sa encrypt and verify its gone from db
	err = ag.DeleteIPSecSADecrypt(saDecrypt.Tenant, saDecrypt.Namespace, saDecrypt.Name)
	AssertOk(t, err, "Error deleting IPSec Decrypt rule")
	_, err = ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	Assert(t, err != nil, "IPSec SA rule was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecSADecrypt(saDecrypt.Tenant, saDecrypt.Namespace, saDecrypt.Name)
	Assert(t, err != nil, "deleting non-existing IPSec SA rule succeeded", ag)
}

func TestIPSecSADecryptUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
			TepVrf:             "default",
		},
	}
	saDecryptUpdate := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
			TepVrf:             "default",
		},
	}
	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	foundSA, err := ag.FindIPSecSADecrypt(saDecrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Decrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSADecrypt", "IPSec SA Decrypt rule names did not match", foundSA)

	err = ag.UpdateIPSecSADecrypt(&saDecryptUpdate)
	AssertOk(t, err, "Error updating IPSec SA Decrypt rule")
}

func TestIPSecSADecryptCreateOnValidTep(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// create backing vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "public",
		},
	}

	err := ag.CreateVrf(&vrf)
	AssertOk(t, err, "Creating vrf failed")

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
			TepVrf:             "public",
		},
	}

	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "IPSec Decrypt Rule creation pointing to a valid tep failed")
}

//--------------------- Corner Case Tests ---------------------//
func TestIPSecSADecryptCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "NonExistentNamespace",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
			TepVrf:             "default",
		},
	}

	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "IPSec Decrypt Rule creation on non existent namespace succeeded")
}

func TestIPSecSADecryptCreateOnInvalidProtocol(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "BadProtocol",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
			TepVrf:             "default",
		},
	}

	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "IPSec Decrypt Rule creation on invalid protocol spec")
}

func TestIPSecSADecryptUpdateOnNonExistingRule(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "badIPSecSADecryptRule",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "BadProtocol",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                1,
			TepVrf:             "default",
		},
	}

	err := ag.UpdateIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "IPSec Decrypt Rule creation on invalid protocol spec")
}

func TestIPSecSADecryptCreateOnEmptyTep(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
		},
	}

	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "IPSec Decrypt Rule creation on empty tep succeeded")
}

func TestIPSecSADecryptCreateOnInValidTep(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Decrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSADecrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:           "ESP",
			AuthAlgo:           "AES_GCM",
			AuthKey:            "someRandomString",
			DecryptAlgo:        "AES_GCM_256",
			DecryptionKey:      "someRandomKey",
			RekeyDecryptAlgo:   "DES3",
			RekeyDecryptionKey: "someRandomString",
			LocalGwIP:          "10.0.0.1",
			RemoteGwIP:         "192.168.1.1",
			SPI:                42,
			TepVrf:             "badTep",
		},
	}

	err := ag.CreateIPSecSADecrypt(&saDecrypt)
	Assert(t, err != nil, "IPSec Decrypt Rule creation pointing to an invalid tep succeeded")
}
