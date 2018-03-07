// This is the output generator for penlop library
// Call Generate() with the output from Parse()
// Copyright 2018 Pensando Systems, Inc.
package penlop

import (
	"fmt"
	"os"
)

type PenLOPGeneratorContext struct {
	output_type int
	debug_on    bool
	outfile     *os.File
	App         *PenLOPAppDef
}

type PenLOPGenerator interface {
	Preprocess() error
	WriteGlobals() error
	WriteState(s *PenLOPState) error
	Postprocess() error
}

func generateImpl(g *PenLOPGenerator, app *PenLOPAppDef) error {
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

func Generate(app *PenLOPApp, outfilename string, output_type int, debug_on bool) error {
	// Create output file from parsed app
	var err error
	var ofile *os.File

	if output_type == OUTPUT_NONE {
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

	context := PenLOPGeneratorContext{}
	context.output_type = output_type
	context.App = &app.App
	context.outfile = ofile
	context.debug_on = debug_on
	var generator PenLOPGenerator
	switch output_type {
	default:
		fmt.Fprintf(os.Stderr, "Writing format NIL\n")
		generator = &NilGenerator{PenLOPGeneratorContext: &context}
	case OUTPUT_LUA:
		fmt.Fprintf(os.Stderr, "Writing format LUA\n")
		generator = &LuaGenerator{PenLOPGeneratorContext: &context}
	case OUTPUT_C:
		fmt.Fprintf(os.Stderr, "Writing format NIL (TODO: C generator)\n")
		generator = &NilGenerator{PenLOPGeneratorContext: &context}
	}

	return generateImpl(&generator, &app.App)
}
