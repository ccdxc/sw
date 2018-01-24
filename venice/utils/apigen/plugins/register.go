package plugin

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"github.com/golang/glog"

	govldtr "github.com/asaskevich/govalidator"
	gogoproto "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	descriptor "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
	mutator "github.com/pensando/sw/venice/utils/apigen/autogrpc"
)

var (
	errIncompletePath = errors.New("incomplete path specification")
	errInvalidField   = errors.New("invalid field specification")
	errInvalidOption  = errors.New("invalid option specification")
)

type scratchVars struct {
	B   [3]bool
	Int [3]int
}

var scratch scratchVars

func (s *scratchVars) setBool(val bool, id int) bool {
	s.B[id] = val
	// Dummy return to satisfy templates
	return val
}

func (s *scratchVars) getBool(id int) bool {
	return s.B[id]
}

func (s *scratchVars) setInt(val int, id int) int {
	s.Int[id] = val
	return val
}

func (s *scratchVars) getInt(id int) int {
	return s.Int[id]
}

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

func parseNaplesRestService(val interface{}) (interface{}, error) {
	c, ok := val.([]*venice.RestEndpoint)
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

// RestServiceOptions holds raw REST options data from .proto files
type RestServiceOptions struct {
	CrudObject string
	Methods    []string
	Pattern    string
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

// getRestSvcOptions returns the ServiceOptions for the service. This call will ensure that the raw venice.naplesRestService
// is passed to the templating logic for customization. This will also avoid generating the *.pb.gw files if we don't
// want them.
func getRestSvcOptions(s *descriptor.Service) ([]RestServiceOptions, error) {
	var restOptions []RestServiceOptions
	//var ok bool
	i, _ := reg.GetExtension("venice.naplesRestService", s)
	for _, r := range i.([]*venice.RestEndpoint) {
		var restService RestServiceOptions
		restService.CrudObject = r.Object
		restService.Methods = r.Method
		restService.Pattern = r.Pattern
		restOptions = append(restOptions, restService)
	}
	glog.V(1).Infof("RestAPIParsing yielded : %#v", restOptions)
	return restOptions, nil
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

type manifestFile struct {
	Pkg       string
	APIServer bool
}

func parseManifestFile(raw []byte) map[string]manifestFile {
	manifest := make(map[string]manifestFile)
	lines := bytes.Split(raw, []byte("\n"))
	for _, line := range lines {
		fields := bytes.Fields(line)
		if len(fields) == 3 {
			apiserver, err := strconv.ParseBool(string(fields[2]))
			if err == nil {
				manifest[string(fields[0])] = manifestFile{
					Pkg:       string(fields[1]),
					APIServer: apiserver,
				}
			}
		}
	}
	return manifest
}

// genManifest generates the current manifest of protos being processed.
func genManifest(desc *descriptor.File, path, pkg, file string) (map[string]manifestFile, error) {
	var manifest map[string]manifestFile
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		manifest = make(map[string]manifestFile)
	} else {
		glog.V(1).Infof("manifest exists, reading from manifest")
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return nil, err
		}
		manifest = parseManifestFile(raw)
	}
	apiserver, _ := isAPIServerServed(desc)
	file = filepath.Base(file)
	manifest[file] = manifestFile{
		Pkg:       pkg,
		APIServer: apiserver,
	}
	return manifest, nil
}

//
type packageDef struct {
	Svcs  map[string]serviceDef
	Files []string
}

type serviceDef struct {
	Version  string
	Messages []string
}

// getServiceManifest retrieves the manifest from file specified in arg
func getServiceManifest(filenm string) (map[string]packageDef, error) {
	manifest := make(map[string]packageDef)
	if _, err := os.Stat(filenm); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", filenm)
		return manifest, nil
	}
	glog.V(1).Infof("manifest exists, reading from manifest")
	raw, err := ioutil.ReadFile(filenm)
	if err != nil {
		glog.V(1).Infof("Reading Manifest failed (%s)", err)
		return nil, err
	}
	err = json.Unmarshal(raw, &manifest)
	if err != nil {
		glog.V(1).Infof("Json Unmarshall of svc manifest file failed ignoring current file")
		return nil, err
	}
	glog.V(1).Infof("manifest has %v packages", len(manifest))
	return manifest, nil
}

// genServiceManifest generates a service manifest at the location specified.
//  The manifest is created in an additive fashion since it is called once per protofile.
func genServiceManifest(filenm string, file *descriptor.File) (string, error) {
	manifest, err := getServiceManifest(filenm)
	if err != nil {
		return "", err
	}
	pkg := file.GoPkg.Name
	var pkgdef packageDef
	ok := false
	if pkgdef, ok = manifest[pkg]; !ok {
		pkgdef = packageDef{Svcs: make(map[string]serviceDef)}
	}
	pkgdef.Files = append(pkgdef.Files, *file.Name)
	for _, svc := range file.Services {
		ver, err := reg.GetExtension("venice.apiVersion", svc)
		if err != nil {
			glog.V(1).Infof("unversioned service, ingnoring for svc manifest [%s](%s)", svc.Name, err)
			continue
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

	return string(ret), nil
}

type checkArgs func(string) bool

var validatorArgMap = map[string][]checkArgs{
	"StrEnum":  {isString},
	"StrLen":   {govldtr.IsInt, govldtr.IsInt},
	"IntRange": {govldtr.IsInt, govldtr.IsInt},
	"IPAddr":   {},
	"IPv4":     {},
	"HostAddr": {},
	"MacAddr":  {},
	"URI":      {},
	"UUID":     {},
}

type validateArg struct {
	Tpe  string
	Str  string
	Intg uint64
}

type validateField struct {
	Fn   string
	Ver  string
	Args []string
}

type validateFields struct {
	Repeated   bool
	Pointer    bool
	Validators []validateField
}
type validateMsg struct {
	Fields map[string]validateFields
}

type validators struct {
	Fmap bool
	Map  map[string]validateMsg
}

func isString(in string) bool {
	if len(in) != 0 {
		return true
	}
	return false
}
func checkValidators(file *descriptor.File, msgmap map[string]bool, name string) bool {
	if _, ok := msgmap[name]; ok {
		return msgmap[name]
	}
	m, err := file.Reg.LookupMsg("", name)
	if err != nil {
		glog.Fatalf("Failed to retrieve message %s", name)
	}
	glog.Infof(" checkValidators on %s", name)
	found := false
	for _, fld := range m.Fields {
		_, err := reg.GetExtension("venice.check", fld)
		if err == nil {
			found = true
		}
		if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
			found = found || checkValidators(file, msgmap, *fld.TypeName)
		}
	}
	msgmap[name] = found
	if found {
		glog.Infof(" checkValidators found true %s", name)
	}
	return found
}

func parseValidator(in string) (validateField, error) {
	ret := validateField{}
	re := regexp.MustCompile("(?P<ver>[a-zA-Z0-9_\\-\\*]*\\:)?(?P<func>[a-zA-Z0-9_\\-]+)\\((?P<args>[a-zA-Z0-9_\\-\\, \\.\\:]*)*\\)")
	params := re.FindStringSubmatch(in)
	if params == nil {
		return ret, fmt.Errorf("Failed to parse validator [%s]", in)
	}

	if params[1] != "" && string(params[1][len(params[1])-1]) == ":" {
		ret.Ver = params[1][:len(params[1])-1]
		if ret.Ver == "*" {
			ret.Ver = "all"
		}
	} else {
		ret.Ver = "all"
	}
	ret.Fn = params[2]
	ret.Args = strings.Split(strings.Replace(params[3], " ", "", -1), ",")
	if len(ret.Args) == 1 && ret.Args[0] == params[3] && params[3] == "" {
		ret.Args = []string{}
	}
	if vargs, ok := validatorArgMap[ret.Fn]; ok {
		if len(vargs) != len(ret.Args) {
			return validateField{}, fmt.Errorf("Incorrect number of args (%d) for %s", len(ret.Args), ret.Fn)
		}
		for i := range vargs {
			if !vargs[i](ret.Args[i]) {
				return validateField{}, fmt.Errorf("validation for arg(%s) failed for %s", ret.Args[i], ret.Fn)
			}
		}
	} else {
		return validateField{}, fmt.Errorf("unknown validator %s", ret.Fn)
	}
	return ret, nil
}

func getValidatorManifest(file *descriptor.File) (validators, error) {
	ret := validators{Map: make(map[string]validateMsg)}
	msgmap := make(map[string]bool)
	// freg := file.Reg
	for _, msg := range file.Messages {
		for _, fld := range msg.Fields {
			r, err := reg.GetExtension("venice.check", fld)
			glog.Infof(" Check validator for message %s.%s", *msg.Name, *fld.Name)

			if err == nil {
				glog.Infof(" Found validator for message %s.%s,[ %v ]", *msg.Name, *fld.Name, r)
				if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
					return ret, fmt.Errorf("Validators allowed on scalar types only [%s]", *fld.Name)
				}
				if _, ok := ret.Map[*msg.Name]; !ok {
					ret.Map[*msg.Name] = validateMsg{Fields: make(map[string]validateFields)}
				}
				repeated := false
				if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
					repeated = true
				}
				ret.Map[*msg.Name].Fields[*fld.Name] = validateFields{Repeated: repeated}

				for _, v := range r.([]string) {
					fldv, err := parseValidator(v)
					if err != nil {
						return ret, err
					}
					vfld := ret.Map[*msg.Name].Fields[*fld.Name]
					vfld.Validators = append(vfld.Validators, fldv)
					ret.Map[*msg.Name].Fields[*fld.Name] = vfld
				}

				ret.Fmap = true
			} else {
				glog.Infof("Failed %s", err)
			}
			if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
				glog.Infof("checking validators for nested message (%s).%s", *msg.Name, *fld.Name)
				if _, ok := msgmap[*fld.TypeName]; !ok {
					msgmap[*fld.TypeName] = checkValidators(file, msgmap, *fld.TypeName)
				}
				if msgmap[*fld.TypeName] == true {
					if _, ok := ret.Map[*msg.Name]; !ok {
						ret.Map[*msg.Name] = validateMsg{Fields: make(map[string]validateFields)}
					}
					if _, ok := ret.Map[*fld.Name]; !ok {
						repeated := false
						pointer := true
						if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
							repeated = true
						}
						if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
							glog.Infof("setting pointer found nullable [%v] for %s]", r, *msg.Name+"/"+*fld.Name)
							pointer = r.(bool)
						} else {
						}
						glog.Infof("setting pointer to [%v] for {%s]", pointer, *msg.Name+"/"+*fld.Name)

						// if it is a embedded field, do not use field name rather use type
						if fld.Embedded {
							// fld.GetTypeName() -> e.g. ".events.EventAttributes"
							temp := strings.Split(fld.GetTypeName(), ".")
							fldType := temp[len(temp)-1]
							ret.Map[*msg.Name].Fields[fldType] = validateFields{Validators: make([]validateField, 0), Repeated: repeated, Pointer: pointer}
						} else {
							ret.Map[*msg.Name].Fields[*fld.Name] = validateFields{Validators: make([]validateField, 0), Repeated: repeated, Pointer: pointer}
						}
					}
				}
			}
		}
	}
	glog.Infof("Validator Manifest is %+v", ret)
	return ret, nil
}

func derefStr(in *string) string {
	return *in
}

func getEnumStrMap(file *descriptor.File, in []string) (string, error) {
	if len(in) != 1 {
		return "", fmt.Errorf("incorrect number of arguments")
	}
	enum := in[0]
	glog.V(1).Infof("Working on enum string %s", enum)
	pkg := ""
	if !strings.HasPrefix(enum, ".") {
		enum = "." + *file.Package + "." + enum
		pkg = file.GoPkg.Name
	}
	parts := strings.Split(enum, ".")
	if pkg == "" {
		pkg = parts[1]
	}
	parts = parts[2:]

	ret := "value"
	for i := len(parts) - 1; i >= 0; i-- {
		ret = parts[i] + "_" + ret
	}
	if pkg != file.GoPkg.Name {
		ret = pkg + "." + ret
	}
	glog.V(1).Infof("Ret:Working on enum string %s", ret)
	return ret, nil
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

func isSpecStatusMessage(msg *descriptor.Message) bool {
	glog.V(1).Infof("Check if SpecStatus message for %s", msg.Name)
	spec := false
	status := false
	for _, v := range msg.Fields {
		if *v.Name == "Spec" {
			spec = true
		}
		if *v.Name == "Status" {
			status = true
		}
	}

	return spec && status
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

func isAPIServerServed(file *descriptor.File) (bool, error) {
	if v, err := reg.GetExtension("venice.fileApiServerBacked", file); err == nil {
		return v.(bool), nil
	}
	return true, nil
}

//getGrpcDestination returns the gRPC destination specified.
func getGrpcDestination(file *descriptor.File) string {
	if v, err := reg.GetExtension("venice.fileGrpcDest", file); err == nil {
		return v.(string)
	}
	return ""
}

// getFileName returns a filename sans the extension and path
func getFileName(name string) string {
	return strings.Title(strings.TrimSuffix(filepath.Base(name), filepath.Ext(filepath.Base(name))))
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
	reg.RegisterOptionParser("venice.check", parseStringSliceOptions)
	reg.RegisterOptionParser("gogoproto.nullable", parseBoolOptions)
	reg.RegisterOptionParser("venice.naplesRestService", parseNaplesRestService)
	reg.RegisterOptionParser("venice.fileApiServerBacked", parseBoolOptions)

	// Register Functions
	reg.RegisterFunc("getDbKey", getDbKey)
	reg.RegisterFunc("getURIKey", getURIKey)
	reg.RegisterFunc("getSvcParams", getSvcParams)
	reg.RegisterFunc("getRestSvcOptions", getRestSvcOptions)
	reg.RegisterFunc("getMethodParams", getMethodParams)
	reg.RegisterFunc("getCWD2", getCWD2)
	reg.RegisterFunc("getSwaggerFileName", getSwaggerFileName)
	reg.RegisterFunc("createDir", createDir)
	reg.RegisterFunc("genManifest", genManifest)
	reg.RegisterFunc("genSvcManifest", genServiceManifest)
	reg.RegisterFunc("getSvcManifest", getServiceManifest)
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
	reg.RegisterFunc("getFileName", getFileName)
	reg.RegisterFunc("getGrpcDestination", getGrpcDestination)
	reg.RegisterFunc("getValidatorManifest", getValidatorManifest)
	reg.RegisterFunc("derefStr", derefStr)
	reg.RegisterFunc("getEnumStrMap", getEnumStrMap)
	reg.RegisterFunc("isSpecStatusMessage", isSpecStatusMessage)
	reg.RegisterFunc("saveBool", scratch.setBool)
	reg.RegisterFunc("getBool", scratch.getBool)
	reg.RegisterFunc("saveInt", scratch.setInt)
	reg.RegisterFunc("getInt", scratch.getInt)
	reg.RegisterFunc("isAPIServerServed", isAPIServerServed)

	// Register request mutators
	reg.RegisterReqMutator("pensando", reqMutator)
}
