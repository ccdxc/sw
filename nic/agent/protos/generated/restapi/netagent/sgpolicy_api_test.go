// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package netproto is a auto generated package.
Input file: sgpolicy.proto
*/
package restapi_test

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestNetworkSecurityPolicyList(t *testing.T) {
	t.Parallel()
	var ok bool
	var networksecuritypolicyList []*netproto.NetworkSecurityPolicy

	err := netutils.HTTPGet("http://"+agentRestURL+"/api/security/policies/", &networksecuritypolicyList)

	AssertOk(t, err, "Error getting networksecuritypolicys from the REST Server")
	for _, o := range networksecuritypolicyList {
		if o.Name == "preCreatedNetworkSecurityPolicy" {
			ok = true
			break
		}
	}
	if !ok {
		t.Errorf("Could not find preCreatedNetworkSecurityPolicy in Response: %v", networksecuritypolicyList)
	}

}

func TestNetworkSecurityPolicyPost(t *testing.T) {
	t.Parallel()
	var resp Response
	var ok bool
	var networksecuritypolicyList []*netproto.NetworkSecurityPolicy

	postData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			VrfName:      "default",
			AttachGroup:  []string{"preCreatedSecurityGroup"},
			AttachTenant: false,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "172.17.0.0/24", "4.4.4.4"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
						},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0", "8.8.8.8/8"},
					},
				},
			},
		},
	}
	err := netutils.HTTPPost("http://"+agentRestURL+"/api/security/policies/", &postData, &resp)
	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/security/policies/", &networksecuritypolicyList)

	AssertOk(t, err, "Error posting networksecuritypolicy to REST Server")
	AssertOk(t, getErr, "Error getting networksecuritypolicys from the REST Server")
	for _, o := range networksecuritypolicyList {
		if o.Name == "testPostNetworkSecurityPolicy" {
			ok = true
			break
		}
	}
	if !ok {
		t.Errorf("Could not find testPostNetworkSecurityPolicy in Response: %v", networksecuritypolicyList)
	}

}

func TestNetworkSecurityPolicyDelete(t *testing.T) {
	t.Parallel()
	var resp Response
	var found bool
	var networksecuritypolicyList []*netproto.NetworkSecurityPolicy

	deleteData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testDeleteNetworkSecurityPolicy",
		},
		Spec: netproto.NetworkSecurityPolicySpec{
			VrfName:      "default",
			AttachTenant: true,
			Rules: []netproto.PolicyRule{
				{
					Action: "PERMIT",
					Src: &netproto.MatchSelector{
						Addresses: []string{"10.0.0.0 - 10.0.1.0", "172.17.0.0/24", "4.4.4.4"},
						AppConfigs: []*netproto.AppConfig{
							{
								Port:     "80",
								Protocol: "tcp",
							},
							{
								Port:     "443",
								Protocol: "tcp",
							},
							{
								Port:     "53",
								Protocol: "udp",
							},
						},
					},

					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.0.1 - 192.168.1.0"},
					},
				},
			},
		},
	}
	postErr := netutils.HTTPPost("http://"+agentRestURL+"/api/security/policies/", &deleteData, &resp)
	err := netutils.HTTPDelete("http://"+agentRestURL+"/api/security/policies/default/default/testDeleteNetworkSecurityPolicy", &deleteData, &resp)
	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/security/policies/", &networksecuritypolicyList)

	AssertOk(t, postErr, "Error posting networksecuritypolicy to REST Server")
	AssertOk(t, err, "Error deleting networksecuritypolicy from REST Server")
	AssertOk(t, getErr, "Error getting networksecuritypolicys from the REST Server")
	for _, o := range networksecuritypolicyList {
		if o.Name == "testDeleteNetworkSecurityPolicy" {
			found = true
			break
		}
	}
	if found {
		t.Errorf("Found testDeleteNetworkSecurityPolicy in Response after deleting: %v", networksecuritypolicyList)
	}

}

func TestNetworkSecurityPolicyUpdate(t *testing.T) {
	t.Parallel()
	var resp Response
	var networksecuritypolicyList []*netproto.NetworkSecurityPolicy

	var actualNetworkSecurityPolicySpec netproto.NetworkSecurityPolicySpec
	updatedNetworkSecurityPolicySpec := netproto.NetworkSecurityPolicySpec{
		VrfName:     "default",
		AttachGroup: []string{"preCreatedSecurityGroup"},
		Rules: []netproto.PolicyRule{
			{
				Action: "DENY",
			},
		},
	}
	putData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Name:      "preCreatedNetworkSecurityPolicy",
			Namespace: "default",
		},
		Spec: updatedNetworkSecurityPolicySpec,
	}
	err := netutils.HTTPPut("http://"+agentRestURL+"/api/security/policies/default/default/preCreatedNetworkSecurityPolicy", &putData, &resp)
	AssertOk(t, err, "Error updating networksecuritypolicy to REST Server")

	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/security/policies/", &networksecuritypolicyList)
	AssertOk(t, getErr, "Error getting networksecuritypolicys from the REST Server")
	for _, o := range networksecuritypolicyList {
		if o.Name == "preCreatedNetworkSecurityPolicy" {
			actualNetworkSecurityPolicySpec = o.Spec
			break
		}
	}
	// Rule hashes are expected to change during update, so mask rule-id hashes here
	for idx := range actualNetworkSecurityPolicySpec.Rules {
		actualNetworkSecurityPolicySpec.Rules[idx].ID = 0
	}
	AssertEquals(t, updatedNetworkSecurityPolicySpec, actualNetworkSecurityPolicySpec, "Could not validate updated spec.")

}

func TestNetworkSecurityPolicyCreateErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badPostData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "",
		},
	}

	err := netutils.HTTPPost("http://"+agentRestURL+"/api/security/policies/", &badPostData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}

func TestNetworkSecurityPolicyDeleteErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badDelData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{Tenant: "default",
			Namespace: "default",
			Name:      "badObject"},
	}

	err := netutils.HTTPDelete("http://"+agentRestURL+"/api/security/policies/default/default/badObject", &badDelData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}

func TestNetworkSecurityPolicyUpdateErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badDelData := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{Tenant: "default",
			Namespace: "default",
			Name:      "badObject"},
	}

	err := netutils.HTTPPut("http://"+agentRestURL+"/api/security/policies/default/default/badObject", &badDelData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}
