package fields_test

import (
	"fmt"
	"reflect"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/api/fields"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
)

func TestSelectorParse(t *testing.T) {
	testGoodStrings := []string{
		"x.a=a,y.b=b c,z.c=c",
		"x.c!=a,y.c=b",
		"w.y[z]=foo bar,x.x[y].z=bar",
		"v.x.y[z]!=foo,w.w[x].y[z]=bar",
		"x.x=loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongone",
	}
	testGoodSetStrings := []string{
		"x.y in (a,b,c)",
		"x.y in\t (a,b,c d)",
		"x.y in (a,b,c  d)",
		"x.x in (a,b),y.y in (c,d)",
		"x.x notin (a,b,c)",
		"x.x in (a,b),y.y notin (a,b)",
		"x.x<a",
		"x.x>1",
		"x.x>1,z<5",
		"x.x=test,y.y>=6,z.z<7",
	}
	testBadStrings := []string{
		"",
		"x.x",
		"x.x=superrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrlooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongone",
		"x.x=a||y.y=b",
		"x.x==a==b",
		"!x.x=a",
		"!x.x",
		"x=",
		"x.x= ",
		"x.x=,z.z= ",
		"x.x= ,z.z= ",
		"x.x in a",
		"x.x in (a",
		"x.x in (a,b",
		"x.x=(a,b)",
	}
	for _, test := range testGoodStrings {
		if _, err := Parse(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testGoodSetStrings {
		if _, err := Parse(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testBadStrings {
		if _, err := Parse(test); err == nil {
			t.Errorf("%v: did not get expected error\n", test)
		}
	}
}

func TestSelectorParseWithEscape(t *testing.T) {
	testGoodStrings := []string{
		"x.x=a\\,b\\,c",
		"x.x!=a\\,b",
	}
	testGoodSetStrings := []string{
		"x.x in (a\\,1,b\\,2,c\\,3)",
		"x.x notin (a\\,1,b\\,2,c\\,3)",
	}
	testBadStrings := []string{
		"",
		"x.x=a\\a||y.y=b",
		"x.x=a\\b",
		"x.x=(a\\\\1,b",
		"x.x=\\",
		"x.x in \\",
		"x.x in (=\\)",
	}
	for _, test := range testGoodStrings {
		if _, err := Parse(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testGoodSetStrings {
		if _, err := Parse(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testBadStrings {
		if _, err := Parse(test); err == nil {
			t.Errorf("%v: did not get expected error\n", test)
		}
	}
}

func TestDeterministicParse(t *testing.T) {
	s1, err := Parse("x.x=a,a.a=x")
	s2, err2 := Parse("a.a=x,x.x=a")
	if err != nil || err2 != nil {
		t.Errorf("Unexpected parse error")
	}
	if s1.String() != s2.String() {
		t.Errorf("Non-deterministic parse")
	}
}

func TestParseWithValidation(t *testing.T) {
	ti := time.Now()
	tests := []struct {
		kind       string
		selStr     string
		expSuccess bool
		selector   Selector
	}{
		{
			kind:       "cluster.Cluster",
			selStr:     "spec.quorum-nodes=192.168.30.11",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.QuorumNodes",
						Operator: "equals",
						Values:   []string{"192.168.30.11"},
					},
				},
			},
		},
		{
			kind:       "cluster.Cluster",
			selStr:     "spec.quorum-nodes notin (192.168.30.11,192.168.30.12),spec.virtual-ip!=192.168.30.11",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.QuorumNodes",
						Operator: "notIn",
						Values:   []string{"192.168.30.11", "192.168.30.12"},
					},
					&Requirement{
						Key:      "Spec.VirtualIP",
						Operator: "notEquals",
						Values:   []string{"192.168.30.11"},
					},
				},
			},
		},
		{
			kind:       "bookstore.Book",
			selStr:     "spec.editions[*].reviews[abc].review in (excellent,good)",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Editions[*].Reviews[abc].Review",
						Operator: "in",
						Values:   []string{"excellent", "good"},
					},
				},
			},
		},
		{
			kind:       "security.SGPolicy",
			selStr:     "spec.rules.apps in (mongo,redis)",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Rules.Apps",
						Operator: "in",
						Values:   []string{"mongo", "redis"},
					},
				},
			},
		},
		{
			kind:       "network.Service",
			selStr:     "spec.tls-client-policy.tls-client-certificates-selector[test]=good",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.TLSClientPolicy.CertificatesSelector[test]",
						Operator: "equals",
						Values:   []string{"good"},
					},
				},
			},
		},
		{
			kind:       "network.LbPolicy",
			selStr:     "spec.health-check.max-timeouts<60,spec.health-check.max-timeouts>50",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "lt",
						Values:   []string{"60"},
					},
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gt",
						Values:   []string{"50"},
					},
				},
			},
		}, {
			kind:       "network.LbPolicy",
			selStr:     "spec.type=Round Robin,spec.health-check.max-timeouts<60",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "lt",
						Values:   []string{"60"},
					},
					&Requirement{
						Key:      "Spec.Type",
						Operator: "equals",
						Values:   []string{"Round Robin"},
					},
				},
			},
		},
		{
			kind:       "monitoring.Alert",
			selStr:     fmt.Sprintf("status.resolved.time=%v", ti.Format(time.RFC3339Nano)),
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "equals",
						Values:   []string{ti.Format(time.RFC3339Nano)},
					},
				},
			},
		},
		{
			kind:       "monitoring.Alert",
			selStr:     fmt.Sprintf("status.resolved.time>=%v,spec.state=OPEN,status.resolved.time<%v", ti.Format(time.RFC3339Nano), ti.Add(10*time.Second).Format(time.RFC3339Nano)),
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.State",
						Operator: "equals",
						Values:   []string{"OPEN"},
					},
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "gte",
						Values:   []string{ti.Format(time.RFC3339Nano)},
					},
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "lt",
						Values:   []string{ti.Add(10 * time.Second).Format(time.RFC3339Nano)},
					},
				},
			},
		},
		{
			kind:       "network.LbPolicy",
			selStr:     "spec.health-check.max-timeouts>test",
			expSuccess: false,
		},
		{
			kind:       "monitoring.Alert",
			selStr:     "status.resolved.time>test",
			expSuccess: false,
		},
		{
			kind:       "security.SGPolicy",
			selStr:     "spec.rules[*].apps in (mongo,redis)", // Slices cant be indexed
			expSuccess: false,
		},
		{
			kind:       "security.SGPolicy",
			selStr:     "spec.inrules.apps in (mongo,redis)", // Non-existent field
			expSuccess: false,
		},
		{
			kind:       "security.SGPolicy",
			selStr:     "spec.rules in (mongo,redis)", // Non-leaf field
			expSuccess: false,
		},
	}
	for ii := range tests {
		sel, err := ParseWithValidation(tests[ii].kind, tests[ii].selStr)
		if !tests[ii].expSuccess {
			if err == nil {
				t.Fatalf("Expected %v to fail, found %v", tests[ii].selStr, sel)
			}
		} else {
			if err != nil {
				t.Fatalf("Expected %v to succeed, failed with error: %v", tests[ii].selStr, err)
			}
			if !reflect.DeepEqual(*sel, tests[ii].selector) {
				t.Fatalf("Expected %+v, got %+v", tests[ii].selector, sel)
			}
		}
	}
}

type RolePerms struct {
	Perms []string `json:"perms"`
}

type UserGroup struct {
	Group string `json:"group"`
}

type UserSpec struct {
	Name     string               `json:"name"`
	Groups   []*UserGroup         `json:"groups"`
	Perms    map[string]RolePerms `json:"perms"`
	PermsIdx map[int8]RolePerms   `json:"permsIdx"`
}

type User struct {
	api.TypeMeta
	api.ObjectMeta
	Spec UserSpec `json:"spec"`
}

func TestMatchesObj(t *testing.T) {
	u := &User{
		Spec: UserSpec{
			Groups: []*UserGroup{
				&UserGroup{},
			},
			Perms: map[string]RolePerms{
				"test": RolePerms{
					Perms: []string{"test"},
				},
				"foo": RolePerms{
					Perms: []string{"bar"},
				},
			},
			PermsIdx: map[int8]RolePerms{
				0: RolePerms{
					Perms: []string{"test"},
				},
				1: RolePerms{
					Perms: []string{"bar"},
				},
			},
		},
	}
	tests := []struct {
		selector Selector
		match    bool
	}{
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Name",
						Operator: "equals",
						Values:   []string{""},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Name",
						Operator: "equals",
						Values:   []string{"foo"},
					},
				},
			},
			match: false,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Perms[*].Perms",
						Operator: "in",
						Values:   []string{"foo", "bar", "test"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Perms[*].Perms",
						Operator: "in",
						Values:   []string{"bar"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Perms[*].Perms",
						Operator: "in",
						Values:   []string{"blah"},
					},
				},
			},
			match: false,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Perms[*].Perms",
						Operator: "notIn",
						Values:   []string{"blah"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.PermsIdx[0].Perms",
						Operator: "equals",
						Values:   []string{"test"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.PermsIdx[0].Perms",
						Operator: "notEquals",
						Values:   []string{"foo"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Groups.Group",
						Operator: "equals",
						Values:   []string{"foo"},
					},
					&Requirement{
						Key:      "Spec.Perms[*].Perms",
						Operator: "equals",
						Values:   []string{"test"},
					},
				},
			},
			match: false,
		},
	}
	for ii := range tests {
		if tests[ii].selector.MatchesObj(u) != tests[ii].match {
			t.Fatalf("Expected to match, but failed: index %v, selector %v", ii, tests[ii].selector)
		}
	}
}

func TestMatchesOnNonStringKeys(t *testing.T) {
	// test relational operators
	l := &network.LbPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "network.LbPolicy", // TODO: GetSchema() failes when just kind "LbPolicy" is passed. It expectes the API group as well. how to get the kind with api group?
		},
		Spec: network.LbPolicySpec{
			Type: "Round Robin",
			HealthCheck: &network.HealthCheckSpec{
				Interval:    120,
				MaxTimeouts: 60,
			},
		},
	}

	tests := []struct {
		selector Selector
		match    bool
	}{
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Type",
						Operator: "equals",
						Values:   []string{"Round Robin"},
					},
					&Requirement{
						Key:      "Spec.HealthCheck.Interval",
						Operator: "lt",
						Values:   []string{"121"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gte",
						Values:   []string{"60"},
					},
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "lt",
						Values:   []string{"120"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gt",
						Values:   []string{"50"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gt",
						Values:   []string{"50"},
					},
					&Requirement{
						Key:      "Spec.HealthCheck.Interval",
						Operator: "gt",
						Values:   []string{"50"},
					},
					&Requirement{
						Key:      "Spec.Type",
						Operator: "equals",
						Values:   []string{"Round Robin"},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gt",
						Values:   []string{"adfaf"},
					},
				},
			},
			match: false,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.HealthCheck.MaxTimeouts",
						Operator: "gt",
						Values:   []string{time.Now().String()},
					},
				},
			},
			match: false,
		},
	}

	for ii := range tests {
		if tests[ii].selector.MatchesObj(l) != tests[ii].match {
			t.Fatalf("Expected to match, but failed: index %v, selector %v", ii, tests[ii].selector)
		}
	}

	// test time fields
	tn := time.Now()
	ti, _ := types.TimestampProto(tn)
	a := &monitoring.Alert{
		TypeMeta: api.TypeMeta{
			Kind: "monitoring.Alert",
		},
		Status: monitoring.AlertStatus{
			Resolved: &monitoring.AuditInfo{
				User: "dummy",
				Time: &api.Timestamp{Timestamp: *ti},
			},
		},
	}

	tests = []struct {
		selector Selector
		match    bool
	}{
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "equals",
						Values:   []string{ti.String()},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "lt",
						Values:   []string{time.Now().Format(time.RFC3339Nano)},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "gte",
						Values:   []string{tn.Format(time.RFC3339Nano)},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "gte",
						Values:   []string{tn.Format(time.RFC3339Nano)},
					},
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "lt",
						Values:   []string{time.Now().Format(time.RFC3339Nano)},
					},
				},
			},
			match: true,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "gte",
						Values:   []string{"invalid"},
					},
				},
			},
			match: false,
		},
		{
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Status.Resolved.Time",
						Operator: "gte",
						Values:   []string{"1234"},
					},
				},
			},
			match: false,
		},
	}

	for ii := range tests {
		if tests[ii].selector.MatchesObj(a) != tests[ii].match {
			t.Fatalf("Expected to match, but failed: index %v, selector %v", ii, tests[ii].selector)
		}
	}
}
