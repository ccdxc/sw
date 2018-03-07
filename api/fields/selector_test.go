package fields

import (
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
		"x=a,y=b,z=c",
		"x!=a,y=b",
	}
	testGoodSetStrings := []string{
		"x in (a,b,c)",
		"x in (a,b),y in (c,d)",
		"x notin (a,b,c)",
		"x in (a,b),y notin (a,b)",
	}
	testBadStrings := []string{
		"x=a||y=b",
		"x==a==b",
		"!x=a",
		"x<a",
		"!x",
		"x>1",
		"x>1,z<5",
		"x=",
		"x= ",
		"x=,z= ",
		"x= ,z= ",
		"x in a",
		"x in (a",
		"x in (a,b",
		"x=(a,b)",
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
		"x=a\\=1,y=b\\\\2,z=c\\,3",
		"x!=a\\=1,y=b\\\\2",
	}
	testGoodSetStrings := []string{
		"x in (a\\=1,b\\\\2,c\\,3)",
		"x in (a\\=1,b\\\\2),y in (c\\,3,d\\=4)",
		"x notin (a\\=1,b\\\\2,c\\,3)",
		"x in (a\\=1,b\\\\2),y notin (a\\=1,b\\\\2)",
	}
	testBadStrings := []string{
		"x=a\\a||y=b",
		"x=a\\b",
		"x=(a\\\\1,b",
		"x=\\",
		"x in \\",
		"x in (=\\)",
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
	s1, err := Parse("x=a,a=x")
	s2, err2 := Parse("a=x,x=a")
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
	expectMatch(t, "x=y", Set{"x": "y"})
	expectMatch(t, "x=y,z=w", Set{"x": "y", "z": "w"})
	expectMatch(t, "x!=y,z!=w", Set{"x": "z", "z": "a"})
	expectMatch(t, "notin=in", Set{"notin": "in"}) // in and notin in exactMatch
	expectNoMatch(t, "x=z", Set{})
	expectNoMatch(t, "x=y", Set{"x": "z"})
	expectNoMatch(t, "x=y,z=w", Set{"x": "w", "z": "w"})
	expectNoMatch(t, "x!=y,z!=w", Set{"x": "z", "z": "w"})
	expectMatch(t, "x in (z)", Set{"x": "z"})
	expectMatch(t, "x in (a,b,z)", Set{"x": "z"})
	expectNoMatch(t, "x in (z)", Set{"x": "d"})
	expectNoMatch(t, "x in (a,b,z)", Set{"x": "d"})

	fieldset := Set{
		"foo": "bar",
		"baz": "blah",
	}
	expectMatch(t, "foo=bar", fieldset)
	expectMatch(t, "baz=blah", fieldset)
	expectMatch(t, "foo=bar,baz=blah", fieldset)
	expectNoMatch(t, "foo=blah", fieldset)
	expectNoMatch(t, "baz=bar", fieldset)
	expectNoMatch(t, "foo=bar,foobar=bar,baz=blah", fieldset)
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
		"foo": "bar",
		"baz": "blah",
	}
	expectNoMatchDirect(t, Set{}, fieldset)
	expectNoMatchDirect(t, Set{"foo": "blah"}, fieldset)
	expectNoMatchDirect(t, Set{"baz": "baz"}, fieldset)
	expectMatchDirect(t, Set{"foo": "bar"}, fieldset)
	expectMatchDirect(t, Set{"baz": "blah"}, fieldset)
	expectMatchDirect(t, Set{"foo": "bar", "baz": "blah"}, fieldset)
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
		{"x", Operator_equals, nil, false},
		{"x", Operator_notEquals, sets.NewString(), false},
		{"x", Operator_equals, sets.NewString("foo"), true},
		{"x", Operator_in, sets.NewString("foo", "bar"), true},
		{"x", Operator_notEquals, sets.NewString("foo"), true},
		{"x", Operator_notIn, sets.NewString("foo", "bar"), true},
		{"1foo", Operator_equals, sets.NewString("bar"), true},
		{"1234", Operator_equals, sets.NewString("bar"), true},
	}
	for _, rc := range requirementConstructorTests {
		if _, err := NewRequirement(rc.Key, rc.Op, rc.Vals.List()); err == nil && !rc.Success {
			t.Errorf("expected error with key:%#v op:%v vals:%v, got no error", rc.Key, rc.Op, rc.Vals)
		} else if err != nil && rc.Success {
			t.Errorf("expected no error with key:%#v op:%v vals:%v, got:%v", rc.Key, rc.Op, rc.Vals, err)
		}
	}
}
