package fields

import (
	"reflect"
	"strings"
	"testing"

	"k8s.io/apimachinery/pkg/util/sets"
)

/*
Copyright 2014 The Kubernetes Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Adapted from k8s.io/apimachinery/pkg/labels/selector_test.go &&
//              k8s.io/apimachinery/pkg/fields/selector_test.go

func TestSelectorParse(t *testing.T) {
	testGoodStrings := []string{
		"",
		"x.a=a,y.b=b,z.c=c",
		"x.c!=a,y.c=b",
	}
	testGoodSetStrings := []string{
		"x.y in (a,b,c)",
		"x.x in (a,b),y.y in (c,d)",
		"x.x notin (a,b,c)",
		"x.x in (a,b),y.y notin (a,b)",
	}
	testBadStrings := []string{
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
		"x. in (a,b)",
		"x..x in (a,b)",
		".x. in (a,b)",
		"x in in",
		"x in .",
		". in (a)",
	}
	for _, test := range testGoodStrings {
		lq, err := Parse(test)
		if err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
		if strings.Replace(test, " ", "", -1) != lq.Print() {
			t.Errorf("%v restring gave: %v\n", test, lq.Print())
		}
	}
	for _, test := range testGoodSetStrings {
		lq, err := Parse(test)
		if err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
		if test != lq.Print() {
			t.Errorf("%v restring gave: %v\n", test, lq.Print())
		}
	}
	for _, test := range testBadStrings {
		_, err := Parse(test)
		if err == nil {
			t.Errorf("%v: did not get expected error\n", test)
		}
	}
}

func TestSelectorParseWithEscape(t *testing.T) {
	testGoodStrings := []string{
		"",
		"x.x=a\\=1,y.y=b\\\\2,z.z=c\\,3",
		"x.x!=a\\=1,y.y=b\\\\2",
	}
	testGoodSetStrings := []string{
		"x.x in (a\\=1,b\\\\2,c\\,3)",
		"x.x in (a\\=1,b\\\\2),y.y in (c\\,3,d\\=4)",
		"x.x notin (a\\=1,b\\\\2,c\\,3)",
		"x.x in (a\\=1,b\\\\2),y.y notin (a\\=1,b\\\\2)",
	}
	testBadStrings := []string{
		"x.x=a\\a||y.y=b",
		"x.x=a\\b",
		"x.x=(a\\\\1,b",
		"x.x=\\",
		"x.x in \\",
		"x.x in (=\\)",
	}
	for _, test := range testGoodStrings {
		lq, err := Parse(test)
		if err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
		if strings.Replace(test, " ", "", -1) != lq.Print() {
			t.Errorf("%v restring gave: %v\n", test, lq.Print())
		}
	}
	for _, test := range testGoodSetStrings {
		lq, err := Parse(test)
		if err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
		if test != lq.Print() {
			t.Errorf("%v restring gave: %v\n", test, lq.Print())
		}
	}
	for _, test := range testBadStrings {
		_, err := Parse(test)
		if err == nil {
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

func expectMatch(t *testing.T, selector string, fs Set) {
	lq, err := Parse(selector)
	if err != nil {
		t.Errorf("Unable to parse %v as a selector\n", selector)
		return
	}
	if !lq.Matches(fs) {
		t.Errorf("Wanted %s to match '%s', but it did not.\n", selector, fs)
	}
}

func expectNoMatch(t *testing.T, selector string, fs Set) {
	lq, err := Parse(selector)
	if err != nil {
		t.Errorf("Unable to parse %v as a selector\n", selector)
		return
	}
	if lq.Matches(fs) {
		t.Errorf("Wanted '%s' to not match '%s', but it did.", selector, fs)
	}
}

func TestSelectorMatches(t *testing.T) {
	expectNoMatch(t, "", Set{"x": "y"})
	expectMatch(t, "x.x=y", Set{"x.x": "y"})
	expectMatch(t, "x.x=y,z.z=w", Set{"x.x": "y", "z.z": "w"})
	expectMatch(t, "x.x!=y,z.z!=w", Set{"x.x": "z", "z.z": "a"})
	expectMatch(t, "notin.notin=in", Set{"notin.notin": "in"}) // in and notin in exactMatch
	expectNoMatch(t, "x.x=z", Set{})
	expectNoMatch(t, "x.x=y", Set{"x.x": "z"})
	expectNoMatch(t, "x.x=y,z.z=w", Set{"x.x": "w", "z.z": "w"})
	expectNoMatch(t, "x.x!=y,z.z!=w", Set{"x.x": "z", "z.z": "w"})
	expectMatch(t, "x.x in (z)", Set{"x.x": "z"})
	expectMatch(t, "x.x in (a,b,z)", Set{"x.x": "z"})
	expectNoMatch(t, "x.x in (z)", Set{"x.x": "d"})
	expectNoMatch(t, "x.x in (a,b,z)", Set{"x.x": "d"})

	fieldset := Set{
		"foo.foo": "bar",
		"baz.baz": "blah",
	}
	expectMatch(t, "foo.foo=bar", fieldset)
	expectMatch(t, "baz.baz=blah", fieldset)
	expectMatch(t, "foo.foo=bar,baz.baz=blah", fieldset)
	expectNoMatch(t, "foo.foo=blah", fieldset)
	expectNoMatch(t, "baz.baz=bar", fieldset)
	expectNoMatch(t, "foo.foo=bar,foobar.foobar=bar,baz.baz=blah", fieldset)
}

func expectMatchDirect(t *testing.T, selector, ls Set) {
	if !SelectorFromSet(selector).Matches(ls) {
		t.Errorf("Wanted %s to match '%s', but it did not.\n", selector, ls)
	}
}

func expectNoMatchDirect(t *testing.T, selector, ls Set) {
	if SelectorFromSet(selector).Matches(ls) {
		t.Errorf("Wanted '%s' to not match '%s', but it did.", selector, ls)
	}
}

func TestSetMatches(t *testing.T) {
	fieldset := Set{
		"foo.foo": "bar",
		"baz.baz": "blah",
	}
	expectNoMatchDirect(t, Set{}, fieldset)
	expectNoMatchDirect(t, Set{"foo.foo": "blah"}, fieldset)
	expectNoMatchDirect(t, Set{"baz.baz": "baz"}, fieldset)
	expectMatchDirect(t, Set{"foo.foo": "bar"}, fieldset)
	expectMatchDirect(t, Set{"baz.baz": "blah"}, fieldset)
	expectMatchDirect(t, Set{"foo.foo": "bar", "baz.baz": "blah"}, fieldset)
}

func TestLexer(t *testing.T) {
	testcases := []struct {
		s string
		t Token
	}{
		{"", EndOfStringToken},
		{",", CommaToken},
		{"notin", NotInToken},
		{"in", InToken},
		{"=", EqualsToken},
		{"!=", NotEqualsToken},
		{"(", OpenParToken},
		{")", ClosedParToken},
		//Non-"special" characters are considered part of an identifier
		{"~", IdentifierToken},
		{"||", IdentifierToken},
	}
	for _, v := range testcases {
		l := &Lexer{s: v.s, pos: 0}
		token, lit := l.Lex()
		if token != v.t {
			t.Errorf("Got %d it should be %d for '%s'", token, v.t, v.s)
		}
		if v.t != ErrorToken && lit != v.s {
			t.Errorf("Got '%s' it should be '%s'", lit, v.s)
		}
	}
}

func min(l, r int) (m int) {
	m = r
	if l < r {
		m = l
	}
	return m
}

func TestLexerSequence(t *testing.T) {
	testcases := []struct {
		s string
		t []Token
	}{
		{"key in ( value )", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, ClosedParToken}},
		{"key notin ( value )", []Token{IdentifierToken, NotInToken, OpenParToken, IdentifierToken, ClosedParToken}},
		{"key in ( value1, value2 )", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, CommaToken, IdentifierToken, ClosedParToken}},
		{"key", []Token{IdentifierToken}},
		{"()", []Token{OpenParToken, ClosedParToken}},
		{"x in (),y", []Token{IdentifierToken, InToken, OpenParToken, ClosedParToken, CommaToken, IdentifierToken}},
		{"!= (), = notin", []Token{NotEqualsToken, OpenParToken, ClosedParToken, CommaToken, EqualsToken, NotInToken}},
	}
	for _, v := range testcases {
		var literals []string
		var tokens []Token
		l := &Lexer{s: v.s, pos: 0}
		for {
			token, lit := l.Lex()
			if token == EndOfStringToken {
				break
			}
			tokens = append(tokens, token)
			literals = append(literals, lit)
		}
		if len(tokens) != len(v.t) {
			t.Errorf("Bad number of tokens for '%s %d, %d", v.s, len(tokens), len(v.t))
		}
		for i := 0; i < min(len(tokens), len(v.t)); i++ {
			if tokens[i] != v.t[i] {
				t.Errorf("Test '%s': Mismatching in token type found '%v' it should be '%v'", v.s, tokens[i], v.t[i])
			}
		}
	}
}

func TestParserLookahead(t *testing.T) {
	testcases := []struct {
		s string
		t []Token
	}{
		{"key in ( value )", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key notin ( value )", []Token{IdentifierToken, NotInToken, OpenParToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key in ( value1, value2 )", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, CommaToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key", []Token{IdentifierToken, EndOfStringToken}},
		{"()", []Token{OpenParToken, ClosedParToken, EndOfStringToken}},
		{"", []Token{EndOfStringToken}},
		{"x in (),y", []Token{IdentifierToken, InToken, OpenParToken, ClosedParToken, CommaToken, IdentifierToken, EndOfStringToken}},
		{"!= (), = notin", []Token{NotEqualsToken, OpenParToken, ClosedParToken, CommaToken, EqualsToken, NotInToken, EndOfStringToken}},
	}
	for _, v := range testcases {
		p := &Parser{l: &Lexer{s: v.s, pos: 0}, position: 0}
		p.scan()
		if len(p.scannedItems) != len(v.t) {
			t.Errorf("Expected %d items found %d", len(v.t), len(p.scannedItems))
		}
		for {
			token, lit := p.lookahead(KeyAndOperator)

			token2, lit2 := p.consume(KeyAndOperator)
			if token == EndOfStringToken {
				break
			}
			if token != token2 || lit != lit2 {
				t.Errorf("Bad values")
			}
		}
	}
}

func TestParserEscape(t *testing.T) {
	testcases := []struct {
		s string
		t []Token
	}{
		{"key=x\\=y", []Token{IdentifierToken, EqualsToken, IdentifierToken, EndOfStringToken}},
		{"key=x\\,y", []Token{IdentifierToken, EqualsToken, IdentifierToken, EndOfStringToken}},
		{"key=x\\\\y", []Token{IdentifierToken, EqualsToken, IdentifierToken, EndOfStringToken}},
		{"key=x\\ay", []Token{IdentifierToken, EqualsToken, ErrorToken, IdentifierToken, EndOfStringToken}},
		{"key!=x\\=y", []Token{IdentifierToken, NotEqualsToken, IdentifierToken, EndOfStringToken}},
		{"key!=x\\,y", []Token{IdentifierToken, NotEqualsToken, IdentifierToken, EndOfStringToken}},
		{"key!=x\\\\y", []Token{IdentifierToken, NotEqualsToken, IdentifierToken, EndOfStringToken}},
		{"key!=x\\ay", []Token{IdentifierToken, NotEqualsToken, ErrorToken, IdentifierToken, EndOfStringToken}},
		{"key in (x\\=y)", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key in (x\\,y,a\\=b)", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, CommaToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key in (x\\\\y,a\\\\b)", []Token{IdentifierToken, InToken, OpenParToken, IdentifierToken, CommaToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key in (x\\ay)", []Token{IdentifierToken, InToken, OpenParToken, ErrorToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key notin (x\\=y,a\\,b)", []Token{IdentifierToken, NotInToken, OpenParToken, IdentifierToken, CommaToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key notin (x\\,y)", []Token{IdentifierToken, NotInToken, OpenParToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key notin (x\\\\y)", []Token{IdentifierToken, NotInToken, OpenParToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
		{"key notin (x\\ay)", []Token{IdentifierToken, NotInToken, OpenParToken, ErrorToken, IdentifierToken, ClosedParToken, EndOfStringToken}},
	}
	for _, v := range testcases {
		p := &Parser{l: &Lexer{s: v.s, pos: 0}, position: 0}
		p.scan()
		if len(p.scannedItems) != len(v.t) {
			t.Errorf("Expected %d items found %d", len(v.t), len(p.scannedItems))
		}
		for {
			token, lit := p.lookahead(KeyAndOperator)

			token2, lit2 := p.consume(KeyAndOperator)
			if token == EndOfStringToken {
				break
			}
			if token != token2 || lit != lit2 {
				t.Errorf("Bad values")
			}
		}
	}
}

func TestRequirementConstructor(t *testing.T) {
	requirementConstructorTests := []struct {
		Key     string
		Op      Operator
		Vals    sets.String
		Success bool
	}{
		{"x.x", Operator_equals, nil, false},
		{"x.x", Operator_notEquals, sets.NewString(), false},
		{"x.x", Operator_equals, sets.NewString("foo"), true},
		{"x.x", Operator_in, sets.NewString("foo", "bar"), true},
		{"x.x", Operator_notEquals, sets.NewString("foo"), true},
		{"x.x", Operator_notIn, sets.NewString("foo", "bar"), true},
		{"1foo.1foo", Operator_equals, sets.NewString("bar"), false},
		{"1234.1234", Operator_equals, sets.NewString("bar"), false},
	}
	for _, rc := range requirementConstructorTests {
		if _, err := NewRequirement(rc.Key, rc.Op, rc.Vals.List()); err == nil && !rc.Success {
			t.Errorf("expected error with key:%#v op:%v vals:%v, got no error", rc.Key, rc.Op, rc.Vals)
		} else if err != nil && rc.Success {
			t.Errorf("expected no error with key:%#v op:%v vals:%v, got:%v", rc.Key, rc.Op, rc.Vals, err)
		}
	}
}

func TestSelectorParseV2(t *testing.T) {
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
		if _, err := ParseV2(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testGoodSetStrings {
		if _, err := ParseV2(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testBadStrings {
		if _, err := ParseV2(test); err == nil {
			t.Errorf("%v: did not get expected error\n", test)
		}
	}
}

func TestSelectorParseWithEscapeV2(t *testing.T) {
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
		if _, err := ParseV2(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testGoodSetStrings {
		if _, err := ParseV2(test); err != nil {
			t.Errorf("%v: error %v (%#v)\n", test, err, err)
		}
	}
	for _, test := range testBadStrings {
		if _, err := ParseV2(test); err == nil {
			t.Errorf("%v: did not get expected error\n", test)
		}
	}
}

func TestDeterministicParseV2(t *testing.T) {
	s1, err := ParseV2("x.x=a,a.a=x")
	s2, err2 := ParseV2("a.a=x,x.x=a")
	if err != nil || err2 != nil {
		t.Errorf("Unexpected parse error")
	}
	if s1.String() != s2.String() {
		t.Errorf("Non-deterministic parse")
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
	Spec UserSpec `json:"spec"`
}

func TestParseForStruct(t *testing.T) {
	u := User{
		Spec: UserSpec{
			Groups: []*UserGroup{
				&UserGroup{},
			},
			Perms: map[string]RolePerms{
				"test": RolePerms{
					Perms: []string{"test"},
				},
			},
			PermsIdx: map[int8]RolePerms{
				0: RolePerms{
					Perms: []string{"test"},
				},
			},
		},
	}
	v := reflect.ValueOf(u)
	goodTests := []struct {
		selString string
		selector  Selector
	}{
		{
			selString: "spec.name=foo",
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Name",
						Operator: "equals",
						Values:   []string{"foo"},
					},
				},
			},
		},
		{
			selString: "spec.groups.group=foo,spec.perms[*].perms=bar",
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
						Values:   []string{"bar"},
					},
				},
			},
		},
		{
			selString: "spec.groups.group=foo,spec.permsIdx[0].perms=bar",
			selector: Selector{
				Requirements: []*Requirement{
					&Requirement{
						Key:      "Spec.Groups.Group",
						Operator: "equals",
						Values:   []string{"foo"},
					},
					&Requirement{
						Key:      "Spec.PermsIdx[0].Perms",
						Operator: "equals",
						Values:   []string{"bar"},
					},
				},
			},
		},
	}
	for ii := range goodTests {
		sel, err := ParseForStruct(v, goodTests[ii].selString)
		if err != nil {
			t.Fatalf("Failed to parse %v with error: %v", goodTests[ii].selString, err)
		}
		if !reflect.DeepEqual(*sel, goodTests[ii].selector) {
			t.Fatalf("Expected %+v, got %+v", goodTests[ii].selector, sel)
		}
	}

	badTests := []string{
		"spec.nonexistent=foo",           // Non existent field
		"spec.groups=foo",                // Non leaf
		"spec.groups[*]=foo",             // Non leaf
		"spec.groups[*].group=foo",       // Cant index slice
		"spec.groups[*].nonexistent=foo", // Non existent field
		"spec.perms[*]=foo",              // Non leaf
		"spec.perms[*].perms[*]=foo",     // Cant index slice
		"spec.permsIdx[256].perms=foo",   // Not uint8
	}
	for ii := range badTests {
		sel, err := ParseForStruct(v, badTests[ii])
		if err == nil {
			t.Fatalf("Expected to fail parsing, but found %v", sel)
		}
	}
}
