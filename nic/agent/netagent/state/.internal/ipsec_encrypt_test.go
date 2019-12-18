package _internal

import (
	"github.com/pensando/sw/nic/agent/netagent/state"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestIPSecSAEncryptCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}
	dupSAEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           42,
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	foundIPSecSA, err := ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA  ecnrypt was not found in DB")
	Assert(t, foundIPSecSA.Name == "testIPSecSAEncrypt", "IPSecSA encrypt names did not match", foundIPSecSA)

	// verify duplicate sa encrypt creations succeed
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating duplicate IPSec SA Encrypt rule")

	// verify duplicate sa encrypt with changed spec doesn't succeed.
	err = ag.CreateIPSecSAEncrypt(&dupSAEncrypt)
	Assert(t, err != nil, "Error creating duplicate IPSec SA Encrypt rule")

	// verify list api works.
	npList := ag.ListIPSecSAEncrypt()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec SA Encrypt rules")

	// delete the ipsec sa encrypt and verify its gone from db
	err = ag.DeleteIPSecSAEncrypt(saEncrypt.Tenant, saEncrypt.Namespace, saEncrypt.Name)
	AssertOk(t, err, "Error deleting IPSec Encrypt rule")
	_, err = ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	Assert(t, err != nil, "IPSec SA rule was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecSAEncrypt(saEncrypt.Tenant, saEncrypt.Namespace, saEncrypt.Name)
	Assert(t, err != nil, "deleting non-existing IPSec SA rule succeeded", ag)
}

func TestIPSecSAEncryptUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}
	saEncryptUpdate := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           42,
			TepVrf:        "default",
		},
	}
	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	foundSA, err := ag.FindIPSecSAEncrypt(saEncrypt.ObjectMeta)
	AssertOk(t, err, "IPSec SA Encrypt rule was not found in DB")
	Assert(t, foundSA.Name == "testIPSecSAEncrypt", "IPSec SA Encrypt rule names did not match", foundSA)

	err = ag.UpdateIPSecSAEncrypt(&saEncryptUpdate)
	AssertOk(t, err, "Error updating IPSec SA Encrypt rule")
}

func TestIPSecSAEncryptCreateOnValidTep(t *testing.T) {
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

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "public",
		},
	}

	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "IPSec Encrypt Rule creation pointing to a valid tep failed")
}

func TestIPSecSAEncryptCreateOnEmptyTep(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "IPSec Encrypt Rule creation pointing to an empty tep must succeed")
}

//--------------------- Corner Case Tests ---------------------//
func TestIPSecSAEncryptCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "NonExistentNamespace",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "IPSec Encrypt Rule creation on non existent namespace succeeded")
}

func TestIPSecSAEncryptCreateOnInvalidProtocol(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "BadProtocol",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "IPSec Encrypt Rule creation on invalid protocol spec")
}

func TestIPSecSAEncryptDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "BadLocalGwIP",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "IPSec Encrypt Rule creation on invalid gw ip succeeded")
}

func TestIPSecSAEncryptUpdateOnNonExistingRule(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "badIPSecSAEncryptRule",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "BadProtocol",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "default",
		},
	}

	err := ag.UpdateIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "IPSec Encrypt Rule creation on invalid protocol spec")
}

func TestIPSecSAEncryptCreateOnInValidTep(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create backing Encrypt and Encrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "someRandomString",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "someRandomKey",
			LocalGwIP:     "10.0.0.1",
			RemoteGwIP:    "192.168.1.1",
			SPI:           1,
			TepVrf:        "badTep",
		},
	}

	err := ag.CreateIPSecSAEncrypt(&saEncrypt)
	Assert(t, err != nil, "IPSec Encrypt Rule creation pointing to an invalid tep succeeded")
}
