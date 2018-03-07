// This file converts the canonical form into Snort Lua
// Top level functions are in interface PenLOPGenerator
// Copyright 2018 Pensando Systems, Inc.

package penlop

import (
	"fmt"
	"net" // for IP address conversion
	"os"
	"strconv"
	"strings"
	"text/template"
	"unicode"
)

const indent_base string = "  "

const lua_globals_templ = `
--[[
# Copyright 2018 Pensando Systems, Inc.
--]]
--[[
detection_name: {{.App.Name}}
version: 1
description: {{.App.Description}}
--]]

require "DetectorCommon"
local DC = DetectorCommon
local FT = flowTrackerModule
DetectorPackageInfo = {
  name = "{{.App.Name}}",
  proto = DC.ipproto.{{.LuaL4Proto}},
  server = {
    init = 'DetectorInit',
    validate = 'DetectorValidator',
  }
}

gServiceName = "{{.App.Name}}"

{{.GlobalPortListString}}
{{.GlobalFastPatternListString}}

function serviceInProcess(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:inProcessService()
    end

    DC.printf('%s: Inprocess, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.inProcess
end

function serviceSuccess(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:addService(gAppId, "", "", gAppId)
    end

    DC.printf('%s: Detected, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.success
end

function serviceFail(context)
    local flowFlag = context.detectorFlow:getFlowFlag(DC.flowFlags.serviceDetected)

    if ((not flowFlag) or (flowFlag == 0)) then
        gDetector:failService()
    end

    context.detectorFlow:clearFlowFlag(DC.flowFlags.continue)
    DC.printf('%s: Failed, packetCount: %d\n', gServiceName, context.packetCount);
    return DC.serviceStatus.nomatch
end

function getFlowTrackerItem(rft, flowField, defStr)
    if rft == nil then
        return defStr
    end
    if rft.flowField == nil then
        return defStr
    end
    return rft.flowField
end

function registerPortsPatterns()
    if (gPorts) then
        for i,v in ipairs(gPorts) do
            gDetector:addPort(v[1], v[2])
            DC.printf('%s: registering port %d\n', gServiceName, v[2]);
        end
    end

    if (gPairs) then
        for i,v in ipairs(gFastPatterns) do
            if ( gDetector:registerPattern(v[1], v[2][1], #v[2][1], v[2][2], gAppId) ~= 0) then
                DC.printf('%s: register pattern failed for %s\n', gServiceName, v[2])
            else
                DC.printf('%s: register pattern successful for %d\n', gServiceName, i)
            end
        end
    end
end

function DetectorInit(detectorInstance)
  gDetector = detectorInstance;
  gAppId = gDetector:open_createApp(gServiceName);

  gDetector:init(gServiceName, 'DetectorValidator', 'DetectorFini')
  registerPortsPatterns()

  DC.printf("Done registration for %s custom appid: %d\n", gServiceName, gAppId);
  return gDetector;
end

function DetectorFini()
  DC.printf('%s:DetectorFini()\n', gServiceName);
end

function DetectorValidator()
  local context = {}
  context.detectorFlow = gDetector:getFlow()
  context.packetDataLen = gDetector:getPacketSize()
  context.packetDir = gDetector:getPacketDir()
  context.srcIp = gDetector:getPktSrcAddr()
  context.dstIp = gDetector:getPktDstAddr()
  context.srcPort = gDetector:getPktSrcPort()
  context.dstPort = gDetector:getPktDstPort()
  context.flowKey = context.detectorFlow:getFlowKey()
  context.packetCount = gDetector:getPktCount()

  DC.printf('%s:DetectorValidator(): packetCount %d, dir %d\n', gServiceName, context.packetCount, context.packetDir);

  if (context.packetDataLen == 0) then
    return serviceInProcess(context)
  end

  {{.App.InitialState}}(context)
  
  return serviceInProcess(context)
end

`

type PktVarDesc struct {
	LuaName string
	ValType int // TODO: move this to parser.go
}

var PktVarDescTable map[string]PktVarDesc = map[string]PktVarDesc{
	"dip": {"context.dstIp", VAL_IPADDR},
	"sip": {"context.srcIp", VAL_IPADDR},
	"dir": {"(context.packetDir==1)", VAL_BOOL},
}

type LuaOpDesc struct {
	//	Opname string
	LuaName string
}

// For now, just include the subset of operators which differ from PenLOP
var LuaOpDescTable map[string]LuaOpDesc = map[string]LuaOpDesc{
	// TODO: bitwise operators are natively supported in Lua 5.3,
	//       but Lua 5.2 requires use of the bit32 library
	"^":  {"~"},
	"!":  {"not"},
	"!=": {"~="},
	"||": {"or"},
	"&&": {"and"},
}

// Store state preprocessing info
type LuaStateInfo struct {
	uses_sessvar bool
}

type LuaGenerator struct {
	//	lua_state   map[string]int
	*PenLOPGeneratorContext

	// Initialized during preprocessing
	IsPreprocessing bool
	LuaL4Proto      string
	CurrentState    string
	SessVarList     map[string]int          // maps string to VarType
	StateInfoList   map[string]LuaStateInfo // maps state name to info
}

// Generate list of L4 ports
func (g *LuaGenerator) GlobalPortListString() string {
	var s string

	// L4Ports
	if len(g.App.L4Ports) > 0 {
		s += "gPorts = {\n"
		for _, port := range g.App.L4Ports[0:] {
			s += fmt.Sprintf("  { DC.ipproto.%s, %d },\n", g.LuaL4Proto, port)
		}
		s += "}\n"
	}

	return s
}

// Generate FastPattern list
func (g *LuaGenerator) GlobalFastPatternListString() string {
	var s string

	// FastPatterns
	var pat_id int
	if len(g.App.PreconditionPatterns) > 0 {
		// Write gPatterns
		s += "gPatterns = {\n"
		for _, pattern := range g.App.PreconditionPatterns[0:] {
			// Trim leading and trailing whitespace
			pattern = strings.TrimSpace(pattern)

			// Count prefix dots, to get start offset (TODO: make sure we don't trim ".*" or ".+")
			pat2 := strings.TrimLeft(pattern, ".")
			if len(pat2) == 0 {
				fmt.Fprintf(os.Stderr, "Warning, PreCondition pattern empty, skipping\n")
				continue
			}

			// After this point, we'll definitely (try) to generate FastPattern
			pat_id++
			start_offset := len(pattern) - len(pat2)

			// Now replace any "\x" sequences, and warn on invalid chars
			pat3, err := g.ToLuaString(pat2)
			if err != nil {
				return ""
			}
			s += fmt.Sprintf("  pat%d = {'%s', %d },\n", pat_id, pat3, start_offset)
		}
		s += "}\n"

		// Write gFastPatterns
		if pat_id > 0 {
			s += "\ngFastPatterns = {\n"
			for i := 0; i < pat_id; i++ {
				s += fmt.Sprintf("  { DC.ipproto.%s, gPatterns.pat%d },\n",
					g.LuaL4Proto, i+1)
			}
			s += "}\n"
		}
	}

	return s
}

func (g *LuaGenerator) ToLuaString(s string) (string, error) {
	return g.ToLuaStringImpl(s, false)
}

// Convert GO/YAML string to LUA string
// Assumes this is a string literal, not a regex pattern
func (g *LuaGenerator) ToLuaStringImpl(s string, allow_regex bool) (string, error) {
	var err error

	// Replace any "\x" sequences in place, and warn on invalid chars
	pat := []byte(s)
	for i := 0; i < len(pat); i++ {
		if pat[i] == '\\' {
			if (len(pat)-i) > 3 && pat[i+1] == 'x' {
				// Hex encoded binary byte "\xHH"
				n, err := strconv.ParseUint(string(pat[i+2:i+4]), 16, 8)
				if err == nil {
					// Convert to Lua "\DDD" decimal format
					dec_str := strconv.FormatUint(n, 10)
					copy(pat[i+1:i+4], "000")
					copy(pat[(i+4)-len(dec_str):i+4], dec_str[0:])
					i += 3 // consume data
				} else {
					fmt.Fprintf(os.Stderr, "Warning, unexpected pattern at offset %d in %s\n",
						i, s)
					err = nil // ignore warning
				}
			} else if (len(pat) - i) > 1 {
				// Regular backslash escape, passthrough
				i++
			}
		} else if !allow_regex && strings.IndexByte(".*?+[", pat[i]) >= 0 {
			// Now generate warning on any regex char
			fmt.Fprintf(os.Stderr, "Warning, PreCondition pattern %s appears to contain regex symbol %v at index %d, treating as literal\n",
				pat, pat[i], i)
		} else if pat[i] == '\'' || pat[i] == '"' || !unicode.IsPrint(rune(pat[i])) {
			// Now generate warning on any unescaped character
			// TODO: append Lua character sequence
			err = fmt.Errorf("Warning, PreCondition pattern %s contains unescaped symbol %d at index %d, treating as literal\n",
				pat, pat[i], i)
			return "", err
		}
	}
	return string(pat), err
}

func (g *LuaGenerator) Preprocess() error {
	var err error
	g.IsPreprocessing = true

	// Populate LuaL4Proto
	switch g.App.L4Protocol {
	default:
		fmt.Fprintf(os.Stderr, "Warning: unknown L4Protocol %d, defaulting to tcp\n", g.App.L4Protocol)
		g.LuaL4Proto = "tcp"
	case 1:
		g.LuaL4Proto = "icmp"
	case 6:
		g.LuaL4Proto = "tcp"
	case 17:
		g.LuaL4Proto = "udp"
	}

	// Initialize maps
	g.SessVarList = make(map[string]int)
	g.StateInfoList = make(map[string]LuaStateInfo)

	// Preprocessing pass for getting details about the states and session variables
	orig_outfile := g.outfile
	g.outfile, _ = os.Create(os.DevNull) // use dummy null file during preprocessing
	for _, state := range g.App.States {
		err = g.WriteState(state)
		if err != nil {
			fmt.Fprintf(os.Stderr, "WriteState(%s) error: %s\n", state.Name, err)
		}
	}

	g.outfile = orig_outfile
	g.IsPreprocessing = false
	return err
}

func (g *LuaGenerator) Postprocess() error {
	return nil
}

func (g *LuaGenerator) WriteGlobals() error {
	var err error

	t := template.Must(template.New("lua_globals").Funcs(template.FuncMap{
		"GlobalPortListString":        g.GlobalPortListString,
		"GlobalFastPatternListString": g.GlobalFastPatternListString}).
		Parse(lua_globals_templ))

	err = t.Execute(g.outfile, g)

	return err
}

func (g *LuaGenerator) WriteState(s *PenLOPState) error {
	var err error

	g.CurrentState = s.Name
	_, err = fmt.Fprintf(g.outfile, "function %s(context)\n", s.Name)
	if err == nil {
		if g.debug_on {
			_, err = fmt.Fprintf(g.outfile, "%sDC.printf('%%s: Entering state %s\\n', gServiceName)\n",
				indent_base, s.Name)
		}
		// Local variable needed for set/get of session variables
		if g.StateInfoList[g.CurrentState].uses_sessvar {
			_, err = fmt.Fprintf(g.outfile, "%slocal rft = FT.getFlowTracker(context.flowKey)\n", indent_base)
		}

		// Write the state
		err = g.WriteStateInline(s, 1)
		if err == nil {
			//			if s.Name == g.App.InitialState {
			//				_, err = fmt.Fprintf(g.outfile, "\n%sreturn serviceFail(context)\n", indent_base)
			//			}
			if err == nil {
				_, err = fmt.Fprintf(g.outfile, "end\n\n")
			}
		}
	}

	if err != nil {
		fmt.Fprintf(os.Stderr, "Error generating Lua function %s\n", s.Name)
	}

	return err
}

func (g *LuaGenerator) WriteStateInline(s *PenLOPState, indent_level int) error {
	var err error

	//	indent := strings.Repeat(indent_base, indent_level)

	// Generate pre-actions
	for _, action := range s.PreActions {
		err = g.WriteAction(action, indent_level)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua function %s preaction: %s\n", s.Name, err)
			return err
		}
	}

	// Generate regex set
	if s.RegexSet != nil {
		err = g.WriteRegexSet(s.RegexSet, indent_level)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua function %s regex set: %s\n", s.Name, err)
			return err
		}
	}

	// Generate post actions
	for _, action := range s.PostActions {
		err = g.WriteAction(action, indent_level)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua state %s postaction: %s\n", s.Name, err)
			return err
		}
	}

	return err
}

// Writes the given expression recursively
// Returns the VAL_TYPE of the underlying expression, and error if any
func (g *LuaGenerator) WriteExpression(e *PenLOPExpression, indent_level int) (int, error) {
	var err error
	var result_type, prev_type int
	var operand *PenLOPExpression
	var lua_operator string

	if len(e.Operator) == 0 {
		err = fmt.Errorf("Empty expression operator")
		return VAL_NONE, err
	}

	// Get details about the operator
	opdesc := OpDescTable[e.Operator]
	lua_opdesc := LuaOpDescTable[e.Operator]
	if len(lua_opdesc.LuaName) == 0 {
		lua_operator = e.Operator
	} else {
		lua_operator = lua_opdesc.LuaName
	}

	// Sanity checks
	if opdesc.OpType != OPTYPE_NONE {
		// Validate operand count
		if len(e.Operands) < int(opdesc.MinOperands) || len(e.Operands) > int(opdesc.MaxOperands) {
			err = fmt.Errorf("Wrong number of parameters for operator %s", e.Operator)
			return VAL_NONE, err
		}
	} else {
		// Unknown operator, try to guess optype from operand count
		// TODO: eventually treat this is as error
		if len(e.Operands) > 0 {
			opdesc.OpType = OPTYPE_FUNC
			fmt.Fprintf(os.Stderr, "Warn: Guessing optype=FUNC for operator %s.\n", e.Operator)
		} else {
			opdesc.OpType = OPTYPE_PKTVAR
			fmt.Fprintf(os.Stderr, "Warn: Guessing optype=PKTVAR for operator %s.\n", e.Operator)
		}
	}

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	// Generate expression recursively
	switch opdesc.OpType {
	case OPTYPE_INFIX:
		// Always surround infix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s(", indent)
		if err != nil {
			return VAL_NONE, err
		}
		first := true
		for _, operand = range e.Operands {
			if !first {
				_, err = fmt.Fprintf(g.outfile, " %s ", lua_operator)
				if err != nil {
					return VAL_NONE, err
				}
			}
			result_type, err = g.WriteExpression(operand, 0)
			if err != nil {
				return VAL_NONE, err
			}

			// Validate result_type
			if (result_type & opdesc.InValTypeMask) == 0 {
				err = fmt.Errorf("Invalid operand type %d for operator %s", result_type, e.Operator)
				return VAL_NONE, err
			}
			if first {
				first = false
			} else {
				if prev_type != result_type {
					// TODO: possibly make this a real error in the future
					fmt.Fprintf(os.Stderr, "Warning: possible non-matching operand for operator %s.\n", e.Operator)
				}
			}
			prev_type = result_type
		}
		fmt.Fprintf(g.outfile, ")")

	case OPTYPE_PREFIX:
		// Always surround prefix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s(%s(", indent, lua_operator)
		if err != nil {
			return VAL_NONE, err
		}
		// Assumes operator count of 1
		result_type, err = g.WriteExpression(e.Operands[0], 0)
		if err != nil {
			return VAL_NONE, err
		}
		// Validate result_type
		if (result_type & opdesc.InValTypeMask) == 0 {
			err = fmt.Errorf("Invalid operand type %d for operator %s", result_type, e.Operator)
			return VAL_NONE, err
		}
		fmt.Fprintf(g.outfile, "))")

	case OPTYPE_POSTFIX:
		// Always surround postfix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s((", indent)
		if err != nil {
			return VAL_NONE, err
		}
		// Assumes operator count of 1
		operand = e.Operands[0]
		result_type, err = g.WriteExpression(operand, 0)
		if err != nil {
			return VAL_NONE, err
		}
		// Validate result_type
		if (result_type & opdesc.InValTypeMask) == 0 {
			err = fmt.Errorf("Invalid operand type %d for operator %s", result_type, e.Operator)
			return VAL_NONE, err
		}
		fmt.Fprintf(g.outfile, ")%s)", lua_operator)

	case OPTYPE_PKTVAR:
		// Map PenLOP name to Lua name
		var pktvardesc PktVarDesc
		if e.Operator == "pktvar" {
			pktvardesc = PktVarDescTable[e.Literal]
		} else {
			// TODO: be stricter in the future
			pktvardesc = PktVarDescTable[e.Operator]
		}
		if len(pktvardesc.LuaName) == 0 {
			err = fmt.Errorf("Unknown pktvar name %s or %s", e.Operator, e.Literal)
			return VAL_NONE, err
		}

		// Generate Lua variable name
		_, err = fmt.Fprintf(g.outfile, "%s", pktvardesc.LuaName)
		if err != nil {
			return VAL_NONE, err
		}
		result_type = pktvardesc.ValType

	case OPTYPE_SESSVAR:
		result_type, err = g.WriteGetSessVar(e.Literal)

	case OPTYPE_FUNC:
		_, err = fmt.Fprintf(g.outfile, "%s%s(", indent, lua_operator)
		if err != nil {
			return VAL_NONE, err
		}

		first := true
		for _, operand = range e.Operands {
			if first {
				first = false
			} else {
				fmt.Fprintf(g.outfile, ", ")
			}
			result_type, err = g.WriteExpression(operand, 0)
			if err != nil {
				return VAL_NONE, err
			}
		}
		fmt.Fprintf(g.outfile, ")\n")

	case OPTYPE_LITERAL:
		result_type = DeduceValType(e.Literal)
		if result_type == VAL_STRING {
			// TODO: May need custom Quote function for Lua compliant strings
			_, err = fmt.Fprintf(g.outfile, "%s", strconv.QuoteToASCII(e.Literal))
			if err != nil {
				return VAL_NONE, err
			}
		} else {
			_, err = fmt.Fprintf(g.outfile, "%s", e.Literal)
			if err != nil {
				return VAL_NONE, err
			}
		}
	}

	// No error, return output result_type
	if (result_type&opdesc.OutValTypeMask) == 0 && opdesc.OutValTypeMask != VAL_NONE {
		result_type = opdesc.OutValTypeMask
	}

	return result_type, nil
}

func (g *LuaGenerator) WriteMatchPattern(p *PenLOPMatchPattern, indent_level int) error {
	var err error

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	// TODO: replace getPcreGroups
	_, err = fmt.Fprintf(g.outfile, "%smatched = gDetector:getPcreGroups(\"%s\", 0)\n",
		indent, p.Regex)
	if err != nil {
		return err
	}

	if p.MatchedAction != nil {
		_, err = fmt.Fprintf(g.outfile, "%sif (matched) then\n", indent)
		if err != nil {
			return err
		}
		err = g.WriteAction(p.MatchedAction, indent_level+1)
		if err != nil {
			return err
		}
		if p.NoMatchedAction != nil {
			_, err = fmt.Fprintf(g.outfile, "%selse\n", indent)
			if err != nil {
				return err
			}
			err = g.WriteAction(p.NoMatchedAction, indent_level+1)
			if err != nil {
				return err
			}
		}
		_, err = fmt.Fprintf(g.outfile, "%send\n", indent)
		if err != nil {
			return err
		}
	} else if p.NoMatchedAction != nil {
		_, err = fmt.Fprintf(g.outfile, "%sif (not matched) then\n", indent)
		if err != nil {
			return err
		}
		err = g.WriteAction(p.NoMatchedAction, indent_level+1)
		if err != nil {
			return err
		}
		_, err = fmt.Fprintf(g.outfile, "%send\n", indent)
		if err != nil {
			return err
		}
	}

	return nil
}

func (g *LuaGenerator) WriteMatchCondition(m *PenLOPMatchCondition, indent_level int) error {
	var err error

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	// Write Condition
	_, err = fmt.Fprintf(g.outfile, "%sif (", indent)
	if err != nil {
		return err
	}

	result_type, err := g.WriteExpression(m.Expression, indent_level)
	if err != nil {
		return err
	}
	if result_type != VAL_BOOL {
		err = fmt.Errorf("Error: Condition expression is not of boolean type")
		return err
	}
	_, err = fmt.Fprintf(g.outfile, ") then\n")
	if err != nil {
		return err
	}

	// Write MatchAction
	if m.MatchedAction != nil {
		err = g.WriteAction(m.MatchedAction, indent_level+1)
		if err != nil {
			return err
		}
	}

	// Write NoMatchAction
	if m.NoMatchedAction != nil {
		_, err = fmt.Fprintf(g.outfile, "%selse\n", indent)
		if err != nil {
			return err
		}
		err = g.WriteAction(m.NoMatchedAction, indent_level+1)
		if err != nil {
			return err
		}
	}

	_, err = fmt.Fprintf(g.outfile, "%send\n", indent)

	return err
}

// Treat as Action
func (g *LuaGenerator) WriteSetSessVar(v *PenLOPSetSessVar, indent_level int) error {
	// TODO: at init time, traverse all SetSessVar to retrieve list+type of each variable
	var err error
	var result_type int

	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.uses_sessvar = true
		g.StateInfoList[g.CurrentState] = info
	} else {
		// Validate var exists
		if g.SessVarList[v.Name] == 0 {
			err = fmt.Errorf("Cannot set_sessvar, %s undefined", v.Name)
			return err
		}
	}

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	// Generate Lua
	_, err = fmt.Fprintf(g.outfile, "%sif (not rft) then\n  %srft = FT.addFlowTracker(context.flowKey, {%s=",
		indent, indent, v.Name)
	if err != nil {
		return err
	}
	if len(v.Literal) > 0 {
		_, err = fmt.Fprintf(g.outfile, "%s})\n%selse\n  %srft.%s = %s",
			v.Literal, indent, indent, v.Name, v.Literal)
		if err != nil {
			return err
		}
		result_type = DeduceValType(v.Literal)
	} else {
		_, err = g.WriteExpression(v.Expression, indent_level+1)
		if err != nil {
			return err
		}
		_, err = fmt.Fprintf(g.outfile, "})\n%selse\n  %srft.%s = ",
			indent, indent, v.Name)
		if err != nil {
			return err
		}
		result_type, err = g.WriteExpression(v.Expression, indent_level+1)
		if err != nil {
			return err
		}
	}
	_, err = fmt.Fprintf(g.outfile, "\n%send\n", indent)
	if err != nil {
		return err
	}

	// Validation
	if g.SessVarList[v.Name] == VAL_NONE {
		g.SessVarList[v.Name] = result_type
	} else if g.SessVarList[v.Name] != result_type {
		err = fmt.Errorf("Expression type does not match variable type for set_sessvar %s", v.Name)
		return err
	}

	return nil
}

// Treat as Expression
func (g *LuaGenerator) WriteGetSessVar(Name string) (int, error) {
	// TODO
	var err error
	var result_type int

	// Check result type (in case we previously saw SetSessVar)
	result_type = g.SessVarList[Name]
	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.uses_sessvar = true
		g.StateInfoList[g.CurrentState] = info

		// It's OKAY if variable is not yet defined
		if result_type == VAL_NONE {
			// Set to wildcard type, to avoid invalid errors
			result_type = VAL_ANY
		}
	} else {
		// Validate var exists
		if result_type == VAL_NONE {
			err = fmt.Errorf("Cannot get_sessvar, %s undefined", Name)
			return result_type, err
		}
	}

	// Generate Lua
	var default_ret_str string
	switch result_type {
	case VAL_INT:
		default_ret_str = "0"
	case VAL_STRING:
		default_ret_str = "\"\""
	case VAL_BOOL:
		default_ret_str = "false"
	default:
		default_ret_str = "nil"
	}
	_, err = fmt.Fprintf(g.outfile, "getFlowTrackerItem(rft, %s, %s)",
		Name, default_ret_str)

	return result_type, err
}

func (g *LuaGenerator) WriteAccumulate(a *PenLOPAccumulate, indent_level int) error {
	// TODO
	var err error

	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.uses_sessvar = true
		g.StateInfoList[g.CurrentState] = info
	} else {
		// Validate var exists
		if g.SessVarList[a.SessVar] == 0 {
			err = fmt.Errorf("Cannot accumulate sessvar, %s undefined", a.SessVar)
			return err
		}
	}

	var result_type int
	var default_val string
	var lua_func string
	var base int
	var count int = a.Count
	var pcre_str string = "(" + strings.Repeat(".", count) + ")"
	switch {
	case strings.HasPrefix(a.Type, "atoi"):
		result_type = VAL_INT
		default_val = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			lua_func = "reverseAsciiStringToNumber"
		} else {
			lua_func = "asciiStringToNumber"
		}
		base = 10
		if count == 0 {
			pcre_str = "[^0-9]*([0-9]+)[^0-9]"
			count = 99
		}
	case strings.HasPrefix(a.Type, "xtoi"):
		result_type = VAL_INT
		default_val = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			lua_func = "reverseAsciiStringToNumber"
		} else {
			lua_func = "asciiStringToNumber"
		}
		base = 16
		if count == 0 {
			pcre_str = "[^0-9a-fA-F]*([0-9a-fA-F]+)[^0-9a-fA-F]"
			count = 99
		}
	case strings.HasPrefix(a.Type, "bin"):
		result_type = VAL_INT
		default_val = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			lua_func = "reverseBinaryStringToNumber"
		} else {
			lua_func = "binaryStringToNumber"
		}
		base = 2
		if count == 0 {
			err = fmt.Errorf("Invalid count 0 for binary accumulation, sessvar %s", a.SessVar)
			return err
		}
	case a.Type == "raw":
		result_type = VAL_STRING
		default_val = "\"\""
		lua_func = "rawString" // TODO: define this
		base = 2
		if count == 0 {
			err = fmt.Errorf("Invalid count 0 for binary accumulation, sessvar %s", a.SessVar)
			return err
		}
	default:
		err = fmt.Errorf("Expression type %s does not match variable type for accumulate sessvar %s",
			a.Type, a.SessVar)
		return err
	}

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)
	indent2 := strings.Repeat(indent_base, indent_level+1)

	// TODO: replace getPcreGroups
	// TODO: how to avoid consuming last byte past numeric string?

	// Generate Lua
	_, err = fmt.Fprintf(g.outfile, "%sif (not rft) then\n  %srft = FT.addFlowTracker(context.flowKey, {%s=%s})\n%send\n",
		indent, indent, a.SessVar, default_val, indent)
	if err != nil {
		return err
	}
	_, err = fmt.Fprintf(g.outfile, "%smatched, tmp = gDetector:getPcreGroups(\"%s\", 0)\n%sif (matched) then\n",
		indent, pcre_str, indent)
	if err != nil {
		return err
	}

	if base == 2 {
		_, err = fmt.Fprintf(g.outfile, "%srft.%s = %s(tmp, %d)\n%send\n",
			indent2, a.SessVar, lua_func, count, indent)
	} else {
		_, err = fmt.Fprintf(g.outfile, "%srft.%s = %s(tmp, %d, %d)\n%send\n",
			indent2, a.SessVar, lua_func, base, count, indent)
	}
	if err != nil {
		return err
	}

	// Validation
	if g.SessVarList[a.SessVar] == VAL_NONE {
		g.SessVarList[a.SessVar] = result_type
	} else if g.SessVarList[a.SessVar] != result_type {
		err = fmt.Errorf("Expression type does not match variable type for accumulate sessvar %s", a.SessVar)
		return err
	}

	return err
}

func (g *LuaGenerator) WriteSetAppStatus(s string, indent_level int) error {
	// TODO
	var err error
	var lua_name string

	switch s {
	case "FOUND":
		lua_name = "serviceSuccess"
	case "NOT_FOUND":
		lua_name = "serviceFail"
	case "IN_PROGRESS":
		lua_name = "serviceInProcess"
	default:
		err = fmt.Errorf("Unknown app state %s", s)
		return err
	}

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	_, err = fmt.Fprintf(g.outfile, "%sreturn %s(context)\n", indent, lua_name)

	return err
}

func (g *LuaGenerator) WriteGotoState(s string, indent_level int) error {
	var err error

	// Validate state exists
	if LookupState(s, g.App.States) == nil {
		err = fmt.Errorf("Cannot goto_state, %s undefined", s)
		return err
	}

	// Indent string
	indent := strings.Repeat(indent_base, indent_level)

	// Generate Lua function call
	_, err = fmt.Fprintf(g.outfile, "%s%s(context)\n", indent, s)

	return err
}

func (g *LuaGenerator) WriteAction(a *PenLOPAction, indent_level int) error {
	// TODO
	var err error

	switch a.Type {
	case "match_pattern":
		err = g.WriteMatchPattern(a.MatchPattern, indent_level)
	case "match_condition":
		err = g.WriteMatchCondition(a.MatchCondition, indent_level)
	case "set_sessvar":
		err = g.WriteSetSessVar(a.SetSessVar, indent_level)
	case "set_app_status":
		err = g.WriteSetAppStatus(a.SetAppStatus, indent_level)
	case "goto_state":
		err = g.WriteGotoState(a.GotoState, indent_level)
	case "accumulate":
		err = g.WriteAccumulate(a.Accumulate, indent_level)
	default:
		err = fmt.Errorf("Unknown action %s", a.Type)
		return err
	}

	return err
}

func (g *LuaGenerator) WriteRegexSet(r *PenLOPRegexSet, indent_level int) error {
	var err error
	match_first := (r.Type == "match_first")

	// Indent string
	inner_indent_level := indent_level + 1
	indent := strings.Repeat(indent_base, indent_level)
	indent2 := indent

	// TODO: replace getPcreGroups

	if match_first {
		indent2 = strings.Repeat(indent_base, indent_level+1)
		inner_indent_level = indent_level + 2
		_, err = fmt.Fprintf(g.outfile, "%smatched = nil\n", indent)
		if err != nil {
			return err
		}
	}

	for _, regex := range r.RegexStates {
		if match_first {
			_, err = fmt.Fprintf(g.outfile, "%sif (not matched) then\n", indent)
			if err != nil {
				return err
			}
		}
		_, err = fmt.Fprintf(g.outfile, "%smatched = gDetector:getPcreGroups(\"%s\", 0)\n",
			indent2, regex.Regex)
		if err != nil {
			return err
		}
		_, err = fmt.Fprintf(g.outfile, "%sif (matched) then\n", indent2)
		if err != nil {
			return err
		}
		if len(regex.State.Name) > 0 {
			// Name of existing state, use a goto
			// Validate that the global state exists
			if LookupState(regex.State.Name, g.App.States) == nil {
				err = fmt.Errorf("Cannot goto undefined state %s", regex.State.Name)
				return err
			}
			err = g.WriteGotoState(regex.State.Name, inner_indent_level)
			if err != nil {
				return err
			}
		} else {
			// Inline state, just write it here
			err = g.WriteStateInline(regex.State, inner_indent_level)
			if err != nil {
				return err
			}
		}
		_, err = fmt.Fprintf(g.outfile, "%send\n", indent2)
		if err != nil {
			return err
		}
		if match_first {
			_, err = fmt.Fprintf(g.outfile, "%send\n", indent)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

func DeduceValType(operand string) int {
	// TODO: no support for float today

	_, e := strconv.ParseInt(operand, 0, 64)
	if e == nil {
		return VAL_INT
	}

	_, e = strconv.ParseBool(operand)
	if e == nil {
		return VAL_BOOL
	}

	ip := net.ParseIP(operand)
	if len(ip) > 0 {
		return VAL_IPADDR
	}

	return VAL_STRING
}
