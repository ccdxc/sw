package generator

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"text/template"

	"github.com/golang/glog"
	"github.com/golang/protobuf/proto"
	plugin "github.com/golang/protobuf/protoc-gen-go/plugin"
	"github.com/pensando/sw/nic/delphi/compiler/protoc-gen-delphi/descriptor"
)

var (
	errNoTargetService = errors.New("no target service defined in the file")
)

// command line flags
var (
	importPrefix = flag.String("import_prefix", "", "prefix to be added to go package paths for imported proto files")
	importPath   = flag.String("import_path", "", "used as the package if no input files declare go_package. If it contains slashes, everything up to the rightmost slash is ignored.")
)

// Generator is an abstraction of code generators.
type Generator interface {
	// Generate generates output files from input .proto files.
	Generate(tmplStr string, fileSuffix string) error
}

type generator struct {
	reg     *descriptor.Registry
	targets []*descriptor.File
}

// New returns a new generator which generates grpc gateway files.
func New() Generator {
	flag.Parse()

	// create new registry
	reg := descriptor.NewRegistry()

	glog.V(2).Info("Processing code generator request")
	req, err := parseReq(os.Stdin)
	if err != nil {
		glog.Fatal(err)
	}
	if req.Parameter != nil {
		for _, p := range strings.Split(req.GetParameter(), ",") {
			spec := strings.SplitN(p, "=", 2)
			if len(spec) == 1 {
				if err = flag.CommandLine.Set(spec[0], ""); err != nil {
					glog.Fatalf("Cannot set flag %s", p)
				}
				continue
			}
			name, value := spec[0], spec[1]
			if strings.HasPrefix(name, "M") {
				reg.AddPkgMap(name[1:], value)
				continue
			}
			if err = flag.CommandLine.Set(name, value); err != nil {
				glog.Fatalf("Cannot set flag %s. Err: %v", p, err)
			}
		}
	}

	reg.SetPrefix(*importPrefix)
	reg.SetImportPath(*importPath)
	if err = reg.Load(req); err != nil {
		emitError(err)
		return nil
	}

	var targets []*descriptor.File
	for _, target := range req.FileToGenerate {
		f, lerr := reg.LookupFile(target)
		if lerr != nil {
			glog.Fatal(lerr)
		}
		targets = append(targets, f)
	}

	return &generator{
		reg:     reg,
		targets: targets,
	}
}

func (g *generator) Generate(tmplStr string, fileSuffix string) error {
	var files []*plugin.CodeGeneratorResponse_File
	for _, file := range g.targets {
		glog.V(1).Infof("Processing %s", file.GetName())
		code, err := applyTemplate(file, fileSuffix, tmplStr)
		if err == errNoTargetService {
			glog.V(1).Infof("%s: %v", file.GetName(), err)
			continue
		}
		if err != nil {
			emitError(err)
			return err
		}

		name := file.GetName()
		if file.GoPkg.Path != "" {
			name = fmt.Sprintf("%s/%s", file.GoPkg.Path, filepath.Base(name))
		}

		ext := filepath.Ext(name)
		base := strings.TrimSuffix(name, ext)
		output := fmt.Sprintf("%s%s", base, fileSuffix)
		files = append(files, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(output),
			Content: proto.String(string(code)),
		})
		glog.V(1).Infof("Will emit %s from %s", output, file.GetName())
	}

	// emit files
	emitFiles(files)

	return nil
}

// parseReq parse protoc plugin input
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

// throwError is used for throwing error from the template
func throwError(errMsg string, locators ...string) (interface{}, error) {
	var locatorStr string
	if len(locators) > 0 {
		for i := 0; i < len(locators); i++ {
			locatorStr = locatorStr + " : " + locators[i]
		}
		locatorStr += " : "
	}
	return errMsg, fmt.Errorf("%s %s", locatorStr, errMsg)
}

func isTrue(boolPtr *bool) bool {
	if boolPtr == nil {
		return false
	}

	return *boolPtr
}

var funcMap = template.FuncMap{
	"ToUpper":    strings.ToUpper,
	"ToLower":    strings.ToLower,
	"ThrowError": throwError,
	"IsTrue":     isTrue,
}

func applyTemplate(file *descriptor.File, tmplName, tmplStr string) (string, error) {
	w := bytes.NewBuffer(nil)

	// parse the template string
	tmpl := template.Must(template.New(tmplName).Funcs(funcMap).Parse(tmplStr))

	// generate file from template
	if err := tmpl.Execute(w, file); err != nil {
		return "", err
	}

	return w.String(), nil
}
