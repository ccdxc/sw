package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"text/template"

	"github.com/pensando/sw/cli/api"
	"github.com/pensando/sw/cli/venice/gen/pregen"
	"github.com/pensando/sw/utils/ref"
)

// genObj structure is intermediate structure fetched from parsing all objects
// which is then used to dynamically generate the commands, command completions,
// object manipulation (CRUD functions), restAPI calling functions, and some utility
// functions
// There is one genObj structure created for every object type and used in most
// of the templates to iteratve over various objects for code generation.
type genObj struct {
	Name        string
	Package     string
	GrpcService string
	Perms       string
	CreateAttrs map[string]string
	Attrs       map[string]string
	Structs     []api.GenObjStruct
}

// this is map of all objects by their name (aka key), to genObj structure
var genObjs = map[string]genObj{}

// some helper functions to help with code generation
func paranthesis(inp string) string {
	return fmt.Sprintf("{%s}", inp)
}

func checkPerms(obj genObj, perm string) bool {
	return strings.Contains(obj.Perms, perm)
}

func unTitle(inp string) string {
	retStr := ""
	if len(inp) > 0 {
		retStr = strings.ToLower(inp[0:1])
		if len(inp) > 1 {
			retStr += inp[1:]
		}
	}
	return retStr
}

var visitedFns = map[string]bool{}

func cleanVisited() string {
	visitedFns = make(map[string]bool)
	return ""
}

func visited(inp string) bool {
	if _, ok := visitedFns[inp]; ok {
		return true
	}
	visitedFns[inp] = true
	return false
}

// map of functions used by go templating for use within code generation
var funcMap = template.FuncMap{
	"title":        strings.Title,
	"untitle":      unTitle,
	"paranthesis":  paranthesis,
	"checkPerms":   checkPerms,
	"visited":      visited,
	"cleanVisited": cleanVisited,
}

// main function is called in two steps:
//  step 1. Pregenerate objects, in this mode, it populates pregen directory for all desired objects
//  step 2. In the next phase, it generates the rest of the code aka CLI objects, testserver objects,
//          cli commands, server backend, yml examples
// TODO: move out the template files outside, this would allow keeping templates outside the main logic
func main() {
	for _, apiObj := range api.Objs {
		genObjs[apiObj.Name] = genObj{
			Name:        apiObj.Name,
			Package:     apiObj.Package,
			GrpcService: apiObj.GrpcService,
			Perms:       apiObj.Perms,
			CreateAttrs: make(map[string]string),
			Attrs:       make(map[string]string),
			Structs:     apiObj.Structs,
		}
	}
	// step 1: pregeneration
	if len(os.Args) > 1 {
		if os.Args[1] == "objs" {
			genLocalObjs()
		} else {
			fmt.Printf("Invalid args... %s", os.Args)
		}
		return
	}

	// step 2: real code generation (requires pregeneration)
	genCliObjs()
	genTestServerObjs()
	genCliCmds()
	genServerBackend()
	genYmlExamples()
}

// genLocalObjs pregenerates the code in pregen package that is use by subsequent code genration
func genLocalObjs() {
	f, err := os.Create("./pregen/genobjs.go")
	if err != nil {
		fmt.Printf("Error: %s", err)
		return
	}
	defer f.Close()

	wr := bufio.NewWriter(f)

	t := template.Must(template.New("objs").Funcs(funcMap).Parse(localObjTmpl))

	t.Execute(wr, api.Objs)
	wr.Flush()
}

// genCLIObjs generates the crud apis and utilities for CLI on various objects
func genCliObjs() {
	f, err := os.Create("../genobjs.go")
	if err != nil {
		fmt.Printf("Error: %s", err)
		return
	}
	defer f.Close()

	wr := bufio.NewWriter(f)

	t := template.Must(template.New("objs").Funcs(funcMap).Parse(objTmpl))
	t.Execute(wr, genObjs)

	t = template.Must(template.New("orderedObjs").Funcs(funcMap).Parse(objTmplOrder))
	t.Execute(wr, api.Objs)

	wr.Flush()
}

// genTestServerObjs generates the code to perform crud operations on various objects
func genTestServerObjs() {
	f, err := os.Create("../testserver/tserver/genobjs.go")
	if err != nil {
		fmt.Printf("Error: %s", err)
		return
	}
	defer f.Close()

	wr := bufio.NewWriter(f)

	t := template.Must(template.New("testserverObjs").Funcs(funcMap).Parse(testserverObjTmpl))

	t.Execute(wr, genObjs)
	wr.Flush()
}

// genCliCmds generates the CLI commands, bash completion functions for those commands for various objects
func genCliCmds() {
	f, err := os.Create("../gencmds.go")
	if err != nil {
		fmt.Printf("Error: %s", err)
		return
	}
	defer f.Close()

	wr := bufio.NewWriter(f)
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}

	for _, apiObj := range api.Objs {
		if _, ok := genObjs[apiObj.Name]; ok {
			oSpec := pregen.GetObjSpec(apiObj.Name)
			kvs := make(map[string]ref.FInfo)
			ref.GetKvs(oSpec, refCtx, kvs)
			for key, fi := range kvs {
				genObjs[apiObj.Name].Attrs[key] = fi.TypeStr
				genObjs[apiObj.Name].CreateAttrs[key] = fi.TypeStr
			}

			kvs = make(map[string]ref.FInfo)
			oStatus := pregen.GetObjStatus(apiObj.Name)
			ref.GetKvs(oStatus, refCtx, kvs)
			for key, fi := range kvs {
				genObjs[apiObj.Name].Attrs[key] = fi.TypeStr
			}
		}
	}
	t := template.Must(template.New("cmds").Funcs(funcMap).Parse(cmdsTmpl))
	t.Execute(wr, genObjs)

	t = template.Must(template.New("cmdFlags").Funcs(funcMap).Parse(cmdFlagsTmpl))
	t.Execute(wr, genObjs)

	t = template.Must(template.New("cmdBashCompleter").Funcs(funcMap).Parse(cmdBashCompleterTmpl))
	t.Execute(wr, genObjs)

	wr.Flush()
}

// genServerBackend generates the code for test martini server
func genServerBackend() {
	f, err := os.Create("../testserver/tserver/tserver.go")
	if err != nil {
		fmt.Printf("Error: %s", err)
		return
	}
	defer f.Close()

	wr := bufio.NewWriter(f)

	t := template.Must(template.New("testServerTmpl").Funcs(funcMap).Parse(testServerTmpl))
	t.Execute(wr, genObjs)

	wr.Flush()
}

// genYMLExamples generates the code from yml example files
// TODO: this nees to be udpated based on latest example definitions
func genYmlExamples() {
	dirname := "../examples"
	fstat, err := os.Stat(dirname)
	if err != nil {
		if os.IsNotExist(err) {
			fmt.Printf("Examples directory does not exist: '%s'\n", dirname)
		} else {
			fmt.Printf("Fatal error '%s' opening file: '%s\n", err, dirname)
		}
		os.Exit(1)
	}

	if !fstat.Mode().IsDir() {
		fmt.Printf("'%s' is not a directory\n", dirname)
		os.Exit(1)
	}

	fis, err := ioutil.ReadDir(dirname)
	if err != nil {
		fmt.Printf("Error reading directory '%s': %s\n", dirname, err)
		os.Exit(1)
	}

	wf, err := os.OpenFile("../exampleYml.go", os.O_RDWR|os.O_CREATE, 0755)
	if err != nil {
		fmt.Printf("%s", err)
		os.Exit(1)
	}
	defer wf.Close()

	const initialBlob = `package main

var exampleYmls = map[string]string{
`

	if _, err := wf.WriteString(initialBlob); err != nil {
		fmt.Printf("%s", err)
		return
	}

	for _, fi := range fis {
		fstrs := strings.Split(fi.Name(), ".")
		if len(fstrs) != 2 || (fstrs[1] != "yml" && fstrs[1] != "yaml" && fstrs[1] != "json") {
			continue
		}
		if txt, err := ioutil.ReadFile(dirname + "/" + fi.Name()); err != nil {
			fmt.Printf("File '%s': %s\n", fstrs[0], err)
			os.Exit(1)
		} else {
			if _, err := wf.WriteString("	`" + fstrs[0] + "`: `"); err != nil {
				fmt.Printf("error %s writing txt to file '%s", err, fstrs[0])
			}
			if _, err := wf.Write(txt); err != nil {
				fmt.Printf("error %s writing txt to file '%s", err, fstrs[0])
			}
			if _, err := wf.WriteString("`,\n"); err != nil {
				fmt.Printf("error %s writing txt to file '%s", err, fstrs[0])
			}
		}
	}

	if _, err := wf.WriteString("}\n"); err != nil {
		fmt.Printf("%s", err)
		return
	}
}

//
// Templates start from here
//

// localObjTmpl generates some structs for local
const localObjTmpl = `// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package pregen

import (
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
)

// GetObjSpec is
func GetObjSpec(objName string) interface{} {
	switch objName {
{{ range $apiObj := . }}
	case "{{$apiObj.Name}}":
		return {{$apiObj.Package}}.{{title $apiObj.Name}}Spec{}
{{ end }}
	}
	return nil
}

// GetSubObj is
func GetSubObj(kind string) interface{} {
	switch kind {
{{ cleanVisited }}
{{ range $apiObj := . }}
	{{ range $subObj := .Structs }}
		{{ if eq (visited $subObj.Name) false }}
			case "{{$subObj.Name}}":
				var v {{$apiObj.Package}}.{{$subObj.Name}}
				return &v
		{{ end }}
	{{ end }}
{{ end }}

	}
	return nil
}

// GetObjStatus is
func GetObjStatus(objName string) interface{} {
	switch objName {
{{ range $apiObj := . }}
	case "{{$apiObj.Name}}":
		o := {{$apiObj.Package}}.{{title $apiObj.Name}}{}
		return o.Status
{{ end }}
	}
	return nil
}
`

// generate objs for processing various objects by the backend
const objTmpl = `
// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package main

import (
	contxt "context"
	"encoding/json"
	"fmt"
	"strings"

	log "github.com/Sirupsen/logrus"
	swapi "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
	"github.com/pensando/sw/cli/venice/gen/pregen"
	"github.com/pensando/sw/utils/ref"
)

func getObj(ctx *context) (obj interface{}, objList interface{}) {
	switch ctx.subcmd {
{{ range $obj := . }}
	case "{{$obj.Name}}":
		return &{{$obj.Package}}.{{title $obj.Name}}{}, &{{$obj.Package}}.{{title $obj.Name}}List{}
{{ end }}
	}
	return nil, nil
}

func getSubObj(kind string) interface{} {
	switch kind {
{{ cleanVisited }}
{{ range $obj := . }}
	{{ range $subObj := .Structs }}
		{{ if eq (visited $subObj.Name) false }}
		case "{{$subObj.Name}}":
			var v {{$obj.Package}}.{{$subObj.Name}}
			return v
		{{ end }}
	{{ end }}
{{ end }}
	}
	return nil
}

func getObjFromList(objList interface{}, idx int) interface{} {
{{ range $obj := . }}
	if ol, ok := objList.(*{{$obj.Package}}.{{title $obj.Name}}List); ok {
		{{$obj.Name}} := ol.Items[idx]
		return {{$obj.Name}}
	}
{{ end }}
	return nil
}

func removeObjOper(obj interface{}) error {
{{ range $obj := . }}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		v.UUID = ""
		v.ResourceVersion = ""
		v.Status = {{$obj.Package}}.{{title $obj.Name}}Status{}
	}
{{ end }}
	return nil
}

func writeObj(obj interface{}, objmKvs, specKvs map[string]ref.FInfo) interface{} {
{{ range $obj := . }}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		return write{{title $obj.Name}}Obj(*v, objmKvs, specKvs)
	}
{{ end }}
	return nil
}

func getAllKvs(ctx *context, objList interface{}) ([]map[string]ref.FInfo, []map[string]ref.FInfo, map[string]bool, map[string]bool) {
	objmKvs := []map[string]ref.FInfo{}
	specKvs := []map[string]ref.FInfo{}
	objmValidKvs := make(map[string]bool)
	specValidKvs := make(map[string]bool)
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}

	switch ctx.subcmd {

{{ range $obj := . }}
	case "{{$obj.Name}}":
		{{$obj.Name}}List := objList.(*{{$obj.Package}}.{{title $obj.Name}}List)
		for idx, o := range {{$obj.Name}}List.Items {
			objmKvs = append(objmKvs, make(map[string]ref.FInfo))
			specKvs = append(specKvs, make(map[string]ref.FInfo))
			ref.GetKvs(o.ObjectMeta, refCtx, objmKvs[idx])

			ref.GetKvs(o.Spec, refCtx, specKvs[idx])
			ref.GetKvs(o.Status, refCtx, specKvs[idx])
			for key, fi := range objmKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					objmValidKvs[key] = true
				}
			}
			for key, fi := range specKvs[idx] {
				if strings.Join(fi.ValueStr, "") != "" && !fi.SSkip {
					specValidKvs[key] = true
				}
			}
		}
{{ end }}
	}

	return objmKvs, specKvs, objmValidKvs, specValidKvs
}

{{ cleanVisited }}
{{ range $obj := . }}
	{{ range $subObj := .Structs }}
		{{ if eq (visited $subObj.Name) false }}
func write{{ $subObj.Name }}Obj(obj {{$obj.Package}}.{{ $subObj.Name }}, specKvs map[string]ref.FInfo) *{{$obj.Package}}.{{ $subObj.Name }} {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}
	new := ref.WriteKvs(obj, refCtx, specKvs)

	new{{ $subObj.Name }} := new.({{$obj.Package}}.{{ $subObj.Name }})

	return &new{{ $subObj.Name }}
}
		{{ end }}
	{{ end }}
{{ end }}


{{ range $obj := . }}
func write{{title $obj.Name}}Obj(obj {{$obj.Package}}.{{title $obj.Name}}, metaKvs, specKvs map[string]ref.FInfo) *{{$obj.Package}}.{{title $obj.Name}} {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj}

	newObjm := ref.WriteKvs(obj.ObjectMeta, refCtx, metaKvs)
	newSpec := ref.WriteKvs(obj.Spec, refCtx, specKvs)

	new{{title $obj.Name}} := &{{$obj.Package}}.{{title $obj.Name}}{
		TypeMeta:   swapi.TypeMeta{Kind: "{{$obj.Name}}"},
		ObjectMeta: newObjm.(swapi.ObjectMeta),
		Spec:       newSpec.({{$obj.Package}}.{{title $obj.Name}}Spec),
	}
	return new{{ title $obj.Name }}
}
{{ end }}

func createObjFromBytes(ctx *context, objName, inp string) error {
	switch ctx.subcmd {
{{ range $obj := . }}
	case "{{$obj.Name}}":
		create{{title $obj.Name}}FromBytes(ctx, inp)
{{ end }}
	}
	return nil
}

{{ range $obj := . }}
func create{{title $obj.Name}}FromBytes(ctx *context, inp string) error {
	{{$obj.Name}} := &{{$obj.Package}}.{{title $obj.Name}}{}
	if err := json.Unmarshal([]byte(inp), {{$obj.Name}}); err != nil {
		fmt.Printf("Unmarshling error: %s\nRec: %s\n", err, inp)
		return err
	}

	{{$obj.Name}}.Tenant = ctx.tenant
	if err := postObj(ctx, {{$obj.Name}}, true); err != nil {
		fmt.Printf("post error %s", err)
	}

	return nil
}
{{ end }}


func updateLabel(obj interface{}, newLabels map[string]string) error {
{{ range $obj := . }}
	if o, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		return updateMetaLabel(&o.ObjectMeta, newLabels)
	}
{{ end }}
	return fmt.Errorf("unknown object")
}

func restGet(url, tenant string, obj interface{}) error {
	log.Debugf("get url: %s", url)

	urlStrs := strings.Split(url, "/")
	objName := urlStrs[len(urlStrs)-1]
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()
{{ range $obj := . }}{{ if ne $obj.Package "api" }}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		objm := v.ObjectMeta
		objm.Name = objName
		objm.Tenant = tenant
		nv, err := restcl.{{title $obj.GrpcService}}V1().{{title $obj.Name}}().Get(ctx, &objm)
		if err != nil {
			return err
		}
		*v = *nv
		return nil
	}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}List); ok {
		opts := swapi.ListWatchOptions{ObjectMeta: swapi.ObjectMeta{Tenant: tenant}}
		nlist, err := restcl.{{title $obj.GrpcService}}V1().{{title $obj.Name}}().List(ctx, &opts)
		if err != nil {
			return err
		}
		v.Items = nlist
		return nil
	}
{{ end }}{{ end }}
	return httpGet(url, obj)
}

func restDelete(objKind, url, tenant string) error {
	log.Debugf("delete url: %s", url)

	urlStrs := strings.Split(url, "/")
	objName := urlStrs[len(urlStrs)-1]
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()
{{ range $obj := . }}{{ if ne $obj.Package "api" }}
	if objKind == "{{$obj.Name}}" {
		objm := swapi.ObjectMeta{}
		objm.Name = objName
		objm.Tenant = tenant
		obj, err := restcl.{{title $obj.GrpcService}}V1().{{title $obj.Name}}().Delete(ctx, &objm)
		if err != nil {
			return err
		}
		out, err := json.Marshal(obj)
		if err == nil {
			fmt.Printf("%s", string(out))
		} else {
			fmt.Printf("Unable to marshal object %+v\n", obj)
		}
		return nil
	}
{{ end }}{{ end }}
	return httpDelete(url)
}

func restPost(url, tenant string, obj interface{}) error {
	log.Debugf("post url: %s", url)

	urlStrs := strings.Split(url, "/")
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()
{{ range $obj := . }}{{ if ne $obj.Package "api" }}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		v.Tenant = tenant 
		_, err := restcl.{{title $obj.GrpcService}}V1().{{title $obj.Name}}().Create(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}
{{ end }}{{ end }}
	return httpPost(url, obj)
}

func restPut(url, tenant string, obj interface{}) error {
	log.Debugf("put url: %s", url)

	urlStrs := strings.Split(url, "/")
	hostName := strings.Join(urlStrs[:3], "/")

	restcl, err := apiclient.NewRestAPIClient(hostName)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	ctx := contxt.Background()
{{ range $obj := . }}{{ if ne $obj.Package "api" }}
	if v, ok := obj.(*{{$obj.Package}}.{{title $obj.Name}}); ok {
		v.Tenant = tenant
		_, err := restcl.{{title $obj.GrpcService}}V1().{{title $obj.Name}}().Update(ctx, v)
		if err != nil {
			return err
		}
		return nil
	}
{{ end }}{{ end }}
	return httpPost(url, obj)
}
`

const objTmplOrder = `
var objOrder = []string{
{{ range $obj := . }}
	"{{$obj.Name}}",
{{ end }}}
`

// generate commands from the template
const cmdsTmpl = `// (c) Pensando Systems, Inc.
// This is a generated file, edit at risk of loosing it !!

package main

import (
	"fmt"
	"strings"

	"github.com/urfave/cli"
)

var editCommands = []cli.Command{
{{ range $obj := . }}{{if checkPerms $obj "w"}}
	{
		Name:         "{{$obj.Name}}",
		Usage:        "create a {{$obj.Name}}",
		ArgsUsage:    "[{{$obj.Name}}]",
		Action:       editCmd,
		BashComplete: bashEdit{{title $obj.Name}}Completer,
		Flags:        append(editFlags, Create{{title $obj.Name}}Flags...),
	},
{{ end }}{{end}}
}

var updateCommands = []cli.Command{
{{ range $obj := . }}{{if checkPerms $obj "w"}}
	{
		Name:         "{{$obj.Name}}",
		Usage:        "create a {{$obj.Name}}",
		ArgsUsage:    "[{{$obj.Name}}]",
		Action:       updateCmd,
		BashComplete: bashUpdate{{title $obj.Name}}Completer,
		Flags:        append(createFlags, Create{{title $obj.Name}}Flags...),
	},
{{ end }}{{end}}
}

var createCommands = []cli.Command{
	{
		Name:         "upload",
		Action:       createFromFile,
		BashComplete: bashFileNameCompleter,
		Usage:        "Specify comma separated file names, directory names or URLs",
		ArgsUsage:    "[filename]",
	},
{{ range $obj := . }}{{if checkPerms $obj "w"}}
	{
		Name:         "{{$obj.Name}}",
		Usage:        "create a {{$obj.Name}}",
		ArgsUsage:    "[{{$obj.Name}}]",
		Action:       createCmd,
		BashComplete: bashCreate{{title $obj.Name}}Completer,
		Flags:        append(createFlags, Create{{title $obj.Name}}Flags...),
	},
{{ end }}{{end}}
}

var readCommands = []cli.Command{
{{ range $obj := . }}
	{
		Name:         "{{$obj.Name}}",
		ArgsUsage:    " ",
		Usage:        "read specified [{{$obj.Name}}](s)",
		Action:       readCmd,
		Flags:        readFlags,
		BashComplete: bashRead{{title $obj.Name}}Completer,
	},
{{ end }}
}

var deleteCommands = []cli.Command{
{{ range $obj := . }}
	{
		Name:         "{{$obj.Name}}",
		ArgsUsage:    "[{{$obj.Name}}]",
		Usage:        "delete specified [{{$obj.Name}}](s)",
		Flags:        deleteFlags,
		Action:       deleteCmd,
		BashComplete: bashDelete{{title $obj.Name}}Completer,
	},
{{ end }}
}

var labelCommands = []cli.Command{
{{ range $obj := . }}
	{
		Name:         "{{$obj.Name}}",
		ArgsUsage:    " ",
		Usage:        "label specified [{{$obj.Name}}](s)",
		Action:       labelCmd,
		Flags:        labelFlags,
		BashComplete: bashLabel{{title $obj.Name}}Completer,
	},
{{ end }}
}

var exampleCommands = []cli.Command{
{{ range $obj := . }}
	{
		Name:         "{{$obj.Name}}",
		Usage:        "show examples of the {{$obj.Name}}",
		Flags:        exampleFlags,
		Action:       exampleCmd,
		BashComplete: bashExample{{title $obj.Name}}Completer,
	},
{{ end }}
}

var definitionCommands = []cli.Command{
{{ range $obj := . }}
	{
		Name:         "{{$obj.Name}}",
		Usage:        "show definifition of the {{$obj.Name}}",
		Flags:        definitionFlags,
		Action:       definitionCmd,
		BashComplete: bashDefinition{{title $obj.Name}}Completer,
	},
{{ end }}
}`

const cmdFlagsTmpl = `
{{ range $obj := . }}

// Create{{title $obj.Name}}Flags specifies flagsfor {{$obj.Name}} create operation
var Create{{title $obj.Name}}Flags = []cli.Flag {
	{{ range $name, $attr := $obj.CreateAttrs }}
	{{ if eq $attr "slice" }}
		cli.StringSliceFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "map" }}
		cli.StringSliceFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "string" }}
		cli.StringFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "bool" }}
		cli.BoolFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "uint" }}
		cli.UintFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "uint8" }}
		cli.UintFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "uint16" }}
		cli.UintFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "uint32" }}
		cli.UintFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "uint64" }}
		cli.UintFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "int" }}
		cli.IntFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "int8" }}
		cli.IntFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "int16" }}
		cli.IntFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "int32" }}
		cli.IntFlag{
			Name: "{{untitle $name}}",
		},
	{{ else if eq $attr "int64" }}
		cli.IntFlag{
			Name: "{{untitle $name}}",
		},
	{{ else }}
			Error: Invalid Attribute '{{$attr}}'
	{{ end }}
	{{ end }}
}
{{ end }}`

const cmdBashCompleterTmpl = `
{{ range $obj := . }}
func bashExample{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, exampleFlags)
}

func bashDefinition{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, definitionFlags)
}

func bashLabel{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, labelFlags)
	bashObject{{title $obj.Name}}Completer(c)
}

func bashRead{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, readFlags)
	bashObject{{title $obj.Name}}Completer(c)
}

func bashDelete{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, deleteFlags)
	bashObject{{title $obj.Name}}Completer(c)
}

func bashObject{{title $obj.Name}}Completer(c *cli.Context) {
	if len(c.Args()) > 0 {
		namesRe := c.Args()[len(c.Args())-1]
		namesRe = strings.Split(namesRe, ",")[0]
		c.Set("re", namesRe)
	}

	ctx := &context{cli: c, tenant: defaultTenant}
	ctx.cmd = "read"
	ctx.subcmd = "{{$obj.Name}}"

	names := getFilteredNames(ctx)

	if len(names) > 40 {
		fmt.Printf("too-many-{{$obj.Name}}s! ")
		return
	}

	for _, name := range names {
		fmt.Printf("%s ", name)
	}
}

func bashEdit{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, editFlags)
	bashObject{{title $obj.Name}}Completer(c)
}

func bashCreate{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, append(createFlags, Create{{title $obj.Name}}Flags...))

	if _, found := getLastFlagSuggestion(c, Create{{title $obj.Name}}Flags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		fmt.Printf("{{paranthesis $obj.Name}} ")
	}
}

func bashUpdate{{title $obj.Name}}Completer(c *cli.Context) {
	bashCompleter(c, []cli.Command{}, append(createFlags, Create{{title $obj.Name}}Flags...))

	if _, found := getLastFlagSuggestion(c, Create{{title $obj.Name}}Flags); found {
		return
	}

	args := c.Args()
	if len(args) <= 0 {
		bashObject{{title $obj.Name}}Completer(c)
	}
}
{{ end }}
`

const testServerTmpl = `
// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package tserver

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"path"
	"strings"

	log "github.com/Sirupsen/logrus"
	"github.com/go-martini/martini"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/kvstore/store"
	"github.com/pensando/sw/utils/runtime"
	"github.com/satori/go.uuid"
)

var (
	kvStore kvstore.Interface
)

// Start spins up a test server on the local host on the specified port
func Start(port string) {
	s := runtime.NewScheme()
{{ range $obj := . }}
	s.AddKnownTypes(&{{$obj.Package}}.{{title $obj.Name}}{}, &{{$obj.Package}}.{{title $obj.Name}}List{})
{{ end }}
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(s)}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}

	kvStore = kv

	mux := NewHTTPServer()

	log.Infof("Starting http server at %v", port)
	go mux.RunOnAddr(port)
}

// NewHTTPServer creates a http server for API endpoints.
func NewHTTPServer() *martini.ClassicMartini {
	m := martini.Classic()
{{ range $obj := . }}
	m.Post("/test"+api.Objs["{{$obj.Name}}"].URL, {{title $obj.Name}}CreateHandler)
	m.Post(api.Objs["{{$obj.Name}}"].URL, {{title $obj.Name}}CreateHandler)
	m.Put(api.Objs["{{$obj.Name}}"].URL+"/:name", {{title $obj.Name}}CreateHandler)
	m.Delete("/test"+api.Objs["{{$obj.Name}}"].URL+"/:name", {{title $obj.Name}}TestDeleteHandler)
	m.Delete(api.Objs["{{$obj.Name}}"].URL+"/:name", {{title $obj.Name}}ActualDeleteHandler)
	m.Get(api.Objs["{{$obj.Name}}"].URL+"/:name", {{title $obj.Name}}GetHandler)
	m.Get(api.Objs["{{$obj.Name}}"].URL, {{title $obj.Name}}ListHandler)
	m.Get("/watch"+api.Objs["{{$obj.Name}}"].URL, {{title $obj.Name}}sWatchHandler)
{{ end }}
	return m
}
{{ range $obj := . }}
// {{title $obj.Name}}CreateHandler creates a {{$obj.Name}}.
func {{title $obj.Name}}CreateHandler(w http.ResponseWriter, req *http.Request) (int, string) {
	decoder := json.NewDecoder(req.Body)
	defer req.Body.Close()

	{{$obj.Name}}Obj := {{$obj.Package}}.{{title $obj.Name}}{}
	old{{$obj.Name}} := {{$obj.Package}}.{{title $obj.Name}}{}
	if err := decoder.Decode(&{{$obj.Name}}Obj); err != nil {
		return http.StatusBadRequest, fmt.Sprintf("Unable to decode\n")
	}

	dryRun := false
	if strings.HasPrefix(req.URL.Path, "/test") {
		req.URL.Path = strings.TrimPrefix(req.URL.Path, "/test")
		dryRun = true
	}

	objName := {{$obj.Name}}Obj.Name
	objUUID := {{$obj.Name}}Obj.UUID
	if objName != "" {
		v, err := findUUIDByName("{{$obj.Name}}", objName)
		if err == nil {
			if objUUID != "" && v != objUUID {
				log.Infof("name '%s' is already used by uuid '%s'\n", v)
				return http.StatusNotFound, fmt.Sprintf("name %s already in use", objName)
			}
			objUUID = v
		}
	}

	if objUUID == "" {
		if objName == "" {
			return http.StatusNotFound, fmt.Sprintf("Either a name or UUID must be specified")
		}
		objUUID = uuid.NewV4().String()
		{{$obj.Name}}Obj.UUID = objUUID
	}

	update := false
	key := path.Join(api.Objs["{{$obj.Name}}"].URL, objUUID)
	if err := kvStore.Get(context.Background(), key, &old{{$obj.Name}}); err == nil {
		{{$obj.Name}}Obj.Status = old{{$obj.Name}}.Status
		{{$obj.Name}}Obj.UUID = old{{$obj.Name}}.UUID
		update = true
	}

	if err := PreCreateCallback(&{{$obj.Name}}Obj, update, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}
	log.Infof("Create key: %s object %+v", key, {{$obj.Name}}Obj)

	if dryRun {
		return http.StatusOK, fmt.Sprintf("{{title $obj.Name}} %q creation would be successful", {{$obj.Name}}Obj.Name)
	}

	if update {
		if err := kvStore.Update(context.Background(), key, &{{$obj.Name}}Obj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	} else {
		if err := kvStore.Create(context.Background(), key, &{{$obj.Name}}Obj); err != nil {
			return http.StatusBadRequest, err.Error()
		}
	}
	saveNameUUID({{$obj.Name}}Obj.Kind, objName, objUUID)

	return http.StatusOK, "{}"
}

// {{title $obj.Name}}TestDeleteHandler is
func {{title $obj.Name}}TestDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return {{title $obj.Name}}DeleteHandler(w, params, true)
}

// {{title $obj.Name}}ActualDeleteHandler is
func {{title $obj.Name}}ActualDeleteHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	return {{title $obj.Name}}DeleteHandler(w, params, false)
}

// {{title $obj.Name}}DeleteHandler is
func {{title $obj.Name}}DeleteHandler(w http.ResponseWriter, params martini.Params, dryRun bool) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("{{$obj.Name}}", objName)
		if err != nil {
			return http.StatusNotFound, fmt.Sprintf("object %s not found", objName)
		}
		objUUID = v
	}

	key := path.Join(api.Objs["{{$obj.Name}}"].URL, objUUID)
	{{$obj.Name}} := {{$obj.Package}}.{{title $obj.Name}}{}
	if err := kvStore.Get(context.Background(), key, &{{$obj.Name}}); err != nil {
		return http.StatusNotFound, fmt.Sprintf("{{title $obj.Name}} %q deletion failed: %v\n", objName, err)
	}

	if err := PreDeleteCallback(&{{$obj.Name}}, dryRun); err != nil {
		return http.StatusInternalServerError, err.Error()
	}

	if dryRun {
		return http.StatusOK, fmt.Sprintf("{{title $obj.Name}} %q creation would be successful", {{$obj.Name}}.Name)
	}

	if err := kvStore.Delete(context.Background(), key, nil); err != nil {
		return http.StatusNotFound, fmt.Sprintf("{{title $obj.Name}} %q deletion failed: %v\n", objName, err)
	}
	clearNameUUID("{{$obj.Name}}", objName, objUUID)

	out, err := json.Marshal(&{{$obj.Name}})
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// {{title $obj.Name}}GetHandler looks up a {{$obj.Name}}.
func {{title $obj.Name}}GetHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	objName := params["name"]
	objUUID := ""
	if objName != "" {
		v, err := findUUIDByName("{{$obj.Name}}", objName)
		if err != nil {
			return http.StatusNotFound, ""
		}
		objUUID = v
	}

	key := path.Join(api.Objs["{{$obj.Name}}"].URL, objUUID)

	{{$obj.Name}} := {{$obj.Package}}.{{title $obj.Name}}{}

	if err := kvStore.Get(context.Background(), key, &{{$obj.Name}}); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("{{title $obj.Name}} %q not found\n", objName)
		}
		return http.StatusInternalServerError, fmt.Sprintf("{{title $obj.Name}} %q get failed with error: %v\n", objName, err)
	}

	out, err := json.Marshal(&{{$obj.Name}})
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}
	return http.StatusOK, string(out)
}

// {{title $obj.Name}}ListHandler lists all {{$obj.Name}}s.
func {{title $obj.Name}}ListHandler(w http.ResponseWriter, params martini.Params) (int, string) {
	{{$obj.Name}}s := {{$obj.Package}}.{{title $obj.Name}}List{}

	// FIXME: URL tenant messup bug
	url := api.Objs["{{$obj.Name}}"].URL
	url = strings.Replace(url, "//", "/", -1)
	if err := kvStore.List(context.Background(), url, &{{$obj.Name}}s); err != nil {
		if kvstore.IsKeyNotFoundError(err) {
			return http.StatusNotFound, fmt.Sprintf("{{title $obj.Name}}s not found\n")
		}
		return http.StatusInternalServerError, fmt.Sprintf("{{title $obj.Name}}s list failed with error: %v\n", err)
	}

	out, err := json.Marshal(&{{$obj.Name}}s)
	if err != nil {
		return http.StatusInternalServerError, fmt.Sprintf("Failed to encode\n")
	}

	return http.StatusOK, string(out)
}

// {{title $obj.Name}}sWatchHandler establishes a watch on {{$obj.Name}}s hierarchy.
func {{title $obj.Name}}sWatchHandler(w http.ResponseWriter, req *http.Request) {
	notifier, ok := w.(http.CloseNotifier)
	if !ok {
		return
	}
	flusher, ok := w.(http.Flusher)
	if !ok {
		return
	}
	encoder := json.NewEncoder(w)
	watcher, err := kvStore.PrefixWatch(context.Background(), api.Objs["{{$obj.Name}}"].URL, "0")
	if err != nil {
		return
	}
	ch := watcher.EventChan()
	for {
		select {
		case event, ok := <-ch:
			if !ok {
				return
			}

			if err := encoder.Encode(event); err != nil {
				return
			}
			if len(ch) == 0 {
				flusher.Flush()
			}
		case <-notifier.CloseNotify():
			return
		}
	}
}
{{ end }}
`

const testserverObjTmpl = `
// (c) Pensando Systems, Inc.
// This is a generated file, please do not hand edit !!

package tserver

import (
	"context"
	"path"

	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
)

{{ range $obj := . }}
// Get{{title $obj.Name}}ByName is
func Get{{title $obj.Name}}ByName(objName string) (*{{$obj.Package}}.{{title $obj.Name}}, error) {
	obj, err := GetObjByName("{{$obj.Name}}", objName)
	if err != nil {
		return nil, err
	}
	{{$obj.Name}}Obj := obj.(*{{$obj.Package}}.{{title $obj.Name}})
	return {{$obj.Name}}Obj, nil
}
{{ end }}

{{ range $obj := . }}
// Update{{title $obj.Name}} is
func Update{{title $obj.Name}}(obj *{{$obj.Package}}.{{title $obj.Name}}) error {
	uuidStr, err := findUUIDByName(obj.Kind, obj.Name)
	if err != nil {
		return err
	}
	key := path.Join(api.Objs["{{$obj.Name}}"].URL, uuidStr)
	err = kvStore.Update(context.Background(), key, obj)
	if err != nil {
		return err
	}
	return nil
}
{{ end }}

func getEmptyObj(kind string) (obj interface{}, objList interface{}) {
	switch kind {
{{ range $obj := . }}
	case "{{$obj.Name}}":
		return &{{$obj.Package}}.{{title $obj.Name}}{}, &{{$obj.Package}}.{{title $obj.Name}}List{}
{{ end }}
	}
	return nil, nil
}

func getObjFromList(objList interface{}, idx int) interface{} {
{{ range $obj := . }}
	if ol, ok := objList.(*{{$obj.Package}}.{{title $obj.Name}}List); ok {
		{{$obj.Name}} := ol.Items[idx]
		return &{{$obj.Name}}
	}
{{ end }}
	return nil
}
`
