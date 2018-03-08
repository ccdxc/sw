// generator.go is the output generator for penlop package.
// Call Generate() with the output from Parse()
// Copyright 2018 Pensando Systems, Inc.

package penlop

import (
	"fmt"
	"net" // for IP address conversion
	"os"
	"strconv"
)

// OutputType enumeration
const (
	OutputTypeNONE = iota
	OutputTypeLUA
	OutputTypeC
)

// GeneratorContext is used internally by Generate() and all output generators.
type GeneratorContext struct {
	outputType int
	debugOn    bool
	outfile    *os.File
	App        *LOPAppDef
}

// Generator is the interface for all output generators.
type Generator interface {
	Preprocess() error
	WriteGlobals() error
	WriteState(s *LOPState) error
	Postprocess() error
}

// generateImpl is the internal implementation of Generate()
func generateImpl(g *Generator, app *LOPAppDef) error {
	var err error

	err = (*g).Preprocess()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Preprocess error: %s\n", err)
		return err
	}

	err = (*g).WriteGlobals()
	if err != nil {
		fmt.Fprintf(os.Stderr, "WriteGlobals error: %s\n", err)
		return err
	}

	for _, state := range app.States {
		err = (*g).WriteState(state)
		if err != nil {
			fmt.Fprintf(os.Stderr, "WriteState(%s) error: %s\n", state.Name, err)
			return err
		}
	}

	err = (*g).Postprocess()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Postprocess error: %s\n", err)
		return err
	}

	return nil
}

// Generate is the main output function to translate from the internal
// format created by Parse() to the specified output language (e.g. Lua or C).
func Generate(app *LOPApp, outfilename string, outputType int, debugOn bool) error {
	// Create output file from parsed app
	var err error
	var ofile *os.File

	if outputType == OutputTypeNONE {
		ofile, _ = os.Create(os.DevNull)
	} else if len(outfilename) == 0 {
		fmt.Fprintf(os.Stderr, "No output file specified, using stdout.\n")
		ofile = os.Stdout
	} else {
		ofile, err = os.Create(outfilename)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Failed to open output file %s, error %s\n", outfilename, err)
			return err
		}
		defer ofile.Close()
	}

	context := GeneratorContext{}
	context.outputType = outputType
	context.App = &app.App
	context.outfile = ofile
	context.debugOn = debugOn
	var generator Generator
	switch outputType {
	default:
		fmt.Fprintf(os.Stderr, "Writing format NIL\n")
		generator = &NilGenerator{GeneratorContext: &context}
	case OutputTypeLUA:
		fmt.Fprintf(os.Stderr, "Writing format LUA\n")
		generator = &LuaGenerator{GeneratorContext: &context}
	case OutputTypeC:
		fmt.Fprintf(os.Stderr, "Writing format NIL (TODO: C generator)\n")
		generator = &LuaGenerator{GeneratorContext: &context}
	}

	return generateImpl(&generator, &app.App)
}

// DeduceValType guesses the actual type of string input based on the value
func DeduceValType(operand string) int {
	// TODO: no support for float today

	_, e := strconv.ParseInt(operand, 0, 64)
	if e == nil {
		return ValINT
	}

	_, e = strconv.ParseBool(operand)
	if e == nil {
		return ValBOOL
	}

	ip := net.ParseIP(operand)
	if len(ip) > 0 {
		return ValIPADDR
	}

	return ValSTRING
}
