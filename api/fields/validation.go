package fields

import (
	"fmt"
	"regexp"
)

// Field Selector validation. A field selector has the below requirements.
//
// 1) It is a set of comma separated <key,operator,value(s)> tuples.
// 2) Only four operators are currently supported "=", "!=", " in ", " notin ".
// 3) Key is used to select a field in a nested structure like an API object.
//    Key can only be a string of alphabets. "." is used as separator to get to
//    the next level. Atleast one "." is required. Slices are not indexable as
//    the order is not guaranteed. Indexing can be skipped when using slices,
//    all the members of a slice are matched. Maps can be indexed using [*] or
//    [map-key].
//    Examples: "a.b", "a.b.c", "a.b[*].c", "a.b[*]", "a.b[x].c", "a.b[x]"
// 4) Operators "=", "!=" require a single value
//    Examples: "x=a", "x!=a"
// 5) Operators " in ", " notin " could have multiple values, but need ().
//    Examples: "x in (a)", "x notin (a,b)"
// 6) Value(s) is/are a string of alphanumeric characters. "." and "/" are
//    allowed in values. A single value does not require any scaffolding.
//    Multiple values are always enclosed by parenthesis, each value is comma
//    separated as shown in above example. It is possible to escape "," in a
//    value using \\.
//    Example: "abc\\,def"

// Key related validations.
var (
	startFmt           = "[A-Za-z]+"                   // Start with alphabets
	varFmt             = "\\.[A-Za-z]+"                // . separated variable
	subscriptedStarFmt = varFmt + "\\[\\*\\]"          // subscript [*] for maps
	subscriptedVarFmt  = varFmt + "\\[[A-Za-z0-9]+\\]" // subscript [key] for maps

	// middleFmt is list of "." separated variables, optionally subscripted by "*"
	// for slices or maps, "key" for maps.
	middleFmt = "(" + subscriptedStarFmt + "|" + subscriptedVarFmt + "|" + varFmt + ")*"

	// Ends with variable or a subscripted variable.
	endFmt = "(" + subscriptedStarFmt + "|" + subscriptedVarFmt + "|" + varFmt + ")"

	fieldKeyFmt = startFmt + middleFmt + endFmt

	keyRE           = regexp.MustCompile(fieldKeyFmt)
	validFieldKeyRE = regexp.MustCompile("^" + fieldKeyFmt + "$")

	fieldKeyErrMsg = "valid field must be a . separated string of alphabets, indexed using [*] or [key] for maps, and must start and end with an alphabet"
)

func validateFieldKey(k string) error {
	if !validFieldKeyRE.MatchString(k) {
		return fmt.Errorf("%v is not valid, %v", k, fieldKeyErrMsg)
	}
	return nil
}

// Op related validations.
var (
	opFmt          = `(\s*=\s*|\s*!=\s*|\s+in\s+|\s+notin\s+)`
	opRE           = regexp.MustCompile(opFmt)
	validFieldOpRE = regexp.MustCompile("^" + opFmt + "$")
	fieldOpErrMsg  = "op must be one of {=, !=, in, notin}"
)

func validateFieldOp(op string) error {
	if !validFieldOpRE.MatchString(op) {
		return fmt.Errorf("%v is not valid, %v", op, fieldOpErrMsg)
	}
	return nil
}

// Value(s) related validations.
var (
	valStart = "[A-Za-z0-9-_/\\.]"             // ".", "/", " " and "_" are ok in values
	valMore  = valStart + `|(\s)` + "|(\\\\,)" // " " and escaping "," is supported in the middle
	valFmt   = "(" + valStart + ")+(" + valMore + ")*"

	valsStartFmt  = "\\((" + valFmt + ")+" // "(" is mandated for multiple values
	valsMiddleFmt = "(,(" + valFmt + ")+" + ")*"
	valsEndFmt    = "\\)"
	valsFmt       = valsStartFmt + valsMiddleFmt + valsEndFmt

	fieldValsFmt = "(" + valsFmt + "|(" + valFmt + "))*"

	valRE            = regexp.MustCompile(valFmt)
	valsRE           = regexp.MustCompile(fieldValsFmt)
	validFieldValsRE = regexp.MustCompile("^" + fieldValsFmt + "$")
	fieldValsErrMsg  = "valid values must be a single alphanumeric value or a comma separated set of alphanumeric values"
)

func validateFieldVals(vals string) error {
	if !validFieldValsRE.MatchString(vals) {
		return fmt.Errorf("%v is not valid, %v", vals, fieldValsErrMsg)
	}
	return nil
}

// Selector validation.
var (
	singleOpFmt  = `(\s*=\s*|\s*!=\s*)`              // spaces are optional around = and !=
	setOpFmt     = `(\s+in\s+|\s+notin\s+)`          // atleast one space is needed for in and notin
	singleValFmt = singleOpFmt + "(" + valFmt + ")?" // = and != need a single value or empty
	setValFmt    = setOpFmt + valsFmt                // in and notin need multiple values
	fieldValFmt  = "(" + singleValFmt + "|" + setValFmt + ")"
	reqFmt       = "(" + fieldKeyFmt + fieldValFmt + ")"
	reqStartFmt  = "^" + reqFmt + "+"
	reqNextFmt   = "(," + reqFmt + ")*$"
	selFmt       = reqStartFmt + reqNextFmt

	validSelRE = regexp.MustCompile(selFmt)
	selErrMsg  = "valid selector must be a comma separated set of <key,op,values> tuples, key is a string of alphabets, with indices for maps, op must be one of {=,!=,in,notin}, values must be a single alphanumeric string or a comma separated set of alphanumeric strings in parentheses"
)

func validateSelector(sel string) error {
	if !validSelRE.MatchString(sel) {
		return fmt.Errorf("%v is not valid, %v", sel, selErrMsg)
	}
	return nil
}
