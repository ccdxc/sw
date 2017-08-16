package plugin

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/golang/glog"

	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	descriptor "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"
	venice "github.com/pensando/sw/utils/apigen/annotations"
	mutator "github.com/pensando/sw/utils/apigen/autogrpc"
)

var (
	errIncompletePath = errors.New("incomplete path specification")
	errInvalidField   = errors.New("invalid field specification")
	errInvalidOption  = errors.New("invalid option specification")
)

func parseStringOptions(val interface{}) (interface{}, error) {
	v, ok := val.(*string)
	if !ok {
		return nil, errInvalidOption
	}
	return *v, nil
}

func parseStringSliceOptions(val interface{}) (interface{}, error) {
	v, ok := val.([]string)
	if !ok {
		return nil, errInvalidOption
	}
	return v, nil
}

func parseInt32Options(val interface{}) (interface{}, error) {
	v, ok := val.(*int32)
	if !ok {
		return nil, errInvalidOption
	}
	return *v, nil
}

func parseObjRelation(val interface{}) (interface{}, error) {
	v, ok := val.(*venice.ObjectRln)
	if !ok {
		return nil, errInvalidOption
	}
	return *v, nil
}

func parseBoolOptions(val interface{}) (interface{}, error) {
	v, ok := val.(*bool)
	if !ok {
		return nil, errInvalidOption
	}
	return *v, nil
}

func parseRestServiceOption(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.RestEndpoint)
	if !ok {
		return nil, errInvalidOption
	}
	return *c, nil
}

func parseGoogleAPIHTTP(val interface{}) (interface{}, error) {
	c, ok := val.(*googapi.HttpRule)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

// ServiceParams is the parameters related to the Service used by templates
type ServiceParams struct {
	// Version is the version of the Service
	Version string
	// Prefix is the prefix for all the resources served by the service.
	Prefix string
}

// MethodParams is the parameters related to a method used by templates
type MethodParams struct {
	// GrpcOnly marks if this method has no REST bindings.
	GrpcOnly bool
	// Oper specifies the type of CRUD operation, valid with GrpcOnly flag.
	Oper string
}

// KeyComponent is a component of the key path, derived from the objectPrefix (and
// objectIdentifier) option(s).
type KeyComponent struct {
	// Type of the compoent - prefix or field
	Type string
	// Val holds a string literal or a field name depending on type
	Val string
}

//--- Functions registered in the funcMap for the plugin  ---//

// findComponentsHelper is a helper function to derive a KeyComponent slice from a string.
func findComponentsHelper(m *descriptor.Message, input string) ([]KeyComponent, error) {
	var output []KeyComponent
	in := input
	for len(in) > 0 {
		s := strings.SplitN(in, "{", 2)
		if s[0] != "" {
			output = append(output, KeyComponent{Type: "prefix", Val: s[0]})
		}
		if len(s) < 2 {
			break
		}

		s = strings.SplitN(s[1], "}", 2)
		if len(s) < 2 {
			glog.V(1).Infof("Found incomplete path processing [%s]", input)
			return output, errIncompletePath
		}
		var finalMd *descriptor.Message
		var field string
		var ok, embedded bool
		if i := strings.LastIndex(s[0], "."); i != -1 {
			pre := s[0][:i]
			field = s[0][i+1:]
			glog.V(1).Infof("Found Pre/Field to be %v/%v", pre, field)

			finalMd, embedded, ok = m.FindMessageField(pre)
			if !ok {
				glog.V(1).Infof("Did not find %v in [%v]", pre, m.GetName())
				return nil, errInvalidField
			}
		} else {
			finalMd = m
			field = s[0]
		}

		if _, _, ok := finalMd.FindMessageField(field); ok {
			glog.V(1).Infof("Found #1 Pre/Field to be %v/%v", field, embedded)
			if embedded {
				output = append(output, KeyComponent{Type: "field", Val: field})
			} else {
				output = append(output, KeyComponent{Type: "field", Val: s[0]})
			}
		} else {
			glog.Errorf("Invalid Field [%s]", field)
			return output, errInvalidField
		}
		if s[0] == "" {
			glog.V(1).Infof("Found incomplete path processing [%s]", input)
			return output, errIncompletePath
		}
		in = s[1]
	}
	return output, nil
}

// getDbKey returns a slice of KeyComponents for the message that is used by
// templates to construct the key.
func getDbKey(m *descriptor.Message) ([]KeyComponent, error) {
	var output []KeyComponent
	var err error
	dbpath, err := reg.GetExtension("venice.objectPrefix", m)
	if err != nil {
		return output, nil
	}
	in := dbpath.(string)
	if len(in) == 0 {
		return output, nil
	}
	if !strings.HasSuffix(in, "/") {
		in = in + "/"
	}

	if output, err = findComponentsHelper(m, in); err == nil {
		// The key generated is /venice/<service prefix>/<object prefix>/<name from object meta>
		output = append(output, KeyComponent{Type: "field", Val: "Name"})
	}

	glog.V(1).Infof("Built DB key [ %v ](%v)", output, err)
	return output, err
}

func getURIKey(m *descriptor.Method) ([]KeyComponent, error) {
	var output []KeyComponent

	params, err := getMethodParams(m)
	if err != nil {
		return output, err
	}
	msg := m.RequestType
	if params.Oper == "ListOper" || params.Oper == "WatchOper" {
		msgtype := ""
		if params.Oper == "ListOper" {
			msgtype, err = getListType(m.ResponseType, true)
			if err != nil {
				return output, err
			}
			msgtype = "." + msgtype
		}
		if params.Oper == "WatchOper" {
			msgtype, err = getWatchType(m.ResponseType, true)
			if err != nil {
				return output, err
			}
			msgtype = "." + msgtype
		}
		msg, err = m.Service.File.Reg.LookupMsg("", msgtype)
		if err != nil {
			return output, err
		}
	}
	r, err := reg.GetExtension("google.api.http", m)
	if err != nil {
		return output, err
	}
	rule := r.(*googapi.HttpRule)
	pattern := ""
	switch params.Oper {
	case "CreateOper":
		pattern = rule.GetPost()
	case "GetOper", "ListOper":
		pattern = rule.GetGet()
	case "DeleteOper":
		pattern = rule.GetDelete()
	case "UpdateOper":
		pattern = rule.GetPut()
	}
	if output, err = findComponentsHelper(msg, pattern); err != nil {
		return output, err
	}
	return output, nil
}

// getSvcParams returns the ServiceParams for the service.
//   Parameters could be initialized to defaults if options were
//   not specified by the user in service.proto.
func getSvcParams(s *descriptor.Service) (ServiceParams, error) {
	var params ServiceParams
	var ok bool
	i, err := reg.GetExtension("venice.apiVersion", s)
	if params.Version, ok = i.(string); err != nil || !ok {
		// Can specify a defaults when not specified.
		params.Version = ""
	}
	i, err = reg.GetExtension("venice.apiPrefix", s)
	if params.Prefix, ok = i.(string); err != nil || !ok {
		params.Prefix = ""
	}
	return params, nil
}

// getMethodParams returns params for the method in a MethodParams object.
//   Parameters could be initialized to defaults if options were
//   not specified by the user in service.proto.
func getMethodParams(m *descriptor.Method) (MethodParams, error) {
	var params MethodParams
	params.GrpcOnly = true
	ok := false
	i, err := reg.GetExtension("venice.methodOper", m)
	if err != nil {
		glog.V(1).Infof("GrpcOnly but no Method specified(%s)", *m.Name)
		return params, nil
	}
	if params.Oper, ok = i.(string); !ok {
		return params, errInvalidOption
	}
	switch strings.ToLower(params.Oper) {
	case "create":
		params.Oper = "CreateOper"
	case "update":
		params.Oper = "UpdateOper"
	case "get":
		params.Oper = "GetOper"
	case "delete":
		params.Oper = "DeleteOper"
	case "list":
		params.Oper = "ListOper"
	case "watch":
		params.Oper = "WatchOper"
	default:
		return params, errInvalidOption
	}
	return params, nil
}

func getSwaggerFileName(file string) (string, error) {
	file = filepath.Base(file)
	if strings.HasSuffix(file, ".proto") {
		f := strings.TrimSuffix(file, ".proto")
		return f + ".swagger.json", nil
	}
	return "", errInvalidOption
}

// getCWD2 retuns the cwd working directory but qualified by the parent directory.
func getCWD2() string {
	cwd, err := os.Getwd()
	if err != nil {
		return ""
	}
	return filepath.Base(filepath.Dir(cwd)) + "/" + filepath.Base(cwd)
}

// createDir creates a directory at the base given.
func createDir(base string, dirs ...string) error {
	name := base
	for _, d := range dirs {
		name = name + "/" + d
	}
	os.MkdirAll(name, 0744)
	return nil
}

func parseManifestFile(raw []byte) map[string]string {
	manifest := make(map[string]string)
	lines := bytes.Split(raw, []byte("\n"))
	for _, line := range lines {
		fields := bytes.Fields(line)
		if len(fields) == 2 {
			manifest[string(fields[0])] = string(fields[1])
		}
	}
	return manifest
}

// genManifest generates the current manifest of protos being processed.
func genManifest(path, pkg, file string) (map[string]string, error) {
	var manifest map[string]string
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		manifest = make(map[string]string)
	} else {
		glog.V(1).Infof("manifest exists, reading from manifest")
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return nil, err
		}
		manifest = parseManifestFile(raw)
	}
	file = filepath.Base(file)
	manifest[file] = pkg
	return manifest, nil
}

//
type packageDef struct {
	Svcs map[string]serviceDef
}

type serviceDef struct {
	Version  string
	Messages []string
}

func genServiceManifest(filenm string, file *descriptor.File) (map[string]packageDef, error) {
	manifest := make(map[string]packageDef)
	if _, err := os.Stat(filenm); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", filenm)
	} else {
		glog.V(1).Infof("manifest exists, reading from manifest")
		raw, err := ioutil.ReadFile(filenm)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return nil, err
		}
		err = json.Unmarshal(raw, &manifest)
		if err != nil {
			glog.V(1).Infof("Json Unmarshall of svc manifest file failed ignoring current file")
		}
	}
	pkg := file.GoPkg.Name
	pkgdef := packageDef{Svcs: make(map[string]serviceDef)}

	for _, svc := range file.Services {
		ver, err := reg.GetExtension("venice.apiVersion", svc)
		if err != nil {
			glog.V(1).Infof("unversioned service, ingnoring for svc manifest [%s](%s)", svc.Name, err)
			return manifest, nil
		}
		svcdef := serviceDef{
			Version: ver.(string),
		}
		crudsvc, err := reg.GetExtension("venice.apiGrpcCrudService", svc)
		if err != nil {
			glog.V(1).Infof("no crudService specified for [%s](%s)", *svc.Name, err)
		} else {
			glog.V(1).Infof("Found crudsvcs %v", crudsvc.([]string))
			svcdef.Messages = crudsvc.([]string)
		}
		if len(svcdef.Messages) > 0 {
			pkgdef.Svcs[*svc.Name] = svcdef
		}
	}
	if len(pkgdef.Svcs) > 0 {
		manifest[pkg] = pkgdef
	}
	ret, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		glog.Fatalf("failed to marshal service manifest")
	}

	cwd, err := os.Getwd()
	fl, err := os.Create(filenm)
	if err != nil {
		glog.Fatalf("could not create service manifest file(%s) cwd (%s)", err, cwd)
	}
	defer fl.Close()
	_, err = fl.WriteString(string(ret))
	if err != nil {
		glog.Fatalf("could not write service manifest file")
	}
	return manifest, nil
}

// relationRef is reference to relations
type relationRef struct {
	Type  string
	To    string
	Field string
}

var relMap = make(map[string][]relationRef)

func addRelations(f *descriptor.Field) error {
	if r, err := reg.GetExtension("venice.objRelation", f); err == nil {
		glog.V(1).Infof("Checking relation to %s", *f.Name)
		name := f.Message.File.GoPkg.Name + "." + *f.Message.Name
		if fr, ok := r.(venice.ObjectRln); ok {
			glog.V(1).Infof("adding relation to %s.%s", name, *f.Name)
			m := relationRef{Type: fr.Type, To: fr.To, Field: *f.Name}
			relMap[name] = append(relMap[name], m)
		}
	} else {
		glog.V(1).Infof("relations.. not found on %s", *f.Name)
	}
	return nil
}

func genRelMap(path string) (string, error) {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("relationRef [%s] not found", path)
	} else {
		glog.V(1).Infof("relationRef exists, reading from manifest")
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Relation failed (%s)", err)
			return "", err
		}
		rmap := make(map[string][]relationRef)
		err = json.Unmarshal(raw, &rmap)
		if err != nil {
			glog.V(1).Infof("Json Unmarshall of rel map file failed ignoring current file")
		} else {
			for k, v := range rmap {
				if _, ok := relMap[k]; ok {
					relMap[k] = append(relMap[k], v...)
				} else {
					relMap[k] = v
				}
			}
		}
	}
	if len(relMap) > 0 {
		ret, err := json.MarshalIndent(relMap, "", "  ")
		if err != nil {
			glog.V(1).Infof("Failed to marshall output rel map")
			return "", err
		}
		str := string(ret[:])
		glog.V(1).Infof("Generated Relations.json %v", str)
		return str, nil
	}
	return "{}", nil
}

// Atlease 2 character long.
func detitle(str string) (string, error) {
	if len(str) > 1 {
		ret := strings.ToLower(string(str[0])) + string(str[1:])
		return ret, nil
	}
	return str, errors.New("too short")
}

func getInputType(pkg string, method *descriptor.Method, fq bool) string {
	if method.RequestType.File.GoPkg.Name == pkg {
		glog.V(1).Infof("Evaluating is same %s [%s]", pkg, method.RequestType.File.GoPkg.Name)
		if fq {
			return fmt.Sprintf("%s.%s", pkg, *method.RequestType.Name)
		}
		return *method.RequestType.Name
	}
	r := method.InputType
	ret := strings.TrimPrefix(*r, ".")
	return ret
}

func getOutputType(pkg string, method *descriptor.Method, fq bool) string {
	if method.ResponseType.File.GoPkg.Name == pkg {
		if fq {
			return fmt.Sprintf("%s.%s", pkg, *method.ResponseType.Name)
		}
		return *method.ResponseType.Name
	}
	r := method.OutputType
	ret := strings.TrimPrefix(*r, ".")
	return ret
}

func getListType(msg *descriptor.Message, fq bool) (string, error) {
	for _, f := range msg.Fields {
		if *f.Name == "Items" {
			if fq {
				return strings.TrimPrefix(*f.TypeName, "."), nil
			}
			parts := strings.Split(*f.TypeName, ".")
			return parts[len(parts)-1], nil
		}
	}
	return "", errors.New("list item not found")
}

func getWatchType(msg *descriptor.Message, fq bool) (string, error) {
	for _, f := range msg.Fields {
		if *f.Name == "Object" {
			if fq {
				return strings.TrimPrefix(*f.TypeName, "."), nil
			}
			parts := strings.Split(*f.TypeName, ".")
			return parts[len(parts)-1], nil
		}
	}
	return "", errors.New("Object item not found")
}

func isListHelper(msg *descriptor.Message) bool {
	v, err := reg.GetExtension("venice.objectAutoGen", msg)
	if err == nil {
		glog.V(1).Infof("Found Extension %s", v.(string))
		return (v.(string) == "listhelper")
	}
	glog.V(1).Infof("Extension not found (%s)", err)

	return false
}

func isWatchHelper(msg *descriptor.Message) bool {
	if v, err := reg.GetExtension("venice.objectAutoGen", msg); err == nil {
		return (v.(string) == "watchhelper")
	}
	return false
}

func isAutoList(meth *descriptor.Method) bool {
	if v, err := reg.GetExtension("venice.methodAutoGen", meth); err == nil {
		if v.(bool) == false {
			return false
		}
		if v1, err := reg.GetExtension("venice.methodOper", meth); err == nil {
			if v1.(string) == "list" {
				return true
			}
		}
	}
	return false
}

func isAutoWatch(meth *descriptor.Method) bool {
	if v, err := reg.GetExtension("venice.methodAutoGen", meth); err == nil {
		if v.(bool) == false {
			return false
		}
		if v1, err := reg.GetExtension("venice.methodOper", meth); err == nil {
			if v1.(string) == "watch" {
				return true
			}
		}
	}
	return false
}

func getPackageCrudObjects(file *descriptor.File) ([]string, error) {
	var crudmap = make(map[string]bool)
	for _, svc := range file.Services {
		cruds, err := reg.GetExtension("venice.apiGrpcCrudService", svc)
		if err == nil {
			for _, v := range cruds.([]string) {
				crudmap[v] = true
			}
		}
	}
	var ret []string
	for k := range crudmap {
		ret = append(ret, k)
	}
	sort.Strings(ret)
	return ret, nil
}

func isAutoGenMethod(meth *descriptor.Method) bool {
	if v, err := reg.GetExtension("venice.methodAutoGen", meth); err == nil {
		return v.(bool)
	}
	return false
}

func isRestExposed(meth *descriptor.Method) bool {
	glog.V(1).Infof("Checking for rest exposed for %s\n", *meth.Name)
	if _, err := reg.GetExtension("google.api.http", meth); err == nil {
		return true
	}
	return false
}

func isRestMethod(svc *descriptor.Service, oper, object string) bool {
	method := oper + object
	for _, v := range svc.Methods {
		if *v.Name == method {
			return isRestExposed(v)
		}
	}
	return false
}

// isNestedMessage checks if the message is a nested message
//  TODO(sanjayt): make this generic for all nested message definitions.
func isNestedMessage(msg *descriptor.Message) bool {
	glog.V(1).Infof("Looking for map_entry in %s)", *msg.Name)
	if opt := msg.GetOptions(); opt != nil {
		return opt.GetMapEntry()
	}
	return false
}

func getAutoRestOper(meth *descriptor.Method) (string, error) {
	if v, err := reg.GetExtension("venice.methodAutoGen", meth); err == nil {
		if v.(bool) == false {
			return "", errors.New("not an autogen method")
		}
		if v1, err := reg.GetExtension("venice.methodOper", meth); err == nil {
			switch v1.(string) {
			case "create":
				return "POST", nil
			case "update":
				return "PUT", nil
			case "get", "list":
				return "GET", nil
			case "delete":
				return "DELETE", nil
			case "watch":
				return "", errors.New("not supported method")
			default:
				return "", errors.New("not supported method")
			}
		}
	}
	return "", errors.New("not an autogen method")
}

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func init() {
	// Register Option Parsers
	reg.RegisterOptionParser("venice.fileGrpcDest", parseStringOptions)
	reg.RegisterOptionParser("venice.apiProfile", parseStringOptions)
	reg.RegisterOptionParser("venice.apiVersion", parseStringOptions)
	reg.RegisterOptionParser("venice.apiGrpcCrudService", parseStringSliceOptions)
	reg.RegisterOptionParser("venice.apiPrefix", parseStringOptions)
	reg.RegisterOptionParser("venice.methodProfile", parseStringOptions)
	reg.RegisterOptionParser("venice.methodOper", parseStringOptions)
	reg.RegisterOptionParser("venice.methodAutoGen", parseBoolOptions)
	reg.RegisterOptionParser("venice.objectIdentifier", parseStringOptions)
	reg.RegisterOptionParser("venice.objectAutoGen", parseStringOptions)
	reg.RegisterOptionParser("venice.objectPrefix", parseStringOptions)
	reg.RegisterOptionParser("venice.objRelation", parseObjRelation)
	reg.RegisterOptionParser("google.api.http", parseGoogleAPIHTTP)

	// Register Functions
	reg.RegisterFunc("getDbKey", getDbKey)
	reg.RegisterFunc("getURIKey", getURIKey)
	reg.RegisterFunc("getSvcParams", getSvcParams)
	reg.RegisterFunc("getMethodParams", getMethodParams)
	reg.RegisterFunc("getCWD2", getCWD2)
	reg.RegisterFunc("getSwaggerFileName", getSwaggerFileName)
	reg.RegisterFunc("createDir", createDir)
	reg.RegisterFunc("genManifest", genManifest)
	reg.RegisterFunc("genSvcManifest", genServiceManifest)
	reg.RegisterFunc("addRelations", addRelations)
	reg.RegisterFunc("genRelMap", genRelMap)
	reg.RegisterFunc("title", strings.Title)
	reg.RegisterFunc("detitle", detitle)
	reg.RegisterFunc("getInputType", getInputType)
	reg.RegisterFunc("getOutputType", getOutputType)
	reg.RegisterFunc("getListType", getListType)
	reg.RegisterFunc("getWatchType", getWatchType)
	reg.RegisterFunc("isAutoWatch", isAutoWatch)
	reg.RegisterFunc("isAutoList", isAutoList)
	reg.RegisterFunc("isWatchHelper", isWatchHelper)
	reg.RegisterFunc("isListHelper", isListHelper)
	reg.RegisterFunc("getPackageCrudObjects", getPackageCrudObjects)
	reg.RegisterFunc("isAutoGenMethod", isAutoGenMethod)
	reg.RegisterFunc("getAutoRestOper", getAutoRestOper)
	reg.RegisterFunc("isRestExposed", isRestExposed)
	reg.RegisterFunc("isRestMethod", isRestMethod)
	reg.RegisterFunc("isNestedMessage", isNestedMessage)

	// Register request mutators
	reg.RegisterReqMutator("pensando", reqMutator)
}
