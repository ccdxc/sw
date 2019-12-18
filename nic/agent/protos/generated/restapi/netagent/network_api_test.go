// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package netproto is a auto generated package.
Input file: network.proto
*/
package restapi_test

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestNetworkList(t *testing.T) {
	t.Parallel()
	var ok bool
	var networkList []*netproto.Network

	err := netutils.HTTPGet("http://"+agentRestURL+"/api/networks/", &networkList)

	AssertOk(t, err, "Error getting networks from the REST Server")
	for _, o := range networkList {
		if o.Name == "preCreatedNetwork" {
			ok = true
			break
		}
	}
	if !ok {
		t.Errorf("Could not find preCreatedNetwork in Response: %v", networkList)
	}

}

func TestNetworkPost(t *testing.T) {
	t.Parallel()
	var resp Response
	var ok bool
	var networkList []*netproto.Network

	postData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostNetwork",
		},
		Spec: netproto.NetworkSpec{
			VrfName: "default",
			VlanID:  42,
		},
	}
	err := netutils.HTTPPost("http://"+agentRestURL+"/api/networks/", &postData, &resp)
	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/networks/", &networkList)

	AssertOk(t, err, "Error posting network to REST Server")
	AssertOk(t, getErr, "Error getting networks from the REST Server")
	for _, o := range networkList {
		if o.Name == "testPostNetwork" {
			ok = true
			break
		}
	}
	if !ok {
		t.Errorf("Could not find testPostNetwork in Response: %v", networkList)
	}

}

func TestNetworkUpdate(t *testing.T) {
	t.Parallel()
	var resp Response
	var networkList []*netproto.Network

	updatedNetworkSpec := netproto.NetworkSpec{
		VrfName: "default",
		VlanID:  42,
	}
	var actualNetworkSpec netproto.NetworkSpec
	putData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "preCreatedNetwork",
		},
		Spec: updatedNetworkSpec,
	}
	err := netutils.HTTPPut("http://"+agentRestURL+"/api/networks/default/default/preCreatedNetwork", &putData, &resp)
	AssertOk(t, err, "Error updating network to REST Server")

	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/networks/", &networkList)
	AssertOk(t, getErr, "Error getting networks from the REST Server")
	for _, o := range networkList {
		if o.Name == "preCreatedNetwork" {
			actualNetworkSpec = o.Spec
			break
		}
	}
	AssertEquals(t, updatedNetworkSpec, actualNetworkSpec, "Could not validated updated spec.")

}

func TestNetworkDelete(t *testing.T) {
	t.Parallel()
	var resp Response
	var found bool
	var networkList []*netproto.Network

	deleteData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testDeleteNetwork",
		},
		Spec: netproto.NetworkSpec{
			VrfName: "default",
			VlanID:  42,
		},
	}
	postErr := netutils.HTTPPost("http://"+agentRestURL+"/api/networks/", &deleteData, &resp)
	err := netutils.HTTPDelete("http://"+agentRestURL+"/api/networks/default/default/testDeleteNetwork", &deleteData, &resp)
	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/networks/", &networkList)

	AssertOk(t, postErr, "Error posting network to REST Server")
	AssertOk(t, err, "Error deleting network from REST Server")
	AssertOk(t, getErr, "Error getting networks from the REST Server")
	for _, o := range networkList {
		if o.Name == "testDeleteNetwork" {
			found = true
			break
		}
	}
	if found {
		t.Errorf("Found testDeleteNetwork in Response after deleting: %v", networkList)
	}

}

func TestNetworkCreateErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badPostData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name: "",
		},
	}

	err := netutils.HTTPPost("http://"+agentRestURL+"/api/networks/", &badPostData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}

func TestNetworkDeleteErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badDelData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{Tenant: "default",
			Namespace: "default",
			Name:      "badObject"},
	}

	err := netutils.HTTPDelete("http://"+agentRestURL+"/api/networks/default/default/badObject", &badDelData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}

func TestNetworkUpdateErr(t *testing.T) {
	t.Parallel()
	var resp Response
	badDelData := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{Tenant: "default",
			Namespace: "default",
			Name:      "badObject"},
	}

	err := netutils.HTTPPut("http://"+agentRestURL+"/api/networks/default/default/badObject", &badDelData, &resp)

	Assert(t, err != nil, "Expected test to error out with 500. It passed instead")
}
