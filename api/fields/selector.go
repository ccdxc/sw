package fields

import (
	"bytes"
	"fmt"
	"reflect"
	"regexp"
	"sort"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
	ustrconv "github.com/pensando/sw/venice/utils/strconv"
)

type matchObject interface {
	GetObjectKind() string // returns the object kind
}

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
//     of characters. See ValidateFieldKey for more details.
func NewRequirement(key string, op Operator, vals []string) (*Requirement, error) {
	if err := ValidateFieldKey(key); err != nil {
		return nil, err
	}
	switch op {
	case Operator_equals, Operator_notEquals, Operator_lt, Operator_gt, Operator_lte, Operator_gte:
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

// MatchesObj returns true if the Requirement matches the input object.
// There is a match in the following cases:
// (1) The operator is Equals or In, Fields has the Requirement's key and obj's
//     value(s) for that key is/are in Requirement's value set.
// (2) The operator is NotEquals or NotIn, Fields has the Requirement's key and
//     obj's value(s) for that key is/are not in Requirement's value set.
// (3) The operator is Lt (supported on non-string fields), Fields has the Requirements's key and
//		obj's value for that key is less than Requirement's value
// (4) The operator is Gt (supported on non-string fields), Fields has the Requirements's key and
//		obj's value for that key is greater than Requirement's value
// (5) The operator is Lte (supported on non-string fields), Fields has the Requirements's key and
//		obj's value for that key is less than or equal to Requirement's value
// (6) The operator is Gte (supported on non-string fields), Fields has the Requirements's key and
//		obj's value for that key is greater than or equal to Requirement's value
func (r *Requirement) MatchesObj(obj matchObject) bool {
	vals, err := ref.FieldValues(reflect.ValueOf(obj), r.Key)
	if err != nil || len(vals) == 0 {
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
	case Operator_lt, Operator_lte, Operator_gt, Operator_gte:
		schemaType := runtime.GetDefaultScheme().Kind2SchemaType(obj.GetObjectKind())
		keyType, err := ref.GetScalarFieldType(schemaType, r.Key)
		if err != nil || keyType == "TYPE_STRING" || keyType == "TYPE_BOOL" {
			return false
		}
		return r.hasRelation(keyType, vals[0])
	default:
		return false
	}
}

// MatchesObjWithObservedValue behaves the same as MatchesObj. Along with bool response,
// it also returns the observed value for this requirement.
func (r *Requirement) MatchesObjWithObservedValue(obj runtime.Object) (bool, string) {
	vals, err := ref.FieldValues(reflect.ValueOf(obj), r.Key)
	if err != nil || len(vals) == 0 {
		return false, ""
	}

	switch Operator(Operator_value[r.Operator]) {
	case Operator_equals:
		if r.hasValue(vals[0]) {
			return true, vals[0]
		}
		return false, ""

	case Operator_notEquals:
		if !r.hasValue(vals[0]) {
			return true, vals[0]
		}
		return false, ""

	case Operator_in:
		for ii := range vals {
			if r.hasValue(vals[ii]) {
				return true, vals[ii]
			}
		}
		return false, ""

	case Operator_notIn:
		for ii := range vals {
			if r.hasValue(vals[ii]) {
				return false, ""
			}
		}
		return true, ""

	case Operator_lt, Operator_lte, Operator_gt, Operator_gte:
		schemaType := runtime.GetDefaultScheme().Kind2SchemaType(obj.GetObjectKind())
		keyType, err := ref.GetScalarFieldType(schemaType, r.Key)
		if err != nil || keyType == "TYPE_STRING" || keyType == "TYPE_BOOL" {
			return false, ""
		}
		if r.hasRelation(keyType, vals[0]) {
			return true, vals[0]
		}
		return false, ""

	default:
		return false, ""
	}
}

// helper function to handle relational operators
func (r *Requirement) hasRelation(keyType, value string) bool {
	if len(r.Values) != 1 {
		return false
	}

	var fieldValue interface{} // field value
	var reqValue interface{}   // requirement value
	var fieldValueErr error
	var reqValueErr error
	var typeTimestamp = "api.Timestamp"

	if keyType == typeTimestamp {
		fieldValue, fieldValueErr = ustrconv.ParseTime(value)
		reqValue, reqValueErr = ustrconv.ParseTime(r.Values[0])
	} else {
		fieldValue, fieldValueErr = ustrconv.ParseFloat64(value)
		reqValue, reqValueErr = ustrconv.ParseFloat64(r.Values[0])
	}
	if fieldValueErr != nil || reqValueErr != nil { // if any of the conversion failed
		return false
	}

	switch Operator(Operator_value[r.Operator]) {
	case Operator_lt:
		if keyType == typeTimestamp {
			return utils.CompareTime(fieldValue.(time.Time), reqValue.(time.Time)) < 0
		}
		return utils.CompareFloat(fieldValue.(float64), reqValue.(float64)) < 0
	case Operator_lte:
		if keyType == typeTimestamp {
			return utils.CompareTime(fieldValue.(time.Time), reqValue.(time.Time)) <= 0
		}
		return utils.CompareFloat(fieldValue.(float64), reqValue.(float64)) <= 0
	case Operator_gt:
		if keyType == typeTimestamp {
			return utils.CompareTime(fieldValue.(time.Time), reqValue.(time.Time)) > 0
		}
		return utils.CompareFloat(fieldValue.(float64), reqValue.(float64)) > 0
	case Operator_gte:
		if keyType == typeTimestamp {
			return utils.CompareTime(fieldValue.(time.Time), reqValue.(time.Time)) >= 0
		}
		return utils.CompareFloat(fieldValue.(float64), reqValue.(float64)) >= 0
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

// PrintSQL returns a SQL style human-readable string that represents this
// Requirement. If called on an invalid Requirement, an error is
// returned. See NewRequirement for creating a valid Requirement.
func (r *Requirement) PrintSQL() (string, error) {

	if len(r.Values) == 0 {
		return "", fmt.Errorf("Values cannot be empty")
	}
	if len(r.Values) > 1 {
		switch Operator(Operator_value[r.Operator]) {
		case Operator_in, Operator_notIn:
		default:
			return "", fmt.Errorf("Values must be of length 1 for operator %v", Operator(Operator_value[r.Operator]))
		}
	}

	var operator string
	// Will be OR for in, and AND for notIn
	var joinOperator string
	// whether or not the value should be surrounded in quotes
	var useQuotes bool

	switch Operator(Operator_value[r.Operator]) {
	case Operator_equals, Operator_in:
		operator = "="
		joinOperator = "OR"
		useQuotes = true
	case Operator_notEquals, Operator_notIn:
		operator = "!="
		joinOperator = "AND"
		useQuotes = true
	case Operator_lt:
		operator = "<"
		useQuotes = false
	case Operator_lte:
		operator = "<="
		useQuotes = false
	case Operator_gt:
		operator = ">"
		useQuotes = false
	case Operator_gte:
		operator = ">="
		useQuotes = false
	default:
		return "", fmt.Errorf("%v not supported", Operator(Operator_value[r.Operator]))
	}

	var reqStrings []string
	for _, v := range r.Values {
		if useQuotes {
			v = fmt.Sprintf("'%s'", v)
		}
		req := fmt.Sprintf("\"%s\" %s %s", r.Key, operator, v)
		reqStrings = append(reqStrings, req)
	}

	if len(reqStrings) == 1 {
		return reqStrings[0], nil
	}

	return fmt.Sprintf("( %s )", strings.Join(reqStrings, fmt.Sprintf(" %s ", joinOperator))), nil
}

// Validate validates the requirement.
// Dummy function to help compile .ext.go files that include Requirement.
func (r *Requirement) Validate(ver, path string, ignoreStatus, ignoreSpec bool) []error {
	var ret []error
	return ret
}

// Normalize normalizes the object if needed
func (r *Requirement) Normalize() {}

// Print returns a human-readable string for the Selector.
func (s *Selector) Print() string {
	var reqs []string
	for ii := range s.Requirements {
		reqs = append(reqs, s.Requirements[ii].Print())
	}
	return strings.Join(reqs, ",")
}

// PrintSQL returns a sql style human-readable string for the Selector.
func (s *Selector) PrintSQL() (string, error) {
	var reqs []string
	for ii := range s.Requirements {
		reqStr, err := s.Requirements[ii].PrintSQL()
		if err != nil {
			return "", err
		}
		reqs = append(reqs, reqStr)
	}
	return strings.Join(reqs, " AND "), nil
}

// Validate validates the selector.
// Dummy function to help compile .ext.go files that include Selector.
func (s *Selector) Validate(ver, path string, ignoreStatus, ignoreSpec bool) []error {
	return nil
}

// Normalize normalizes the object if needed
func (s *Selector) Normalize() {}

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

var (
	spaceRE           = regexp.MustCompile(`\s*`)
	maxSelectorLength = 1000
)

// parse parses a selector string in to requirements.
func parse(sel string) ([]*Requirement, error) {
	// Check selector length.
	if len(sel) > maxSelectorLength {
		return nil, fmt.Errorf("Maximum supported field selector length: %v, found: %v", maxSelectorLength, len(sel))
	}
	// Validate the selector using regex.
	if err := ValidateSelector(sel); err != nil {
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
		case "<":
			op = Operator_lt
		case "<=":
			op = Operator_lte
		case ">":
			op = Operator_gt
		case ">=":
			op = Operator_gte
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

// Parse takes a string representing a selector and returns a selector
// object, or an error. selector syntax is defined in validation.go.
func Parse(selector string) (*Selector, error) {
	requirements, err := parse(selector)
	if err != nil {
		return &Selector{}, err
	}
	sort.Sort(ByKey(requirements)) // sort to grant determistic parsing
	return &Selector{
		Requirements: requirements,
	}, nil
}

// ParseWithValidation uses the provided schema type (ex: cluster.Cluster) to look
// up the default schema to:
// 1) Validate the selector string
//    a) Field should be present
//    b) Only maps can be indexed
//    c) Map's index string matches the map's key type
// 2) Converts json values to field names
//
// Returns an error on failure.
//
// Valid examples for keys:
//   spec.vlan                               => Spec.Vlan
//   spec.networks[*].vlan                   => Spec.Networks[*].Vlan
//   spec.networks[*].ipaddresses[*].gateway => Spec.Networks[*].IpAddresses[*].Gateway
//   spec.networkMap[abc].vlan               => Spec.NetworkMap[abc].Vlan
//
func ParseWithValidation(schemaType string, selector string) (*Selector, error) {
	sel, err := Parse(selector)
	if err != nil {
		return nil, err
	}

	if err := sel.ValidateRequirements(schemaType, false); err != nil {
		return nil, err
	}

	return sel, nil
}

// ValidateRequirements validates each requirement to ensure that the
// given values are parseable to key's actual type for relational operators.
// e.g.
// 	key    : Spec.HealthCheck.Interval (TYPE_INT32)
// 	op     : Operator_lt, Operator_gt, Operator_lte, Operator_gte
// 	values : should be of type INT32
//
func (s *Selector) ValidateRequirements(schemaType string, ignoreNonExistentFields bool) error {
	for ii := range s.Requirements {
		if _, found := Operator_value[s.Requirements[ii].GetOperator()]; !found {
			return fmt.Errorf("operator %v not supported", s.Requirements[ii].GetOperator())
		}

		result, err := ref.FieldByJSONTag(schemaType, s.Requirements[ii].Key)
		if err != nil {
			if ignoreNonExistentFields && strings.Contains(err.Error(), "Did not find field") {
				continue
			}
			return err
		}

		fieldType, err := ref.GetScalarFieldType(schemaType, result)
		if err != nil {
			return err
		}

		switch Operator(Operator_value[s.Requirements[ii].GetOperator()]) {
		case Operator_lt, Operator_gt, Operator_lte, Operator_gte:
			if fieldType == "TYPE_STRING" || fieldType == "TYPE_BOOL" { // not supported for relational operators
				return fmt.Errorf("operator not supported on the key [%s]", s.Requirements[ii].Key)
			}
			fallthrough
		default:
			if !ref.ParseableVal(fieldType, s.Requirements[ii].Values[0]) {
				return fmt.Errorf("given value [%s] does not match the key's [%s] actual type [%s]",
					s.Requirements[ii].Values[0], s.Requirements[ii].Key, fieldType)
			}
		}

		s.Requirements[ii].Key = result
	}

	return nil
}

// MatchesObj for a Selector returns true if all of the Requirements match the
// provided object. It returns false for an empty selector.
func (s *Selector) MatchesObj(obj matchObject) bool {
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
