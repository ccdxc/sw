package fields

import (
	"bytes"
	"fmt"
	"reflect"
	"regexp"
	"sort"
	"strings"

	"k8s.io/apimachinery/pkg/util/sets"

	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
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

// Adapted from k8s.io/apimachinery/pkg/labels/selector.go &&
//              k8s.io/apimachinery/pkg/fields/selector.go
//
// - Empty selector does not match any Fields
// - Reduced number of operators to 4, Equals, NotEquals, In and NotIn

// ByKey sorts requirements by key to obtain deterministic parser
type ByKey []*Requirement

func (a ByKey) Len() int { return len(a) }

func (a ByKey) Swap(i, j int) { a[i], a[j] = a[j], a[i] }

func (a ByKey) Less(i, j int) bool { return a[i].Key < a[j].Key }

// NewRequirement is the constructor for a Requirement.
// If any of these rules is violated, an error is returned:
// (1) The operator can only be Equals, NotEquals, In, NotIn.
// (2) Values can be one or more depending on the operator. Equals, NotEquals
//     require one Value. In, NotIn require one or more Values.
// (3) The key is invalid due to its length, or sequence
//     of characters. See validateFieldKey for more details.
func NewRequirement(key string, op Operator, vals []string) (*Requirement, error) {
	if err := validateFieldKey(key); err != nil {
		return nil, err
	}
	switch op {
	case Operator_equals, Operator_notEquals:
		if len(vals) != 1 {
			return nil, fmt.Errorf("values must contain one value")
		}
	case Operator_in, Operator_notIn:
		if len(vals) == 0 {
			return nil, fmt.Errorf("values must contain one or more values")
		}
	default:
		return nil, fmt.Errorf("operator '%v' is not recognized", op)
	}

	sort.Strings(vals)
	return &Requirement{Key: key, Operator: Operator_name[int32(op)], Values: vals}, nil
}

func (r *Requirement) hasValue(value string) bool {
	for i := range r.Values {
		if r.Values[i] == value {
			return true
		}
	}
	return false
}

// Matches returns true if the Requirement matches the input Fields.
// There is a match in the following cases:
// (1) The operator is Equals or In, Fields has the Requirement's key and Fields'
//     value for that key is in Requirement's value set.
// (2) The operator is NotEquals or NotIn, Fields has the Requirement's key and
//     Fields' value for that key is not in Requirement's value set.
func (r *Requirement) Matches(fs Fields) bool {
	switch Operator(Operator_value[r.Operator]) {
	case Operator_equals, Operator_in:
		if !fs.Has(r.Key) {
			return false
		}
		return r.hasValue(fs.Get(r.Key))
	case Operator_notEquals, Operator_notIn:
		if !fs.Has(r.Key) {
			return true
		}
		return !r.hasValue(fs.Get(r.Key))
	default:
		return false
	}
}

// MatchesObj returns true if the Requirement matches the input object.
// There is a match in the following cases:
// (1) The operator is Equals or In, Fields has the Requirement's key and obj's
//     value(s) for that key is/are in Requirement's value set.
// (2) The operator is NotEquals or NotIn, Fields has the Requirement's key and
//     obj's value(s) for that key is/are not in Requirement's value set.
func (r *Requirement) MatchesObj(obj runtime.Object) bool {
	vals, err := ref.FieldValues(reflect.ValueOf(obj), r.Key)
	if err != nil {
		return false
	}
	switch Operator(Operator_value[r.Operator]) {
	case Operator_equals, Operator_in:
		for ii := range vals {
			if r.hasValue(vals[ii]) {
				return true
			}
		}
		return false
	case Operator_notEquals, Operator_notIn:
		for ii := range vals {
			if r.hasValue(vals[ii]) {
				return false
			}
		}
		return true
	default:
		return false
	}
}

// Print returns a human-readable string that represents this
// Requirement. If called on an invalid Requirement, an error is
// returned. See NewRequirement for creating a valid Requirement.
func (r *Requirement) Print() string {
	var buffer bytes.Buffer
	buffer.WriteString(r.Key)

	switch Operator(Operator_value[r.Operator]) {
	case Operator_equals:
		buffer.WriteString("=")
	case Operator_notEquals:
		buffer.WriteString("!=")
	case Operator_in:
		buffer.WriteString(" in ")
	case Operator_notIn:
		buffer.WriteString(" notin ")
	}

	if len(r.Values) == 1 {
		buffer.WriteString(r.Values[0])
	} else {
		buffer.WriteString(fmt.Sprintf("(%v)", strings.Join(r.Values, ",")))
	}

	return buffer.String()
}

// Matches for a Selector returns true if all of the Requirements match the
// provided fields. It returns false for an empty selector.
func (s *Selector) Matches(fs Fields) bool {
	if len(s.Requirements) == 0 {
		return false
	}
	for ii := range s.Requirements {
		if matches := s.Requirements[ii].Matches(fs); !matches {
			return false
		}
	}
	return true
}

// Print returns a human-readable string for the Selector.
func (s *Selector) Print() string {
	var reqs []string
	for ii := range s.Requirements {
		reqs = append(reqs, s.Requirements[ii].Print())
	}
	return strings.Join(reqs, ",")
}

// Validate validates the selector.
// Dummy function to help compile .ext.go files that include Selector.
func (s *Selector) Validate(ver, path string, ignoreStatus bool) []error {
	return nil
}

// Token represents constant definition for lexer token
type Token int

const (
	// ErrorToken represents scan error
	ErrorToken Token = iota
	// EndOfStringToken represents end of string
	EndOfStringToken
	// ClosedParToken represents close parenthesis
	ClosedParToken
	// CommaToken represents the comma
	CommaToken
	// EqualsToken represents equal
	EqualsToken
	// IdentifierToken represents identifier, e.g. keys and values
	IdentifierToken
	// InToken represents in
	InToken
	// NotEqualsToken represents not equal
	NotEqualsToken
	// NotInToken represents not in
	NotInToken
	// OpenParToken represents open parenthesis
	OpenParToken
)

// string2token contains the mapping between lexer Token and token literal
// (except IdentifierToken, EndOfStringToken and ErrorToken since it makes no sense)
var string2token = map[string]Token{
	")":     ClosedParToken,
	",":     CommaToken,
	"=":     EqualsToken,
	"in":    InToken,
	"!=":    NotEqualsToken,
	"notin": NotInToken,
	"(":     OpenParToken,
}

// ScannedItem contains the Token and the literal produced by the lexer.
type ScannedItem struct {
	tok     Token
	literal string
}

// isWhitespace returns true if the rune is a space, tab, or newline.
func isWhitespace(ch byte) bool {
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'
}

// isSpecialSymbol detect if the character ch can be an operator
func isSpecialSymbol(ch byte) bool {
	switch ch {
	case '=', '!', '(', ')', ',':
		return true
	}
	return false
}

// isEscapeSymbol detects if the character is an escape symbol
func isEscapeSymbol(ch byte) bool {
	switch ch {
	case '\\':
		return true
	}
	return false
}

// Lexer represents the Lexer struct for field selector.
// It contains necessary informationt to tokenize the input string
type Lexer struct {
	// s stores the string to be tokenized
	s string
	// pos is the position currently tokenized
	pos int
}

// read return the character currently lexed
// increment the position and check the buffer overflow
func (l *Lexer) read() (b byte) {
	b = 0
	if l.pos < len(l.s) {
		b = l.s[l.pos]
		l.pos++
	}
	return b
}

// unread 'undoes' the last read character
func (l *Lexer) unread() {
	l.pos--
}

// scanIDOrKeyword scans string to recognize literal token (for example 'in') or an identifier.
func (l *Lexer) scanIDOrKeyword() (tok Token, lit string) {
	var buffer []byte
IdentifierLoop:
	for {
		switch ch := l.read(); {
		case ch == 0:
			break IdentifierLoop
		case isEscapeSymbol(ch):
			chNxt := l.read()
			switch chNxt {
			case '\\', ',', '=':
			default:
				return ErrorToken, fmt.Sprintf("error expected: '\\\\' or '\\,' or '\\=', found '%v'", chNxt)
			}
			buffer = append(buffer, ch, chNxt)
		case isSpecialSymbol(ch) || isWhitespace(ch):
			l.unread()
			break IdentifierLoop
		default:
			buffer = append(buffer, ch)
		}
	}
	s := string(buffer)
	if val, ok := string2token[s]; ok { // is a literal token?
		return val, s
	}
	return IdentifierToken, s // otherwise is an identifier
}

// scanSpecialSymbol scans string starting with special symbol.
// special symbol identify non literal operators. "!=", "="
func (l *Lexer) scanSpecialSymbol() (Token, string) {
	lastScannedItem := ScannedItem{}
	var buffer []byte
SpecialSymbolLoop:
	for {
		switch ch := l.read(); {
		case ch == 0:
			break SpecialSymbolLoop
		case isSpecialSymbol(ch):
			buffer = append(buffer, ch)
			if token, ok := string2token[string(buffer)]; ok {
				lastScannedItem = ScannedItem{tok: token, literal: string(buffer)}
			} else if lastScannedItem.tok != 0 {
				l.unread()
				break SpecialSymbolLoop
			}
		default:
			l.unread()
			break SpecialSymbolLoop
		}
	}
	if lastScannedItem.tok == 0 {
		return ErrorToken, fmt.Sprintf("error expected: keyword found '%s'", buffer)
	}
	return lastScannedItem.tok, lastScannedItem.literal
}

// skipWhiteSpaces consumes all blank characters
// returning the first non blank character
func (l *Lexer) skipWhiteSpaces(ch byte) byte {
	for {
		if !isWhitespace(ch) {
			return ch
		}
		ch = l.read()
	}
}

// Lex returns a pair of Token and the literal
// literal is meaningfull only for IdentifierToken token
func (l *Lexer) Lex() (tok Token, lit string) {
	switch ch := l.skipWhiteSpaces(l.read()); {
	case ch == 0:
		return EndOfStringToken, ""
	case isSpecialSymbol(ch):
		l.unread()
		return l.scanSpecialSymbol()
	default:
		l.unread()
		return l.scanIDOrKeyword()
	}
}

// Parser data structure contains the field selector parser data structure
type Parser struct {
	l            *Lexer
	scannedItems []ScannedItem
	position     int
}

// ParserContext represents context during parsing:
// some literal for example 'in' and 'notin' can be
// recognized as operator for example 'x in (a)' but
// it can be recognized as value for example 'value in (in)'
type ParserContext int

const (
	// KeyAndOperator represents key and operator
	KeyAndOperator ParserContext = iota
	// Values represents values
	Values
)

// lookahead func returns the current token and string. No increment of current position
func (p *Parser) lookahead(context ParserContext) (Token, string) {
	tok, lit := p.scannedItems[p.position].tok, p.scannedItems[p.position].literal
	if context == Values {
		switch tok {
		case InToken, NotInToken:
			tok = IdentifierToken
		}
	}
	return tok, lit
}

// consume returns current token and string. Increments the position
func (p *Parser) consume(context ParserContext) (Token, string) {
	p.position++
	tok, lit := p.scannedItems[p.position-1].tok, p.scannedItems[p.position-1].literal
	if context == Values {
		switch tok {
		case InToken, NotInToken:
			tok = IdentifierToken
		}
	}
	return tok, lit
}

// scan runs through the input string and stores the ScannedItem in an array
// Parser can now lookahead and consume the tokens
func (p *Parser) scan() {
	for {
		token, literal := p.l.Lex()
		p.scannedItems = append(p.scannedItems, ScannedItem{token, literal})
		if token == EndOfStringToken {
			break
		}
	}
}

// parse runs the left recursive descending algorithm
// on input string. It returns a list of Requirement objects.
func (p *Parser) parse() ([]*Requirement, error) {
	p.scan() // init scannedItems

	requirements := make([]*Requirement, 0)
	for {
		tok, lit := p.lookahead(Values)
		switch tok {
		case IdentifierToken:
			r, err := p.parseRequirement()
			if err != nil {
				return nil, fmt.Errorf("unable to parse requirement: %v", err)
			}
			requirements = append(requirements, r)
			t, l := p.consume(Values)
			switch t {
			case EndOfStringToken:
				return requirements, nil
			case CommaToken:
				t2, l2 := p.lookahead(Values)
				if t2 != IdentifierToken {
					return nil, fmt.Errorf("found '%s', expected: identifier after ','", l2)
				}
			default:
				return nil, fmt.Errorf("found '%s', expected: ',' or 'end of string'", l)
			}
		case EndOfStringToken:
			return requirements, nil
		default:
			return nil, fmt.Errorf("found '%s', expected: !, identifier, or 'end of string'", lit)
		}
	}
}

func (p *Parser) parseRequirement() (*Requirement, error) {
	key, operator, err := p.parseKeyAndInferOperator()
	if err != nil {
		return nil, err
	}
	operator, err = p.parseOperator()
	if err != nil {
		return nil, err
	}
	var values sets.String
	switch operator {
	case Operator_in, Operator_notIn:
		values, err = p.parseValues()
	case Operator_equals, Operator_notEquals:
		values, err = p.parseExactValue()
	}
	if err != nil {
		return nil, err
	}
	return NewRequirement(key, operator, values.List())

}

// parseKeyAndInferOperator parse literals.
func (p *Parser) parseKeyAndInferOperator() (string, Operator, error) {
	var operator Operator
	tok, literal := p.consume(Values)
	if tok != IdentifierToken {
		err := fmt.Errorf("found '%s', expected: identifier", literal)
		return "", -1, err
	}
	if err := validateFieldKey(literal); err != nil {
		return "", -1, err
	}
	return literal, operator, nil
}

// parseOperator return operator and eventually matchType
// matchType can be exact
func (p *Parser) parseOperator() (op Operator, err error) {
	tok, lit := p.consume(KeyAndOperator)
	switch tok {
	// DoesNotExistToken shouldn't be here because it's a unary operator, not a binary operator
	case InToken:
		op = Operator_in
	case EqualsToken:
		op = Operator_equals
	case NotInToken:
		op = Operator_notIn
	case NotEqualsToken:
		op = Operator_notEquals
	default:
		return -1, fmt.Errorf("found '%s', expected: '=', '!=', 'in', notin'", lit)
	}
	return op, nil
}

// parseValues parses the values for set based matching (x,y,z)
func (p *Parser) parseValues() (sets.String, error) {
	tok, lit := p.consume(Values)
	if tok != OpenParToken {
		return nil, fmt.Errorf("found '%s' expected: '('", lit)
	}
	tok, lit = p.lookahead(Values)
	switch tok {
	case IdentifierToken, CommaToken:
		s, err := p.parseIdentifiersList() // handles general cases
		if err != nil {
			return s, err
		}
		if tok, _ = p.consume(Values); tok != ClosedParToken {
			return nil, fmt.Errorf("found '%s', expected: ')'", lit)
		}
		return s, nil
	case ClosedParToken: // handles "()"
		p.consume(Values)
		return sets.NewString(""), nil
	default:
		return nil, fmt.Errorf("found '%s', expected: ',', ')' or identifier", lit)
	}
}

// parseIdentifiersList parses a (possibly empty) list of
// of comma separated (possibly empty) identifiers
func (p *Parser) parseIdentifiersList() (sets.String, error) {
	s := sets.NewString()
	for {
		tok, lit := p.consume(Values)
		switch tok {
		case IdentifierToken:
			s.Insert(lit)
			tok2, lit2 := p.lookahead(Values)
			switch tok2 {
			case CommaToken:
				continue
			case ClosedParToken:
				return s, nil
			default:
				return nil, fmt.Errorf("found '%s', expected: ',' or ')'", lit2)
			}
		case CommaToken: // handled here since we can have "(,"
			if s.Len() == 0 {
				s.Insert("") // to handle (,
			}
			tok2, _ := p.lookahead(Values)
			if tok2 == ClosedParToken {
				s.Insert("") // to handle ,)  Double "" removed by StringSet
				return s, nil
			}
			if tok2 == CommaToken {
				p.consume(Values)
				s.Insert("") // to handle ,, Double "" removed by StringSet
			}
		default: // it can be operator
			return s, fmt.Errorf("found '%s', expected: ',', or identifier", lit)
		}
	}
}

// parseExactValue parses the only value for exact match style
func (p *Parser) parseExactValue() (sets.String, error) {
	s := sets.NewString()
	tok, lit := p.lookahead(Values)
	if tok == EndOfStringToken || tok == CommaToken {
		return nil, fmt.Errorf("expected: identifier")
	}
	tok, lit = p.consume(Values)
	if tok == IdentifierToken {
		s.Insert(lit)
		return s, nil
	}
	return nil, fmt.Errorf("found '%s', expected: identifier", lit)
}

// Parse takes a string representing a selector and returns a selector
// object, or an error. This parsing function differs from ParseSelector
// as they parse different selectors with different syntaxes.
// The input will cause an error if it does not follow this form:
//
//  <selector-syntax>         ::= <requirement> | <requirement> "," <selector-syntax>
//  <requirement>             ::= KEY [ <set-based-restriction> | <exact-match-restriction> ]
//  <set-based-restriction>   ::= <inclusion-exclusion> <value-set>
//  <inclusion-exclusion>     ::= <inclusion> | <exclusion>
//  <exclusion>               ::= "notin"
//  <inclusion>               ::= "in"
//  <value-set>               ::= "(" <values> ")"
//  <values>                  ::= VALUE | VALUE "," <values>
//  <exact-match-restriction> ::= ["="|"!="] VALUE
//
// KEY is a sequence of one or more characters, which may be "." separated. For slices and maps,
// there is support for indexing. Please see validation.go for those requirements.
// VALUE is a sequence of zero or more characters "([A-Za-z0-9_-\.])". Max length is 63 characters.
// Delimiter is white space: (' ', '\t')
// Example of valid syntax:
//  "x.x in (foo,,baz),z notin ()"
//
// Note:
//  (1) Inclusion - " in " - denotes that the KEY exists and is equal to any of the
//      VALUEs in its requirement
//  (2) Exclusion - " notin " - denotes that the KEY is not equal to any
//      of the VALUEs in its requirement or does not exist
//  (3) The empty string is a valid VALUE
//
func Parse(selector string) (*Selector, error) {
	p := &Parser{l: &Lexer{s: selector, pos: 0}}
	requirements, err := p.parse()
	if err != nil {
		return &Selector{}, err
	}
	sort.Sort(ByKey(requirements)) // sort to grant determistic parsing
	return &Selector{
		Requirements: requirements,
	}, nil
}

// SelectorFromSet returns a Selector which will match exactly the given Set.
func SelectorFromSet(fs Set) *Selector {
	if fs == nil || len(fs) == 0 {
		return &Selector{}
	}
	requirements := make([]*Requirement, 0)
	for field, value := range fs {
		r, err := NewRequirement(field, Operator_equals, []string{value})
		if err == nil {
			requirements = append(requirements, r)
		}
	}
	// sort to have deterministic string representation
	sort.Sort(ByKey(requirements))
	return &Selector{
		Requirements: requirements,
	}
}

// SelectorFromValidatedSet returns a Selector which will match exactly the given Set.
// It assumes that Set is already validated and doesn't do any validation.
func SelectorFromValidatedSet(fs Set) *Selector {
	if fs == nil || len(fs) == 0 {
		return &Selector{}
	}
	requirements := make([]*Requirement, 0)
	for field, value := range fs {
		requirements = append(requirements, &Requirement{Key: field, Operator: Operator_name[int32(Operator_equals)], Values: []string{value}})
	}
	// sort to have deterministic string representation
	sort.Sort(ByKey(requirements))
	return &Selector{
		Requirements: requirements,
	}
}

// SelectorParser implements ref.CustomParser for field selector.
type SelectorParser struct {
}

// Print prints a selector
func (s *SelectorParser) Print(v reflect.Value) string {
	if v.Kind() == reflect.Ptr {
		v = reflect.Indirect(v)
	}
	sel, ok := v.Interface().(Selector)
	if !ok {
		return ""
	}
	return (&sel).Print()
}

// Parse parses in to a selector
func (s *SelectorParser) Parse(in string) (reflect.Value, error) {
	sel, err := Parse(in)
	return reflect.ValueOf(*sel), err
}

// TODO: Remove old lexer and parser
var (
	spaceRE           = regexp.MustCompile(`\s*`)
	maxSelectorLength = 1000
)

// parseV2 parses a selector string in to requirements.
func parseV2(sel string) ([]*Requirement, error) {
	// Check selector length.
	if len(sel) > maxSelectorLength {
		return nil, fmt.Errorf("Maximum supported field selector length: %v, found: %v", maxSelectorLength, len(sel))
	}
	// Validate the selector using regex.
	if err := validateSelector(sel); err != nil {
		return nil, err
	}
	// Selector is now a comma separated list of <key op val(s)*> tuples.
	reqs := make([]*Requirement, 0)
	ii := 0
	for ii < len(sel) {
		// Parse the key
		k := keyRE.FindString(sel[ii:])
		// Defensive check
		if len(k) == 0 {
			return nil, fmt.Errorf("Unexpected error parsing key with: %v", sel[ii:])
		}
		ii += len(k)

		// Parse the op
		o := opRE.FindString(sel[ii:])
		// Defensive check
		if len(o) == 0 {
			return nil, fmt.Errorf("Unexpected error parsing operator with: %v", sel[ii:])
		}
		ii += len(o)
		o = spaceRE.ReplaceAllString(o, "")
		var op Operator
		switch o {
		case "=":
			op = Operator_equals
		case "!=":
			op = Operator_notEquals
		case "in":
			op = Operator_in
		case "notin":
			op = Operator_notIn
		default:
			return nil, fmt.Errorf("Unexpected operator: %q, key %v", o, k)
		}

		// Parse the value(s)
		vals := []string{}
		v := valsRE.FindString(sel[ii:])
		if len(v) == 0 {
			// Empty value is ok for "=" and "!="
			if op == Operator_equals || op == Operator_notEquals {
				r, err := NewRequirement(k, op, vals)
				if err != nil {
					return nil, err
				}
				reqs = append(reqs, r)
				continue
			}
			return nil, fmt.Errorf("Unexpected error parsing values with: %v", sel[ii:])
		}
		ii += len(v)
		v = strings.TrimPrefix(v, "(")
		v = strings.TrimSuffix(v, ")")
		jj := 0
		// Extract value(s)
		for jj < len(v) {
			val := valRE.FindString(v[jj:])
			if len(val) == 0 {
				return nil, fmt.Errorf("Unexpected error parsing value with: %v", v[jj:])
			}
			jj += len(val) + 1 // 1 for comma
			vals = append(vals, val)
		}
		r, err := NewRequirement(k, op, vals)
		if err != nil {
			return nil, err
		}
		reqs = append(reqs, r)
		ii++ // go past the "," in case there are more requirements.
	}
	return reqs, nil
}

// ParseV2 takes a string representing a selector and returns a selector
// object, or an error. This parsing function differs from ParseSelector
// as they parse different selectors with different syntaxes.
// The input will cause an error if it does not follow this form:
//
//  <selector-syntax>         ::= <requirement> | <requirement> "," <selector-syntax>
//  <requirement>             ::= KEY [ <set-based-restriction> | <exact-match-restriction> ]
//  <set-based-restriction>   ::= <inclusion-exclusion> <value-set>
//  <inclusion-exclusion>     ::= <inclusion> | <exclusion>
//  <exclusion>               ::= "notin"
//  <inclusion>               ::= "in"
//  <value-set>               ::= "(" <values> ")"
//  <values>                  ::= VALUE | VALUE "," <values>
//  <exact-match-restriction> ::= ["="|"!="] VALUE
//
// KEY is a sequence of one or more characters, which may be "." separated. For slices and maps,
// there is support for indexing. Please see validation.go for those requirements.
// VALUE is a sequence of zero or more characters (see validation.go).
// Delimiter is white space: (' ', '\t')
// Example(s) of valid syntax:
//  "x.x in (foo,baz),z notin ()"
//  "x.y[z]=foo"
//
// Note:
//  (1) Inclusion - " in " - denotes that the KEY exists and is equal to any of the
//      VALUEs in its requirement
//  (2) Exclusion - " notin " - denotes that the KEY is not equal to any
//      of the VALUEs in its requirement or does not exist
//  (3) The empty string is a valid VALUE
//
func ParseV2(selector string) (*Selector, error) {
	requirements, err := parseV2(selector)
	if err != nil {
		return &Selector{}, err
	}
	sort.Sort(ByKey(requirements)) // sort to grant determistic parsing
	return &Selector{
		Requirements: requirements,
	}, nil
}

// ParseForStruct parses a selector for the provided Struct (reflect.Value).
// In addition to Parse, it does two additional things:
// 1) Check that the key is a valid field of the struct
// 2) Coverts json value of the key field into name of the field
//
// ref.FieldJSONByTag validates that key indexing can only happen on maps or
// slices. maps support two forms of indexing - "*" for any key OR a specific
// key. Slices only support "*".
//
// Returns an error on failure.
//
// Valid examples for keys:
//   spec.vlan                               => Spec.Vlan
//   spec.networks[*].vlan                   => Spec.Networks[*].Vlan
//   spec.networks[*].ipaddresses[*].gateway => Spec.Networks[*].IpAddresses[*].Gateway
//   spec.networkMap[abc].vlan               => Spec.NetworkMap[abc].Vlan
//
func ParseForStruct(v reflect.Value, selector string) (*Selector, error) {
	sel, err := ParseV2(selector)
	if err != nil {
		return nil, err
	}
	for ii := range sel.Requirements {
		found, err := ref.FieldByJSONTag(v, sel.Requirements[ii].Key)
		if err != nil {
			return nil, err
		}
		sel.Requirements[ii].Key = found
	}
	return sel, err
}

// MatchesObj for a Selector returns true if all of the Requirements match the
// provided object. It returns false for an empty selector.
func (s *Selector) MatchesObj(obj runtime.Object) bool {
	if len(s.Requirements) == 0 {
		return false
	}
	for ii := range s.Requirements {
		if matches := s.Requirements[ii].MatchesObj(obj); !matches {
			return false
		}
	}
	return true
}
