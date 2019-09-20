package cache

import (
	"os"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	logger = log.GetNewLogger(log.GetDefaultConfig("cache_test"))
	c      = NewCache(logger)
)

func TestMain(m *testing.M) {
	setup()
	retCode := m.Run()
	tearDown()
	os.Exit(retCode)
}
func setup() {
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"any"},
		},
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"10.1.1.1", "10.1.1.2", "10.1.1.4/30"},
			ToIPAddresses:   []string{"10.1.1.5"},
		},
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"10.1.1.1", "10.1.1.3", "10.1.1.10/30"},
			ToIPAddresses:   []string{"10.1.1.5-10.1.1.10"},
		},
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"10.1.1.1", "10.1.1.13", "10.1.1.10/30"},
			ToIPAddresses:   []string{"10.1.1.7"},
		},
	}
	// mock data in cache here
	obj := security.NetworkSecurityPolicy{TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		}}

	c.UpdateObject(&obj)
}
func tearDown() {

}

func TestCache_SearchPolicy_MISS(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "10.0.100.1", NetworkSecurityPolicy: "testpolicy", Namespace: "default", Tenant: "default", Protocol: "tcp", Port: "80"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MISS.String(), "Should have no results after search")

}

func TestCache_SearchPolicy_MATCH_ONE(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "172.0.0.1", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 1, "Should only have one result")
}

func TestCache_SearchPolicy_MATCH_MULTIPLE(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "10.1.1.1", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 3, "Should have multiple result")
}

func TestCache_SearchPolicy_MultiQuery_MISS(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "10.1.1.1", ToIPAddress: "10.2.2.2", Tenant: "default", App: "testApp"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MISS.String(), "Should have no results after search")
}

func TestCache_SearchPolicy_MultiQuery_MATCH_ONE(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "172.0.0.1", ToIPAddress: "any", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 1, "Should only have one result")
}

func TestCache_SearchPolicy_MultiQuery_MATCH_ANY(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "172.0.0.1", ToIPAddress: "1.2.3.4", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 1, "Should only have one result")
}

func TestCache_SearchPolicy_MultiQuery_MATCH_MULTIPLE(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "10.1.1.1", ToIPAddress: "10.1.1.5", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 2, "Should only have multiple result")
}

func TestCache_SearchOnlyProtocol(t *testing.T) {
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{Tenant: "default",
		Namespace:             "default",
		NetworkSecurityPolicy: "testpolicy",
		Protocol:              "tcp",
		FromIPAddress:         "any",
		ToIPAddress:           "any"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MATCH.String(), "Should have results after search")
	Assert(t, len(resp.Results["testpolicy"].Entries) == 4, "Should only have multiple result")
	for i := 0; i <= 2; i++ {
		AssertEquals(t, false, reflect.DeepEqual(resp.Results["testpolicy"].Entries[i], resp.Results["testpolicy"].Entries[i+1]), "Every entry should be unique")
	}
}

func TestCache_DeleteObject(t *testing.T) {
	// mock data in cache here
	obj := security.NetworkSecurityPolicy{TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
		}}
	c.DeleteObject(&obj)
	resp, err := c.SearchPolicy(&search.PolicySearchRequest{FromIPAddress: "10.1.1.1", Tenant: "default"})
	AssertOk(t, err, "Search failed")
	Assert(t, resp.Status == search.PolicySearchResponse_MISS.String(), "Should Miss because we have no obj")
}
