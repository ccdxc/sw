package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"text/template"

	"github.com/pensando/sw/venice/cli/api"
	"github.com/pensando/sw/venice/cli/gen/pregen"
	"github.com/pensando/sw/venice/utils/ref"
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
	Structs     []string
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

func catFile(inp string) string {
	bytes, err := ioutil.ReadFile(inp)
	if err != nil {
		return "error reading file" + inp
	}
	return string(bytes)
}

// map of functions used by go templating for use within code generation
var funcMap = template.FuncMap{
	"title":        strings.Title,
	"untitle":      unTitle,
	"paranthesis":  paranthesis,
	"checkPerms":   checkPerms,
	"visited":      visited,
	"cleanVisited": cleanVisited,
	"include":      catFile,
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
			if err := genFromTemplates("./pregen/genobjs.go", "./templates/localObj.tmpl"); err != nil {
				fmt.Printf("Error: %s\n", err)
			}
		} else {
			fmt.Printf("Invalid args... %s", os.Args)
		}
		return
	}

	// step 2: code generation (requires pregeneration)
	if err := genFromTemplates("../genobjs.go", "./templates/cliObj.tmpl", "./templates/cliObjOrder.tmpl"); err != nil {
		fmt.Printf("Error: %s\n", err)
	}
	if err := genFromTemplates("../testserver/tserver/genobjs.go", "./templates/testServerObj.tmpl"); err != nil {
		fmt.Printf("Error: %s\n", err)
	}
	cleanAPIObjs()
	if err := genFromTemplates("../gencmds.go", "./templates/cmds.tmpl", "./templates/cmdFlags.tmpl", "./templates/cmdBashCompleter.tmpl"); err != nil {
		fmt.Printf("Error: %s\n", err)
	}
	if err := genFromTemplates("../testserver/tserver/tserver.go", "./templates/testServer.tmpl"); err != nil {
		fmt.Printf("Error: %s\n", err)
	}
	genYmlExamples()
}

// generate templates takes input template file(s) and generate the output file for various objects
func genFromTemplates(ofile string, ifiles ...string) error {
	f, err := os.Create(ofile)
	if err != nil {
		return fmt.Errorf("%s", err)
	}
	defer f.Close()

	wr := bufio.NewWriter(f)

	for _, ifile := range ifiles {
		tmplBytes, err := ioutil.ReadFile(ifile)
		if err != nil {
			return fmt.Errorf("%s", err)
		}
		t := template.Must(template.New(ifile).Funcs(funcMap).Parse(string(tmplBytes)))
		t.Execute(wr, genObjs)
	}

	wr.Flush()
	return nil
}

func cleanAPIObjs() {
	refCtx := &ref.RfCtx{GetSubObj: pregen.GetSubObj, UseJSONTag: true}

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
}

const initialBlob = `package vcli

var exampleYmls = map[string]string{
`

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
