package fields

import (
	"bytes"
	"fmt"
	"reflect"
	"regexp"
	"sort"
	"strings"

	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

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

// ParseWithValidation uses the provided kind (ex: cluster.Cluster) to look
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
func ParseWithValidation(kind string, selector string) (*Selector, error) {
	sel, err := Parse(selector)
	if err != nil {
		return nil, err
	}
	for ii := range sel.Requirements {
		fList := strings.Split(sel.Requirements[ii].Key, ".")
		result := ""
		s := kind
		for jj := range fList {
			schema := runtime.GetDefaultScheme().GetSchema(s)
			if schema == nil {
				return nil, fmt.Errorf("Unknown type %v", s)
			}
			jsonStr := fList[jj]
			indexStr := ""
			kk := strings.Index(jsonStr, "[")
			if kk != -1 {
				jsonStr = jsonStr[:kk]
				indexStr = fList[jj][kk+1 : len(fList[jj])-1]
			}
			field, ok := schema.FindFieldByJSONTag(jsonStr)
			if !ok {
				return nil, fmt.Errorf("Did not find field %v", jsonStr)
			}
			if kk != -1 {
				if field.Slice {
					return nil, fmt.Errorf("Indexing is not supported on slice %v, found %v", jsonStr, indexStr)
				}
				if !field.Map {
					return nil, fmt.Errorf("Indexing is not supported on non map field %v, found %v", jsonStr, indexStr)
				}
				// Indexing by "*" is ok for all maps. Otherwise the indexStr needs to match map's key type.
				if indexStr != "*" {
					if !checkVal(field.KeyType, indexStr) {
						return nil, fmt.Errorf("map %v's index %v does not match its type %v", jsonStr, indexStr, field.KeyType)
					}
				}
			}
			if result != "" {
				result += "."
			}
			result += field.Name
			if kk != -1 {
				result += fList[jj][kk:]
			}
			s = field.Type
		}
		if !runtime.IsScalar(s) {
			return nil, fmt.Errorf("Leaf type is %v, not scalar", s)
		}
		sel.Requirements[ii].Key = result
	}
	return sel, nil
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
