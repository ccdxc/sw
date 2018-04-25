package gengateway

import (
	"bytes"
	"errors"
	"fmt"
	"go/format"
	"os"
	"path"
	"path/filepath"
	"strings"
	"text/template"

	"github.com/gogo/protobuf/proto"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gen "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/generator"
	ext "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	options "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"
)

var (
	errNoTargetService  = errors.New("no target service defined in the file")
	errTemplateNotFound = errors.New("Could not find Template")
)

type generator struct {
	reg               *descriptor.Registry
	baseImports       []descriptor.GoPackage
	useRequestContext bool
}

// New returns a new generator which generates grpc gateway files.
func New(reg *descriptor.Registry, useRequestContext bool) gen.Generator {
	var imports []descriptor.GoPackage
	for _, pkgpath := range []string{
		"io",
		"net/http",
		"github.com/pensando/grpc-gateway/runtime",
		"github.com/pensando/grpc-gateway/utilities",
		"github.com/gogo/protobuf/proto",
		"golang.org/x/net/context",
		"google.golang.org/grpc",
		"google.golang.org/grpc/codes",
		"google.golang.org/grpc/grpclog",
	} {
		pkg := descriptor.GoPackage{
			Path: pkgpath,
			Name: path.Base(pkgpath),
		}
		if err := reg.ReserveGoPackageAlias(pkg.Name, pkg.Path); err != nil {
			for i := 0; ; i++ {
				alias := fmt.Sprintf("%s_%d", pkg.Name, i)
				if err := reg.ReserveGoPackageAlias(alias, pkg.Path); err != nil {
					continue
				}
				pkg.Alias = alias
				break
			}
		}
		imports = append(imports, pkg)
	}
	return &generator{reg: reg, baseImports: imports, useRequestContext: useRequestContext}
}

func (g *generator) Generate(targets []*descriptor.File) ([]*plugin.CodeGeneratorResponse_File, error) {
	var files []*plugin.CodeGeneratorResponse_File
	for _, file := range targets {
		glog.V(1).Infof("Processing %s", file.GetName())
		code, err := g.generate(file)
		if err == errNoTargetService {
			glog.V(1).Infof("%s: %v", file.GetName(), err)
			continue
		}
		if err != nil {
			return nil, err
		}
		formatted, err := format.Source([]byte(code))
		if err != nil {
			glog.Errorf("%v: %s", err, code)
			return nil, err
		}
		name := filepath.Base(file.GetName())
		ext := filepath.Ext(name)
		base := strings.TrimSuffix(name, ext)
		output := fmt.Sprintf("%s.pb.gw.go", base)
		files = append(files, &plugin.CodeGeneratorResponse_File{
			Name:    proto.String(output),
			Content: proto.String(string(formatted)),
		})
		glog.V(1).Infof("Will emit %s", output)
	}
	return files, nil
}

func (g *generator) generate(file *descriptor.File) (string, error) {
	pkgSeen := make(map[string]bool)
	var imports []descriptor.GoPackage
	for _, pkg := range g.baseImports {
		pkgSeen[pkg.Path] = true
		imports = append(imports, pkg)
	}
	for _, svc := range file.Services {
		for _, m := range svc.Methods {
			pkg := m.RequestType.File.GoPkg
			if m.Options == nil || !proto.HasExtension(m.Options, options.E_Http) ||
				pkg == file.GoPkg || pkgSeen[pkg.Path] {
				continue
			}
			pkgSeen[pkg.Path] = true
			imports = append(imports, pkg)
		}
	}
	return applyTemplate(param{File: file, Imports: imports, UseRequestContext: g.useRequestContext})
}

func testEqualStr(a, b string) bool {
	return a == b
}
func contains(str, substr string) bool {
	return strings.Contains(str, substr)
}

func toUpper(str string) string {
	return strings.ToUpper(str)
}

func toLower(str string) string {
	return strings.ToLower(str)
}

func getFieldName(in string) string {
	s := []byte(in)
	if i := strings.LastIndex(in, "."); i != -1 {
		return string(s[i+1:])
	}
	return in
}

// GenerateFromTemplates processes each element in paths agains input .proto and outputs as per TemplateDef.
func (g *generator) GenerateFromTemplates(target *descriptor.File, paths []gen.TemplateDef, index int) ([]*plugin.CodeGeneratorResponse_File, error) {
	var files []*plugin.CodeGeneratorResponse_File
	tmpls := []string{}

	glog.V(1).Infof("Processing Templates for [%v] [%v]", target.Name, paths)
	for _, t := range paths {
		tmpls = append(tmpls, t.Template)
	}

	funcMap := template.FuncMap{
		"strEq":        testEqualStr,
		"subStr":       contains,
		"getFieldName": getFieldName,
		"getExtension": ext.GetExtension,
		"hasExtension": ext.HasExtension,
		"upper":        toUpper,
		"lower":        toLower,
	}

	for k, v := range ext.FuncMap {
		funcMap[k] = v
	}

	tmpl, err := template.New("GenTemplates").Funcs(funcMap).ParseFiles(tmpls...)
	if err != nil {
		glog.V(1).Infof("Failed to parse Templates (%s)", err)
		return nil, err
	}

	// Setup up the base path
	target.GoPkg.RelPath, err = getrelpath()
	if err != nil {
		glog.V(1).Infof("Could not figure out relative path of package")
		return nil, err
	}
	for _, p := range paths {
		if p.Once == true && index != 0 {
			continue
		}
		_, file := filepath.Split(p.Template)
		t := tmpl.Lookup(file)
		if t == nil {
			glog.V(1).Infof("Error Generating file [%s] (%s)", p.Template, errTemplateNotFound)
			return nil, errTemplateNotFound
		}
		w := bytes.NewBuffer(nil)
		err := t.Execute(w, target)
		if err != nil {
			glog.V(1).Infof("Error Parsing template [%s] (%s)", p.Template, err)
			return nil, err
		}
		// Generate an output if an output path is specified, else the output is ignored.
		if p.OutputPath != "" {
			// The path might specify dynamic path parameters where the output path is derived
			// from generation context. Currently supported parameters are
			//   {PACKAGE} : replaced by the package specified in the protobuf file
			//   {{FILE}} : replaced by the protobuf filename (minus the ".proto" extension)
			outputPath := getDynPath(target, p.OutputPath)
			code := w.String()
			files = append(files, &plugin.CodeGeneratorResponse_File{
				Name:    proto.String(outputPath),
				Content: proto.String(code),
			})
			glog.V(1).Infof("generated File is %v", w.String())
		}
	}

	return files, nil
}

func Getwsroot() (string, error) {
	gpath := os.Getenv("GOPATH")
	if gpath == "" {
		return gpath, errors.New("Not Found")
	}
	return gpath, nil
}

func getrelpath() (string, error) {
	cwd, err := os.Getwd()
	if err != nil {
		return "", nil
	}
	wsroot, err := Getwsroot()
	if err != nil {
		return "", err
	}
	r, err := filepath.Rel(wsroot+"/src", cwd)
	if err != nil {
		return "", err
	}

	return r, nil
}

// getDynPath returns the resolved path after substituting dynamic parameters in the path
//  specification.
func getDynPath(target *descriptor.File, path string) string {
	file := *target.Name
	pkg := target.GoPkg.Name

	file = filepath.Base(file)
	if strings.HasSuffix(file, ".proto") {
		file = strings.TrimSuffix(file, ".proto")
	}

	ret := strings.Replace(path, "{PACKAGE}", pkg, -1)
	ret = strings.Replace(ret, "{FILE}", file, -1)
	return ret
}
