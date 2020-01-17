// +build ignore

package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIPAMPolicyCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()
	name := "testPolicy"
	updateIP := "101.1.1.1"
	policy := netproto.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      name,
		},
		Spec: netproto.IPAMPolicySpec{
			DHCPRelay: &netproto.DHCPRelayPolicy{},
		},
		Status: netproto.IPAMPolicyStatus{},
	}
	server := &netproto.DHCPServer{
		IPAddress:     "100.1.1.1",
		VirtualRouter: "default",
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server)
	preExistingCount := len(ag.ListIPAMPolicy())

	// create IPAMPolicy
	err := ag.CreateIPAMPolicy(&policy)
	AssertOk(t, err, "Error creating IPAMPolicy")
	p, err := ag.FindIPAMPolicy(policy.ObjectMeta)
	AssertOk(t, err, "IPAMPolicy was not found in DB")
	Assert(t, p.Name == name, "IPAMPolicy name did not match", policy)

	// verify list api works.
	policyList := ag.ListIPAMPolicy()
	Assert(t, len(policyList) == preExistingCount+1, "Incorrect number of IPAM policies")

	// update the IPAMPolicy
	policy.Spec.DHCPRelay.Servers[0].IPAddress = updateIP
	err = ag.UpdateIPAMPolicy(&policy)
	AssertOk(t, err, "Error updating IPAMPolicy")
	p, err = ag.FindIPAMPolicy(policy.ObjectMeta)
	AssertOk(t, err, "IPAMPolicy was not found in DB")
	Assert(t, (p.Spec.DHCPRelay.Servers[0].IPAddress == updateIP), "DHCP server IPAddr did not match", policy)

	// delete the IPAMPolkcy and verify its gone from db
	err = ag.DeleteIPAMPolicy(policy.Tenant, policy.Namespace, policy.Name)
	AssertOk(t, err, "Error deleting IPAMPolicy")
	_, err = ag.FindIPAMPolicy(policy.ObjectMeta)
	Assert(t, (err != nil), "IPAMPolicy was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteIPAMPolicy(policy.Tenant, policy.Namespace, policy.Name)
	Assert(t, err != nil, "deleting non-existing IPAMPolicy succeeded", ag)
}
