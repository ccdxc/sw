package common

import (
	"errors"
	"fmt"
	"reflect"
	"regexp"
	"strconv"
	"strings"

	govldtr "github.com/asaskevich/govalidator"
	gogoproto "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	"github.com/golang/glog"

	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
)

var (
	errIncompletePath = errors.New("incomplete path specification")
	errInvalidField   = errors.New("invalid field specification")
	errInvalidOption  = errors.New("invalid option specification")
)

// Option Parsers

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

func parseAPIActions(val interface{}) (interface{}, error) {
	c, ok := val.([]*venice.ActionEndpoint)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parseObjectPrefix(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.ObjectPrefix)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

// CheckArgs defines a function to check args for validators
type CheckArgs func(string) bool

// ValidatorArgMap defines the argument types for validators
var ValidatorArgMap = map[string][]CheckArgs{
	"StrEnum":  {IsString},
	"StrLen":   {govldtr.IsInt, govldtr.IsInt},
	"IntRange": {govldtr.IsInt, govldtr.IsInt},
	"IPAddr":   {},
	"IPv4":     {},
	"HostAddr": {},
	"MacAddr":  {},
	"URI":      {},
	"UUID":     {},
}

// FieldProfile defines a profile for a field, including validators, defaults,
//   documentation helpers and examples.
type FieldProfile struct {
	MinInt    map[string]int64
	MaxInt    map[string]int64
	MinLen    map[string]int64
	MaxLen    map[string]int64
	Default   map[string]string
	Enum      map[string][]string
	EnumHints map[string]map[string]string
	Example   map[string]string
	DocString map[string]string
}

// Init is a helper routine to initialize a FieldProfile object
func (f *FieldProfile) Init() {
	f.MinInt = make(map[string]int64)
	f.MaxInt = make(map[string]int64)
	f.MinLen = make(map[string]int64)
	f.MaxLen = make(map[string]int64)
	f.Default = make(map[string]string)
	f.Enum = make(map[string][]string)
	f.EnumHints = make(map[string]map[string]string)
	f.DocString = make(map[string]string)
	f.Example = make(map[string]string)
}

// ValidatorProfileMap maps each validator to a profile function
var ValidatorProfileMap = map[string]func(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, v *FieldProfile) error{
	"StrEnum":  strEnumProfile,
	"StrLen":   strLenProfile,
	"IntRange": intRangeProfile,
	"IPAddr":   ipAddrProfile,
	"IPv4":     ipv4Profile,
	"HostAddr": hostAddrProfile,
	"MacAddr":  macAddrProfile,
	"URI":      uriProfile,
	"UUID":     uuidProfile,
}

// convInt is a utility function to get convert string to integer
func convInt(in string) (int64, bool) {
	ret, err := strconv.ParseInt(in, 10, 64)
	if err != nil {
		return 0, false
	}
	return ret, true
}

func strEnumProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	eName := ""
	if strings.HasPrefix(args[0], ".") {
		eName = args[0]
	} else {
		eName = "." + field.Message.File.GoPkg.Name + "." + args[0]
	}
	enum, err := reg.LookupEnum("", eName)
	if err != nil {
		return err
	}

	for _, v := range enum.GetValue() {
		if *v.Number == 0 {
			prof.Default[ver] = *v.Name
		}
		prof.Enum[ver] = append(prof.Enum[ver], *v.Name)
	}
	// Find the UI hints
	// Assume all outers are from the same file. Cross file StrEnums can still be specified as long as the
	// the Enum and the outers for the Enum are in the same file.
	glog.V(1).Infof("parsing UI-Hints for enum [%v] outers [%v]", enum.GetName(), enum.Outers)
	prof.EnumHints[ver] = make(map[string]string)
	msgPathID := MsgType
	enumPathID := EnumType
	// fqdn := "." + enum.File.GoPkg.String() + "."
	enums := enum.File.GetEnumType()
	iters := 0
	msgs := enum.File.MessageType
	var path []int

	for _, m := range enum.Outers {
		enumPathID = NestedEnumType
		for idx, msg := range msgs {
			// mfqdn := fqdn + msg.GetName()
			if msg.GetName() == m {
				path = append(path, []int{msgPathID, idx}...)
				msgs = msg.GetNestedType()
				enums = msg.GetEnumType()
				// fqdn = fqdn + msg.GetName() + "."
				iters++
			}
		}
		msgPathID = NestedMsgType
	}
	if iters != len(enum.Outers) {
		return errors.New("could not resolve full path")
	}
	for idx, e := range enums {
		if e.GetName() == enum.GetName() {
			path = append(path, []int{enumPathID, idx}...)
			break
		}
	}
	glog.V(1).Infof("Using path %v to evaluate UI-Hints", path)
	for idx, v := range enum.GetValue() {
		vpath := append(path, EnumValType, idx)
		loc, err := GetLocation(enum.File.SourceCodeInfo, vpath)
		if err != nil {
			return fmt.Errorf("could not find enum path [%v]", vpath)
		}
		hint := ""
		glog.V(1).Infof("evaluating Enum comments for UI-Hints [%v]", loc.GetLeadingComments())
		if loc.GetLeadingComments() != "" {
			for _, line := range strings.Split(loc.GetLeadingComments(), "\n") {
				line = strings.TrimSpace(line)
				if strings.HasPrefix(line, "ui-hint:") {
					hint = strings.TrimPrefix(line, "ui-hint:")
					hint = strings.TrimSpace(hint)
					glog.V(1).Infof("added UI-Hint %s", hint)
					break
				}
			}
		}
		if hint == "" && loc.GetTrailingComments() != "" {
			for _, line := range strings.Split(loc.GetTrailingComments(), "\n") {
				line = strings.TrimSpace(line)
				if strings.HasPrefix(line, "ui-hint:") {
					hint = strings.TrimPrefix(line, "ui-hint:")
					hint = strings.TrimSpace(hint)
					glog.V(1).Infof("added UI-Hint %s", hint)
					break
				}
			}
		}
		if hint != "" {
			prof.EnumHints[ver][v.GetName()] = hint
		}
	}

	return nil
}

func strLenProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	min, ok := convInt(args[0])
	if !ok {
		return errInvalidOption
	}
	max, ok := convInt(args[1])
	if !ok {
		return errInvalidOption
	}
	prof.MinLen[ver] = min
	prof.MaxLen[ver] = max
	prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("length of string should be between %v and %v\n", min, max)
	return nil
}

func intRangeProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	min, ok := convInt(args[0])
	if !ok {
		return errInvalidOption
	}
	max, ok := convInt(args[1])
	if !ok {
		return errInvalidOption
	}
	prof.MinInt[ver] = min
	prof.MaxInt[ver] = max
	prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("value should be between %v and %v\n", min, max)
	return nil
}

func ipAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1, ff02::5 "
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid v4 or v6 IP address\n"
	return nil
}

func ipv4Profile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1 "
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid IPv4 address\n"
	return nil
}

func hostAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1, ff02::5, localhost, example.domain.com "
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid host address, IP address or hostname\n"
	prof.Example[ver] = prof.Example[ver] + str
	return nil
}

func macAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "aa:BB:cc:DD:00:00, aabb.ccdd.0000, aa-BB-cc-DD-00-00"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid MAC address\n"
	return nil
}

func uriProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "https://10.1.1.1, ldap://10.1.1.1:800, /path/to/x"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid URI\n"
	return nil
}

func uuidProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "49943a2c-9d76-11e7-abc4-cec278b6b50a"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid UUID\n"
	return nil
}

// GetFieldProfile returns the FieldProfile for a field by parsing all options defined on the field.
func GetFieldProfile(field *descriptor.Field, reg *descriptor.Registry) FieldProfile {
	var ret FieldProfile
	return ret
}

// ValidateField specifies a validator specified on a field.
type ValidateField struct {
	Fn   string
	Ver  string
	Args []string
}

// IsString  is s utility function to check if a string is valid
func IsString(in string) bool {
	if len(in) != 0 {
		return true
	}
	return false
}

// ParseValidator  parsese a given validator and returns a ValidateField object
func ParseValidator(in string) (ValidateField, error) {
	ret := ValidateField{}
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
	if vargs, ok := ValidatorArgMap[ret.Fn]; ok {
		if len(vargs) != len(ret.Args) {
			return ValidateField{}, fmt.Errorf("Incorrect number of args (%d) for %s", len(ret.Args), ret.Fn)
		}
		for i := range vargs {
			if !vargs[i](ret.Args[i]) {
				return ValidateField{}, fmt.Errorf("validation for arg(%s) failed for %s", ret.Args[i], ret.Fn)
			}
		}
	} else {
		return ValidateField{}, fmt.Errorf("unknown validator %s", ret.Fn)
	}
	return ret, nil
}

// Defaults is defaults specified by field.
type Defaults struct {
	Repeated bool
	Pointer  bool
	Nested   bool
	// Map is map[version]defaultValue
	Map map[string]string
}

func getEnumFQDN(file *descriptor.File, enum string) string {
	if !strings.HasPrefix(enum, ".") {
		enum = "." + *file.Package + "." + enum
	}
	return enum
}

// GetEnumStr returns a path to the enum value object
func GetEnumStr(file *descriptor.File, in []string, suffix string) (string, error) {
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

	ret := suffix
	for i := len(parts) - 1; i >= 0; i-- {
		ret = parts[i] + "_" + ret
	}
	if pkg != file.GoPkg.Name {
		ret = pkg + "." + ret
	}
	glog.V(1).Infof("Ret:Working on enum string %s", ret)
	return ret, nil
}

func parseDefault(in string) (string, string, error) {
	index := strings.Index(in, ":")
	ver := "all"
	val := in
	// We want version to be specified as "<ver>:" so event "^:..." is no version hence the check against 1 instead of 0
	if index < 1 {
		glog.V(1).Infof("parseDefaulter - No version specified [%s]", in)
	} else {
		if in[index-1] == '\\' {
			glog.V(1).Info("parseDefaulter - escaped : ignoring.. [%s]", in)
		} else {
			ver = in[:index]
			val = in[index+1:]
			if ver == "*" {
				ver = "all"
			}
		}
	}
	return ver, val, nil
}

func implicitDefaults(file *descriptor.File, f *descriptor.Field) (Defaults, bool) {
	ret := Defaults{Map: make(map[string]string)}
	if f.GetType() == gogoproto.FieldDescriptorProto_TYPE_STRING {
		r, err := reg.GetExtension("venice.check", f)
		if err != nil {
			return ret, false
		}
		if f.Label != nil && *f.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
			ret.Repeated = true
		}
		for _, v := range r.([]string) {
			validator, err := ParseValidator(v)
			if err != nil || validator.Fn != "StrEnum" {
				continue
			}
			ver := validator.Ver
			en := getEnumFQDN(file, validator.Args[0])
			enum, err := file.Reg.LookupEnum("", en)
			if err != nil {
				glog.Fatalf("Unknown Enum[%v] in StrEnum for [%v.%v]", en, *f.Message.Name, *f.Name)
			}

			for _, v := range enum.GetValue() {
				if *v.Number == 0 {
					ret.Map[ver] = "\"" + *v.Name + "\""
					break
				}
			}
		}
		return ret, true
	}
	return ret, false
}

// ParseDefaults parses defaults specified on a field considering versions and implicit defaults.
func ParseDefaults(file *descriptor.File, f *descriptor.Field) (Defaults, bool, error) {
	iret, found := implicitDefaults(file, f)
	ret := Defaults{Map: make(map[string]string)}
	if found {
		for k, v := range iret.Map {
			ret.Map[k] = v
		}
	}
	r, err := reg.GetExtension("venice.default", f)
	ds := []string{}
	if f.Label != nil && *f.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
		ret.Repeated = true
	}
	if err == nil {
		ds = r.([]string)
	}
	for _, d := range ds {
		found = true
		ver, val, err := parseDefault(d)
		if err != nil {
			return ret, true, err
		}
		switch f.GetType() {
		case gogoproto.FieldDescriptorProto_TYPE_GROUP, gogoproto.FieldDescriptorProto_TYPE_MESSAGE, gogoproto.FieldDescriptorProto_TYPE_BYTES:
			// Aggregate types are not allowed to have defaults. Only scalar values
			return ret, true, fmt.Errorf("[%s]not allowed", *f.Name)
		case gogoproto.FieldDescriptorProto_TYPE_BOOL:
			if _, err := strconv.ParseBool(val); err != nil {
				return ret, false, fmt.Errorf("[%s]cound not parse bool (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_ENUM:
			// not validate yet - TBD
		case gogoproto.FieldDescriptorProto_TYPE_INT32, gogoproto.FieldDescriptorProto_TYPE_SFIXED32, gogoproto.FieldDescriptorProto_TYPE_SINT32:
			if _, err := strconv.ParseInt(val, 10, 32); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_INT64, gogoproto.FieldDescriptorProto_TYPE_SFIXED64, gogoproto.FieldDescriptorProto_TYPE_SINT64:
			if _, err := strconv.ParseInt(val, 10, 64); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_UINT32, gogoproto.FieldDescriptorProto_TYPE_FIXED32:
			if _, err := strconv.ParseUint(val, 10, 32); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_UINT64, gogoproto.FieldDescriptorProto_TYPE_FIXED64:
			if _, err := strconv.ParseUint(val, 10, 64); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_FLOAT:
			if _, err := strconv.ParseFloat(val, 32); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_DOUBLE:
			if _, err := strconv.ParseFloat(val, 64); err != nil {
				return ret, true, fmt.Errorf("[%s]cound not parse int (%s)", *f.Name, err)
			}
		case gogoproto.FieldDescriptorProto_TYPE_STRING:
			val = "\"" + val + "\""
		default:
			return ret, true, fmt.Errorf(" [%s]unknown type", *f.Name)
		}
		if ev, eok := ret.Map[ver]; eok {
			iv, iok := iret.Map[ver]
			if !iok || iv != ev {
				return ret, true, fmt.Errorf("[%s] got duplicate default for version %s", *f.Name, ver)
			}
		}
		ret.Map[ver] = val
	}
	return ret, found, nil
}

// IsAutoGenMethod returns true if method is a autogenerated CRUD method
func IsAutoGenMethod(meth *descriptor.Method) bool {
	if v, err := reg.GetExtension("venice.methodAutoGen", meth); err == nil {
		return v.(bool)
	}
	return false
}

// GetJSONTag retrieves the JSON tag for a field if specified
func GetJSONTag(fld *descriptor.Field) string {
	if r, err := reg.GetExtension("gogoproto.jsontag", fld); err == nil {
		t := strings.Split(r.(string), ",")
		if len(t) > 0 {
			return t[0]
		}
	}
	return ""
}

// Hardcoded path Ids for the types
const (
	MsgType        = 4
	NestedMsgType  = 3
	NestedEnumType = 4
	FieldType      = 2
	FileType       = 1
	SvcType        = 6
	MethType       = 2
	EnumType       = 5
	EnumValType    = 2
	SvcOfset       = 10
)

// GetLocation retrieves SourceCodeInfo_Location from SourceCodeInfo given the location path
func GetLocation(sci *gogoproto.SourceCodeInfo, locs []int) (*gogoproto.SourceCodeInfo_Location, error) {
	if len(locs) == 0 || sci == nil {
		return nil, fmt.Errorf("getLocation() need tuples")
	}
	path := []int32{}
	for _, v := range locs {
		path = append(path, int32(v))
	}
	for _, l := range sci.Location {
		if len(l.GetPath()) == len(path) && reflect.DeepEqual(l.GetPath(), path) {
			return l, nil
		}
	}
	return nil, errors.New("not found")
}

// RegisterOptionParsers registers all options parsers
func RegisterOptionParsers() {
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
	reg.RegisterOptionParser("venice.objectPrefix", parseObjectPrefix)
	reg.RegisterOptionParser("venice.objRelation", parseObjRelation)
	reg.RegisterOptionParser("google.api.http", parseGoogleAPIHTTP)
	reg.RegisterOptionParser("venice.check", parseStringSliceOptions)
	reg.RegisterOptionParser("venice.storageTransformer", parseStringSliceOptions)
	reg.RegisterOptionParser("gogoproto.nullable", parseBoolOptions)
	reg.RegisterOptionParser("gogoproto.jsontag", parseStringOptions)
	reg.RegisterOptionParser("venice.naplesRestService", parseNaplesRestService)
	reg.RegisterOptionParser("venice.fileApiServerBacked", parseBoolOptions)
	reg.RegisterOptionParser("venice.apiAction", parseAPIActions)
	reg.RegisterOptionParser("venice.default", parseStringSliceOptions)
}
