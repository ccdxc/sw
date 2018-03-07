// This is the top level file for penlop library
// To run the tool, call Parse() from your main()
// Copyright 2018 Pensando Systems, Inc.

package penlop

import (
	"io/ioutil"
	"os"
	//	"encoding/json"
	"fmt"
	"github.com/davecgh/go-spew/spew"
	"gopkg.in/yaml.v2"
)

const (
	OPTYPE_NONE = iota
	OPTYPE_LITERAL
	OPTYPE_INFIX
	OPTYPE_PREFIX
	OPTYPE_POSTFIX
	OPTYPE_PKTVAR
	OPTYPE_SESSVAR
	OPTYPE_FUNC
)

// typemask
const (
	VAL_STRING = 1 << iota
	VAL_IPADDR
	VAL_INT
	VAL_BOOL
)

const VAL_NONE = 0
const VAL_ADD = VAL_STRING | VAL_INT
const VAL_COMPARE = VAL_STRING | VAL_IPADDR | VAL_INT | VAL_BOOL
const VAL_ANY = VAL_STRING | VAL_IPADDR | VAL_INT | VAL_BOOL

type PenLOPOpDesc struct {
	//	Opname string
	OpType         uint8
	MinOperands    uint8
	MaxOperands    uint8
	InValTypeMask  int
	OutValTypeMask int
	//	luaEquiv string
}

const OPERAND_MAX_COUNT uint8 = 99

var OpDescTable map[string]PenLOPOpDesc = map[string]PenLOPOpDesc{
	// opname: optype, operand_count, operand_types, output_type

	// Arithmetic operators
	"+":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_ADD, VAL_ADD},
	"-":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_ADD, VAL_ADD},
	"*":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"/":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"%":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"|":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"&":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"^":  {OPTYPE_INFIX, 2, OPERAND_MAX_COUNT, VAL_INT, VAL_INT},
	"<<": {OPTYPE_INFIX, 2, 2, VAL_INT, VAL_INT},
	">>": {OPTYPE_INFIX, 2, 2, VAL_INT, VAL_INT},

	// Logical operators
	"!":  {OPTYPE_PREFIX, 1, 1, VAL_BOOL, VAL_BOOL},
	"==": {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	"!=": {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	"<":  {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	">":  {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	"<=": {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	">=": {OPTYPE_INFIX, 2, 2, VAL_COMPARE, VAL_BOOL},
	"||": {OPTYPE_INFIX, 2, 2, VAL_BOOL, VAL_BOOL},
	"&&": {OPTYPE_INFIX, 2, 2, VAL_BOOL, VAL_BOOL},

	// Literal or variable value
	"literal": {OPTYPE_LITERAL, 0, 0, VAL_ANY, VAL_ANY},
	"pktvar":  {OPTYPE_PKTVAR, 0, 0, VAL_ANY, VAL_ANY},
	"sessvar": {OPTYPE_SESSVAR, 0, 0, VAL_ANY, VAL_ANY},

	// Pkt variables
	"dip": {OPTYPE_PKTVAR, 0, 0, VAL_NONE, VAL_IPADDR},
	"sip": {OPTYPE_PKTVAR, 0, 0, VAL_NONE, VAL_IPADDR},
	"dir": {OPTYPE_PKTVAR, 0, 0, VAL_NONE, VAL_BOOL},
}

// Used for both pktvar and sessvar
type PenLOPVarDesc struct {
	VarType int
}

type PenLOPRegexState struct {
	Regex string       `yaml:"regex"`
	State *PenLOPState `yaml:"state"`
}

type PenLOPRegexSet struct {
	Type         string              `yaml:"type"` // one of MatchFirst, MatchLongest
	RegexStates  []*PenLOPRegexState `yaml:"regex_states"`
	DefaultState *PenLOPState        `yaml:"default_state"`
}

type PenLOPState struct {
	Name        string          `yaml:"name"`
	PreActions  []*PenLOPAction `yaml:"pre_actions"`
	RegexSet    *PenLOPRegexSet `yaml:"regex_set"`
	PostActions []*PenLOPAction `yaml:"post_actions"`
}

type PenLOPExpression struct {
	Operator string              `yaml:"op"` // one of "literal", "+", "-", "dir", "pktvar", "session_var", etc.
	Literal  string              `yaml:"literal"`
	Operands []*PenLOPExpression `yaml:"operands"`
}

type PenLOPMatchPattern struct {
	Regex           string        `yaml:"regex"`
	MatchedAction   *PenLOPAction `yaml:"matched_action"`
	NoMatchedAction *PenLOPAction `yaml:"nomatched_action"`
}

type PenLOPMatchCondition struct {
	Expression      *PenLOPExpression `yaml:"expression"`
	MatchedAction   *PenLOPAction     `yaml:"matched_action"`
	NoMatchedAction *PenLOPAction     `yaml:"nomatched_action"`
}

type PenLOPSetSessVar struct {
	Name string `yaml:"name"` // either a session var name or a scratch var name
	// Note: scratch variables have packet scope
	Expression *PenLOPExpression `yaml:"expression"`
	Literal    string            `yaml:"literal"`
}

type PenLOPAccumulate struct {
	Type    string `yaml:"type"`    // atoi, xtoi, bin, atoi_le, xtoi_le, bin_le, raw
	SessVar string `yaml:"sessvar"` // sessvar name
	Count   int    `yaml:"end_count"`
	//	EndChars string `yaml:"end_chars"`
}

type PenLOPAction struct {
	Type           string                `yaml:"type"` // one of sip, dip, match_pattern, assign, etc.
	MatchPattern   *PenLOPMatchPattern   `yaml:"match_pattern"`
	MatchCondition *PenLOPMatchCondition `yaml:"match_condition"`
	SetSessVar     *PenLOPSetSessVar     `yaml:"set_sessvar"`
	SetAppStatus   string                `yaml:"set_app_status"`
	GotoState      string                `yaml:"goto_state"`
	Accumulate     *PenLOPAccumulate     `yaml:"accumulate"`
}

type PenLOPSessionVar struct {
	Name string `yaml:"name"`
	Type string `yaml:"type"` // one of [u]int8,[u]int16,[u]int32,[u]int64,ipaddr,string
}

type PenLOPAppDef struct {
	Name                 string              `yaml:"name"`
	Description          string              `yaml:"description"`
	BaseProtocol         string              `yaml:"base_protocol"`
	L4Ports              []uint16            `yaml:"l4_ports"`
	L4Protocol           uint16              `yaml:"l4_protocol"`
	InitialState         string              `yaml:"initial_state"`
	PreconditionPatterns []string            `yaml:"precondition_patterns"`
	States               []*PenLOPState      `yaml:"states"`
	SessionVars          []*PenLOPSessionVar `yaml:"session_vars"`
}

type PenLOPApp struct {
	App PenLOPAppDef `yaml:"app"`
}

const (
	OUTPUT_NONE = iota
	OUTPUT_LUA
	OUTPUT_C
)

func LookupState(name string, a []*PenLOPState) *PenLOPState {
	// TODO, sort states by name

	for _, state := range a {
		if state.Name == name {
			return state
		}
	}
	return nil
}

func Parse(infilename string, verbose bool) (*PenLOPApp, error) {
	// copy file to a buffer
	// TODO: use Decoder and io.Reader instead, for large input files
	buf, err := ioutil.ReadFile(infilename)
	if err != nil {
		panic(err)
	}

	// Parse YAML input
	var app PenLOPApp
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
