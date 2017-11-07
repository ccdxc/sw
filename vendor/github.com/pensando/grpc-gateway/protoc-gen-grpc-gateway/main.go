// Command protoc-gen-grpc-gateway is a plugin for Google protocol buffer
// compiler to generate a reverse-proxy, which converts incoming RESTful
// HTTP/1 requests gRPC invocation.
// You rarely need to run this program directly. Instead, put this program
// into your $PATH with a name "protoc-gen-grpc-gateway" and run
//   protoc --grpc-gateway_out=output_directory path/to/input.proto
//
// See README.md for more details.
package main

import (
	"flag"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"golang.org/x/tools/imports"
	"gopkg.in/yaml.v2"

	"github.com/gogo/protobuf/proto"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"

	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/generator"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/gengateway"
	gwplugin "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
)

var (
	importPrefix      = flag.String("import_prefix", "", "prefix to be added to go package paths for imported proto files")
	useRequestContext = flag.Bool("request_context", false, "determine whether to use http.Request's context or not")
	templateSet       = flag.String("templates", "", "set of Templates to be applied")
	gengw             = flag.Bool("gengw", true, "generate default gateway code")
)

type templateConfig struct {
	BasePath  string
	Templates []generator.TemplateDef
}

func (cfg *templateConfig) ReadConfig(cfgpath string) error {
	tmplCfg, err := ioutil.ReadFile(cfgpath)
	if err != nil {
		return err
	}
	glog.V(1).Infof("Config Raw %v", string(tmplCfg))
	err = yaml.Unmarshal(tmplCfg, cfg)
	if err != nil {
		emitError(err)
		return err
	}
	if !filepath.IsAbs(cfg.BasePath) {
		path, err := gengateway.Getwsroot()
		if err != nil {
			emitError(err)
			return err
		}
		cfg.BasePath = filepath.Join(path, "src", cfg.BasePath)
	}
	for i := range cfg.Templates {
		cfg.Templates[i].Template = filepath.Join(cfg.BasePath, cfg.Templates[i].Template)
	}
	glog.V(1).Infof("Config Parsed %+v", *cfg)
	return nil
}

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
	req, err := parseReq(os.Stdin)
	if err != nil {
		glog.Fatal(err)
	}
	if req.Parameter != nil {
		for _, p := range strings.Split(req.GetParameter(), ",") {
			spec := strings.SplitN(p, "=", 2)
			if len(spec) == 1 {
				if err := flag.CommandLine.Set(spec[0], ""); err != nil {
					glog.Fatalf("Cannot set flag %s", p)
				}
				continue
			}
			name, value := spec[0], spec[1]
			if strings.HasPrefix(name, "M") {
				reg.AddPkgMap(name[1:], value)
				continue
			}
			if err := flag.CommandLine.Set(name, value); err != nil {
				glog.Fatalf("Cannot set flag %s", p)
			}
		}
	}

	for k, v := range gwplugin.MutatorMap {
		glog.V(1).Infof("Calling Mutator func %s", k)
		v(req)
	}

	g := gengateway.New(reg, *useRequestContext)

	reg.SetPrefix(*importPrefix)
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
	glog.V(1).Infof("Processing request for %+v", targets)
	if *templateSet != "" {
		glog.V(1).Infof("Received config is %v", *templateSet)
		var cfg templateConfig
		var path string
		var err error
		if filepath.IsAbs(*templateSet) {
			path = *templateSet
		} else {
			path, err = gengateway.Getwsroot()
			if err != nil {
				emitError(err)
				return
			}
			path = filepath.Join(path, "src", *templateSet)
		}

		err = cfg.ReadConfig(path)
		if err != nil {
			emitError(err)
			return
		}
		glog.V(1).Infof("Parsed Config %+v", cfg.Templates)
		var templateOut []*plugin.CodeGeneratorResponse_File
		for _, t := range targets {
			out1, err := g.GenerateFromTemplates(t, cfg.Templates)
			if err != nil {
				emitError(err)
				return
			}
			for _, o := range out1 {
				templateOut = append(templateOut, o)
			}
		}
		emitFiles(templateOut)
	}

	if *gengw == true {
		out, err := g.Generate(targets)
		glog.V(1).Info("Processed code generator request")
		if err != nil {
			emitError(err)
			return
		}
		emitFiles(out)
	}
}

func emitFiles(out []*plugin.CodeGeneratorResponse_File) {
	for _, genOut := range out {
		filename := *genOut.Name
		ext := filepath.Ext(filename)
		if ext == ".go" {
			importContent, err := goImportContent(filename, []byte(*genOut.Content))
			if err != nil {
				glog.Errorf("Could not auto format generated source. Please run go imports on: %v", *genOut.Name)
				continue
			}
			genOut.Content = &importContent
		}
	}
	emitResp(&plugin.CodeGeneratorResponse{File: out})
}

func emitError(err error) {
	emitResp(&plugin.CodeGeneratorResponse{Error: proto.String(err.Error())})
}

func goImportContent(filename string, src []byte) (string, error) {
	importContent, err := imports.Process(filename, src, nil)
	return string(importContent), err
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
