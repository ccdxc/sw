// Command protoc-gen-delphi is a plugin for Google protocol buffer
// compiler to generate Delphi framework specific files from protobuf definitions.
// You rarely need to run this program directly. Instead, put this program
// into your $PATH with a name "protoc-gen-delphi" and run
//   protoc --delphi_out=output_directory path/to/input.proto
//
// See README.md for more details.
package main

import (
	"github.com/golang/glog"
	"github.com/pensando/sw/nic/delphi/compiler/protoc-gen-delphi/generator"
	"github.com/pensando/sw/nic/delphi/compiler/protoc-gen-delphi/templates"
)

func main() {
	defer glog.Flush()

	g := generator.New()

	// generate Hdr file
	err := g.Generate(templates.HeaderTemplate, ".delphi.hpp")
	if err != nil {
		glog.Errorf("Error while processing .delphi.hpp generation. Err: %v", err)
		return
	}
	glog.V(2).Info("Processed .delphi.hpp generator request")

	// generate source file
	err = g.Generate(templates.SrcTemplate, ".delphi.cc")
	if err != nil {
		glog.Errorf("Error while processing .delphi.cc generation. Err: %v", err)
		return
	}
	glog.V(2).Info("Processed .delphi.cc generator request")

	// generate utest file
	err = g.Generate(templates.UtestTemplate, ".delphi_utest.hpp")
	if err != nil {
		glog.Errorf("Error while processing .delphi_utest.hpp generation. Err: %v", err)
		return
	}
	glog.V(2).Info("Processed .delphi_utest.hpp generator request")
}
