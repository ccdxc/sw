package common

import (
	"errors"
	"fmt"
	"reflect"
	"regexp"
	"strconv"
	"strings"
	"time"

	govldtr "github.com/asaskevich/govalidator"
	gogoproto "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	"github.com/golang/glog"

	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	"github.com/pensando/sw/venice/globals"
	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
	"github.com/pensando/sw/venice/utils/apigen/validators"
)

var (
	errIncompletePath = errors.New("incomplete path specification")
	errInvalidField   = errors.New("invalid field specification")
	errInvalidOption  = errors.New("invalid option specification")
)

// InternalGroups is list of API groups that are not to be exposed to the user
var InternalGroups = []string{"bookstore"}

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
		return nil, errors.New("invalid value")
	}
	switch v.Type {
	case "NamedRef", "SelectorRef", "WeakRef":
	default:
		return nil, errors.New("invalid type")
	}
	if parts := strings.Split(v.To, "/"); len(parts) != 2 || parts[0] == "" || parts[1] == "" {
		return nil, errors.New("invalid ref destination")
	}
	return *v, nil
}

func parsePdsaFieldOptions(val interface{}) (interface{}, error) {
	v, ok := val.(*venice.PdsaFields)
	if !ok {
		return nil, errors.New("invalid value")
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
	c, ok := val.([]*venice.RestEndpoint)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parseGoogleAPIHTTP(val interface{}) (interface{}, error) {
	c, ok := val.(*googapi.HttpRule)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parsePenctlCmd(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.PenCtlCmd)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parseGlobalOpts(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.GlobalOpts)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parsePenctlParentCmd(val interface{}) (interface{}, error) {
	c, ok := val.([]*venice.PenCtlCmd)
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

func parseProxyEndpoint(val interface{}) (interface{}, error) {
	c, ok := val.([]*venice.ProxyEndpoint)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parseMetricInfo(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.MetricInfo)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

func parseMetricFieldInfo(val interface{}) (interface{}, error) {
	c, ok := val.(*venice.MetricFieldInfo)
	if !ok {
		return nil, errInvalidOption
	}
	return c, nil
}

// CheckArgs defines a function to check args for validators
type CheckArgs func(string) bool

// ValidatorArgMap defines the argument types for validators
var ValidatorArgMap = map[string][]CheckArgs{
	"StrEnum":         {IsString},
	"StrLen":          {govldtr.IsInt, govldtr.IsInt},
	"EmptyOrStrLen":   {govldtr.IsInt, govldtr.IsInt},
	"IntRange":        {govldtr.IsInt, govldtr.IsInt},
	"IntMin":          {govldtr.IsInt},
	"CIDR":            {},
	"IPAddr":          {},
	"IPv4":            {},
	"HostAddr":        {},
	"MacAddr":         {},
	"URI":             {},
	"UUID":            {},
	"Duration":        {IsString, IsString},
	"EmptyOrDuration": {IsString, IsString},
	"ProtoPort":       {},
	"RegExp":          {IsValidRegExp},
	"EmptyOrRegExp":   {IsValidRegExp},
	"ValidGroup":      {},
	"ValidKind":       {},
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
	Required  map[string]bool
	Pattern   map[string]string
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
	f.Required = make(map[string]bool)
	f.Pattern = make(map[string]string)
}

// ValidatorProfileMap maps each validator to a profile function
var ValidatorProfileMap = map[string]func(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, v *FieldProfile) error{
	"StrEnum":         strEnumProfile,
	"StrLen":          strLenProfile,
	"EmptyOrStrLen":   emptyOrStrLenProfile,
	"IntRange":        intRangeProfile,
	"IntMin":          intMinProfile,
	"CIDR":            cidrProfile,
	"IPAddr":          ipAddrProfile,
	"IPv4":            ipv4Profile,
	"HostAddr":        hostAddrProfile,
	"MacAddr":         macAddrProfile,
	"URI":             uriProfile,
	"UUID":            uuidProfile,
	"Duration":        durationProfile,
	"EmptyOrDuration": emptyOrDurationProfile,
	"ProtoPort":       protoPortProfile,
	"RegExp":          regexpProfile,
	"EmptyOrRegExp":   emptyOrRegexpProfile,
	"ValidGroup":      validGroupProfile,
	"ValidKind":       validKindProfile,
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
			prof.Default[ver] = GetVName(v)
		}
		prof.Enum[ver] = append(prof.Enum[ver], GetVName(v))
	}
	prof.Required[ver] = true
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
			prof.EnumHints[ver][GetVName(v)] = hint
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
	if max < 0 {
		prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("length of string should be at least %v", min)
	} else {
		prof.MaxLen[ver] = max
		prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("length of string should be between %v and %v", min, max)
	}
	prof.Required[ver] = true
	prof.MinLen[ver] = min
	return nil
}

func emptyOrStrLenProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	err := strLenProfile(field, reg, ver, args, prof)
	prof.Required[ver] = false
	return err
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
	prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("value should be between %v and %v", min, max)
	prof.Required[ver] = true
	return nil
}

func intMinProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	min, ok := convInt(args[0])
	if !ok {
		return errInvalidOption
	}
	prof.MinInt[ver] = min
	prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("value should be at least %v", min)
	prof.Required[ver] = true
	return nil
}

func cidrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1/24, ff02::5/32 "
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid v4 or v6 CIDR block"
	prof.Required[ver] = true
	return nil
}

func ipAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1, ff02::5 "
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid v4 or v6 IP address"
	prof.Required[ver] = true
	return nil
}

func ipv4Profile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1 "
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid IPv4 address"
	prof.Required[ver] = true
	return nil
}

func hostAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "10.1.1.1, ff02::5, localhost, example.domain.com "
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid host address, IP address or hostname"
	prof.Example[ver] = prof.Example[ver] + str
	prof.Required[ver] = true
	return nil
}

func macAddrProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "aabb.ccdd.0000, aabb.ccdd.0000, aabb.ccdd.0000"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid MAC address"
	prof.Required[ver] = true
	return nil
}

func uriProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "https://10.1.1.1, ldap://10.1.1.1:800, /path/to/x"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid URI"
	prof.Required[ver] = true
	return nil
}

func uuidProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "49943a2c-9d76-11e7-abc4-cec278b6b50a"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid UUID"
	prof.Required[ver] = true
	return nil
}

func durationProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "2h"
	prof.Example[ver] = prof.Example[ver] + str
	min, err := time.ParseDuration(args[0])
	if err != nil {
		return errInvalidOption
	}
	max, err := time.ParseDuration(args[1])
	if err != nil {
		return errInvalidOption
	}

	if min == 0 && max == 0 {
		prof.DocString[ver] = prof.DocString[ver] + "should be a valid time duration\n"
	} else if max == 0 {
		prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("should be a valid time duration of at least %s", min)
	} else if min == 0 {
		prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("should be a valid time duration of at most %s", max)
	} else {
		if min.Nanoseconds() >= max.Nanoseconds() {
			return errInvalidOption
		}
		prof.DocString[ver] = prof.DocString[ver] + fmt.Sprintf("should be a valid time duration between %s and %s", min, max)
	}
	prof.Required[ver] = true
	return nil
}

func emptyOrDurationProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	err := durationProfile(field, reg, ver, args, prof)
	prof.Required[ver] = false
	return err
}

func protoPortProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "tcp/1234, arp"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid layer3 or layer 4 protocol and port/type"
	prof.Required[ver] = true
	return nil
}

func validGroupProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "auth"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid API Group"
	prof.Required[ver] = true
	return nil
}

func validKindProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	str := "Network"
	prof.Example[ver] = prof.Example[ver] + str
	prof.DocString[ver] = prof.DocString[ver] + "should be a valid object Kind"
	prof.Required[ver] = true
	return nil
}

func regexpProfile(_ *descriptor.Field, _ *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	if regEntry, ok := validators.RegexpList[args[0]]; ok {
		prof.Pattern[ver] = regEntry.Str
		prof.DocString[ver] = regEntry.HelpStr
		prof.Required[ver] = true
		return nil
	}
	return fmt.Errorf("unknown regular expression [%s]", args[0])
}

func emptyOrRegexpProfile(field *descriptor.Field, reg *descriptor.Registry, ver string, args []string, prof *FieldProfile) error {
	err := regexpProfile(field, reg, ver, args, prof)
	prof.Required[ver] = false
	return err
}

// ValidateField specifies a validator specified on a field.
type ValidateField struct {
	Fn         string
	Ver        string
	Args       []string
	AllowEmpty bool
}

// IsString is a utility function to check if a string is valid
func IsString(in string) bool {
	if len(in) != 0 {
		return true
	}
	return false
}

// IsValidRegExp validates that the value is one of the valid Regexps
func IsValidRegExp(in string) bool {
	if IsString(in) {
		if _, ok := validators.RegexpList[in]; ok {
			return true
		}
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
	if strings.Contains(in, "EmptyOr(") {
		ret.AllowEmpty = true
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

// GenerateVeniceCheckFieldProfile returns the field validators profile.
func GenerateVeniceCheckFieldProfile(field *descriptor.Field, registry *descriptor.Registry) (*FieldProfile, error) {
	r, err := reg.GetExtension("venice.check", field)
	if err == nil {
		// We have some options specified
		profile := FieldProfile{}
		profile.Init()
		for _, v := range r.([]string) {
			fldv, err := ParseValidator(v)
			if err != nil {
				return nil, err
			}
			fn, ok := ValidatorProfileMap[fldv.Fn]
			if !ok {
				glog.Fatalf("Did not find entry in profile map for %v", fldv.Fn)
			}
			ver := fldv.Ver
			if ver == "" || ver == "*" {
				ver = "all"
			}
			err = fn(field, registry, ver, fldv.Args, &profile)
			if err != nil {
				glog.Fatalf("cannot parse validator (%s)", err)
			}
			if fldv.AllowEmpty {
				profile.Required[ver] = false
			}
		}
		return &profile, nil
	}
	return nil, nil
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
	if suffix == "" {
		ret = strings.TrimSuffix(ret, "_")
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
			glog.V(1).Infof("parseDefaulter - escaped : ignoring.. [%s]", in)
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
					ret.Map[ver] = "\"" + GetVName(v) + "\""
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
			tag := t[0]
			if len(strings.TrimSpace(tag)) != len(tag) {
				glog.Fatalf("JSON tag %s has either leading or trailing spaces", tag)
			}
			return tag
		}
	}
	return ""
}

// IsInline returns true if the field is an inline field.
func IsInline(fld *descriptor.Field) bool {
	if *fld.Type != gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
		return false
	}
	if r, err := reg.GetExtension("gogoproto.jsontag", fld); err == nil {
		if strings.Contains(r.(string), ",inline") {
			return true
		}
	}
	return false
}

// IsEmbed returns true if the field is an inline field.
func IsEmbed(fld *descriptor.Field) bool {
	if *fld.Type != gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
		return false
	}
	if i, err := reg.GetExtension("gogoproto.embed", fld); err == nil {
		return i.(bool)
	}
	return false
}

// IsMutable returns true if the field is mutable.
// Only valid for scalar fields. Default is true.
func IsMutable(fld *descriptor.Field) bool {
	v, err := reg.GetExtension("venice.mutable", fld)
	if err != nil {
		return true
	}
	if !fld.IsScalar() && !fld.IsString() {
		glog.Fatalf("Flag venice.mutable specified for non-scalar field %+v", fld)
	}
	return v.(bool)
}

// ServiceParams is the parameters related to the Service used by templates
type ServiceParams struct {
	// Version is the version of the Service
	Version string
	// Prefix is the prefix for all the resources served by the service.
	Prefix string
	// URIPath is the URI Path prefix for this service. This is combination of
	// Version and Prefix and the catogory that is inherited from
	//  the fileCategory options specified at the file level
	URIPath string
	// StagingPath is the URI path prefix for this service if it supports config
	//  staging. If the service does not support staging then it is empty.
	StagingPath string
}

// GetSvcParams returns the ServiceParams for the service.
//   Parameters could be initialized to defaults if options were
//   not specified by the user in service.proto.
func GetSvcParams(s *descriptor.Service) (ServiceParams, error) {
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
	category := globals.ConfigURIPrefix
	if i, err = reg.GetExtension("venice.fileCategory", s.File); err == nil {
		if category, ok = i.(string); !ok {
			category = globals.ConfigURIPrefix
		}
	} else {
		glog.V(1).Infof("Did not find Category %s", err)
	}
	if params.Prefix == "" {
		params.URIPath = "/" + category + "/" + params.Version
		if category == globals.ConfigURIPrefix {
			params.StagingPath = "/staging/{{TOCTX.BufferId}}/" + params.Version
		}
	} else {
		params.URIPath = "/" + category + "/" + params.Prefix + "/" + params.Version
		if category == globals.ConfigURIPrefix {
			params.StagingPath = "/staging/{TOCTX.BufferId}/" + params.Prefix + "/" + params.Version
		}
	}
	return params, nil
}

// FormParam is parameters for one element of a multipart form
type FormParam struct {
	Name        string
	Type        string
	Required    bool
	Description string
}

// ProxyPath is parameters for reverse proxy endpoints
type ProxyPath struct {
	Prefix     string
	TrimPath   string
	Path       string
	FullPath   string
	Backend    string
	DocString  string
	Response   string
	FormParams []FormParam
}

// GetProxyPaths retrieves all the proxy paths defined in the service
func GetProxyPaths(svc *descriptor.Service) ([]ProxyPath, error) {
	var ret []ProxyPath
	svcParams, err := GetSvcParams(svc)
	if err != nil {
		glog.V(1).Infof("unable to get proxy paths for service [%s]", *svc.Name)
		return ret, err
	}
	i, err := reg.GetExtension("venice.proxyPrefix", svc)
	if err != nil {
		glog.V(1).Infof("no proxy options found on service [%s](%s)", *svc.Name, err)
		return ret, nil
	}
	opts, ok := i.([]*venice.ProxyEndpoint)
	if !ok {
		return ret, fmt.Errorf("could not parse proxy option for service [%s] [%+v]", *svc.Name, opts)
	}
	glog.V(1).Infof("found proxy options on service [%s] [%+v]", *svc.Name, opts)
	pathMap := make(map[string]bool)
	category := globals.ConfigURIPrefix
	if i, err = reg.GetExtension("venice.fileCategory", svc.File); err == nil {
		if category, ok = i.(string); !ok {
			category = globals.ConfigURIPrefix
		}
	} else {
		glog.V(1).Infof("Did not find Category %s", err)
	}
	for _, opt := range opts {
		if _, ok := pathMap[opt.GetPathPrefix()]; ok {
			glog.Fatalf("duplicate path detected in proxy paths service [%s] path [%s]", *svc.Name, opt.GetPathPrefix())
		}
		fullpath := "/" + category + "/" + svcParams.Prefix + "/" + svcParams.Version + "/" + strings.TrimPrefix(opt.GetPath(), "/")
		if svcParams.Prefix == "" {
			fullpath = "/" + category + "/" + svcParams.Version + "/" + strings.TrimPrefix(opt.GetPath(), "/")
		}

		trimpath := "/" + category + "/" + svcParams.Prefix + "/" + svcParams.Version + "/"
		if svcParams.Prefix == "" {
			trimpath = "/" + category + "/" + svcParams.Version + "/"
		}
		path := opt.Path
		path = strings.TrimPrefix(path, "/")
		path = strings.TrimSuffix(path, "/")
		prefix := opt.PathPrefix
		prefix = strings.TrimPrefix(prefix, "/")
		prefix = strings.TrimSuffix(prefix, "/")
		prefix = "/" + prefix

		msgName := opt.Response
		if !strings.HasPrefix(msgName, ".") {
			msgName = "." + strings.ToLower(svc.File.GoPkg.Name) + "." + msgName
		}
		_, err = svc.File.Reg.LookupMsg("", msgName)
		if err != nil {
			return ret, err
		}

		p := ProxyPath{Prefix: prefix, Path: path, TrimPath: trimpath, FullPath: fullpath, Backend: opt.GetBackend(), Response: msgName, DocString: opt.DocString}
		for _, fd := range opt.FormParams {
			p.FormParams = append(p.FormParams, FormParam{Name: fd.Name, Type: fd.Type, Description: fd.Description, Required: fd.Required})
		}
		ret = append(ret, p)
	}
	return ret, nil
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
	PackageType    = 2
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

// GetVName returns the effective enum string value for use with string enums
func GetVName(v *gogoproto.EnumValueDescriptorProto) string {
	o, err := reg.GetExtension("venice.enumValueStr", v)
	if err != nil {
		// All enums are converted to lower case
		return strings.ToLower(*v.Name)
	}
	return o.(string)
}

// RegisterOptionParsers registers all options parsers
func RegisterOptionParsers() {
	reg.RegisterOptionParser("venice.fileGrpcDest", parseStringOptions)
	reg.RegisterOptionParser("venice.apiProfile", parseStringOptions)
	reg.RegisterOptionParser("venice.apiVersion", parseStringOptions)
	reg.RegisterOptionParser("venice.apiGrpcCrudService", parseStringSliceOptions)
	reg.RegisterOptionParser("venice.apiPrefix", parseStringOptions)
	reg.RegisterOptionParser("venice.apiRestService", parseRestServiceOption)
	reg.RegisterOptionParser("venice.methodProfile", parseStringOptions)
	reg.RegisterOptionParser("venice.methodOper", parseStringOptions)
	reg.RegisterOptionParser("venice.methodAutoGen", parseBoolOptions)
	reg.RegisterOptionParser("venice.methodSrvBinaryStream", parseBoolOptions)
	reg.RegisterOptionParser("venice.methodTenantDefault", parseBoolOptions)
	reg.RegisterOptionParser("venice.objectIdentifier", parseStringOptions)
	reg.RegisterOptionParser("venice.objectAutoGen", parseStringOptions)
	reg.RegisterOptionParser("venice.objectPrefix", parseObjectPrefix)
	reg.RegisterOptionParser("venice.objRelation", parseObjRelation)
	reg.RegisterOptionParser("google.api.http", parseGoogleAPIHTTP)
	reg.RegisterOptionParser("venice.check", parseStringSliceOptions)
	reg.RegisterOptionParser("venice.storageTransformer", parseStringSliceOptions)
	reg.RegisterOptionParser("gogoproto.nullable", parseBoolOptions)
	reg.RegisterOptionParser("gogoproto.embed", parseBoolOptions)
	reg.RegisterOptionParser("gogoproto.jsontag", parseStringOptions)
	reg.RegisterOptionParser("venice.naplesRestService", parseNaplesRestService)
	reg.RegisterOptionParser("venice.penctlCmd", parsePenctlCmd)
	reg.RegisterOptionParser("venice.penctlParentCmd", parsePenctlParentCmd)
	reg.RegisterOptionParser("venice.fileApiServerBacked", parseBoolOptions)
	reg.RegisterOptionParser("venice.apiAction", parseAPIActions)
	reg.RegisterOptionParser("venice.default", parseStringSliceOptions)
	reg.RegisterOptionParser("venice.fileCategory", parseStringOptions)
	reg.RegisterOptionParser("venice.proxyPrefix", parseProxyEndpoint)
	reg.RegisterOptionParser("venice.methodActionObject", parseStringOptions)
	reg.RegisterOptionParser("venice.forceDoc", parseBoolOptions)
	reg.RegisterOptionParser("venice.mutable", parseBoolOptions)
	reg.RegisterOptionParser("venice.metricInfo", parseMetricInfo)
	reg.RegisterOptionParser("venice.metricsField", parseMetricFieldInfo)
	reg.RegisterOptionParser("venice.enumValueStr", parseStringOptions)
	reg.RegisterOptionParser("venice.pdsaFields", parsePdsaFieldOptions)
	reg.RegisterOptionParser("venice.pdsaSetGlobOpts", parseGlobalOpts)
	reg.RegisterOptionParser("venice.pdsaGetGlobOpts", parseGlobalOpts)
}
