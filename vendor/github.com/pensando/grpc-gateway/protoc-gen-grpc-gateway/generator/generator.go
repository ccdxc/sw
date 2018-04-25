// Package generator provides an abstract interface to code generators.
package generator

import (
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
)

// TemplateDef is a specification of the template to be applied.
type TemplateDef struct {
	// Location of the Template to be applied
	Template string
	// Output File to which the result will be written to.
	OutputPath string
	// Once is boolean that specifies this template is to be applied only once on the first file.
	Once bool
}

// Generator is an abstraction of code generators.
type Generator interface {
	// Generate generates output files from input .proto files.
	Generate(targets []*descriptor.File) ([]*plugin.CodeGeneratorResponse_File, error)
	// GenerateFromTemplates processes each element in paths agains input .proto and outputs as per TemplateDef.
	GenerateFromTemplates(targets *descriptor.File, paths []TemplateDef, index int) ([]*plugin.CodeGeneratorResponse_File, error)
}
