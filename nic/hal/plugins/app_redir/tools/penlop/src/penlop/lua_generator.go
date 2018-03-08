// File lua_generator.go implements the Generator interface.
// It is used for converting the internal format to Snort Lua.
// Copyright 2018 Pensando Systems, Inc.

package penlop

import (
	"fmt"
	"os"
	"strconv"
	"strings"
	"text/template"
	"unicode"
)

const indentBase string = "  "

const luaGlobalsTempl = `
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

type luaPktVarDesc struct {
	LuaName string
	ValType int // TODO: move this to parser.go
}

var luaPktVarDescTable = map[string]luaPktVarDesc{
	"dip": {"context.dstIp", ValIPADDR},
	"sip": {"context.srcIp", ValIPADDR},
	"dir": {"(context.packetDir==1)", ValBOOL},
}

type luaOpDesc struct {
	//	Opname string
	LuaName string
}

// For now, just include the subset of operators which differ from LOP
var luaOpDescTable = map[string]luaOpDesc{
	// TODO: bitwise operators are natively supported in Lua 5.3,
	//       but Lua 5.2 requires use of the bit32 library
	"^":  {"~"},
	"!":  {"not"},
	"!=": {"~="},
	"||": {"or"},
	"&&": {"and"},
}

// Store state preprocessing info
type luaStateInfo struct {
	usesSessvar bool
}

// LuaGenerator contains all details needed to run Generate() for Lua output
type LuaGenerator struct {
	*GeneratorContext

	// Initialized during preprocessing
	IsPreprocessing bool
	LuaL4Proto      string
	CurrentState    string
	SessVarList     map[string]int          // maps string to VarType
	StateInfoList   map[string]luaStateInfo // maps state name to info
}

// GlobalPortListString is used by template to generate gPorts.
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

// GlobalFastPatternListString is used by template to generate gPatterns.
func (g *LuaGenerator) GlobalFastPatternListString() string {
	var s string

	// FastPatterns
	var patid int
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
			patid++
			startOffset := len(pattern) - len(pat2)

			// Now replace any "\x" sequences, and warn on invalid chars
			pat3, err := g.ToLuaString(pat2)
			if err != nil {
				return ""
			}
			s += fmt.Sprintf("  pat%d = {'%s', %d },\n", patid, pat3, startOffset)
		}
		s += "}\n"

		// Write gFastPatterns
		if patid > 0 {
			s += "\ngFastPatterns = {\n"
			for i := 0; i < patid; i++ {
				s += fmt.Sprintf("  { DC.ipproto.%s, gPatterns.pat%d },\n",
					g.LuaL4Proto, i+1)
			}
			s += "}\n"
		}
	}

	return s
}

// ToLuaString converts an input YAML/JSON string to Lua string.
func (g *LuaGenerator) ToLuaString(s string) (string, error) {
	return g.toLuaStringImpl(s, false)
}

// Convert GO/YAML string to LUA string
// Assumes this is a string literal, not a regex pattern
func (g *LuaGenerator) toLuaStringImpl(s string, allowRegex bool) (string, error) {
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
					decstr := strconv.FormatUint(n, 10)
					copy(pat[i+1:i+4], "000")
					copy(pat[(i+4)-len(decstr):i+4], decstr[0:])
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
		} else if !allowRegex && strings.IndexByte(".*?+[", pat[i]) >= 0 {
			// Now generate warning on any regex char
			fmt.Fprintf(os.Stderr, "Warning, precondition pattern %s appears to contain regex symbol %v at index %d, treating as literal\n",
				pat, pat[i], i)
		} else if pat[i] == '\'' || pat[i] == '"' || !unicode.IsPrint(rune(pat[i])) {
			// Now generate warning on any unescaped character
			// TODO: append Lua character sequence
			err = fmt.Errorf("precondition pattern %s contains unescaped symbol %d at index %d, treating as literal",
				pat, pat[i], i)
			return "", err
		}
	}
	return string(pat), err
}

// Preprocess is called by Generate() before all other functions.
// It does an initial pass to gather context details and does some validation.
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
	g.StateInfoList = make(map[string]luaStateInfo)

	// Preprocessing pass for getting details about the states and session variables
	origOutfile := g.outfile
	g.outfile, _ = os.Create(os.DevNull) // use dummy null file during preprocessing
	for _, state := range g.App.States {
		err = g.WriteState(state)
		if err != nil {
			fmt.Fprintf(os.Stderr, "WriteState(%s) error: %s\n", state.Name, err)
		}
	}

	g.outfile = origOutfile
	g.IsPreprocessing = false
	return err
}

// Postprocess is called by Generate() after all other functions, to do cleanup.
func (g *LuaGenerator) Postprocess() error {
	return nil
}

// WriteGlobals is called by Generate() immediately after Preprocess().
// It used to write any Lua global statements.
func (g *LuaGenerator) WriteGlobals() error {
	var err error

	t := template.Must(template.New("lua_globals").Funcs(template.FuncMap{
		"GlobalPortListString":        g.GlobalPortListString,
		"GlobalFastPatternListString": g.GlobalFastPatternListString}).
		Parse(luaGlobalsTempl))

	err = t.Execute(g.outfile, g)

	return err
}

// WriteState is called by Generate() for each LOPState in the input.
// It writes a Lua function.
func (g *LuaGenerator) WriteState(s *LOPState) error {
	var err error

	g.CurrentState = s.Name
	_, err = fmt.Fprintf(g.outfile, "function %s(context)\n", s.Name)
	if err == nil {
		if g.debugOn {
			_, err = fmt.Fprintf(g.outfile, "%sDC.printf('%%s: Entering state %s\\n', gServiceName)\n",
				indentBase, s.Name)
		}
		// Local variable needed for set/get of session variables
		if g.StateInfoList[g.CurrentState].usesSessvar {
			_, err = fmt.Fprintf(g.outfile, "%slocal rft = FT.getFlowTracker(context.flowKey)\n", indentBase)
		}

		// Write the state
		err = g.WriteStateInline(s, 1)
		if err == nil {
			//			if s.Name == g.App.InitialState {
			//				_, err = fmt.Fprintf(g.outfile, "\n%sreturn serviceFail(context)\n", indentBase)
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

// WriteStateInline writes the Lua function statements without assuming it's an
// actual Lua function
func (g *LuaGenerator) WriteStateInline(s *LOPState, indentLevel int) error {
	var err error

	//	indent := strings.Repeat(indentBase, indentLevel)

	// Generate pre-actions
	for _, action := range s.PreActions {
		err = g.WriteAction(action, indentLevel)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua function %s preaction: %s\n", s.Name, err)
			return err
		}
	}

	// Generate regex set
	if s.RegexSet != nil {
		err = g.WriteRegexSet(s.RegexSet, indentLevel)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua function %s regex set: %s\n", s.Name, err)
			return err
		}
	}

	// Generate post actions
	for _, action := range s.PostActions {
		err = g.WriteAction(action, indentLevel)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error generating Lua state %s postaction: %s\n", s.Name, err)
			return err
		}
	}

	return err
}

// WriteExpression writes the given LOPExpression recursively as Lua output.
// It returns the ValTYPE of the underlying expression.
func (g *LuaGenerator) WriteExpression(e *LOPExpression, indentLevel int) (int, error) {
	var err error
	var resultType, prevType int
	var operand *LOPExpression
	var luaop string

	if len(e.Operator) == 0 {
		err = fmt.Errorf("empty expression operator")
		return ValNONE, err
	}

	// Get details about the operator
	opdesc := OpDescTable[e.Operator]
	luaopdesc := luaOpDescTable[e.Operator]
	if len(luaopdesc.LuaName) == 0 {
		luaop = e.Operator
	} else {
		luaop = luaopdesc.LuaName
	}

	// Sanity checks
	if opdesc.OpType != OptypeNONE {
		// Validate operand count
		if len(e.Operands) < int(opdesc.MinOperands) || len(e.Operands) > int(opdesc.MaxOperands) {
			err = fmt.Errorf("wrong number of parameters for operator %s", e.Operator)
			return ValNONE, err
		}
	} else {
		// Unknown operator, try to guess optype from operand count
		// TODO: eventually treat this is as error
		if len(e.Operands) > 0 {
			opdesc.OpType = OptypeFUNC
			fmt.Fprintf(os.Stderr, "Warn: Guessing optype=FUNC for operator %s.\n", e.Operator)
		} else {
			opdesc.OpType = OptypePKTVAR
			fmt.Fprintf(os.Stderr, "Warn: Guessing optype=PKTVAR for operator %s.\n", e.Operator)
		}
	}

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

	// Generate expression recursively
	switch opdesc.OpType {
	case OptypeINFIX:
		// Always surround infix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s(", indent)
		if err != nil {
			return ValNONE, err
		}
		first := true
		for _, operand = range e.Operands {
			if !first {
				_, err = fmt.Fprintf(g.outfile, " %s ", luaop)
				if err != nil {
					return ValNONE, err
				}
			}
			resultType, err = g.WriteExpression(operand, 0)
			if err != nil {
				return ValNONE, err
			}

			// Validate resultType
			if (resultType & opdesc.InValTypeMask) == 0 {
				err = fmt.Errorf("invalid operand type %d for operator %s", resultType, e.Operator)
				return ValNONE, err
			}
			if first {
				first = false
			} else {
				if prevType != resultType {
					// TODO: possibly make this a real error in the future
					fmt.Fprintf(os.Stderr, "Warning: possible non-matching operand for operator %s.\n", e.Operator)
				}
			}
			prevType = resultType
		}
		fmt.Fprintf(g.outfile, ")")

	case OptypePREFIX:
		// Always surround prefix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s(%s(", indent, luaop)
		if err != nil {
			return ValNONE, err
		}
		// Assumes operator count of 1
		resultType, err = g.WriteExpression(e.Operands[0], 0)
		if err != nil {
			return ValNONE, err
		}
		// Validate resultType
		if (resultType & opdesc.InValTypeMask) == 0 {
			err = fmt.Errorf("invalid operand type %d for operator %s", resultType, e.Operator)
			return ValNONE, err
		}
		fmt.Fprintf(g.outfile, "))")

	case OptypePOSTFIX:
		// Always surround postfix expressions with parentheses, for safety
		_, err = fmt.Fprintf(g.outfile, "%s((", indent)
		if err != nil {
			return ValNONE, err
		}
		// Assumes operator count of 1
		operand = e.Operands[0]
		resultType, err = g.WriteExpression(operand, 0)
		if err != nil {
			return ValNONE, err
		}
		// Validate resultType
		if (resultType & opdesc.InValTypeMask) == 0 {
			err = fmt.Errorf("invalid operand type %d for operator %s", resultType, e.Operator)
			return ValNONE, err
		}
		fmt.Fprintf(g.outfile, ")%s)", luaop)

	case OptypePKTVAR:
		// Map LOP name to Lua name
		var pktvardesc luaPktVarDesc
		if e.Operator == "pktvar" {
			pktvardesc = luaPktVarDescTable[e.Literal]
		} else {
			// TODO: be stricter in the future
			pktvardesc = luaPktVarDescTable[e.Operator]
		}
		if len(pktvardesc.LuaName) == 0 {
			err = fmt.Errorf("unknown pktvar name %s or %s", e.Operator, e.Literal)
			return ValNONE, err
		}

		// Generate Lua variable name
		_, err = fmt.Fprintf(g.outfile, "%s", pktvardesc.LuaName)
		if err != nil {
			return ValNONE, err
		}
		resultType = pktvardesc.ValType

	case OptypeSESSVAR:
		resultType, err = g.writeGetSessVar(e.Literal)

	case OptypeFUNC:
		_, err = fmt.Fprintf(g.outfile, "%s%s(", indent, luaop)
		if err != nil {
			return ValNONE, err
		}

		first := true
		for _, operand = range e.Operands {
			if first {
				first = false
			} else {
				fmt.Fprintf(g.outfile, ", ")
			}
			resultType, err = g.WriteExpression(operand, 0)
			if err != nil {
				return ValNONE, err
			}
		}
		fmt.Fprintf(g.outfile, ")\n")

	case OptypeLITERAL:
		resultType = DeduceValType(e.Literal)
		if resultType == ValSTRING {
			// TODO: May need custom Quote function for Lua compliant strings
			_, err = fmt.Fprintf(g.outfile, "%s", strconv.QuoteToASCII(e.Literal))
			if err != nil {
				return ValNONE, err
			}
		} else {
			_, err = fmt.Fprintf(g.outfile, "%s", e.Literal)
			if err != nil {
				return ValNONE, err
			}
		}
	}

	// No error, return output resultType
	if (resultType&opdesc.OutValTypeMask) == 0 && opdesc.OutValTypeMask != ValNONE {
		resultType = opdesc.OutValTypeMask
	}

	return resultType, nil
}

func (g *LuaGenerator) writeMatchPattern(p *LOPMatchPattern, indentLevel int) error {
	var err error

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

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
		err = g.WriteAction(p.MatchedAction, indentLevel+1)
		if err != nil {
			return err
		}
		if p.NoMatchedAction != nil {
			_, err = fmt.Fprintf(g.outfile, "%selse\n", indent)
			if err != nil {
				return err
			}
			err = g.WriteAction(p.NoMatchedAction, indentLevel+1)
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
		err = g.WriteAction(p.NoMatchedAction, indentLevel+1)
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

func (g *LuaGenerator) writeMatchCondition(m *LOPMatchCondition, indentLevel int) error {
	var err error

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

	// Write Condition
	_, err = fmt.Fprintf(g.outfile, "%sif (", indent)
	if err != nil {
		return err
	}

	resultType, err := g.WriteExpression(m.Expression, indentLevel)
	if err != nil {
		return err
	}
	if resultType != ValBOOL {
		err = fmt.Errorf("condition expression is not of boolean type")
		return err
	}
	_, err = fmt.Fprintf(g.outfile, ") then\n")
	if err != nil {
		return err
	}

	// Write MatchAction
	if m.MatchedAction != nil {
		err = g.WriteAction(m.MatchedAction, indentLevel+1)
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
		err = g.WriteAction(m.NoMatchedAction, indentLevel+1)
		if err != nil {
			return err
		}
	}

	_, err = fmt.Fprintf(g.outfile, "%send\n", indent)

	return err
}

// Treat as Action
func (g *LuaGenerator) writeSetSessVar(v *LOPSetSessVar, indentLevel int) error {
	// TODO: at init time, traverse all SetSessVar to retrieve list+type of each variable
	var err error
	var resultType int

	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.usesSessvar = true
		g.StateInfoList[g.CurrentState] = info
	} else {
		// Validate var exists
		if g.SessVarList[v.Name] == 0 {
			err = fmt.Errorf("cannot set_sessvar, %s undefined", v.Name)
			return err
		}
	}

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

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
		resultType = DeduceValType(v.Literal)
	} else {
		_, err = g.WriteExpression(v.Expression, indentLevel+1)
		if err != nil {
			return err
		}
		_, err = fmt.Fprintf(g.outfile, "})\n%selse\n  %srft.%s = ",
			indent, indent, v.Name)
		if err != nil {
			return err
		}
		resultType, err = g.WriteExpression(v.Expression, indentLevel+1)
		if err != nil {
			return err
		}
	}
	_, err = fmt.Fprintf(g.outfile, "\n%send\n", indent)
	if err != nil {
		return err
	}

	// Validation
	if g.SessVarList[v.Name] == ValNONE {
		g.SessVarList[v.Name] = resultType
	} else if g.SessVarList[v.Name] != resultType {
		err = fmt.Errorf("expression type does not match variable type for set_sessvar %s", v.Name)
		return err
	}

	return nil
}

// Treat as Expression
func (g *LuaGenerator) writeGetSessVar(Name string) (int, error) {
	// TODO
	var err error
	var resultType int

	// Check result type (in case we previously saw SetSessVar)
	resultType = g.SessVarList[Name]
	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.usesSessvar = true
		g.StateInfoList[g.CurrentState] = info

		// It's OKAY if variable is not yet defined
		if resultType == ValNONE {
			// Set to wildcard type, to avoid invalid errors
			resultType = ValANY
		}
	} else {
		// Validate var exists
		if resultType == ValNONE {
			err = fmt.Errorf("cannot get_sessvar, %s undefined", Name)
			return resultType, err
		}
	}

	// Generate Lua
	var defaultRetstr string
	switch resultType {
	case ValINT:
		defaultRetstr = "0"
	case ValSTRING:
		defaultRetstr = "\"\""
	case ValBOOL:
		defaultRetstr = "false"
	default:
		defaultRetstr = "nil"
	}
	_, err = fmt.Fprintf(g.outfile, "getFlowTrackerItem(rft, %s, %s)",
		Name, defaultRetstr)

	return resultType, err
}

func (g *LuaGenerator) writeAccumulate(a *LOPAccumulate, indentLevel int) error {
	// TODO
	var err error

	if g.IsPreprocessing {
		// Remember that the current state is using session variables
		info := g.StateInfoList[g.CurrentState]
		info.usesSessvar = true
		g.StateInfoList[g.CurrentState] = info
	} else {
		// Validate var exists
		if g.SessVarList[a.SessVar] == 0 {
			err = fmt.Errorf("cannot accumulate sessvar, %s undefined", a.SessVar)
			return err
		}
	}

	var resultType int
	var defaultVal string
	var luaFunc string
	var base int
	var count = a.Count
	var pcreStr = "(" + strings.Repeat(".", count) + ")"
	switch {
	case strings.HasPrefix(a.Type, "atoi"):
		resultType = ValINT
		defaultVal = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			luaFunc = "reverseAsciiStringToNumber"
		} else {
			luaFunc = "asciiStringToNumber"
		}
		base = 10
		if count == 0 {
			pcreStr = "[^0-9]*([0-9]+)[^0-9]"
			count = 99
		}
	case strings.HasPrefix(a.Type, "xtoi"):
		resultType = ValINT
		defaultVal = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			luaFunc = "reverseAsciiStringToNumber"
		} else {
			luaFunc = "asciiStringToNumber"
		}
		base = 16
		if count == 0 {
			pcreStr = "[^0-9a-fA-F]*([0-9a-fA-F]+)[^0-9a-fA-F]"
			count = 99
		}
	case strings.HasPrefix(a.Type, "bin"):
		resultType = ValINT
		defaultVal = "0"
		if strings.HasSuffix(a.Type, "le") {
			// little-endian
			luaFunc = "reverseBinaryStringToNumber"
		} else {
			luaFunc = "binaryStringToNumber"
		}
		base = 2
		if count == 0 {
			err = fmt.Errorf("invalid count 0 for binary accumulation, sessvar %s", a.SessVar)
			return err
		}
	case a.Type == "raw":
		resultType = ValSTRING
		defaultVal = "\"\""
		luaFunc = "rawString" // TODO: define this
		base = 2
		if count == 0 {
			err = fmt.Errorf("invalid count 0 for binary accumulation, sessvar %s", a.SessVar)
			return err
		}
	default:
		err = fmt.Errorf("expression type %s does not match variable type for accumulate sessvar %s",
			a.Type, a.SessVar)
		return err
	}

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)
	indent2 := strings.Repeat(indentBase, indentLevel+1)

	// TODO: replace getPcreGroups
	// TODO: how to avoid consuming last byte past numeric string?

	// Generate Lua
	_, err = fmt.Fprintf(g.outfile, "%sif (not rft) then\n  %srft = FT.addFlowTracker(context.flowKey, {%s=%s})\n%send\n",
		indent, indent, a.SessVar, defaultVal, indent)
	if err != nil {
		return err
	}
	_, err = fmt.Fprintf(g.outfile, "%smatched, tmp = gDetector:getPcreGroups(\"%s\", 0)\n%sif (matched) then\n",
		indent, pcreStr, indent)
	if err != nil {
		return err
	}

	if base == 2 {
		_, err = fmt.Fprintf(g.outfile, "%srft.%s = %s(tmp, %d)\n%send\n",
			indent2, a.SessVar, luaFunc, count, indent)
	} else {
		_, err = fmt.Fprintf(g.outfile, "%srft.%s = %s(tmp, %d, %d)\n%send\n",
			indent2, a.SessVar, luaFunc, base, count, indent)
	}
	if err != nil {
		return err
	}

	// Validation
	if g.SessVarList[a.SessVar] == ValNONE {
		g.SessVarList[a.SessVar] = resultType
	} else if g.SessVarList[a.SessVar] != resultType {
		err = fmt.Errorf("expression type does not match variable type for accumulate sessvar %s", a.SessVar)
		return err
	}

	return err
}

func (g *LuaGenerator) writeSetAppStatus(s string, indentLevel int) error {
	// TODO
	var err error
	var luaName string

	switch s {
	case "FOUND":
		luaName = "serviceSuccess"
	case "NOT_FOUND":
		luaName = "serviceFail"
	case "IN_PROGRESS":
		luaName = "serviceInProcess"
	default:
		err = fmt.Errorf("unknown app state %s", s)
		return err
	}

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

	_, err = fmt.Fprintf(g.outfile, "%sreturn %s(context)\n", indent, luaName)

	return err
}

func (g *LuaGenerator) writeGotoState(s string, indentLevel int) error {
	var err error

	// Validate state exists
	if LookupState(s, g.App.States) == nil {
		err = fmt.Errorf("cannot goto_state, %s undefined", s)
		return err
	}

	// Indent string
	indent := strings.Repeat(indentBase, indentLevel)

	// Generate Lua function call
	_, err = fmt.Fprintf(g.outfile, "%s%s(context)\n", indent, s)

	return err
}

// WriteAction writes a LOPAction as Lua output
func (g *LuaGenerator) WriteAction(a *LOPAction, indentLevel int) error {
	// TODO
	var err error

	switch a.Type {
	case "match_pattern":
		err = g.writeMatchPattern(a.MatchPattern, indentLevel)
	case "match_condition":
		err = g.writeMatchCondition(a.MatchCondition, indentLevel)
	case "set_sessvar":
		err = g.writeSetSessVar(a.SetSessVar, indentLevel)
	case "set_app_status":
		err = g.writeSetAppStatus(a.SetAppStatus, indentLevel)
	case "goto_state":
		err = g.writeGotoState(a.GotoState, indentLevel)
	case "accumulate":
		err = g.writeAccumulate(a.Accumulate, indentLevel)
	default:
		err = fmt.Errorf("unknown action %s", a.Type)
		return err
	}

	return err
}

// WriteRegexSet writes a LOPRegexSet as Lua output
func (g *LuaGenerator) WriteRegexSet(r *LOPRegexSet, indentLevel int) error {
	var err error
	matchFirst := (r.Type == "match_first")

	// Indent string
	innerIndentLevel := indentLevel + 1
	indent := strings.Repeat(indentBase, indentLevel)
	indent2 := indent

	// TODO: replace getPcreGroups

	if matchFirst {
		indent2 = strings.Repeat(indentBase, indentLevel+1)
		innerIndentLevel = indentLevel + 2
		_, err = fmt.Fprintf(g.outfile, "%smatched = nil\n", indent)
		if err != nil {
			return err
		}
	}

	for _, regex := range r.RegexStates {
		if matchFirst {
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
				err = fmt.Errorf("cannot goto undefined state %s", regex.State.Name)
				return err
			}
			err = g.writeGotoState(regex.State.Name, innerIndentLevel)
			if err != nil {
				return err
			}
		} else {
			// Inline state, just write it here
			err = g.WriteStateInline(regex.State, innerIndentLevel)
			if err != nil {
				return err
			}
		}
		_, err = fmt.Fprintf(g.outfile, "%send\n", indent2)
		if err != nil {
			return err
		}
		if matchFirst {
			_, err = fmt.Fprintf(g.outfile, "%send\n", indent)
			if err != nil {
				return err
			}
		}
	}

	return nil
}
