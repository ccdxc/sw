// File nil_generator.go is a blank version of the Generator interface
// Copyright 2018 Pensando Systems, Inc.

package penlop

//import (
//  "fmt"
//)

// NilGenerator contains all details needed to run Generate() for Nil output.
type NilGenerator struct {
	*GeneratorContext
	CallCount int
}

// Preprocess is called by Generate()
func (g *NilGenerator) Preprocess() error {
	g.CallCount++
	return nil
}

// WriteGlobals is called by Generate()
func (g *NilGenerator) WriteGlobals() error {
	g.CallCount++
	return nil
}

// WriteState is called by Generate()
func (g *NilGenerator) WriteState(s *LOPState) error {
	g.CallCount++
	return nil
}

// Postprocess is called by Generate()
func (g *NilGenerator) Postprocess() error {
	g.CallCount++
	return nil
}
