package state

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestRouteCreateDelete(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			Interface: "default-uplink-1",
			GatewayIP: "10.1.1.1",
		},
	}

	// create route
	err := ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating route")
	route, err := ag.FindRoute(rt.ObjectMeta)
	AssertOk(t, err, "Route was not found in DB")
	Assert(t, route.Name == "testRoute", "Route names did not match", route)

	// verify duplicate route creations succeed
	err = ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating duplicate route")

	// verify list api works.
	rtList := ag.ListRoute()
	Assert(t, len(rtList) == 1, "Incorrect number of routes")

	// delete the route and verify its gone from db
	err = ag.DeleteRoute(&rt)
	AssertOk(t, err, "Error deleting route")
	_, err = ag.FindNatPolicy(rt.ObjectMeta)
	Assert(t, err != nil, "Route was still found in database after deleting", ag)

	// verify you can not delete non-existing tenant
	err = ag.DeleteRoute(&rt)
	Assert(t, err != nil, "deleting non-existing route succeeded", ag)
}

func TestRouteUpdate(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	// route
	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			GatewayIP: "10.1.1.1",
			Interface: "default-uplink-1",
		},
	}

	// create nat policy
	err := ag.CreateRoute(&rt)
	AssertOk(t, err, "Error creating route")
	route, err := ag.FindRoute(rt.ObjectMeta)
	AssertOk(t, err, "Route not found in DB")
	Assert(t, route.Name == "testRoute", "Route names did not match", route)

	rtSpec := netproto.RouteSpec{
		Interface: "default-uplink-3",
	}

	rt.Spec = rtSpec

	err = ag.UpdateRoute(&rt)
	AssertOk(t, err, "Error updating route")

	updRt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			GatewayIP: "10.1.1.100",
			Interface: "default-uplink-1",
		},
	}
	err = ag.UpdateRoute(&updRt)
	AssertOk(t, err, "Error updating route")

}

//--------------------- Corner Case Tests ---------------------//
func TestRouteCreateOnNonExistingNamespace(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "BadNamsepace",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			Interface: "default-uplink-1",
			GatewayIP: "10.1.1.1",
		},
	}

	// create route
	err := ag.CreateRoute(&rt)
	Assert(t, err != nil, "route create on non existent namespace should fail")
}

func TestRouteCreateDatapathFailure(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			Interface: "default-uplink-1",
			GatewayIP: "BadGwIP",
		},
	}

	// create route
	err := ag.CreateRoute(&rt)
	Assert(t, err != nil, "route create with bad gateway ip should fail")
}

func TestRouteUpdateOnNonExistentRoute(t *testing.T) {
	// create netagent
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	rt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testRoute",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.1.1.0/24",
			Interface: "default-uplink-1",
			GatewayIP: "10.1.1.1",
		},
	}

	// create route
	err := ag.UpdateRoute(&rt)
	Assert(t, err != nil, "route udpate with non existent route should fail")
}
