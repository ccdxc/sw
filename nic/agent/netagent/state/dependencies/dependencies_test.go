package dependencies

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestStateDependencies_Add(t *testing.T) {
	s := NewDepSolver()
	parent := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}

	c1 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNS",
		},
	}

	c2 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testNetwork1",
		},
	}
	c3 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testNetwork2",
		},
	}
	err := s.Add(&parent, &c1, &c2, &c3)
	AssertOk(t, err, "failed to add dependency")
	AssertEquals(t, 3, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 3 dependent objects.")
	deps := s.DB["tenant|testTenant"]

	// check if the dependencies match
	_, ok := deps["namespace|testTenant|testNS"]
	AssertEquals(t, true, ok, "missing child namespace dependency. %v", s.DB["tenant|testTenant"])

	_, ok = deps["network|testTenant|testNS|testNetwork1"]
	AssertEquals(t, true, ok, "missing child network dependency. %v", s.DB["tenant|testTenant"])

	_, ok = deps["network|testTenant|testNS|testNetwork2"]
	AssertEquals(t, true, ok, "missing child network dependency. %v", s.DB["tenant|testTenant"])

}

func TestStateDependencies_Remove(t *testing.T) {
	s := NewDepSolver()
	parent := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}
	c1 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNS",
		},
	}
	err := s.Add(&parent, &c1)
	AssertOk(t, err, "failed to add dependency")
	AssertEquals(t, 1, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 3 dependent objects.")
	deps := s.DB["tenant|testTenant"]
	_, ok := deps["namespace|testTenant|testNS"]
	AssertEquals(t, true, ok, "missing child namespace dependency. %v", s.DB["tenant|testTenant"])

	// Remove the dependency
	err = s.Remove(&parent, &c1)
	AssertOk(t, err, "failed to remove dependency")
	AssertEquals(t, 0, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 0 dependent objects.")

	// Add again to verify the dependency
	err = s.Add(&parent, &c1)
	AssertOk(t, err, "failed to add dependency")
	AssertEquals(t, 1, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 3 dependent objects.")
	deps = s.DB["tenant|testTenant"]
	_, ok = deps["namespace|testTenant|testNS"]
	AssertEquals(t, true, ok, "missing child namespace dependency. %v", s.DB["tenant|testTenant"])

}

func TestStateDependencies_Solve(t *testing.T) {
	s := NewDepSolver()
	parent := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}
	c1 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNS",
		},
	}
	err := s.Add(&parent, &c1)
	AssertOk(t, err, "failed to add dependency")
	AssertEquals(t, 1, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 3 dependent objects.")
	deps := s.DB["tenant|testTenant"]
	_, ok := deps["namespace|testTenant|testNS"]
	AssertEquals(t, true, ok, "missing child namespace dependency. %v", s.DB["tenant|testTenant"])

	// Solve the dependency. Solve should exit with ErrCannotDelete
	err = s.Solve(&parent)
	exp := &types.ErrCannotDelete{
		References: []string{"/api/namespaces/testTenant/testNS"},
	}
	AssertEquals(t, exp, err, "failed to resolve dependency")

	// Remove the dependency
	err = s.Remove(&parent, &c1)
	AssertOk(t, err, "failed to remove dependency")
	AssertEquals(t, 0, len(s.DB["tenant|testTenant"]), "Expected the tenant to have 0 dependent objects.")

	// Solve should report that the object is clear for deletion
	err = s.Solve(&parent)
	AssertOk(t, err, "solve reported failure when there are no dependencies")

}

func TestStateDependencies_ResolveTypeSelfLink(t *testing.T) {
	s := NewDepSolver()
	var m meta
	// resolve vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testVrf"},
	}
	tMeta, oMeta, err := s.resolveObjectType(&vrf)
	AssertOk(t, err, "failed to resolve vrf object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err := s.composeKeySelfLink(&m)

	meta := s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for vrf")
	AssertEquals(t, "vrf|testTenant|testNS|testVrf", key, "failed to compose key for vrf object")
	AssertEquals(t, "/api/vrfs/testTenant/testNS/testVrf", selfLink, "failed to compose key for network")

	// resolve tenant
	tn := netproto.Tenant{
		TypeMeta:   api.TypeMeta{Kind: "tenant"},
		ObjectMeta: api.ObjectMeta{Name: "testTenant"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&tn)
	AssertOk(t, err, "failed to resolve tenant object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for tenant")
	AssertEquals(t, "tenant|testTenant", key, "failed to compose key for tenant object")
	AssertEquals(t, "/api/tenants/testTenant", selfLink, "failed to compose key for tenant object")

	// resolve namespace
	ns := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "namespace"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testTenant",
			Name:   "testNS"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&ns)
	AssertOk(t, err, "failed to resolve namespace object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for namespace")
	AssertEquals(t, "namespace|testTenant|testNS", key, "failed to compose key for namespace object")
	AssertEquals(t, "/api/namespaces/testTenant/testNS", selfLink, "failed to compose key for namespace object")

	// resolve network
	nt := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testNetwork"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&nt)
	AssertOk(t, err, "failed to resolve network object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for network")
	AssertEquals(t, "network|testTenant|testNS|testNetwork", key, "failed to compose key for network object")
	AssertEquals(t, "/api/networks/testTenant/testNS/testNetwork", selfLink, "failed to compose key for network object")

	// resolve endpoint
	ep := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testEndpoint"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&ep)
	AssertOk(t, err, "failed to resolve endpoint object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for endpoint")
	AssertEquals(t, "endpoint|testTenant|testNS|testEndpoint", key, "failed to compose key for endpoint object")
	AssertEquals(t, "/api/endpoints/testTenant/testNS/testEndpoint", selfLink, "failed to compose key for endpoint object")

	// resolve sgpolicy
	sgPolicy := netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testNetworkSecurityPolicy"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&sgPolicy)
	AssertOk(t, err, "failed to resolve sgpolicy object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for sgpolicy")
	AssertEquals(t, "networksecuritypolicy|testTenant|testNS|testNetworkSecurityPolicy", key, "failed to compose key for networksecuritypolicy object")
	AssertEquals(t, "/api/security/policies/testTenant/testNS/testNetworkSecurityPolicy", selfLink, "failed to compose key for sgpolicy object")

	// resolve tunnel
	tun := netproto.Tunnel{
		TypeMeta: api.TypeMeta{Kind: "tunnel"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testTunnel"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&tun)
	AssertOk(t, err, "failed to resolve tunnel object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for tunnel")
	AssertEquals(t, "tunnel|testTenant|testNS|testTunnel", key, "failed to compose key for tunnel object")
	AssertEquals(t, "/api/tunnels/testTenant/testNS/testTunnel", selfLink, "failed to compose key for tunnel object")
	app := netproto.App{
		TypeMeta: api.TypeMeta{Kind: "app"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "testTenant",
			Namespace: "testNS",
			Name:      "testApp"},
	}
	tMeta, oMeta, err = s.resolveObjectType(&app)
	AssertOk(t, err, "failed to resolve app object type")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta != nil, "regenerating meta from object key failed")
	AssertOk(t, err, "failed to compose self link for tunnel")
	AssertEquals(t, "app|testTenant|testNS|testApp", key, "failed to compose key for app object")
	AssertEquals(t, "/api/apps/testTenant/testNS/testApp", selfLink, "failed to compose key for app object")

	// invalid object resolution
	tMeta, oMeta, err = s.resolveObjectType(nil)
	AssertEquals(t, api.TypeMeta{}, tMeta, "expected type meta to be of zero value")
	AssertEquals(t, api.ObjectMeta{}, oMeta, "expected object meta to be of zero value")
	Assert(t, err != nil, "nil type object resolution should return a valid error")
	m.T = tMeta
	m.O = oMeta
	key, selfLink, err = s.composeKeySelfLink(&m)
	meta = s.composeMetaFromKey(key)
	Assert(t, meta == nil, "regenerating meta from object key failed")
	AssertEquals(t, "", key, "expected key to be empty")
	AssertEquals(t, "", selfLink, "expected self link to be empty")
	Assert(t, err != nil, "nil object self link should return a valid error")

}
