package penlop

//import (
//  "fmt"
//)

type NilGenerator struct {
	*PenLOPGeneratorContext
	CallCount int
}

func (g *NilGenerator) Preprocess() error {
	g.CallCount++
	return nil
}

func (g *NilGenerator) WriteGlobals() error {
	g.CallCount++
	return nil
}

func (g *NilGenerator) WriteState(s *PenLOPState) error {
	g.CallCount++
	return nil
}

func (g *NilGenerator) Postprocess() error {
	g.CallCount++
	return nil
}
