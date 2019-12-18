package _internal

import (
	"github.com/pensando/sw/nic/agent/netagent/state"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestIPSecPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

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
			TepVrf:             "infra",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSADecrypt",
					SAType: "DECRYPT",
					SPI:    42,
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPSec policy")
	foundIPSecPolicy, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "IPSec Policy was not found in DB")
	Assert(t, foundIPSecPolicy.Name == "testIPSecPolicy", "IPSecPolicy names did not match", foundIPSecPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating duplicate IPSec policy")

	// verify list api works.
	npList := ag.ListIPSecPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec policies")

	// delete the ipsec policy and verify its gone from db
	err = ag.DeleteIPSecPolicy(ipSecPolicy.Tenant, ipSecPolicy.Namespace, ipSecPolicy.Name)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecPolicy(ipSecPolicy.Tenant, ipSecPolicy.Namespace, ipSecPolicy.Name)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

func TestIPSecPolicyUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}

	ipSecPolicyUpdate := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.0.0.0 -  20.1.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "IPSec Policy creation failed")

	err = ag.UpdateIPSecPolicy(&ipSecPolicyUpdate)
	AssertOk(t, err, "IPSec Policy update failed")

}

func TestIPSecPolicyCreateDeleteNonDefaultVrf(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

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
			TepVrf:             "infra",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			VrfName: "infra",
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSADecrypt",
					SAType: "DECRYPT",
					SPI:    42,
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPSec policy")
	foundIPSecPolicy, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "IPSec Policy was not found in DB")
	Assert(t, foundIPSecPolicy.Name == "testIPSecPolicy", "IPSecPolicy names did not match", foundIPSecPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating duplicate IPSec policy")

	// verify list api works.
	npList := ag.ListIPSecPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec policies")

	// delete the ipsec policy and verify its gone from db
	err = ag.DeleteIPSecPolicy(ipSecPolicy.Tenant, ipSecPolicy.Namespace, ipSecPolicy.Name)
	AssertOk(t, err, "Error deleting nat policy")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Pool was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPSecPolicy(ipSecPolicy.Tenant, ipSecPolicy.Namespace, ipSecPolicy.Name)
	Assert(t, err != nil, "deleting non-existing nat policy succeeded", ag)
}

//--------------------- Corner Case Tests ---------------------//

func TestIPSecPolicyCreateDeleteOnRemoteSARule(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create remote NS
	rns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "remoteNS",
		},
	}
	err := ag.CreateNamespace(&rns)
	AssertOk(t, err, "Could not create remote namespace")

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err = ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

	// Create remote NS backing Encrypt and Decrypt rules
	saEncrypt := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "kg2-ipsec-sa-encrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			LocalGwIP:     "20.1.1.1",
			RemoteGwIP:    "20.1.1.2",
			SPI:           1,
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "remoteNS",
			Name:      "kg2-ipsec-sa-decrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			DecryptAlgo:   "AES_GCM_256",
			DecryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			SPI:           1,
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0-10.0.255.255"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0-10.0.255.255"},
					},
					SAName: "remoteNS/kg2-ipsec-sa-encrypt",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "remoteNS/kg2-ipsec-sa-decrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating IPSec policy")
	foundIPSecPolicy, err := ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	AssertOk(t, err, "IPSec Policy was not found in DB")
	Assert(t, foundIPSecPolicy.Name == "kg2-ipsec-decrypt-policy", "IPSecPolicy names did not match", foundIPSecPolicy)

	// verify duplicate tenant creations succeed
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	AssertOk(t, err, "Error creating duplicate IPSec policy")

	// verify list api works.
	npList := ag.ListIPSecPolicy()
	Assert(t, len(npList) == 1, "Incorrect number of IPSec policies")
}

func TestIPSecPolicyCreateDeleteOnNonExistentSARule(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}

	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-sa-decrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			DecryptAlgo:   "AES_GCM_256",
			DecryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			SPI:           1,
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Decrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0-10.0.255.255"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0-10.0.255.255"},
					},
					SAName: "nonExistentEncryptSA",
					SAType: "ENCRYPT",
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "remoteNS/kg2-ipsec-sa-decrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existen encrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "nonExistentDecryptPolicy",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	// IPSec POlicy with missing decrypt SA
	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "nonExistentDecryptSA",
					SAType: "DECRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")

	// IPSec Policy with SPI Value in encrypt policy
	ipSecPolicy = netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "spi-on-encrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}

	// create IPSec policy
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec policy creates on non existent decrypt SA Rules should fail")
	_, err = ag.FindIPSecPolicy(ipSecPolicy.ObjectMeta)
	Assert(t, err != nil, "IPSec Policy was not found in DB")
}

func TestIPSecPolicyCreateInvalidRule(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "BADRULE TYPE",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy with invalid rules should fail")

}

func TestIPSecPolicyDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - Bad IP"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy with invalid address range should fail")

}

func TestIPSecPolicyCreateOnNonExistentNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "BadNamespace",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.1.0.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy with non-existent namespace should fail")
}

func TestIPSecPolicyUpdateOnNonExistentIPSecPolicy(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "NonExistentIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.1.0.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.UpdateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy update on non-existent ipsec policy should fail")
}

func TestIPSecPolicyCreateOnInvalidEncryptSA(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

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
			TepVrf:        "infra",
		},
	}
	err = ag.CreateIPSecSAEncrypt(&saEncrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.1.0.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "bad/formatted/testIPSecSAEncrypt",
					SAType: "ENCRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy create with bad formatted encrypt rule should fail")
}

func TestIPSecPolicyCreateOnInvalidDecryptSA(t *testing.T) {
	// create netagent
	ag, _, _ := state.createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// Create an infra VRF
	infraVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	err := ag.CreateVrf(&infraVrf)
	AssertOk(t, err, "infra vrf creates must succeed")

	// Create backing Encrypt and Decrypt rules
	saDecrypt := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecSAEncrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:   "ESP",
			AuthAlgo:   "AES_GCM",
			AuthKey:    "someRandomString",
			LocalGwIP:  "10.0.0.1",
			RemoteGwIP: "192.168.1.1",
			TepVrf:     "infra",
		},
	}
	err = ag.CreateIPSecSADecrypt(&saDecrypt)
	AssertOk(t, err, "Error creating IPSec SA Encrypt rule")

	ipSecPolicy := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testIPSecPolicy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.1.0.0"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
					SAName: "bad/formatted/testIPSecSADecrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}
	err = ag.CreateIPSecPolicy(&ipSecPolicy)
	Assert(t, err != nil, "IPSec Policy create with bad formatted decrypt rule should fail")
}
