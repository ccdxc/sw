package fields_test

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/api/fields"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
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
	}
	testBadStrings := []string{
		"",
		"x.x",
		"x.x=superrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrlooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongone",
		"x.x=a||y.y=b",
		"x.x==a==b",
		"!x.x=a",
		"x.x<a",
		"!x.x",
		"x.x>1",
		"x.x>1,z<5",
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
			kind:       "security.Sgpolicy",
			selStr:     "spec.in-rules.apps in (mongo,redis)",
			expSuccess: true,
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.InRules.Apps",
						Operator: "in",
						Values:   []string{"mongo", "redis"},
					},
				},
			},
		},
		{
			kind:       "security.Sgpolicy",
			selStr:     "spec.in-rules[*].apps in (mongo,redis)", // Slices cant be indexed
			expSuccess: false,
		},
		{
			kind:       "security.Sgpolicy",
			selStr:     "spec.inrules.apps in (mongo,redis)", // Non-existent field
			expSuccess: false,
		},
		{
			kind:       "security.Sgpolicy",
			selStr:     "spec.inrules in (mongo,redis)", // Non-leaf field
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
