// Package penlop is a translator between the PenLOP canonical form
// in YAML or JSON into a target language, such as Lua or C.
// parser.go is the top level file for the package.
// To run the tool, call Parse() and then Generate() from your main().
// Copyright 2018 Pensando Systems, Inc.
package penlop

import (
	"fmt"
	"io/ioutil"
	"os"

	"github.com/davecgh/go-spew/spew"
	yaml "gopkg.in/yaml.v2"
)

// Optype enumeration
const (
	OptypeNONE = iota
	OptypeLITERAL
	OptypeINFIX
	OptypePREFIX
	OptypePOSTFIX
	OptypePKTVAR
	OptypeSESSVAR
	OptypeFUNC
)

// Val type enumeration
const (
	ValSTRING = 1 << iota
	ValIPADDR
	ValINT
	ValBOOL
)

// ValNONE defines an unknown valtype.
const ValNONE = 0

// ValADD is a mask of valtypes supported by +/- operators.
const ValADD = ValSTRING | ValINT

// ValCOMPARE is a mask of valtypes supported by comparison operators.
const ValCOMPARE = ValSTRING | ValIPADDR | ValINT | ValBOOL

// ValANY is a mask of all valtypes.
const ValANY = ValSTRING | ValIPADDR | ValINT | ValBOOL

// OperatorDesc struct type defines LOP operators.
type OperatorDesc struct {
	//	Opname string
	OpType         uint8
	MinOperands    uint8
	MaxOperands    uint8
	InValTypeMask  int
	OutValTypeMask int
}

// OperandMaxCount defines the maximum number of operands allowable by any operator
const OperandMaxCount uint8 = 99

// OpDescTable is a map of all LOP operator descriptions.
var OpDescTable = map[string]OperatorDesc{
	// opname: optype, operand_count, operand_types, output_type

	// Arithmetic operators
	"+":  {OptypeINFIX, 2, OperandMaxCount, ValADD, ValADD},
	"-":  {OptypeINFIX, 2, OperandMaxCount, ValADD, ValADD},
	"*":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"/":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"%":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"|":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"&":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"^":  {OptypeINFIX, 2, OperandMaxCount, ValINT, ValINT},
	"<<": {OptypeINFIX, 2, 2, ValINT, ValINT},
	">>": {OptypeINFIX, 2, 2, ValINT, ValINT},

	// Logical operators
	"!":  {OptypePREFIX, 1, 1, ValBOOL, ValBOOL},
	"==": {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	"!=": {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	"<":  {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	">":  {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	"<=": {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	">=": {OptypeINFIX, 2, 2, ValCOMPARE, ValBOOL},
	"||": {OptypeINFIX, 2, 2, ValBOOL, ValBOOL},
	"&&": {OptypeINFIX, 2, 2, ValBOOL, ValBOOL},

	// Literal or variable value
	"literal": {OptypeLITERAL, 0, 0, ValANY, ValANY},
	"pktvar":  {OptypePKTVAR, 0, 0, ValANY, ValANY},
	"sessvar": {OptypeSESSVAR, 0, 0, ValANY, ValANY},

	// Pkt variables
	"dip": {OptypePKTVAR, 0, 0, ValNONE, ValIPADDR},
	"sip": {OptypePKTVAR, 0, 0, ValNONE, ValIPADDR},
	"dir": {OptypePKTVAR, 0, 0, ValNONE, ValBOOL},
}

// LOPRegexState struct type defines a regex and associated state instance.
type LOPRegexState struct {
	Regex string    `yaml:"regex"`
	State *LOPState `yaml:"state"`
}

// LOPRegexSet struct type defines a regex set instance.
type LOPRegexSet struct {
	Type         string           `yaml:"type"` // one of MatchFirst, MatchLongest
	RegexStates  []*LOPRegexState `yaml:"regex_states"`
	DefaultState *LOPState        `yaml:"default_state"`
}

// LOPState struct type defines a LOP state instance.
// A state is similar in scope and size to a function.
type LOPState struct {
	Name        string       `yaml:"name"`
	PreActions  []*LOPAction `yaml:"pre_actions"`
	RegexSet    *LOPRegexSet `yaml:"regex_set"`
	PostActions []*LOPAction `yaml:"post_actions"`
}

// LOPExpression struct type defines an expression instance.
type LOPExpression struct {
	Operator string           `yaml:"op"` // one of "literal", "+", "-", "dir", "pktvar", "session_var", etc.
	Literal  string           `yaml:"literal"`
	Operands []*LOPExpression `yaml:"operands"`
}

// LOPMatchPattern struct type defines a match_pattern instance.
type LOPMatchPattern struct {
	Regex           string     `yaml:"regex"`
	MatchedAction   *LOPAction `yaml:"matched_action"`
	NoMatchedAction *LOPAction `yaml:"nomatched_action"`
}

// LOPMatchCondition struct type defines a match_condition instance.
type LOPMatchCondition struct {
	Expression      *LOPExpression `yaml:"expression"`
	MatchedAction   *LOPAction     `yaml:"matched_action"`
	NoMatchedAction *LOPAction     `yaml:"nomatched_action"`
}

// LOPSetSessVar struct type defines a set_sess_var instance.
type LOPSetSessVar struct {
	Name string `yaml:"name"` // either a session var name or a scratch var name
	// Note: scratch variables have packet scope
	Expression *LOPExpression `yaml:"expression"`
	Literal    string         `yaml:"literal"`
}

// LOPAccumulate struct type defines an accumulate instance.
type LOPAccumulate struct {
	Type    string `yaml:"type"`    // atoi, xtoi, bin, atoi_le, xtoi_le, bin_le, raw
	SessVar string `yaml:"sessvar"` // sessvar name
	Count   int    `yaml:"end_count"`
	//	EndChars string `yaml:"end_chars"`
}

// LOPAction struct type defines an action instance.
type LOPAction struct {
	Type           string             `yaml:"type"` // one of sip, dip, match_pattern, assign, etc.
	MatchPattern   *LOPMatchPattern   `yaml:"match_pattern"`
	MatchCondition *LOPMatchCondition `yaml:"match_condition"`
	SetSessVar     *LOPSetSessVar     `yaml:"set_sessvar"`
	SetAppStatus   string             `yaml:"set_app_status"`
	GotoState      string             `yaml:"goto_state"`
	Accumulate     *LOPAccumulate     `yaml:"accumulate"`
}

// LOPSessionVar struct type defines a session_var instance.
type LOPSessionVar struct {
	Name string `yaml:"name"`
	Type string `yaml:"type"` // one of [u]int8,[u]int16,[u]int32,[u]int64,ipaddr,string
}

// LOPAppDef struct type defines the entire description of an application.
type LOPAppDef struct {
	Name                 string           `yaml:"name"`
	Description          string           `yaml:"description"`
	BaseProtocol         string           `yaml:"base_protocol"`
	L4Ports              []uint16         `yaml:"l4_ports"`
	L4Protocol           uint16           `yaml:"l4_protocol"`
	InitialState         string           `yaml:"initial_state"`
	PreconditionPatterns []string         `yaml:"precondition_patterns"`
	States               []*LOPState      `yaml:"states"`
	SessionVars          []*LOPSessionVar `yaml:"session_vars"`
}

// LOPApp struct type is a container for LOPAppDef.
type LOPApp struct {
	App LOPAppDef `yaml:"app"`
}

// LookupState returns a pointer to state, given the name.
func LookupState(name string, a []*LOPState) *LOPState {
	// TODO, sort states by name

	for _, state := range a {
		if state.Name == name {
			return state
		}
	}
	return nil
}

// Parse is the top-level function for the penlop package.
// Input: input file name (typically .yaml or .json)
// Output: an internal representation of the application defined by input file
func Parse(infilename string, verbose bool) (*LOPApp, error) {
	// copy file to a buffer
	// TODO: use Decoder and io.Reader instead, for large input files
	buf, err := ioutil.ReadFile(infilename)
	if err != nil {
		panic(err)
	}

	// Parse YAML input
	var app LOPApp
	err = yaml.Unmarshal(buf, &app)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Failed to parse YAML file, error %s\n", err)
		os.Exit(1)
	}
	fmt.Fprintf(os.Stderr, "Parsed YAML file successfully\n")

	// Initial state validation
	if len(app.App.InitialState) > 0 {
		if LookupState(app.App.InitialState, app.App.States) == nil {
			fmt.Fprintf(os.Stderr, "Initial state %s undefined\n", app.App.InitialState)
			os.Exit(1)
		}
	}

	// Dump the structure for debugging
	if verbose {
		fmt.Fprintf(os.Stderr, "Dumping app info...\n")
		spew.Dump(app)
	}

	return &app, err
}
