// +build iris

package iris

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestHandleEndpointLocal(t *testing.T) {
	t.Parallel()
	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "skywalker",
			IPv4Addresses: []string{"10.1.1.1", "10.1.1.2"},
			MacAddress:    "baba.baba.baba",
			NodeUUID:      "luke",
			UsegVlan:      42,
		},
		Status: netproto.EndpointStatus{
			EnicID: 42, // Simulate a local EP
		},
	}

	err := HandleEndpoint(infraAPI, epClient, intfClient, types.Create, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Update, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleEndpoint(infraAPI, epClient, intfClient, 42, ep, 65, 42)
	if err == nil {
		t.Fatal("Invalid op must return a valid error.")
	}
}

func TestHandleEndpointRemote(t *testing.T) {
	t.Parallel()
	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "skywalker",
			IPv4Addresses: []string{"10.1.1.1", "10.1.1.2"},
			MacAddress:    "baba.baba.baba",
			NodeUUID:      "luke",
			UsegVlan:      42,
		},
	}

	err := HandleEndpoint(infraAPI, epClient, intfClient, types.Create, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Update, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}

	err = HandleEndpoint(infraAPI, epClient, intfClient, types.Delete, ep, 65, 42)
	if err != nil {
		t.Fatal(err)
	}
}

func TestHandleEndpointLocalInfraFailures(t *testing.T) {
	t.Parallel()
	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testEndpoint",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "skywalker",
			IPv4Addresses: []string{"10.1.1.1", "10.1.1.2"},
			MacAddress:    "baba.baba.baba",
			NodeUUID:      "luke",
			UsegVlan:      42,
		},
		Status: netproto.EndpointStatus{
			EnicID: 42, // Simulate a local EP
		},
	}
	i := newBadInfraAPI()
	err := HandleEndpoint(i, epClient, intfClient, types.Create, ep, 65, 42)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleEndpoint(i, epClient, intfClient, types.Update, ep, 65, 42)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}

	err = HandleEndpoint(i, epClient, intfClient, types.Delete, ep, 65, 42)
	if err == nil {
		t.Fatalf("Must return a valid error. Err: %v", err)
	}
}
