package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strings"

	"github.com/gogo/protobuf/proto"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	"github.com/pensando/grpc-gateway/protoc-gen-swagger/genswagger"
	gwplugin "github.com/pensando/grpc-gateway/protoc-gen-swagger/plugins"
)

var (
	importPrefix    = flag.String("import_prefix", "", "prefix to be added to go package paths for imported proto files")
	file            = flag.String("file", "stdin", "where to load data from")
	allowDeleteBody = flag.Bool("allow_delete_body", false, "unless set, HTTP DELETE methods may not have a body")
	mode            = flag.String("mode", "internal", "what mode the generation will run in")
)

func parseReq(r io.Reader) (*plugin.CodeGeneratorRequest, error) {
	glog.V(1).Info("Parsing code generator request")
	input, err := ioutil.ReadAll(r)
	if err != nil {
		glog.Errorf("Failed to read code generator request: %v", err)
		return nil, err
	}
	req := new(plugin.CodeGeneratorRequest)
	if err = proto.Unmarshal(input, req); err != nil {
		glog.Errorf("Failed to unmarshal code generator request: %v", err)
		return nil, err
	}
	glog.V(1).Info("Parsed code generator request")
	return req, nil
}

func main() {
	flag.Parse()
	defer glog.Flush()

	reg := descriptor.NewRegistry()

	glog.V(1).Info("Processing code generator request")
	f := os.Stdin
	if *file != "stdin" {
		f, _ = os.Open("input.txt")
	}
	req, err := parseReq(f)
	if err != nil {
		glog.Fatal(err)
	}
	pkgMap := make(map[string]string)
	if req.Parameter != nil {
		err := parseReqParam(req.GetParameter(), flag.CommandLine, pkgMap)
		if err != nil {
			glog.Fatalf("Error parsing flags: %v", err)
		}
	}

	for k, v := range gwplugin.MutatorMap {
		glog.V(1).Infof("Calling Mutator func %s", k)
		v(req)
	}

	reg.SetPrefix(*importPrefix)
	reg.SetAllowDeleteBody(*allowDeleteBody)
	for k, v := range pkgMap {
		reg.AddPkgMap(k, v)
	}

	var selectedMode genswagger.GeneratorMode
	selectedMode, ok := genswagger.GeneratorModeOptions[strings.ToLower(*mode)]
	if !ok {
		glog.Fatalf("Error parsing mode flag: %s is not a valid value", *mode)
	}

	opts := genswagger.Opts{
		Mode: selectedMode,
	}

	g := genswagger.New(reg, opts)

	if err := reg.Load(req); err != nil {
		emitError(err)
		return
	}

	var targets []*descriptor.File
	for _, target := range req.FileToGenerate {
		f, err := reg.LookupFile(target)
		if err != nil {
			glog.Fatal(err)
		}
		targets = append(targets, f)
	}

	out, err := g.Generate(targets)
	glog.V(1).Info("Processed code generator request")
	if err != nil {
		emitError(err)
		return
	}
	emitFiles(out)
}

func emitFiles(out []*plugin.CodeGeneratorResponse_File) {
	emitResp(&plugin.CodeGeneratorResponse{File: out})
}

func emitError(err error) {
	emitResp(&plugin.CodeGeneratorResponse{Error: proto.String(err.Error())})
}

func emitResp(resp *plugin.CodeGeneratorResponse) {
	buf, err := proto.Marshal(resp)
	if err != nil {
		glog.Fatal(err)
	}
	if _, err := os.Stdout.Write(buf); err != nil {
		glog.Fatal(err)
	}
}

// parseReqParam parses a CodeGeneratorRequest parameter and adds the
// extracted values to the given FlagSet and pkgMap. Returns a non-nil
// error if setting a flag failed.
func parseReqParam(param string, f *flag.FlagSet, pkgMap map[string]string) error {
	if param == "" {
		return nil
	}
	for _, p := range strings.Split(param, ",") {
		spec := strings.SplitN(p, "=", 2)
		if len(spec) == 1 {
			if spec[0] == "allow_delete_body" {
				err := f.Set(spec[0], "true")
				if err != nil {
					return fmt.Errorf("Cannot set flag %s: %v", p, err)
				}
				continue
			}
			err := f.Set(spec[0], "")
			if err != nil {
				return fmt.Errorf("Cannot set flag %s: %v", p, err)
			}
			continue
		}
		name, value := spec[0], spec[1]
		if strings.HasPrefix(name, "M") {
			pkgMap[name[1:]] = value
			continue
		}
		if err := f.Set(name, value); err != nil {
			return fmt.Errorf("Cannot set flag %s: %v", p, err)
		}
	}
	return nil
}
