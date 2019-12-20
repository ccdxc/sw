package plugin

import (
	"bytes"
	"encoding/json"
	"encoding/xml"
	"errors"
	"flag"
	"fmt"
	"go/build"
	"io/ioutil"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"github.com/gogo/protobuf/proto"

	gogoproto "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogen "github.com/gogo/protobuf/protoc-gen-gogo/generator"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	cgen "github.com/pensando/sw/venice/cli/gen"
	"github.com/pensando/sw/venice/globals"
	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
	mutator "github.com/pensando/sw/venice/utils/apigen/autogrpc"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
)

var (
	errIncompletePath = errors.New("incomplete path specification")
	errInvalidField   = errors.New("invalid field specification")
	errInvalidOption  = errors.New("invalid option specification")
)

type scratchVars struct {
	B   [3]bool
	Int [3]int
	Str [3]string
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

func (s *scratchVars) setStr(val string, id int) string {
	s.Str[id] = val
	return val
}

func (s *scratchVars) getStr(id int) string {
	return s.Str[id]
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

// PdsaGlobalOpts holds raw venice.pdsaSetGlobOpts
type PdsaGlobalOpts struct {
	OidLen string
	OidFam string
	Struct string
	Mib    string
	FillFn string
}

// PenctlCmdOpts holds raw PenctlCmd options data from .proto files
type PenctlCmdOpts struct {
	Cmd          string
	HelpStr      string
	ShortHelpStr string
	RootCmd      string
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
	// TenantDefault marks that this method has a flavor with inferred tenant default.
	TenantDefault bool
}

// KeyComponent is a component of the key path, derived from the objectPrefix option.
type KeyComponent struct {
	// Type of the compoent - prefix or field
	Type string
	// Val holds a string literal or a field name depending on type
	Val string
}

// CamInfo is the struct to parse metaswitch mib.xml
type CamInfo struct {
	XMLName   xml.Name `xml:"camInfo"`
	Text      string   `xml:",chardata"`
	Constants struct {
		Text  string `xml:",chardata"`
		Value []struct {
			Text     string `xml:",chardata"`
			CodeName string `xml:"codeName,attr"`
		} `xml:"value"`
	} `xml:"constants"`
	Groups struct {
		Text       string `xml:",chardata"`
		EnumValues []struct {
			Text  string `xml:",chardata"`
			Group string `xml:"group,attr"`
			Value []struct {
				Text     string `xml:",chardata"`
				Name     string `xml:"name,attr"`
				Asn1Name string `xml:"asn1Name,attr"`
				TextName string `xml:"textName,attr"`
				CodeName string `xml:"codeName,attr"`
			} `xml:"value"`
		} `xml:"enumValues"`
	} `xml:"groups"`
	Mibs struct {
		Text    string `xml:",chardata"`
		MibInfo []struct {
			Text           string `xml:",chardata"`
			Mib            string `xml:"mib,attr"`
			Asn1Name       string `xml:"asn1Name,attr"`
			TextName       string `xml:"textName,attr"`
			CodeName       string `xml:"codeName,attr"`
			Droid          string `xml:"droid,attr"`
			File           string `xml:"file,attr"`
			Owner          string `xml:"owner,attr"`
			MibType        string `xml:"mibType,attr"`
			RowStatusField string `xml:"rowStatusField,attr"`
			MaxAccess      string `xml:"maxAccess,attr"`
			Description    string `xml:"description"`
			FieldInfo      []struct {
				Text           string `xml:",chardata"`
				Name           string `xml:"name,attr"`
				Asn1Name       string `xml:"asn1Name,attr"`
				TextName       string `xml:"textName,attr"`
				CodeName       string `xml:"codeName,attr"`
				Droid          string `xml:"droid,attr"`
				Oid            string `xml:"oid,attr"`
				IsIndex        string `xml:"isIndex,attr"`
				FieldType      string `xml:"fieldType,attr"`
				Access         string `xml:"access,attr"`
				Format         string `xml:"format,attr"`
				Mandatory      string `xml:"mandatory,attr"`
				Detail         string `xml:"detail,attr"`
				CodeLengthName string `xml:"codeLengthName,attr"`
				MinLength      string `xml:"minLength,attr"`
				MaxLength      string `xml:"maxLength,attr"`
				Units          string `xml:"units,attr"`
				Description    string `xml:"description"`
				MinValue       string `xml:"minValue"`
				MaxValue       string `xml:"maxValue"`
				EnumValues     struct {
					Text     string `xml:",chardata"`
					Group    string `xml:"group,attr"`
					EnumType string `xml:"enumType,attr"`
					Value    []struct {
						Text     string `xml:",chardata"`
						Name     string `xml:"name,attr"`
						Asn1Name string `xml:"asn1Name,attr"`
						TextName string `xml:"textName,attr"`
						CodeName string `xml:"codeName,attr"`
					} `xml:"value"`
				} `xml:"enumValues"`
				DefaultValue string `xml:"defaultValue"`
			} `xml:"fieldInfo"`
		} `xml:"mibInfo"`
	} `xml:"mibs"`
	Traps struct {
		Text     string `xml:",chardata"`
		TrapInfo []struct {
			Text        string `xml:",chardata"`
			Trap        string `xml:"trap,attr"`
			Asn1Name    string `xml:"asn1Name,attr"`
			TextName    string `xml:"textName,attr"`
			CodeName    string `xml:"codeName,attr"`
			TrapType    string `xml:"trapType,attr"`
			File        string `xml:"file,attr"`
			Owner       string `xml:"owner,attr"`
			MaxAccess   string `xml:"maxAccess,attr"`
			Description string `xml:"description"`
			FieldInfo   []struct {
				Text           string `xml:",chardata"`
				Name           string `xml:"name,attr"`
				Asn1Name       string `xml:"asn1Name,attr"`
				TextName       string `xml:"textName,attr"`
				CodeName       string `xml:"codeName,attr"`
				Oid            string `xml:"oid,attr"`
				FieldType      string `xml:"fieldType,attr"`
				Access         string `xml:"access,attr"`
				Format         string `xml:"format,attr"`
				Detail         string `xml:"detail,attr"`
				CodeLengthName string `xml:"codeLengthName,attr"`
				MinLength      string `xml:"minLength,attr"`
				MaxLength      string `xml:"maxLength,attr"`
				Units          string `xml:"units,attr"`
				Description    string `xml:"description"`
				MinValue       string `xml:"minValue"`
				MaxValue       string `xml:"maxValue"`
				EnumValues     struct {
					Text     string `xml:",chardata"`
					Group    string `xml:"group,attr"`
					EnumType string `xml:"enumType,attr"`
					Value    []struct {
						Text     string `xml:",chardata"`
						Name     string `xml:"name,attr"`
						Asn1Name string `xml:"asn1Name,attr"`
						TextName string `xml:"textName,attr"`
						CodeName string `xml:"codeName,attr"`
					} `xml:"value"`
				} `xml:"enumValues"`
			} `xml:"fieldInfo"`
		} `xml:"trapInfo"`
	} `xml:"traps"`
	GenerationTime struct {
		Text      string `xml:",chardata"`
		Timestamp string `xml:"timestamp"`
	} `xml:"generationTime"`
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
	in := dbpath.(*venice.ObjectPrefix)
	prefx := ""
	singleton := false
	if prefx = in.GetCollection(); prefx != "" {
		prefx = strings.TrimPrefix(prefx, "/")
	} else if prefx = in.GetSingleton(); prefx != "" {
		singleton = true
		prefx = strings.TrimPrefix(prefx, "/")
	}
	if len(prefx) == 0 {
		return output, nil
	}
	if in.Path != "" {
		prefx = prefx + "/"
		path := strings.TrimSuffix(in.Path, "/")
		path = strings.TrimPrefix(path, "/")
		prefx = prefx + path

	}
	if !singleton {
		prefx = prefx + "/"
	}
	if output, err = findComponentsHelper(m, prefx); err == nil {
		// The key generated is /venice/<service prefix>/<object prefix>/<name from object meta>
		if !singleton {
			output = append(output, KeyComponent{Type: "field", Val: "Name"})
		} else {
			output = append(output, KeyComponent{Type: "prefix", Val: "/Singleton"})
		}
	}

	glog.V(1).Infof("Built DB key [ %v ](%v)", output, err)
	return output, err
}

// URIKey specifies a URI
type URIKey struct {
	// Str is the string for for the URI
	Str string
	// Ref is set to true if any references to the object exist in the URI
	Ref bool
}

func getMsgURIKey(m *descriptor.Message, prefix string) (URIKey, error) {
	var out URIKey
	var output []KeyComponent
	var err error

	if output, err = getMsgURI(m, "", prefix); err != nil {
		return out, nil
	}
	out.Str = ""
	out.Ref = false
	sep := ""
	for _, v := range output {
		if v.Type == "prefix" {
			out.Str = fmt.Sprintf("%s%s\"%s\"", out.Str, sep, v.Val)
		} else if v.Type == "field" {
			out.Ref = true
			out.Str = fmt.Sprintf("%s%sin.%s", out.Str, sep, v.Val)
		}
		sep = ", "
	}
	return out, nil
}

// getURIKey gets the URI key given the method. The req parameter specifies
//  if this is in the req direction or resp. In the response direction the URI
//  is always the URI that can be used to access the object.
func getURIKey(m *descriptor.Method, ver string, req bool) (URIKey, error) {
	var output []KeyComponent
	var out URIKey

	params, err := getMethodParams(m)
	if err != nil {
		return out, err
	}
	msg := m.RequestType
	if params.Oper == "ListOper" || params.Oper == "WatchOper" {
		msgtype := ""
		if params.Oper == "ListOper" {
			msgtype, err = getListType(m.ResponseType, true)
			if err != nil {
				return out, err
			}
			msgtype = "." + msgtype
		}
		if params.Oper == "WatchOper" {
			msgtype, err = getWatchType(m.ResponseType, true)
			if err != nil {
				return out, err
			}
			msgtype = "." + msgtype
		}
		msg, err = m.Service.File.Reg.LookupMsg("", msgtype)
		if err != nil {
			return out, err
		}
	}
	svcParams, err := getSvcParams(m.Service)
	if err != nil {
		return out, err
	}
	if req {
		r, err := reg.GetExtension("google.api.http", m)
		rule := r.(*googapi.HttpRule)
		pattern := ""
		switch params.Oper {
		case "CreateOper":
			pattern = rule.GetPost()
		case "GetOper", "ListOper", "WatchOper":
			pattern = rule.GetGet()
		case "DeleteOper":
			pattern = rule.GetDelete()
		case "UpdateOper":
			pattern = rule.GetPut()
		}
		if output, err = findComponentsHelper(msg, pattern); err != nil {
			return out, err
		}
	} else {
		if output, err = getMsgURI(msg, ver, svcParams.Prefix); err != nil {
			return out, err
		}
	}

	out.Str = ""
	out.Ref = false
	sep := ""
	for _, v := range output {
		if v.Type == "prefix" {
			out.Str = fmt.Sprintf("%s%s\"%s\"", out.Str, sep, v.Val)
		} else if v.Type == "field" {
			out.Ref = true
			out.Str = fmt.Sprintf("%s%sin.%s", out.Str, sep, v.Val)
		}
		sep = ", "
	}
	return out, nil
}

// getMsgURI returns the key for the Message URI
func getMsgURI(m *descriptor.Message, ver, svcPrefix string) ([]KeyComponent, error) {
	var output []KeyComponent
	str, err := mutator.GetMessageURI(m.DescriptorProto)
	if err != nil {
		return output, err
	}
	svcPrefix = strings.TrimSuffix(svcPrefix, "/")
	svcPrefix = strings.TrimPrefix(svcPrefix, "/")
	if ver != "" {
		str = svcPrefix + "/" + ver + str
	} else {
		str = svcPrefix + str
	}
	if output, err = findComponentsHelper(m, str); err != nil {
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

var (
	_ = flag.String("S_prefix", "", "prefix for the generation (passed via proto params")
	_ = flag.String("S_path", "", "root path for generation (passed via proto params")
)

func getGenParamsPrefix() (string, error) {
	f := flag.CommandLine.Lookup("S_prefix")
	if f == nil {
		return "", errors.New("not found")
	}
	if f.Value.String() == "" {
		return "", errors.New("not set")
	}
	return f.Value.String(), nil
}

func getGenParamsPath() (string, error) {
	f := flag.CommandLine.Lookup("S_path")
	if f == nil {
		return "", errors.New("not found")
	}
	if f.Value.String() == "" {
		return "", errors.New("not set")
	}
	return f.Value.String(), nil
}

func parseMetricsManifest(raw []byte) map[string]bool {
	manifest := make(map[string]bool)
	lines := bytes.Split(raw, []byte("\n"))
	for _, line := range lines {
		fields := bytes.Fields(line)
		if len(fields) == 1 && string(fields[0]) != "" {
			manifest[string(fields[0])] = true
		}
	}
	return manifest
}

func hasMetricsInFile(file *descriptor.File) (bool, error) {
	for _, m := range file.Messages {
		_, err := reg.GetExtension("venice.metricInfo", m)
		if err == nil {
			return true, nil
		}
	}
	return false, nil
}

func genMetricsManifest(desc *descriptor.File, path string) (string, error) {
	var manifest map[string]bool
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		manifest = make(map[string]bool)
	} else {
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return "", err
		}
		manifest = parseMetricsManifest(raw)
	}
	retStr := ""
	file := filepath.Base(*desc.Name)
	// retStr = fmt.Sprintf("got file [%v]\n", *desc.Name)
	if ok, _ := hasMetricsInFile(desc); ok {
		// retStr = retStr + fmt.Sprintf("Has metrics is true")
		manifest[file] = true
	}
	keys := []string{}
	for k := range manifest {
		keys = append(keys, k)
	}
	sort.Strings(keys)

	for _, k := range keys {
		retStr = fmt.Sprintf("%s%s\n", retStr, k)
	}
	return retStr, nil
}

type fileMetricOptions struct {
	FileName string             `json:",omitempty"`
	Package  string             `json:",omitempty"`
	Prefix   string             `json:",omitempty"`
	Messages []msgMetricOptions `json:",omitempty"`
}

type pdsaFieldOpt struct {
	Field             string
	Name              string
	IsKey             bool
	IsReadOnly        bool
	SetFieldIdx       string
	GetFieldIdx       string
	SetFieldFn        string
	GetFieldFn        string
	SetKeyOidFn       string
	GetKeyOidFn       string
	SetKeyOidIndex    string
	GetKeyOidIndex    string
	SetKeyOidLenIndex string
	GetKeyOidLenIndex string
	CppDataType       string
	FieldLen          int
	FieldHasLen       bool
	IgnoreIfEmpty     bool
}

type fieldMetricOptions struct {
	Name          string   `json:",omitempty"`
	DisplayName   string   `json:",omitempty"`
	Description   string   `json:",omitempty"`
	Units         string   `json:",omitempty"`
	ScaleMin      int32    `json:",omitempty"`
	ScaleMax      int32    `json:",omitempty"`
	BaseType      string   `json:",omitempty"`
	AllowedValues []string `json:",omitempty"`
	Tags          []string `json:",omitempty"`
}

type msgMetricOptions struct {
	Name        string               `json:",omitempty"`
	Description string               `json:",omitempty"`
	DisplayName string               `json:",omitempty"`
	Fields      []fieldMetricOptions `json:",omitempty"`
	Tags        []string             `json:",omitempty"`
	Scope       string               `json:",omitempty"`
}

type fileMsgFieldMapOpt struct {
	FileName string              `json:",omitempty"`
	Package  string              `json:",omitempty"`
	Prefix   string              `json:",omitempty"`
	Maps     map[string][]string `json:",omitempty"`
}

func mapScalarTypes(in gogoproto.FieldDescriptorProto_Type) string {
	switch in {
	case gogoproto.FieldDescriptorProto_TYPE_DOUBLE, gogoproto.FieldDescriptorProto_TYPE_FLOAT:
		return "float"
	case gogoproto.FieldDescriptorProto_TYPE_INT32, gogoproto.FieldDescriptorProto_TYPE_SFIXED32:
		return "int32"
	case gogoproto.FieldDescriptorProto_TYPE_UINT32, gogoproto.FieldDescriptorProto_TYPE_FIXED32:
		return "uint32"
	case gogoproto.FieldDescriptorProto_TYPE_INT64, gogoproto.FieldDescriptorProto_TYPE_SFIXED64:
		return "int64"
	case gogoproto.FieldDescriptorProto_TYPE_UINT64, gogoproto.FieldDescriptorProto_TYPE_FIXED64:
		return "uint64"
	case gogoproto.FieldDescriptorProto_TYPE_STRING:
		return "string"
	case gogoproto.FieldDescriptorProto_TYPE_BOOL:
		return "bool"
	case gogoproto.FieldDescriptorProto_TYPE_ENUM:
		return "enum"
	default:
		return "unknown"
	}
}

func getFieldIsReadOnlyFromCam(cam *CamInfo, table string, field string) bool {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field {
					if fieldInfo.Access == "readOnly" {
						return true
					}
				}
			}
		}
	}
	return false
}

func getFieldIsKeyFromCam(cam *CamInfo, table string, field string) bool {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field {
					if fieldInfo.IsIndex == "1" {
						return true
					}
				}
			}
		}
	}
	return false
}

func getFieldLenFromCam(cam *CamInfo, table string, field string) int {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field && fieldInfo.MaxLength != "" {
					val, err := strconv.Atoi(fieldInfo.MaxLength)
					if err != nil {
						glog.Fatalf("unable to convert to int")
					}
					return val
				}
			}
		}
	}
	return 0
}

func fieldHasCodeLengthName(cam *CamInfo, table string, field string) bool {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field && fieldInfo.CodeLengthName != "" {
					return true
				}
			}
		}
	}
	return false
}

func getFieldIdxFromCam(cam *CamInfo, table string, field string) string {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field {
					return fieldInfo.Oid
				}
			}
		}
	}
	return ""
}

func getFieldDataTypeFromCam(cam *CamInfo, table string, field string) string {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field {
					return fieldInfo.FieldType
				}
			}
		}
	}
	return ""
}

func isFieldInCamTable(cam *CamInfo, table string, field string) bool {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.CodeName == table {
			for _, fieldInfo := range mibInfo.FieldInfo {
				if fieldInfo.CodeName == field {
					return true
				}
			}
		}
	}
	return false
}

func isPdsaFieldInTable(f *descriptor.Field, cam *CamInfo, table string) bool {
	i, err := reg.GetExtension("venice.pdsaFields", f)
	if err == nil {
		o := i.(venice.PdsaFields)
		return isFieldInCamTable(cam, table, o.Field)
	}
	return false
}

func getPdsaFieldOpt(f *descriptor.Field, cam *CamInfo, table string) (pdsaFieldOpt, error) {
	ret := pdsaFieldOpt{}
	i, err := reg.GetExtension("venice.pdsaFields", f)
	if err == nil {
		o := i.(venice.PdsaFields)
		ret.Name = *f.Name
		ret.Field = o.Field
		ret.SetFieldFn = o.SetFieldFn
		ret.GetFieldFn = o.GetFieldFn
		ret.SetKeyOidFn = o.SetKeyOidFn
		ret.GetKeyOidFn = o.GetKeyOidFn
		ret.SetKeyOidIndex = o.SetKeyOidIndex
		ret.GetKeyOidIndex = o.GetKeyOidIndex
		ret.SetKeyOidLenIndex = o.SetKeyOidLenIndex
		ret.GetKeyOidLenIndex = o.GetKeyOidLenIndex
		ret.IgnoreIfEmpty = o.IgnoreIfEmpty
		ret.FieldHasLen = false
		if ret.SetKeyOidLenIndex != "" || ret.GetKeyOidLenIndex != "" || fieldHasCodeLengthName(cam, table, o.Field) {
			ret.FieldHasLen = true
		}
		ret.FieldLen = getFieldLenFromCam(cam, table, o.Field)
		ret.IsKey = getFieldIsKeyFromCam(cam, table, o.Field)
		ret.IsReadOnly = getFieldIsReadOnlyFromCam(cam, table, o.Field)
		ret.SetFieldIdx = getFieldIdxFromCam(cam, table, o.Field)
		ret.CppDataType = getFieldDataTypeFromCam(cam, table, o.Field)
	}
	return ret, err
}

func getPdsaCastSetFunc(protoFieldTypeName gogoproto.FieldDescriptorProto_Type, camInfoFieldTypeName string, f pdsaFieldOpt) string {
	if protoFieldTypeName == gogoproto.FieldDescriptorProto_TYPE_FIXED32 && camInfoFieldTypeName == "byteArray" {
		return "NBB_PUT_LONG"
	}
	if protoFieldTypeName == gogoproto.FieldDescriptorProto_TYPE_STRING && camInfoFieldTypeName == "byteArray" {
		if f.SetKeyOidLenIndex != "" || f.FieldHasLen == true {
			return "pdsa_set_string_in_byte_array_with_len"
		}
		return "pdsa_set_string_in_byte_array"
	}
	return ""
}

func getPdsaCastGetFunc(protoFieldTypeName gogoproto.FieldDescriptorProto_Type, camInfoFieldTypeName string, f pdsaFieldOpt) string {
	if protoFieldTypeName == gogoproto.FieldDescriptorProto_TYPE_FIXED32 && camInfoFieldTypeName == "byteArray" {
		return "pdsa_nbb_get_long"
	}
	if protoFieldTypeName == gogoproto.FieldDescriptorProto_TYPE_STRING && camInfoFieldTypeName == "byteArray" {
		if f.GetKeyOidLenIndex != "" || f.FieldHasLen == true {
			return "pdsa_get_string_in_byte_array_with_len"
		}
		return "pdsa_get_string_in_byte_array"
	}
	return ""
}

func getCppTypeFieldFromProto(protoFieldType gogoproto.FieldDescriptorProto_Type, protoFieldTypeName string) string {
	if protoFieldType == gogoproto.FieldDescriptorProto_TYPE_ENUM {
		return strings.ReplaceAll(protoFieldTypeName, ".", "::")
	}
	return ""
}

func getFieldMetricOptions(f *descriptor.Field) (fieldMetricOptions, bool) {
	ret := fieldMetricOptions{}
	i, err := reg.GetExtension("venice.metricsField", f)
	if err == nil {
		o := i.(*venice.MetricFieldInfo)
		ret.Name = gogen.CamelCase(*f.Name)
		ret.DisplayName = o.DisplayName
		ret.Description = o.Description
		ret.Units = o.Units.String()
		ret.ScaleMax = o.ScaleMax
		ret.ScaleMin = o.ScaleMin
		for _, t := range o.Tags {
			ret.Tags = append(ret.Tags, t.String())
		}
		if isScalarType(f.Type.String()) {
			ret.BaseType = mapScalarTypes(*f.Type)
			switch *f.Type {
			case gogoproto.FieldDescriptorProto_TYPE_ENUM:
				en, err := f.Message.File.Reg.LookupEnum("", *f.TypeName)
				if err != nil {
					glog.Fatalf("failed to get enum [%v] (%s)", *f.TypeName, err)
				}
				for _, v := range en.Value {
					ret.AllowedValues = append(ret.AllowedValues, fmt.Sprintf("%d", v.GetNumber()))
				}
			case gogoproto.FieldDescriptorProto_TYPE_STRING:
				if profile, err := common.GenerateVeniceCheckFieldProfile(f, f.Message.File.Reg); err == nil {
					if profile != nil && len(profile.Enum) > 0 {
						if strs, ok := profile.Enum["all"]; ok {
							ret.AllowedValues = strs
						}
					}
				}
			}
		} else {
			switch f.GetTypeName() {
			case ".delphi.Counter":
				if o.Units.String() == "Bitmap" {
					ret.BaseType = "Bitmap"
					eName := ""
					if strings.HasPrefix(o.AllowedVal, ".") {
						eName = o.AllowedVal
					} else {
						eName = "." + f.Message.File.GoPkg.Name + "." + o.AllowedVal
					}
					glog.V(1).Infof("Enum is %s", eName)

					enum, err := f.Message.File.Reg.LookupEnum("", eName)
					if err != nil {
						glog.V(1).Infof("Enum not found")
						return ret, false
					}
					glog.V(1).Infof("Enum is %s", enum.String())
					var allowedMap = make(map[string]int32)
					for _, v := range enum.Value {
						n := common.GetVName(v)
						idx := v.GetNumber()
						allowedMap[n] = idx
						ret.AllowedValues = append(ret.AllowedValues, n)
					}

					glog.V(1).Infof("Allowed values %s", ret.AllowedValues)
				} else {
					ret.BaseType = "Counter"
				}
			case ".delphi.Gauge":
				ret.BaseType = "Gauge"
			default:
				ret.BaseType = f.GetTypeName()
			}
		}
		return ret, true
	}
	return ret, false
}

func getMsgMetricOptions(m *descriptor.Message) (msgMetricOptions, bool) {
	ret := msgMetricOptions{}
	i, err := reg.GetExtension("venice.metricInfo", m)
	if err == nil {
		om := i.(*venice.MetricInfo)
		ret.Name = gogen.CamelCase(*m.Name)
		ret.Description = om.Description
		ret.DisplayName = om.DisplayName
		ret.Scope = om.Scope.String()
		for _, t := range om.Tags {
			ret.Tags = append(ret.Tags, t.String())
		}

		for _, f := range m.Fields {
			fopts, ok := getFieldMetricOptions(f)
			if ok {
				ret.Fields = append(ret.Fields, fopts)
			}
		}
		return ret, true
	}
	return ret, false
}
func getMsgMetricOptionsHdlr(m *descriptor.Message) (msgMetricOptions, error) {
	ret, ok := getMsgMetricOptions(m)
	if !ok {
		return msgMetricOptions{}, errors.New("failed to get metric options")
	}
	return ret, nil
}

func getFileMetricsOptions(f *descriptor.File) (fileMetricOptions, bool) {
	ret := fileMetricOptions{}
	ret.FileName = *f.Name
	ret.Package = f.GoPkg.Name

	for _, m := range f.Messages {
		mopts, ok := getMsgMetricOptions(m)
		if ok {
			ret.Messages = append(ret.Messages, mopts)
		}
	}
	return ret, true
}

func genFileMetricsJSON(f *descriptor.File, prefix string) (string, error) {
	fopts, ok := getFileMetricsOptions(f)
	if !ok {
		return "", errors.New("failed to generate metrics json")
	}
	fopts.Prefix = prefix
	ret, err := json.MarshalIndent(fopts, "", "  ")
	if err != nil {
		return "", err
	}
	return string(ret), nil
}

func getFileMsgFieldMap(f *descriptor.File) (fileMsgFieldMapOpt, error) {
	mapOpt := fileMsgFieldMapOpt{}
	mapOpt.FileName = *f.Name
	mapOpt.Package = f.GoPkg.Name
	mapOpt.Maps = map[string][]string{}

	for _, m := range f.Messages {
		mopts, ok := getMsgMetricOptions(m)
		if ok {
			key := mopts.Name
			values := []string{}
			for _, field := range mopts.Fields {
				values = append(values, field.Name)
			}
			mapOpt.Maps[key] = values
		}
	}
	return mapOpt, nil
}

func getPenctlParentCmdOptions(m *descriptor.Message) ([]PenctlCmdOpts, error) {
	var penctlCmdOpts []PenctlCmdOpts
	i, _ := reg.GetExtension("venice.penctlParentCmd", m)
	for _, r := range i.([]*venice.PenCtlCmd) {
		var penctlCmdOpt PenctlCmdOpts
		penctlCmdOpt.Cmd = r.Cmd
		penctlCmdOpt.RootCmd = r.RootCmd
		penctlCmdOpt.HelpStr = r.HelpStr
		penctlCmdOpt.ShortHelpStr = r.ShortHelpStr
		if penctlCmdOpt.HelpStr == "" {
			// generate the help string from options defined on the fields
			if mopts, ok := getMsgMetricOptions(m); ok {
				penctlCmdOpt.HelpStr = fmt.Sprintf("%s\\n%s\\n", mopts.DisplayName, mopts.Description)
			}
		}
		if mopts, ok := getMsgMetricOptions(m); ok {
			if !strings.HasSuffix(penctlCmdOpt.HelpStr, "\n") {
				penctlCmdOpt.HelpStr = penctlCmdOpt.HelpStr + "\\n"
			}
			for _, fld := range mopts.Fields {
				if fld.Description != "" {
					penctlCmdOpt.HelpStr = fmt.Sprintf("%s%s\t: %s\\n", penctlCmdOpt.HelpStr, fld.Name, fld.Description)
				}
			}
		}

		penctlCmdOpts = append(penctlCmdOpts, penctlCmdOpt)
	}
	return penctlCmdOpts, nil
}

func getPenctlCmdOptions(m *descriptor.Message) (PenctlCmdOpts, error) {
	var penctlCmdOpts PenctlCmdOpts
	i, _ := reg.GetExtension("venice.penctlCmd", m)
	r := i.(*venice.PenCtlCmd)
	penctlCmdOpts.Cmd = r.Cmd
	penctlCmdOpts.RootCmd = r.RootCmd
	penctlCmdOpts.HelpStr = r.HelpStr
	penctlCmdOpts.ShortHelpStr = r.ShortHelpStr
	if penctlCmdOpts.HelpStr == "" {
		// generate the help string from options defined on the fields
		if mopts, ok := getMsgMetricOptions(m); ok {
			penctlCmdOpts.HelpStr = fmt.Sprintf("%s\\n%s\\n", mopts.DisplayName, mopts.Description)
		}
	}
	if mopts, ok := getMsgMetricOptions(m); ok {
		if !strings.HasSuffix(penctlCmdOpts.HelpStr, "\n") {
			penctlCmdOpts.HelpStr = penctlCmdOpts.HelpStr + "\\n"
		}
		for _, fld := range mopts.Fields {
			if fld.Description != "" {
				penctlCmdOpts.HelpStr = fmt.Sprintf("%s%s\t: %s\\n", penctlCmdOpts.HelpStr, fld.Name, fld.Description)
			}
		}
	}
	return penctlCmdOpts, nil
}

func getFamFromCam(cam *CamInfo, table string) string {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.Mib == table {
			return mibInfo.Droid
		}
	}
	return ""
}

func getStructFromCam(cam *CamInfo, table string) string {
	for _, mibInfo := range cam.Mibs.MibInfo {
		if mibInfo.Mib == table {
			return mibInfo.CodeName
		}
	}
	return ""
}

func getPdsaSetGlobalOpts(m *descriptor.Message, cam *CamInfo) (PdsaGlobalOpts, error) {
	var pdsaGlobOpts PdsaGlobalOpts
	i, _ := reg.GetExtension("venice.pdsaSetGlobOpts", m)
	r := i.(*venice.GlobalOpts)
	pdsaGlobOpts.OidLen = r.OidLen
	pdsaGlobOpts.Mib = r.Mib
	pdsaGlobOpts.Struct = getStructFromCam(cam, r.Mib)
	pdsaGlobOpts.OidFam = getFamFromCam(cam, r.Mib)
	pdsaGlobOpts.FillFn = r.FillFn
	return pdsaGlobOpts, nil
}

func getPdsaGetGlobalOpts(m *descriptor.Message, cam *CamInfo) (PdsaGlobalOpts, error) {
	var pdsaGlobOpts PdsaGlobalOpts
	i, _ := reg.GetExtension("venice.pdsaGetGlobOpts", m)
	r := i.(*venice.GlobalOpts)
	pdsaGlobOpts.OidLen = r.OidLen
	pdsaGlobOpts.Mib = r.Mib
	pdsaGlobOpts.Struct = getStructFromCam(cam, r.Mib)
	pdsaGlobOpts.OidFam = getFamFromCam(cam, r.Mib)
	return pdsaGlobOpts, nil
}

// getRestSvcOptions returns the ServiceOptions for the service. This call will ensure that the raw venice.naplesRestService
// is passed to the templating logic for customization. This will also avoid generating the *.pb.gw files if we don't
// want them.
func getRestSvcOptions(s *descriptor.Service) ([]RestServiceOptions, error) {
	var restOptions []RestServiceOptions
	i, _ := reg.GetExtension("venice.naplesRestService", s)
	for _, r := range i.([]*venice.RestEndpoint) {
		var restService RestServiceOptions
		restService.CrudObject = r.Object
		restService.Methods = r.Method
		restService.Pattern = r.Pattern
		restOptions = append(restOptions, restService)
	}
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
	if d, err := reg.GetExtension("venice.methodTenantDefault", m); err == nil {
		params.TenantDefault = d.(bool)
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

// getCWD2 returns the cwd working directory but qualified by the parent directory.
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

type swaggerFile struct {
	Description string `json:"description,omitempty"`
	Path        string `json:"path,omitempty"`
}

var groupDocString = map[string]string{
	"auth":            "Authentication and Authorization related APIs",
	"audit":           "Audit logs for the cluster and related APIs",
	"browser":         "browse configuration objects and relations",
	"cluster":         "Configure and Manage the Venice cluster, manage nodes in the cluster, SmartNics, Tenants etc.",
	"diagnostics":     "Diagnostics APIs for the cluster",
	"events":          "Monitor events on the cluster",
	"monitoring":      "Configure and manage Event, Stats, Logging, Alerts, Mirror Sessions and other policies",
	"network":         "Configure and Manage Networks and Services",
	"rollout":         "Configure and manage rollout feature to upgrade/downgrade software for the cluster",
	"search":          "Powerful search API to search configuration policies, events etc.",
	"security":        "Configure and manage security features like Security Groups, Rules, Certificates etc.",
	"staging":         "APIS to stage configuration and commit in transactions",
	"telemetry_query": "Query telemetry information for the cluster",
	"tokenauth":       "Manage tokens to access nodes in the cluster",
	"workload":        "Configure and manage Workloads, Endpoints etc.",
}

func genSwaggerMap(desc *descriptor.File, path string) (map[string]swaggerFile, error) {
	var index map[string]swaggerFile

	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		index = make(map[string]swaggerFile)
	} else {
		glog.V(1).Infof("manifest exists, reading from manifest")
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return index, err
		}
		err = json.Unmarshal(raw, &index)
		if err != nil {
			glog.Fatalf("unable to parse swagger index file")
		}
	}

	internal := false
	for _, v := range common.InternalGroups {
		if v == desc.GoPkg.Name {
			internal = true
			break
		}
	}
	if !internal {
		descStr := strings.Title(desc.GoPkg.Name)
		descStr = strings.Replace(descStr, "_", " ", -1)
		docstr := groupDocString[desc.GoPkg.Name]
		if docstr == "" {
			docstr = descStr + " API reference"
		}
		sf := swaggerFile{
			Description: docstr,
			Path:        "/swagger/" + desc.GoPkg.Name,
		}
		index[desc.GoPkg.Name] = sf
	}
	return index, nil
}

func genSwaggerIndex(desc *descriptor.File, path string) (string, error) {
	index, err := genSwaggerMap(desc, path)
	if err != nil {
		return "", err
	}
	out, err := json.MarshalIndent(index, "  ", "  ")
	if err != nil {
		glog.Fatalf("unable to marshall swagger index file")
	}
	return string(out), nil
}

func getSwaggerMD(desc *descriptor.File, path string) (string, error) {
	index, err := genSwaggerMap(desc, path)
	if err != nil {
		return "", err
	}
	keys := []string{}
	for k := range index {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	ret := "\n# Swagger Definitions\n\n|API group   | Description    |\n"
	ret = ret + "|:---------- |:-------------- |\n"
	for _, k := range keys {
		grp := index[k]
		ret = fmt.Sprintf("%s| [%s](%s) | %s |\n", ret, k, grp.Path, grp.Description)
	}
	return ret, nil
}

func getAPIRefMD(desc *descriptor.File, path string) (string, error) {
	index, err := genSwaggerMap(desc, path)
	if err != nil {
		return "", err
	}
	keys := []string{}
	for k := range index {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	ret := "\n|API group   | Description    |\n"
	ret = ret + "|:-----------|:---------------|\n"
	for _, k := range keys {
		grp := index[k]
		ret = fmt.Sprintf("%s| [%s](%s) | %s |\n", ret, strings.Title(k), fmt.Sprintf("generated/apiref/%s/index.html", k), grp.Description)
	}
	return ret, nil
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

type nimbusManifestFile struct {
	Object  string
	Service string
}

func parseNimbusManifestFile(raw []byte) map[string]nimbusManifestFile {
	manifest := make(map[string]nimbusManifestFile)
	lines := bytes.Split(raw, []byte("\n"))
	for _, line := range lines {
		fields := bytes.Fields(line)
		if len(fields) == 2 {
			manifest[string(fields[0])] = nimbusManifestFile{
				Object:  string(fields[0]),
				Service: string(fields[1]),
			}
		}
	}
	return manifest
}

// genManifest generates the current manifest of protos being processed.
func genNimbusManifest(path, object, service string) (map[string]nimbusManifestFile, error) {
	var manifest map[string]nimbusManifestFile
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		manifest = make(map[string]nimbusManifestFile)
	} else {
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return nil, err
		}
		manifest = parseNimbusManifestFile(raw)
	}
	manifest[service] = nimbusManifestFile{
		Object:  object,
		Service: service,
	}
	return manifest, nil
}

// getNimbusManifest gets nimbus manifest file
func getNimbusManifest(path string) (map[string]nimbusManifestFile, error) {
	var manifest map[string]nimbusManifestFile
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		return nil, errors.New("Manifest not found")
	}
	raw, err := ioutil.ReadFile(path)
	if err != nil {
		glog.V(1).Infof("Reading Manifest failed (%s)", err)
		return nil, err
	}
	manifest = parseNimbusManifestFile(raw)
	return manifest, nil
}

func getServiceKey(filename, pkg, service, message string) (string, error) {
	manifest, err := getServiceManifest(filename)
	if err != nil {
		return "", errors.New("Error opening service manifest")
	}
	pkgDef, ok := manifest[pkg]
	if !ok {
		return "", errors.New("Package not found")
	}
	svcs, ok := pkgDef.Svcs[service]
	if !ok {
		return "", errors.New("service not found")
	}

	for msg, msgDef := range svcs.Properties {
		if message == msg {
			ss := strings.Split(msgDef.URI, "/")
			return ss[len(ss)-1], nil
		}
	}
	return "", errors.New("Service key not found")
}

type pkgManifest struct {
	Files     []string
	APIServer bool
}

func parsePkgManifest(raw []byte) map[string]pkgManifest {
	manifest := make(map[string]pkgManifest)
	lines := bytes.Split(raw, []byte("\n"))
	for _, line := range lines {
		fields := bytes.Fields(line)
		if len(fields) > 2 {
			apiserver, err := strconv.ParseBool(string(fields[0]))
			if err != nil {
				glog.Fatalf("malformed pkg manifest [%s]", string(line))
			}
			pkg := string(fields[1])
			files := []string{}
			for i := 2; i < len(fields); i++ {
				files = append(files, string(fields[i]))
			}
			manifest[pkg] = pkgManifest{
				APIServer: apiserver,
				Files:     files,
			}
		}
	}
	return manifest
}

func genPkgManifest(desc *descriptor.File, path, pkg, file string) (map[string]pkgManifest, error) {
	var manifest map[string]pkgManifest
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", path)
		manifest = make(map[string]pkgManifest)
	} else {
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Manifest failed (%s)", err)
			return nil, err
		}
		manifest = parsePkgManifest(raw)
	}
	apiserver, _ := isAPIServerServed(desc)
	file = filepath.Base(file)
	var ok bool
	var m pkgManifest
	if m, ok = manifest[pkg]; !ok {
		m = pkgManifest{APIServer: apiserver}
	}
	found := false
	for i := range m.Files {
		if m.Files[i] == file {
			found = true
			break
		}
	}
	if !found {
		m.Files = append(m.Files, file)
	}

	manifest[pkg] = m
	return manifest, nil
}

//
type packageDef struct {
	Svcs  map[string]serviceDef
	Files []string
}

type serviceDef struct {
	Version    string
	Messages   []string
	Properties map[string]messageDef `json:",omitempty"`
}

type messageDef struct {
	Scopes      []string `json:",omitempty"`
	RestMethods []string `json:",omitempty"`
	Actions     []string `json:",omitempty"`
	URI         string
}

// getServiceManifest retrieves the manifest from file specified in arg
func getServiceManifest(filenm string) (map[string]packageDef, error) {
	manifest := make(map[string]packageDef)
	if _, err := os.Stat(filenm); os.IsNotExist(err) {
		glog.V(1).Infof("manifest [%s] not found", filenm)
		return manifest, nil
	}
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
func genServiceManifestInternal(filenm string, file *descriptor.File) (map[string]packageDef, error) {
	manifest, err := getServiceManifest(filenm)
	if err != nil {
		return manifest, err
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
			glog.V(1).Infof("unversioned service, ingnoring for svc manifest [%s](%s)", *svc.Name, err)
			continue
		}
		svcdef := serviceDef{
			Version:    ver.(string),
			Properties: make(map[string]messageDef),
		}
		crudsvc, err := reg.GetExtension("venice.apiGrpcCrudService", svc)
		if err != nil {
			glog.V(1).Infof("no crudService specified for [%s](%s)", *svc.Name, err)
		} else {
			glog.V(1).Infof("Found crudsvcs %v", crudsvc.([]string))
			svcdef.Messages = crudsvc.([]string)
		}
		restSvc := make(map[string][]string)
		rOps, err := reg.GetExtension("venice.apiRestService", svc)
		if err == nil {
			r := rOps.([]*venice.RestEndpoint)
			for _, v := range r {
				restSvc[v.Object] = v.Method
			}
		}
		actSvc := make(map[string][]string)
		aOps, err := reg.GetExtension("venice.apiAction", svc)
		if err == nil {
			r := aOps.([]*venice.ActionEndpoint)
			for _, v := range r {
				t := v.GetObject()
				if t == "" {
					t = v.GetCollection()
				}
				if t != "" {
					l := actSvc[t]
					l = append(l, v.Action)
					actSvc[t] = l
				}
			}
		}
		svcParams, err := getSvcParams(svc)
		if err != nil {
			glog.V(1).Infof("failed to get svc params for [%v](%s)", *svc.Name, err)
		}
		if len(svcdef.Messages) > 0 {
			for _, m := range svcdef.Messages {
				mname := fmt.Sprintf(".%s.%s", file.GoPkg.Name, m)
				if msg, err := file.Reg.LookupMsg("", mname); err != nil {
					glog.V(1).Infof("Failed to retrieve message %v for svc manifest", mname)
					continue
				} else {
					cat, _ := getFileCategory(msg)
					kc, err := getMsgURI(msg, svcParams.Version, svcParams.Prefix)
					if err != nil {
						glog.Fatalf("unable to get URI key for message [%s],(%s", *msg.Name, err)
					}
					uri := "/" + cat
					for _, k := range kc {
						if k.Type == "prefix" {
							uri = uri + "/" + k.Val
						} else {
							uri = uri + "/{" + k.Val + "}"
						}
					}
					msgdef := messageDef{
						URI: strings.TrimSuffix(strings.Replace(strings.Replace(uri, "//", "/", -1), "tenant/{Tenant}/", "", -1), "/{Name}"),
					}
					if y, _ := isTenanted(msg); y {
						msgdef.Scopes = append(msgdef.Scopes, "tenant")
					} else {
						msgdef.Scopes = append(msgdef.Scopes, "cluster")
					}
					if y, _ := isNamespaced(msg); y {
						msgdef.Scopes = append(msgdef.Scopes, "namespace")
					}
					if r, ok := restSvc[m]; ok {
						msgdef.RestMethods = r
					}
					if a, ok := actSvc[m]; ok {
						msgdef.Actions = a
					}
					svcdef.Properties[m] = msgdef
				}
			}
			pkgdef.Svcs[*svc.Name] = svcdef
		}
	}
	if len(pkgdef.Svcs) > 0 {
		manifest[pkg] = pkgdef
	}
	return manifest, nil
}

// genServiceManifest generates a service manifest at the location specified.
//  The manifest is created in an additive fashion since it is called once per protofile.
func genServiceManifest(filenm string, file *descriptor.File) (string, error) {
	manifest, err := genServiceManifestInternal(filenm, file)
	if err != nil {
		glog.Fatalf("failed to Generate service manifest")
	}
	ret, err := json.MarshalIndent(manifest, "", "  ")
	if err != nil {
		glog.Fatalf("failed to marshal service manifest")
	}

	return string(ret), nil
}

func genObjectURIs(filenm string, file *descriptor.File) (string, error) {
	ret := make(map[string][]string)
	manifest, err := genServiceManifestInternal(filenm, file)
	if err != nil {
		glog.Fatalf("failed to Generate service manifest")
	}
	for g, p := range manifest {
		internal := false
		for _, v := range common.InternalGroups {
			if v == g {
				internal = true
				break
			}
		}
		if internal {
			continue
		}
		for _, s := range p.Svcs {
			for k, m := range s.Properties {
				u := ret[k]
				u = append(u, m.URI)
				ret[k] = u
			}
		}
	}
	keys := []string{}
	for k := range ret {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	retstr := "| Object | URI |\n"
	retstr = retstr + "|:---------|:------------|\n"
	for _, k := range keys {
		uris := ""
		sep := ""
		for _, us := range ret[k] {
			uris = uris + sep + us
			sep = ", "
		}
		retstr = retstr + fmt.Sprintf("| %s | %v |\n", k, uris)
	}
	return retstr, nil
}

type validateArg struct {
	Tpe  string
	Str  string
	Intg uint64
}

type validateFields struct {
	Repeated   bool
	Pointer    bool
	Validators []common.ValidateField
}
type validateMsg struct {
	Fields map[string]validateFields
}

type validators struct {
	Fmap bool
	Map  map[string]validateMsg
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
	// add in map with temp value to handle recursion.
	msgmap[name] = found
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
					fldv, err := common.ParseValidator(v)
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
				msgname := *msg.Name
				if isNestedMessage(msg) {
					if msg.GetOneofDecl() != nil {
						continue
					}
					msgname, _ = getNestedMsgName(msg)
				}
				if msgmap[*fld.TypeName] == true {
					if _, ok := ret.Map[*msg.Name]; !ok {
						ret.Map[msgname] = validateMsg{Fields: make(map[string]validateFields)}
					}
					if _, ok := ret.Map[*fld.TypeName]; !ok {
						repeated := false
						pointer := true
						if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
							repeated = true
						}
						if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
							glog.Infof("setting pointer found nullable [%v] for %s]", r, msgname+"/"+*fld.Name)
							pointer = r.(bool)
						}
						glog.Infof("setting pointer to [%v] for {%s]", pointer, msgname+"/"+*fld.Name)
						// if it is a embedded field, do not use field name rather use type
						if fld.Embedded {
							// fld.GetTypeName() -> e.g. ".events.EventAttributes"
							temp := strings.Split(fld.GetTypeName(), ".")
							fldType := temp[len(temp)-1]
							ret.Map[msgname].Fields[fldType] = validateFields{Validators: make([]common.ValidateField, 0), Repeated: repeated, Pointer: pointer}
						} else {
							ret.Map[msgname].Fields[*fld.Name] = validateFields{Validators: make([]common.ValidateField, 0), Repeated: repeated, Pointer: pointer}
						}
					}
				}
			}
		}
	}
	glog.Infof("Validator Manifest is %+v", ret)
	return ret, nil
}

// VerDefaults is defaults for a field by version
type VerDefaults struct {
	Repeated bool
	Pointer  bool
	Nested   bool
	Val      string
}

type msgDefaults struct {
	Fields   map[string]common.Defaults
	Versions map[string]map[string]VerDefaults
}

type fileDefaults struct {
	Fmap bool
	Map  map[string]msgDefaults
}

func checkDefaults(file *descriptor.File, msgmap map[string]bool, name string, ret *fileDefaults) (bool, error) {
	if _, ok := msgmap[name]; ok {
		return msgmap[name], nil
	}
	defaulted := false
	msg, err := file.Reg.LookupMsg("", name)
	if err != nil {
		// must be a internal type generated like map entry
		glog.Infof("Failed to retrieve message %s", name)
		return false, nil
	}
	if msg.File != file {
		glog.V(1).Infof("from a different file [%s]", *msg.File.Name)
		if strings.HasPrefix(*msg.File.Name, "github.com/pensando/sw/api/protos/") {
			return true, nil
		}
		return false, nil
	}
	for _, fld := range msg.Fields {
		r, found, err := common.ParseDefaults(file, fld)
		if err != nil {
			glog.V(1).Infof("[%v.%v]got error parsing defaulters (%s)", *msg.Name, *fld.Name, err)
			return false, err
		}
		if found {
			if _, ok := ret.Map[*msg.Name]; !ok {
				glog.V(1).Infof("Creating a new map for [%s]", *msg.Name)
				ret.Map[*msg.Name] = msgDefaults{
					Fields:   make(map[string]common.Defaults),
					Versions: make(map[string]map[string]VerDefaults),
				}
			} else {
				glog.V(1).Infof("Found Existing map for [%s][%+v]", *msg.Name, ret.Map[*msg.Name])
			}

			fldName := *fld.Name
			if fld.Embedded {
				temp := strings.Split(fld.GetTypeName(), ".")
				fldName = temp[len(temp)-1]
			}
			ret.Map[*msg.Name].Fields[fldName] = r
			for fv, fd := range r.Map {
				glog.V(1).Infof("RetMap is [%s][%+v]", *msg.Name, ret.Map[*msg.Name])
				if _, ok := ret.Map[*msg.Name].Versions[fv]; !ok {
					ret.Map[*msg.Name].Versions[fv] = make(map[string]VerDefaults)
				}
				verDef := VerDefaults{
					Repeated: r.Repeated,
					Pointer:  r.Pointer,
					Nested:   r.Nested,
					Val:      fd,
				}
				ret.Map[*msg.Name].Versions[fv][fldName] = verDef
			}
			defaulted = true
		}
		if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
			found, err = checkDefaults(file, msgmap, *fld.TypeName, ret)
			if found {
				defaulted = true
				repeated := false
				pointer := true
				if _, ok := ret.Map[*msg.Name]; !ok {
					ret.Map[*msg.Name] = msgDefaults{Fields: make(map[string]common.Defaults), Versions: make(map[string]map[string]VerDefaults)}
				}
				fldName := *fld.Name
				if fld.Embedded {
					temp := strings.Split(fld.GetTypeName(), ".")
					fldName = temp[len(temp)-1]
				}
				if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
					repeated = true
				}
				if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
					glog.Infof("setting pointer found nullable [%v] for %s]", r, *msg.Name+"/"+*fld.Name)
					pointer = r.(bool)
				}

				ret.Map[*msg.Name].Fields[fldName] = common.Defaults{Pointer: pointer, Repeated: repeated, Nested: true}
			}
		}
	}

	return defaulted, nil
}

func getDefaulterManifest(file *descriptor.File) (fileDefaults, error) {
	ret := fileDefaults{Map: make(map[string]msgDefaults)}
	msgmap := make(map[string]bool)
	for _, msg := range file.Messages {
		var found bool
		var err error
		name := "." + file.GoPkg.Name + "." + *msg.Name
		if found, err = checkDefaults(file, msgmap, name, &ret); err != nil {
			return ret, err
		}
		ret.Fmap = ret.Fmap || found
	}
	return ret, nil
}

// -- Storage Transformers ---
type storageTransformerArgs func(string) bool

// storageTransformerArgMap contains storage transformers recognized by the
// parser. The key is the name of the transformer (as it appears in the
// venice.StorageTransformer="NAME(Args)" annotation) and the value is a list of
// validators, one for each expected argument.
// Currently the only available transformer is "Secret()" (see sw/docs/secrets.md)
//
// Storage transformers must implement the interface defined in
// sw/venice/utils/transformers/storage/types.go and the implementation
// should be placed in the same directory.
// Autogenerated code tries to instantiate a storage transformer called
// <NAME> by invoking the factory function New<NAME>ValueTransformer()
//
// See the setion named "storage transformers" in docs/apidef.md for more details.
var storageTransformerArgMap = map[string][]storageTransformerArgs{
	"Secret": {},
}

type storageTransformerField struct {
	Fn   string
	Args []string
}

type storageTransformerFields struct {
	Repeated     bool
	Pointer      bool
	TypeCast     string
	Transformers []storageTransformerField
}
type storageTransformerMsg struct {
	Fields          map[string]storageTransformerFields
	HasTransformers bool // true if this message or any nested message has fields with storageTransformers
}

type storageTransformers struct {
	Fmap bool
	Map  map[string]storageTransformerMsg
}

func checkStorageTransformers(file *descriptor.File, msgmap map[string]bool, name string) bool {
	if _, ok := msgmap[name]; ok {
		return msgmap[name]
	}
	m, err := file.Reg.LookupMsg("", name)
	if err != nil {
		glog.Fatalf("Failed to retrieve message %s", name)
	}
	glog.Infof(" checkStorageTransformers on %s", name)
	found := false
	// add in map with temp value to handle recursion.
	msgmap[name] = found
	for _, fld := range m.Fields {
		_, err := reg.GetExtension("venice.storageTransformer", fld)
		if err == nil {
			found = true
		}
		if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
			found = found || checkStorageTransformers(file, msgmap, *fld.TypeName)
		}
	}
	msgmap[name] = found
	if found {
		glog.Infof(" checkStorageTransformers found true %s", name)
	}
	return found
}

func parseStorageTransformers(in string) (*storageTransformerField, error) {
	ret := storageTransformerField{}
	re := regexp.MustCompile("^(?P<func>[a-zA-Z0-9_\\-]+)\\((?P<args>[a-zA-Z0-9_\\-\\, \\.\\:]*)*\\)")
	params := re.FindStringSubmatch(in)
	if params == nil {
		return nil, fmt.Errorf("Failed to parse storageTransformer [%s]", in)
	}
	ret.Fn = params[1]
	ret.Args = strings.Split(strings.Replace(params[2], " ", "", -1), ",")
	if len(ret.Args) == 1 && ret.Args[0] == params[2] && params[2] == "" {
		ret.Args = []string{}
	}
	if targs, ok := storageTransformerArgMap[ret.Fn]; ok {
		if len(targs) != len(ret.Args) {
			return nil, fmt.Errorf("Incorrect number of args (%d) for %s", len(ret.Args), ret.Fn)
		}
		for i := range targs {
			if !targs[i](ret.Args[i]) {
				return nil, fmt.Errorf("storageTransformer validation for arg(%s) failed for %s", ret.Args[i], ret.Fn)
			}
		}
	} else {
		return nil, fmt.Errorf("unknown storageTransformer %s", ret.Fn)
	}
	return &ret, nil
}

func getMetaswitchMibTablesInfo() (*CamInfo, error) {
	gopath := os.Getenv("GOPATH")
	if gopath == "" {
		gopath = build.Default.GOPATH
	}
	path := gopath + "/src/github.com/pensando/sw/nic/metaswitch/stubs/mgmt/gen"
	_ = os.Mkdir(path+"/mgmt", 755)
	_ = os.Mkdir(path+"/svc", 755)

	// Open our xmlFile
	xmlFile, err := os.Open(gopath + "/src/github.com/pensando/sw/nic/third-party/metaswitch/mibref/mib.xml")
	// if we os.Open returns an error then handle it
	if err != nil {
		fmt.Println(err)
		return nil, err
	}

	// defer the closing of our xmlFile so that we can parse it later on
	defer xmlFile.Close()

	// read our opened xmlFile as a byte array.
	byteValue, _ := ioutil.ReadAll(xmlFile)

	var camInfo CamInfo
	xml.Unmarshal(byteValue, &camInfo)
	return &camInfo, nil
}

func getStorageTransformersManifest(file *descriptor.File) (*storageTransformers, error) {
	ret := storageTransformers{Map: make(map[string]storageTransformerMsg)}
	msgmap := make(map[string]bool)
	for _, msg := range file.Messages {
		for _, fld := range msg.Fields {
			r, err := reg.GetExtension("venice.storageTransformer", fld)
			glog.Infof(" Check storageTransformer for message %s.%s", *msg.Name, *fld.Name)

			if err == nil {
				glog.Infof(" Found storageTransformer for message %s.%s,[ %v ]", *msg.Name, *fld.Name, r)
				if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
					return nil, fmt.Errorf("StorageTransformers allowed on scalar types only [%s]", *fld.Name)
				}
				if *fld.Type != gogoproto.FieldDescriptorProto_TYPE_STRING &&
					*fld.Type != gogoproto.FieldDescriptorProto_TYPE_BYTES {
					return nil, fmt.Errorf("StorageTransformers allowed on types \"strings\" and \"bytes\" only [%s]", *fld.Name)
				}
				if _, ok := ret.Map[*msg.Name]; !ok {
					ret.Map[*msg.Name] = storageTransformerMsg{
						Fields:          make(map[string]storageTransformerFields),
						HasTransformers: true,
					}
				}
				repeated := false
				if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
					repeated = true
				}
				typeCast := ""
				if fld.IsString() {
					typeCast = "string"
				} else if fld.IsBytes() {
					typeCast = "[]byte"
				} else {
					return nil, fmt.Errorf("Transformers allowed on string and bytes types only [%s]", *fld.Name)
				}
				ret.Map[*msg.Name].Fields[*fld.Name] = storageTransformerFields{
					Repeated: repeated,
					TypeCast: typeCast,
				}
				for _, v := range r.([]string) {
					fldv, err := parseStorageTransformers(v)
					if err != nil {
						return nil, err
					}
					tfld := ret.Map[*msg.Name].Fields[*fld.Name]
					tfld.Transformers = append(tfld.Transformers, *fldv)
					ret.Map[*msg.Name].Fields[*fld.Name] = tfld
				}

				ret.Fmap = true
			} else {
				glog.Infof("Failed %s", err)
			}
			if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
				glog.Infof("checking storageTransformer for nested message (%s).%s", *msg.Name, *fld.Name)
				if _, ok := msgmap[*fld.TypeName]; !ok {
					msgmap[*fld.TypeName] = checkStorageTransformers(file, msgmap, *fld.TypeName)
				}
				msgname := *msg.Name
				if isNestedMessage(msg) {
					msgname, _ = getNestedMsgName(msg)
				}
				if msgmap[*fld.TypeName] == true {
					if _, ok := ret.Map[*msg.Name]; !ok {
						ret.Map[msgname] = storageTransformerMsg{
							Fields:          make(map[string]storageTransformerFields),
							HasTransformers: true,
						}
					}
					if _, ok := ret.Map[*fld.Name]; !ok {
						repeated := false
						pointer := true
						if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
							repeated = true
						}
						if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
							glog.Infof("setting pointer found nullable [%v] for %s]", r, msgname+"/"+*fld.Name)
							pointer = r.(bool)
						}
						glog.Infof("setting pointer to [%v] for {%s]", pointer, msgname+"/"+*fld.Name)
						// if it is a embedded field, do not use field name rather use type
						if fld.Embedded {
							// fld.GetTypeName() -> e.g. ".events.EventAttributes"
							temp := strings.Split(fld.GetTypeName(), ".")
							fldType := temp[len(temp)-1]
							ret.Map[msgname].Fields[fldType] = storageTransformerFields{Transformers: make([]storageTransformerField, 0), Repeated: repeated, Pointer: pointer}
						} else {
							ret.Map[msgname].Fields[*fld.Name] = storageTransformerFields{Transformers: make([]storageTransformerField, 0), Repeated: repeated, Pointer: pointer}
						}
					}
				}
			}
		}
	}
	glog.Infof("StorageTransformers Manifest is %+v", ret.Map)
	return &ret, nil
}

// --- End Storage Transformers ---

type reqsField struct {
	RefType  string
	Repeated bool
	Pointer  bool
	Scalar   bool
	Tag      string
	Service  string
	Kind     string
}

type reqsMsg struct {
	Fields map[string]reqsField
}
type apiReqs struct {
	Map map[string]reqsMsg
}

func getFieldParams(fld *descriptor.Field) (repeated, pointer bool) {
	if *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
		repeated = true
	}
	if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
		pointer = true
	}
	if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
		pointer = r.(bool)
	}
	return
}

func parseRequirement(ref venice.ObjectRln) *reqsField {
	var ret reqsField
	switch ref.Type {
	case "WeakRef", "NamedRef", "SelectorRef":
		ret.RefType = ref.Type
		parts := strings.Split(ref.To, "/")
		ret.Service = parts[0]
		ret.Kind = parts[1]
		ret.Scalar = true
	default:
		glog.Fatalf("unknown reference type %v", ref.Type)
	}
	return &ret
}

func checkReqs(file *descriptor.File, msgmap map[string]bool, name string) bool {
	if _, ok := msgmap[name]; ok {
		return msgmap[name]
	}
	m, err := file.Reg.LookupMsg("", name)
	if err != nil {
		glog.Fatalf("Failed to retrieve message %s", name)
	}
	found := false
	// add in map with temp value to handle recursion.
	msgmap[name] = found
	for _, fld := range m.Fields {
		v, err := reg.GetExtension("venice.objRelation", fld)
		if err == nil {
			req := parseRequirement(v.(venice.ObjectRln))
			if req != nil {
				found = true
			}
		}
		if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
			found = found || checkReqs(file, msgmap, *fld.TypeName)
		}
	}
	msgmap[name] = found
	return found
}

func getRequirementsManifest(file *descriptor.File) (apiReqs, error) {
	ret := apiReqs{Map: make(map[string]reqsMsg)}
	msgmap := make(map[string]bool)
	for _, msg := range file.Messages {
		if t, _ := isTenanted(msg); isSpecStatusMessage(msg) && t {
			req := reqsField{Scalar: true, Service: "cluster", Kind: "Tenant", RefType: "NamedRef"}
			if _, ok := ret.Map[*msg.Name]; !ok {
				ret.Map[*msg.Name] = reqsMsg{Fields: make(map[string]reqsField)}
			}
			rmsg := ret.Map[*msg.Name]
			rmsg.Fields["Tenant"] = req
		}
		for _, fld := range msg.Fields {
			v, err := reg.GetExtension("venice.objRelation", fld)
			if err == nil {
				if gogoproto.FieldDescriptorProto_TYPE_STRING != *fld.Type {
					glog.Fatalf("Relations allowed only on string types [%v/%v]", *msg.Name, *fld.Name)
				}
				req := parseRequirement(v.(venice.ObjectRln))
				if req == nil {
					continue
				}
				// XXX-TODO(sanjayt): add a check to make ensure that the Kind is relation is valid. This has to be
				//  done after all the protos have been parsed as post processing using the relations and svc manifest.
				req.Repeated, req.Pointer = getFieldParams(fld)
				if _, ok := ret.Map[*msg.Name]; !ok {
					ret.Map[*msg.Name] = reqsMsg{Fields: make(map[string]reqsField)}
				}
				if req.Repeated && req.RefType == "SelectorRef" {
					glog.Fatalf("Repeated field not allowed for selector references[%v/%v]", *msg.Name, *fld.Name)
				}
				rmsg := ret.Map[*msg.Name]
				rmsg.Fields[*fld.Name] = *req
				ret.Map[*msg.Name] = rmsg
			}

			if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
				glog.Infof("checking requirements for message (%s).%s", *msg.Name, *fld.Name)
				if _, ok := msgmap[*fld.TypeName]; !ok {
					msgmap[*fld.TypeName] = checkReqs(file, msgmap, *fld.TypeName)
				}
				msgname := *msg.Name
				if isNestedMessage(msg) {
					if msg.GetOneofDecl() != nil {
						continue
					}
					msgname, _ = getNestedMsgName(msg)
				}
				if _, ok := ret.Map[msgname]; !ok {
					ret.Map[msgname] = reqsMsg{Fields: make(map[string]reqsField)}
				}
				if msgmap[*fld.TypeName] {
					req := reqsField{}
					req.Repeated, req.Pointer = getFieldParams(fld)
					name := *fld.Name
					if fld.Embedded {
						// fld.GetTypeName() -> e.g. ".events.EventAttributes"
						temp := strings.Split(fld.GetTypeName(), ".")
						name = temp[len(temp)-1]
					}
					ret.Map[msgname].Fields[name] = req
				}
			}
		}
	}
	glog.Infof("Requirements manifest is %+v", ret)
	return ret, nil
}

func getRequirementPath(file *descriptor.File, req reqsField, tenant, name string) (string, error) {
	dest, err := file.Reg.LookupMsg("", fmt.Sprintf(".%s.%s", req.Service, req.Kind))
	if err != nil {
		return "", err
	}
	if req.RefType == "SelectorRef" {
		return fmt.Sprintf("\"labels:%s:%s:\" + %s + \":\" + %s", req.Service, req.Kind, tenant, name), nil
	}
	kc, err := getDbKey(dest)
	if err != nil {
		return "", err
	}
	ret := fmt.Sprintf("globals.ConfigRootPrefix + \"/%s/\"", req.Service)
	for i := range kc {
		switch kc[i].Type {
		case "prefix":
			ret = fmt.Sprintf("%s + \"%s\"", ret, kc[i].Val)
		case "field":
			switch kc[i].Val {
			case "Tenant":
				ret = fmt.Sprintf("%s + %s", ret, tenant)
			case "Name":
				ret = fmt.Sprintf("%s + %s", ret, name)
			default:
				glog.Fatalf("cannot handle field [%v]in path for [%v]", kc[i].Val, *dest.Name)
			}
		}
	}
	return ret, nil
}

func derefStr(in *string) string {
	return *in
}

func getEnumStrMap(file *descriptor.File, in []string) (string, error) {
	return common.GetEnumStr(file, in, "vvalue")
}

func getEnumStrVMap(file *descriptor.File, in []string) (string, error) {
	return common.GetEnumStr(file, in, "vvalue")
}

func getEnumStrNormalMap(file *descriptor.File, in []string) (string, error) {
	return common.GetEnumStr(file, in, "normal")
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
func getRelMap(path string) error {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		glog.V(1).Infof("relationRef [%s] not found", path)
	} else {
		raw, err := ioutil.ReadFile(path)
		if err != nil {
			glog.V(1).Infof("Reading Relation failed (%s)", err)
			return err
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
	return nil
}

func genRelMap(path string) (string, error) {
	getRelMap(path)
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

type relationsMap struct {
	Keys []string
	Map  map[string][]relationRef
}

func genRelMapGo(path string) (relationsMap, error) {
	ret := relationsMap{}
	keys := []string{}
	if err := getRelMap(path); err != nil {
		return ret, err
	}
	for k := range relMap {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	ret.Keys = keys
	ret.Map = relMap
	return ret, nil
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

func getListTypeMsg(msg *descriptor.Message) (*descriptor.Message, error) {
	for _, f := range msg.Fields {
		if *f.Name == "Items" {
			ret, err := msg.File.Reg.LookupMsg("", f.GetTypeName())
			if err == nil {
				return ret, nil
			}
			return nil, err
		}
	}
	return nil, errors.New("list item not found")
}

func getWatchType(msg *descriptor.Message, fq bool) (string, error) {
	for _, f := range msg.Fields {
		if *f.Name == "Events" {
			nmsg, err := msg.File.Reg.LookupMsg("", f.GetTypeName())
			if err != nil {
				return "", errors.New("Object type not found")
			}
			for _, nf := range nmsg.Fields {
				if *nf.Name == "Object" {
					if fq {
						return strings.TrimPrefix(nf.GetTypeName(), "."), nil
					}
					parts := strings.Split(nf.GetTypeName(), ".")
					return parts[len(parts)-1], nil
				}
			}
		}
	}
	return "", errors.New("Object item not found")
}

func getWatchTypeMsg(msg *descriptor.Message) (*descriptor.Message, error) {
	for _, f := range msg.Fields {
		if *f.Name == "Events" {
			nmsg, err := msg.File.Reg.LookupMsg("", f.GetTypeName())
			if err != nil {
				return nil, errors.New("Object type not found")
			}
			for _, nf := range nmsg.Fields {
				if *nf.Name == "Object" {
					ret, err := msg.File.Reg.LookupMsg("", nf.GetTypeName())
					if err == nil {
						return ret, nil
					}
					return nil, err
				}
			}
		}
	}
	return nil, errors.New("Object item not found")
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

// ActionEndpoints specifies parameters of an action
type ActionEndpoints struct {
	Name              string
	Request, Response string
}

func getSvcCrudObjects(svc *descriptor.Service) ([]string, error) {
	var ret []string
	cruds, err := reg.GetExtension("venice.apiGrpcCrudService", svc)
	if err == nil {
		ret = cruds.([]string)
	}
	return ret, nil
}

func getSvcActionEndpoints(svc *descriptor.Service, target string) ([]ActionEndpoints, error) {
	var ret []ActionEndpoints
	act, err := reg.GetExtension("venice.apiAction", svc)
	if err != nil {
		return ret, nil
	}
	for _, r := range act.([]*venice.ActionEndpoint) {
		tgt := ""
		if tgt = r.GetCollection(); tgt == "" {
			if tgt = r.GetObject(); tgt == "" {
				continue
			}
		}
		if tgt == target {
			obj := ActionEndpoints{Name: strings.Title(r.Action), Request: r.Request, Response: r.Response}
			ret = append(ret, obj)
		}
	}
	return ret, nil
}

func getActionTarget(meth *descriptor.Method) (string, error) {
	if act, err := reg.GetExtension("venice.methodActionObject", meth); err == nil {
		return act.(string), nil
	}
	return "", errors.New("not found")
}

func isActionMethod(meth *descriptor.Method) bool {
	_, err := getActionTarget(meth)
	return err == nil
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

// isMapEntry checks if the message is a auto generated map entry message
func isMapEntry(msg *descriptor.Message) bool {
	glog.V(1).Infof("Looking for mapEntry in %s)", *msg.Name)
	if opt := msg.GetOptions(); opt != nil {
		return opt.GetMapEntry()
	}
	return false
}

func isNestedMessage(msg *descriptor.Message) bool {
	glog.V(1).Infof("Check nested message %s[%v]", *msg.Name, msg.Outers)
	if len(msg.Outers) != 0 {
		return true
	}
	return false
}

func getNestedMsgName(msg *descriptor.Message) (string, error) {
	if len(msg.Outers) == 0 {
		return "", errors.New("not a nested message")
	}
	ret := ""
	dlmtr := ""
	for _, n := range msg.Outers {
		ret = ret + dlmtr + n
		dlmtr = "_"
	}
	return ret + "_" + *msg.Name, nil
}

func isSpecStatusMessage(msg *descriptor.Message) bool {
	glog.V(1).Infof("Check if SpecStatus message for %s", *msg.Name)
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

func hasTypeMeta(msg *descriptor.Message) bool {
	if isListHelper(msg) || isWatchHelper(msg) {
		return false
	}
	for _, v := range msg.Fields {
		if *v.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE && *v.TypeName == ".api.TypeMeta" {
			return true
		}
	}
	return false
}

func hasListHelper(msg *descriptor.Message) bool {
	lhelper := "." + msg.File.GoPkg.Name + "." + *msg.Name + "List"
	if _, err := msg.File.Reg.LookupMsg("", lhelper); err == nil {
		return true
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

// Field represents the schema details of a field
type Field struct {
	Name       string
	JSONTag    string
	CLITag     cliInfo
	Pointer    bool
	Inline     bool
	Embed      bool
	FromInline bool
	Mutable    bool
	Slice      bool
	Map        bool
	// KeyType is valid only when Map is true
	KeyType string
	Type    string
}

// cliInfo captures all the parameters related to CLI
type cliInfo struct {
	tag     string
	cliType string
	path    string
	ins     string
	skip    bool
	help    string
}

// Struct represents the schema details of a field
type Struct struct {
	Kind     string
	APIGroup string
	Scopes   []string
	CLITags  map[string]cliInfo
	Fields   map[string]Field
	// keys is used to keep a stable order of Fields when generating the schema. This is
	//  a ordered set of keys in the Fielda map and follows the order in the corresponding
	//  slice in DescriptorProto.
	keys     []string
	mapEntry bool
}

// cliTagRegex is a regex for validating CLI parameters. Initialized in init.
var cliTagRegex *regexp.Regexp

// list of valid CLI tags
const (
	CLISSkipTag  = "verbose-only"
	CLIInsertTag = "ins"
	CLIIdTag     = "id"
)

// Validate string CLI tags
func validateCLITag(in string) bool {
	return cliTagRegex.MatchString(in)
}

// parseCLITags parses the cli-tags: string and updates the passed in Field
func parseCLITags(in string, fld *Field) {
	in = strings.TrimSpace(in)
	in = strings.TrimPrefix(in, "cli-tags:")
	fields := strings.Fields(in)
	for _, f := range fields {
		kv := strings.Split(f, "=")
		if len(kv) != 2 {
			glog.Fatalf("Invalid CLI tag specification [%v]", f)
		}
		switch kv[0] {
		case CLISSkipTag:
			b, err := strconv.ParseBool(kv[1])
			if err != nil {
				glog.Fatalf("Invalid format for CLI tag for %s [%v]", CLISSkipTag, f)
			}
			fld.CLITag.skip = b
		case CLIInsertTag:
			if !validateCLITag(kv[1]) {
				glog.Fatalf("Invalid format for CLI tag [%v]", f)
			}
			fld.CLITag.ins = kv[1]
		case CLIIdTag:
			if !validateCLITag(kv[1]) {
				glog.Fatalf("Invalid format for CLI tag [%v]", f)
			}
			fld.CLITag.tag = kv[1]
		}
	}
}

func parseFieldCLIParams(fld *descriptor.Field, strct *Struct, msg *descriptor.Message, path string, locs []int, file *descriptor.File, msgMap map[string]Struct) Field {
	name := *fld.Name
	if common.IsEmbed(fld) {
		p := strings.Split(*fld.TypeName, ".")
		name = p[len(p)-1]
	}
	sfld, ok := strct.Fields[name]
	if !ok {
		glog.Fatalf("did not find struct field for %s.%s", path, name)
	}
	loc, err := common.GetLocation(file.SourceCodeInfo, locs)
	if err != nil {
		return sfld
	}
	for _, line := range strings.Split(loc.GetLeadingComments(), "\n") {
		line = strings.TrimSpace(line)
		if strings.HasPrefix(line, "cli-tags:") {
			parseCLITags(line, &sfld)
		}
		if strings.HasPrefix(line, "cli-help:") {
			sfld.CLITag.help = strings.TrimSpace(strings.TrimPrefix(line, "cli-help:"))
		}
	}
	return sfld
}

// parseMessageCLIParams parses and updates CLI tags and help strings for a message
func parseMessageCLIParams(strct *Struct, msg *descriptor.Message, path string, locs []int, file *descriptor.File, msgMap map[string]Struct) {
	for flid, fld := range msg.Fields {
		flocs := append(locs, common.FieldType, flid)
		name := *fld.Name
		if common.IsEmbed(fld) {
			p := strings.Split(*fld.TypeName, ".")
			name = p[len(p)-1]
		}
		strct.Fields[name] = parseFieldCLIParams(fld, strct, msg, path, flocs, file, msgMap)
	}
	for nid, nmsg := range msg.NestedType {
		nfqname := path + "." + *nmsg.Name
		nstrct, ok := msgMap[nfqname]
		if !ok {
			glog.Fatalf("did not find struct for %s", nfqname)
		}
		nestedMsg, err := file.Reg.LookupMsg("", nfqname)
		if err != nil {
			glog.Fatalf("Could not find nested message %v (%s)", *nmsg.Name, err)
		}
		nlocs := append(locs, common.NestedMsgType, nid)
		parseMessageCLIParams(&nstrct, nestedMsg, nfqname, nlocs, file, msgMap)
		msgMap[nfqname] = nstrct
	}
}

// getCLIParams parses and updates CLI tags and help strings for a file.
func getCLIParams(file *descriptor.File, msgMap map[string]Struct) {
	pkg := file.GoPkg.Name
	for id, msg := range file.Messages {
		fqname := pkg + "." + *msg.Name
		if len(msg.Outers) > 0 {
			fqname = msg.Outers[0] + "." + *msg.Name
			fqname = strings.TrimPrefix(fqname, ".")
			fqname = pkg + "." + fqname
		}
		strct, ok := msgMap[fqname]
		if !ok {
			glog.Fatalf("did not find struct for %s", fqname)
		}
		locs := []int{common.MsgType, id}
		parseMessageCLIParams(&strct, msg, fqname, locs, file, msgMap)
		msgMap[fqname] = strct
	}
}

func genField(msg string, fld *descriptor.Field, file *descriptor.File) (Field, error) {
	ret := Field{}
	repeated := false
	if fld.Label != nil && *fld.Label == gogoproto.FieldDescriptorProto_LABEL_REPEATED {
		repeated = true
	}
	pointer := true
	if isScalarType(fld.Type.String()) {
		pointer = false
	}
	if r, err := reg.GetExtension("gogoproto.nullable", fld); err == nil {
		glog.Infof("setting pointer found nullable [%v] for %s]", r, msg+"/"+*fld.Name)
		pointer = r.(bool)
	}
	inline := common.IsInline(fld)
	embed := common.IsEmbed(fld)
	mutable := common.IsMutable(fld)
	isMap := false
	typeName := ""
	keyType := ""
	if *fld.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
		typeName = fld.GetTypeName()
		fmsg, err := file.Reg.LookupMsg("", typeName)
		if err != nil {
			glog.Infof("failed to get field %s", *fld.Name)
			return ret, fmt.Errorf("failed to get field %s", *fld.Name)
		}
		if isMapEntry(fmsg) {
			isMap = true
			repeated = false
			for _, v := range fmsg.Fields {
				if *v.Name == "value" {
					if v.GetType() == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
						typeName = v.GetTypeName()
						typeName = strings.TrimPrefix(typeName, ".")
					} else {
						typeName = gogoproto.FieldDescriptorProto_Type_name[int32(v.GetType())]
					}
				}
				if *v.Name == "key" {
					if v.GetType() == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
						keyType = v.GetTypeName()
						keyType = strings.TrimPrefix(keyType, ".")
					} else {
						keyType = gogoproto.FieldDescriptorProto_Type_name[int32(v.GetType())]
					}
				}
			}
		} else {
			typeName = strings.TrimPrefix(typeName, ".")
		}
	} else {
		typeName = gogoproto.FieldDescriptorProto_Type_name[int32(fld.GetType())]
	}
	// TODO: Handle other cases for CLI tag resolution
	cliTag := common.GetJSONTag(fld)
	if cliTag == "" {
		cliTag = *fld.Name
	}
	name := *fld.Name
	if embed {
		p := strings.Split(*fld.TypeName, ".")
		name = p[len(p)-1]
	}
	ret = Field{
		Name:    name,
		JSONTag: common.GetJSONTag(fld),
		CLITag:  cliInfo{tag: cliTag},
		Pointer: pointer,
		Slice:   repeated,
		Embed:   embed,
		Map:     isMap,
		Inline:  inline,
		KeyType: keyType,
		Mutable: mutable,
		Type:    typeName,
	}
	return ret, nil
}

// isScalarType returns true for all protobuf scalars
func isScalarType(in string) bool {
	switch in {
	case "TYPE_FLOAT", "TYPE_DOUBLE", "TYPE_INT64", "TYPE_UINT64", "TYPE_INT32", "TYPE_FIXED64", "TYPE_FIXED32", "TYPE_BOOL":
		return true
	case "TYPE_STRING", "TYPE_BYTES", "TYPE_UINT32", "TYPE_ENUM", "TYPE_SFIXED32", "TYPE_SFIXED64", "TYPE_SINT32", "TYPE_SINT64":
		return true
	}
	return false
}

func typeIsMessage(f *descriptor.Field) bool {
	if *f.Type == gogoproto.FieldDescriptorProto_TYPE_MESSAGE {
		return true
	}

	return false
}

func getCLIType(fld *Field) string {
	if fld.Map || fld.Slice {
		return "StringSlice"
	}

	m := map[string]string{
		"TYPE_INT64":    "Int",
		"TYPE_INT32":    "Int",
		"TYPE_UINT64":   "Uint",
		"TYPE_FIXED64":  "Uint",
		"TYPE_FIXED32":  "Uint",
		"TYPE_BOOL":     "Bool",
		"TYPE_FLOAT":    "String",
		"TYPE_DOUBLE":   "String",
		"TYPE_STRING":   "String",
		"TYPE_BYTES":    "String",
		"TYPE_UINT32":   "String",
		"TYPE_ENUM":     "String",
		"TYPE_SFIXED32": "String",
		"TYPE_SFIXED64": "String",
		"TYPE_SINT32":   "String",
		"TYPE_SINT64":   "String",
	}
	return m[fld.Type]
}

// getCLITags updates the CLITags for the give Struct. It recurses through all fields and their
//   types in the message.
func getCLITags(strct Struct, path, prefix string, msgMap map[string]Struct, m map[string]cliInfo) error {
	if path != "" {
		path = path + "."
	}
	for _, k := range strct.keys {
		fld := strct.Fields[k]
		fpath := path + fld.Name
		if fld.Map || fld.Slice {
			fpath = fpath + "[]"
		}
		if isScalarType(fld.Type) {
			fpath := path + fld.Name
			tag := prefix + fld.CLITag.tag
			if _, ok := m[tag]; ok {
				// panic(fmt.Sprintf("duplicate tag [%s] at [%s]", fld.CLITag, fpath))
				// Dont panic during initial development. Will panic in production
				glog.V(1).Infof("Duplicate tag [%v] at [%s] Will CRASH&BURN", fld.CLITag, fpath)
			}
			fld.CLITag.path = fpath
			fld.CLITag.cliType = getCLIType(&fld)
			m[tag] = fld.CLITag
			continue
		}
		fldPrefix := prefix
		if fld.CLITag.ins != "" {
			fldPrefix = prefix + fld.CLITag.ins + "-"
		}
		getCLITags(msgMap[fld.Type], fpath, fldPrefix, msgMap, m)
	}
	return nil
}

// flattenEmbedded flattens any inlined structures in to the parent struct.
// XXX-TODO(sanjayt): Carry CLI Tags from the inlined elements to the flattened members.
func flattenEmbedded(file *descriptor.File, field Field) (map[string]Field, []string, error) {
	glog.V(1).Infof("Generating Inline for field %v/%v", field.Type, field.Name)
	ret := make(map[string]Field)
	keys := []string{}
	msg, err := file.Reg.LookupMsg("", field.Type)
	if err != nil {
		return ret, keys, err
	}
	glog.V(1).Infof("Got Message %v", *msg.Name)
	for _, fld := range msg.Fields {
		f, err := genField("", fld, file)
		if err != nil {
			return ret, keys, err
		}
		f.FromInline = true
		glog.V(1).Infof("Inline Got field %v", f.Name)
		name := f.Name
		if f.Embed {
			p := strings.Split(f.Type, ".")
			name = p[len(p)-1]
		}

		if _, ok := ret[name]; !ok {
			glog.V(1).Infof("Inline adding field %v", f.Name)
			ret[name] = f
			keys = append(keys, name)
		}
		if common.IsEmbed(fld) {
			flds, ks, err := flattenEmbedded(file, f)
			if err != nil {
				return ret, keys, err
			}
			for _, v := range ks {
				if _, ok := ret[v]; !ok {
					ret[v] = flds[v]
					keys = append(keys, v)
				}
			}
		}
	}
	return ret, keys, nil
}

// genMsgMap parses and generates the schema map for the file.
func genMsgMap(file *descriptor.File) (map[string]Struct, []string, error) {
	pkg := file.GoPkg.Name
	ret := make(map[string]Struct)
	var keys []string
	for _, msg := range file.Messages {
		var kind, group string
		var scopes []string
		if isSpecStatusMessage(msg) || (hasTypeMeta(msg) && hasListHelper(msg)) {
			kind = *msg.Name
			group = file.GoPkg.Name
			if v, _ := isTenanted(msg); v {
				scopes = []string{"Tenant"}
			} else {
				scopes = []string{"Cluster"}
			}

		}
		fqname := pkg + "." + *msg.Name
		if len(msg.Outers) > 0 {
			fqname = msg.Outers[0] + "." + *msg.Name
			fqname = strings.TrimPrefix(fqname, ".")
			fqname = pkg + "." + fqname
		}
		node := Struct{Kind: kind, APIGroup: group, Scopes: scopes, CLITags: make(map[string]cliInfo), Fields: make(map[string]Field), mapEntry: isMapEntry(msg)}
		embeddedStructs := []Field{}
		for _, fld := range msg.Fields {
			f, err := genField(fqname, fld, file)
			if err != nil {
				return ret, keys, err
			}
			name := f.Name
			if f.Embed {
				p := strings.Split(f.Type, ".")
				name = p[len(p)-1]
			}
			node.Fields[name] = f
			node.keys = append(node.keys, name)
			if f.Embed {
				embeddedStructs = append(embeddedStructs, f)
			}
		}
		// flatten inlines
		for _, f := range embeddedStructs {
			// Flatten the fields that are inlined and add them here.
			flds, names, err := flattenEmbedded(file, f)
			if err == nil {
				for _, k := range names {
					glog.V(1).Infof("check inline field %v", k)
					if _, ok := node.Fields[k]; !ok {
						glog.V(1).Infof("add inline field %v", k)
						node.Fields[k] = flds[k]
						node.keys = append(node.keys, k)
					}
				}
			} else {
				return ret, keys, err
			}
		}

		ret[fqname] = node
		keys = append(keys, fqname)
	}
	getCLIParams(file, ret)
	for _, msg := range file.Messages {
		if !isSpecStatusMessage(msg) {
			continue
		}
		fqname := pkg + "." + *msg.Name
		strct := ret[fqname]
		getCLITags(strct, "", "", ret, strct.CLITags)
		ret[fqname] = strct
	}
	return ret, keys, nil
}

func getMsgToSvcPrefix(file *descriptor.File) map[string][]string {
	ret := make(map[string][]string)
	for _, svc := range file.Services {
		params, err := getSvcParams(svc)
		if err != nil {
			glog.Fatalf("failed to get service params for service (%s)", err)
		}
		cruds, err := reg.GetExtension("venice.apiGrpcCrudService", svc)
		if err == nil {
			for _, v := range cruds.([]string) {
				ret[v] = append(ret[v], params.Prefix)
			}
		}
	}
	return ret
}

type pathsMap struct {
	URI string
	Key string
}

func genPathsMap(file *descriptor.File) (map[string][]pathsMap, []string) {
	ret := make(map[string][]pathsMap)

	maps := getMsgToSvcPrefix(file)
	pkg := file.GoPkg.Name
	for k, v := range maps {
		kind := pkg + "." + k
		for _, v1 := range v {
			msg, err := file.Reg.LookupMsg("", "."+kind)
			if err != nil {
				glog.Fatalf("failed to get message [%v] (%s)", kind, err)
			}

			// keycomponents for Key
			uriC, err := getMsgURI(msg, "v1", v1)
			if err != nil {
				glog.Fatalf("failed to get uri components for [%v] (%s)", kind, err)
			}
			// XXX-TODO(sanjayt): insert appropriate version for group here
			uri := "/" + globals.ConfigURIPrefix + "/"
			for _, kc := range uriC {
				if kc.Type == "prefix" {
					uri = uri + kc.Val
				} else {
					uri = uri + "{" + kc.Val + "}"
				}
			}

			// Key components for URI
			keyC, err := getDbKey(msg)
			if err != nil {
				glog.Fatalf("failed to get key components for [%v] (%s)", kind, err)
			}
			key := globals.ConfigRootPrefix + "/" + v1 + "/"
			for _, kc := range keyC {
				if kc.Type == "prefix" {
					key = key + kc.Val
				} else {
					key = key + "{" + kc.Val + "}"
				}
			}

			// insert into return map
			p := pathsMap{URI: uri, Key: key}
			ret[kind] = append(ret[kind], p)
		}
	}
	keys := []string{}
	for k := range ret {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	return ret, keys
}
func getPathsMap(file *descriptor.File) (string, error) {
	ret, keys := genPathsMap(file)
	retstr := ""
	for _, k := range keys {
		retstr = fmt.Sprintf("%s\n \"%s\": []api.PathsMap{", retstr, k)
		for _, v := range ret[k] {
			retstr = fmt.Sprintf("%s\n{ URI:\"%s\", Key:\"%s\" },", retstr, v.URI, v.Key)
		}
		retstr = fmt.Sprintf("%s},", retstr)
	}
	return retstr, nil
}

// getMsgMap is the template function used by templates to emit the schema for the file.
func getMsgMap(file *descriptor.File) (string, error) {
	msgs, keys, err := genMsgMap(file)
	if err != nil {
		return "", err
	}
	ret := ""
	pkg := "api."
	if file.GoPkg.Name == "api" {
		pkg = ""
	}
	for _, k := range keys {
		s := msgs[k]
		kpaths := strings.Split(k, ".")
		objPath := strings.Join(kpaths[1:], "_")
		if s.mapEntry {
			ret = fmt.Sprintf("%s\n\"%s\": &%vStruct{\n Fields: map[string]%vField {", ret, k, pkg, pkg)
		} else {
			scopeStr := "{"
			for i := range s.Scopes {
				scopeStr = scopeStr + "\"" + s.Scopes[i] + "\"" + ","
			}
			scopeStr = strings.TrimSuffix(scopeStr, ",") + "}"
			ret = fmt.Sprintf("%s\n\"%s\": &%vStruct{\n Kind: \"%s\", APIGroup: \"%s\", Scopes: []string%s, GetTypeFn: func() reflect.Type { return reflect.TypeOf(%s{}) }, \nFields: map[string]%vField {", ret, k, pkg, s.Kind, s.APIGroup, scopeStr, objPath, pkg)
		}

		for _, k1 := range s.keys {
			f := s.Fields[k1]
			ret = ret + fmt.Sprintf("\n\"%s\":%vField{Name: \"%s\", CLITag: %vCLIInfo{ID: \"%s\", Path: \"%s\", Skip: %v, Insert: \"%s\", Help:\"%s\"}, JSONTag: \"%s\", Pointer: %v, Slice:%v, Mutable: %v, Map:%v, Inline: %v, FromInline: %v, KeyType: \"%v\", Type: \"%s\"},\n",
				f.Name, pkg, f.Name, pkg, f.CLITag.tag, f.CLITag.path, f.CLITag.skip, f.CLITag.ins, f.CLITag.help, f.JSONTag, f.Pointer, f.Slice, f.Mutable, f.Map, f.Inline, f.FromInline, f.KeyType, f.Type)
		}
		ret = ret + "}, \n"
		if len(s.CLITags) > 0 {
			clikeys := []string{}
			for k1 := range s.CLITags {
				clikeys = append(clikeys, k1)
			}
			sort.Strings(clikeys)
			ret = ret + "\n CLITags: map[string]" + pkg + "CLIInfo { \n"
			for _, v := range clikeys {
				ret = ret + fmt.Sprintf("\"%v\": %vCLIInfo{Path: \"%s\", Skip: %v, Insert: \"%s\", Help:\"%s\"},\n", v, pkg, s.CLITags[v].path, s.CLITags[v].skip, s.CLITags[v].ins, s.CLITags[v].help)
			}
			ret = ret + "}, \n"
		}

		ret = ret + "},"
	}
	return ret, nil
}

func getCLIFlagMap(file *descriptor.File) map[string][]cgen.CliFlag {
	cliFlagMap := make(map[string][]cgen.CliFlag)

	msgs, _, err := genMsgMap(file)
	if err != nil {
		return cliFlagMap
	}
	for msgName, s := range msgs {
		cliFlags := []cgen.CliFlag{}

		clikeys := []string{}
		for k1 := range s.CLITags {
			clikeys = append(clikeys, k1)
		}
		sort.Strings(clikeys)
		for _, clikey := range clikeys {
			cliTag := s.CLITags[clikey]
			// cli flags are only needed for Spec fields
			if !strings.HasPrefix(cliTag.path, "Spec.") {
				continue
			}
			cf := cgen.CliFlag{ID: cliTag.tag, Type: cliTag.cliType, Help: cliTag.help, Skip: cliTag.skip, Insert: cliTag.ins}

			// when the field is part of a slice or map
			if strings.Contains(cliTag.path, "[]") {
				cf.Type = "StringSlice"
			}
			cliFlags = append(cliFlags, cf)
		}

		if len(s.CLITags) > 0 {
			cliFlagMap[msgName] = cliFlags
		}
	}
	return cliFlagMap
}

type svcObj struct {
	Svc     string
	ObjName string
}

func splitSvcObj(svcObjStr string) svcObj {
	ret := svcObj{ObjName: fmt.Sprintf("Error: invalid svc-obj '%s'", svcObjStr)}
	subStrs := strings.Split(svcObjStr, ".")
	if len(subStrs) != 2 {
		return ret
	}
	ret.Svc = subStrs[0]
	ret.ObjName = subStrs[1]
	return ret
}

func isAPIServerServed(file *descriptor.File) (bool, error) {
	if v, err := reg.GetExtension("venice.fileApiServerBacked", file); err == nil {
		return v.(bool), nil
	}
	return true, nil
}

func getRelPath(file *descriptor.File) (string, error) {
	return "github.com/pensando/sw/api", nil
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

// EventType represents the event type and it's attributes
type EventType struct {
	EType      string // event type
	Severity   string // severity
	Category   string // category
	Desc       string // description or UI hint
	SuppressMM bool   // suppress during maintenance mode
}

// returns the list of event types found from given file
func getEventTypes(file *descriptor.File) ([]*EventType, error) {
	var ets []*EventType

	if len(file.Enums) > 1 {
		return nil, errors.New("more than 1 enum is not allowed for event types")
	}

	for _, val := range file.Enums[0].Value {
		if val.Options != nil {
			et := &EventType{EType: val.GetName()}

			// get severity
			v, err := proto.GetExtension(val.Options, eventtypes.E_Severity)
			if err == nil && v.(*eventattrs.Severity) != nil {
				et.Severity = fmt.Sprintf("%v", v.(*eventattrs.Severity))
			} else {
				return nil, err
			}

			// get category
			v, err = proto.GetExtension(val.Options, eventtypes.E_Category)
			if err == nil && v.(*eventattrs.Category) != nil {
				et.Category = fmt.Sprintf("%v", v.(*eventattrs.Category))
			} else {
				return nil, err
			}

			// get description
			v, err = proto.GetExtension(val.Options, eventtypes.E_Desc)
			if err == nil && v.(*string) != nil {
				et.Desc = *(v.(*string))
			} else {
				return nil, err
			}

			// get suppress_mm
			v, err = proto.GetExtension(val.Options, eventtypes.E_SuppressMm)
			if err == nil && v.(*bool) != nil {
				et.SuppressMM = *(v.(*bool))
			}

			ets = append(ets, et)
		} else {
			return nil, errors.New("all event types must define it's attributes using annotations")
		}

	}

	return ets, nil
}

// genEventTypesJSON returns the JSON string of event types grouped by category
func genEventTypesJSON() (string, error) {
	evtsByCategory := eventtypes.GetEventsByCategory()
	if len(evtsByCategory) > 0 {
		ret, err := json.MarshalIndent(evtsByCategory, "", " ")
		if err != nil {
			glog.V(1).Infof("Failed to marshal event types")
			return "", err
		}

		str := string(ret[:])
		glog.V(1).Infof("Generated eventtypes.json %v", str)
		return str, nil
	}

	return "{}", nil
}

func getFileCategory(m *descriptor.Message) (string, error) {
	if ext, err := reg.GetExtension("venice.fileCategory", m.File); err == nil {
		return ext.(string), nil
	}
	return globals.ConfigURIPrefix, nil
}

func isStreaming(m *descriptor.Method) (bool, error) {
	return (m.ClientStreaming != nil && *m.ClientStreaming) || (m.ServerStreaming != nil && *m.ServerStreaming), nil
}

func isClientStreaming(m *descriptor.Method) (bool, error) {
	return (m.ClientStreaming != nil && *m.ClientStreaming), nil
}

func isSvcWatch(meth *descriptor.Method, svcName string) bool {
	return meth.GetName() == "AutoWatchSvc"+svcName
}

// getGolangTypeName returns golang type name for the field
func getGolangTypeName(t gogoproto.FieldDescriptorProto_Type, n string) string {
	var proto3TypeNames = map[gogoproto.FieldDescriptorProto_Type]string{
		gogoproto.FieldDescriptorProto_TYPE_DOUBLE:  "float64",
		gogoproto.FieldDescriptorProto_TYPE_FLOAT:   "float",
		gogoproto.FieldDescriptorProto_TYPE_INT64:   "int64",
		gogoproto.FieldDescriptorProto_TYPE_UINT64:  "uint64",
		gogoproto.FieldDescriptorProto_TYPE_INT32:   "int32",
		gogoproto.FieldDescriptorProto_TYPE_FIXED64: "uint64",
		gogoproto.FieldDescriptorProto_TYPE_FIXED32: "uint32",
		gogoproto.FieldDescriptorProto_TYPE_BOOL:    "bool",
		gogoproto.FieldDescriptorProto_TYPE_STRING:  "string",
		// FieldDescriptorProto_TYPE_GROUP
		// FieldDescriptorProto_TYPE_MESSAGE
		gogoproto.FieldDescriptorProto_TYPE_BYTES:  "string",
		gogoproto.FieldDescriptorProto_TYPE_UINT32: "uint32",
		// FieldDescriptorProto_TYPE_ENUM
		// TODO(yugui) Handle Enum
		gogoproto.FieldDescriptorProto_TYPE_SFIXED32: "int32",
		gogoproto.FieldDescriptorProto_TYPE_SFIXED64: "int64",
		gogoproto.FieldDescriptorProto_TYPE_SINT32:   "int32",
		gogoproto.FieldDescriptorProto_TYPE_SINT64:   "int64",
	}

	switch t {
	case gogoproto.FieldDescriptorProto_TYPE_MESSAGE:
		sl := strings.Split(n, ".")
		if len(sl) > 2 {
			return "goproto." + sl[2]
		} else if len(sl) > 1 {
			return "goproto." + sl[1]
		} else {
			return "goproto." + sl[0]
		}
	default:
		return proto3TypeNames[t]
	}
}

func getAPIOperType(in string) (string, error) {
	switch in {
	case "CreateOper":
		return "create", nil
	case "UpdateOper":
		return "update", nil
	case "GetOper":
		return "get", nil
	case "DeleteOper":
		return "delete", nil
	case "ListOper":
		return "list", nil
	case "WatchOper":
		return "watch", nil
	default:
		return "unknown", errors.New("unknown oper")
	}
}

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func getWatchHelperName(in string) string {
	return "AutoMsg" + in + "WatchHelper"
}

func isTenanted(msg *descriptor.Message) (bool, error) {
	dbk, err := getDbKey(msg)
	if err != nil {
		return false, err
	}
	for _, c := range dbk {
		if c.Type == "field" && c.Val == "Tenant" {
			return true, nil
		}
	}
	return false, nil
}

func isObjTenanted(file *descriptor.File, obj string) (bool, error) {
	name := "." + file.GoPkg.Name + "." + obj
	msg, err := file.Reg.LookupMsg("", name)
	if err != nil {
		return false, err
	}
	return isTenanted(msg)
}

func isNamespaced(msg *descriptor.Message) (bool, error) {
	dbk, err := getDbKey(msg)
	if err != nil {
		return false, err
	}
	for _, c := range dbk {
		if c.Type == "field" && c.Val == "Namespace" {
			return true, nil
		}
	}
	return false, nil
}

func isObjNamespaced(file *descriptor.File, obj string) (bool, error) {
	name := "." + file.GoPkg.Name + "." + obj
	msg, err := file.Reg.LookupMsg("", name)
	if err != nil {
		return false, err
	}
	return isNamespaced(msg)
}

// ProxyPath is parameters for reverse proxy endpoints
type ProxyPath struct {
	Prefix   string
	TrimPath string
	Path     string
	FullPath string
	Backend  string
}

func getProxyPaths(svc *descriptor.Service) ([]ProxyPath, error) {
	var ret []ProxyPath
	svcParams, err := getSvcParams(svc)
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

		ret = append(ret, ProxyPath{Prefix: prefix, Path: path, TrimPath: trimpath, FullPath: fullpath, Backend: opt.GetBackend()})
	}
	return ret, nil
}

func getJSONTagByName(msg *descriptor.Message, name string) (string, error) {
	// Special handling for tenant
	if isSpecStatusMessage(msg) && name == "Tenant" {
		return "meta.tenant", nil
	}

	for _, fld := range msg.Fields {
		if *fld.Name == name {
			ret := common.GetJSONTag(fld)
			if ret == "" {
				return *fld.Name, nil
			}
			return ret, nil
		}
	}
	// Did not find any direct fields. Try inlined fields
	for _, fld := range msg.Fields {
		if common.IsInline(fld) {
			nmsg, err := msg.File.Reg.LookupMsg("", *fld.TypeName)
			if err != nil {
				return "", fmt.Errorf("error getting jsontag for inline object [%v](%s)", *fld.TypeName, err)
			}
			return getJSONTagByName(nmsg, name)
		}
	}
	return "", fmt.Errorf("field [%v] not found", name)
}

func getMsgName(msg *descriptor.Message) (string, error) {
	if len(msg.Outers) == 0 {
		return *msg.Name, nil
	}
	ret := ""
	for _, v := range msg.Outers {
		ret = ret + v + "_"
	}
	return ret + *msg.Name, nil
}

func joinFields(dlmtr string, in ...string) string {
	ret := strings.Join(in, dlmtr)
	return ret
}

func isSrvBinStream(m *descriptor.Method) (bool, error) {
	bin, err := reg.GetExtension("venice.methodSrvBinaryStream", m)
	if err != nil {
		return false, nil
	}
	return bin.(bool), nil
}

func getEnumName(e *descriptor.Enum) (name string, err error) {
	for _, v := range e.Outers {
		name = name + v + "_"
	}
	name = name + *e.Name
	return name, nil
}

func getEnumVNameName(e *descriptor.Enum) (name string, err error) {
	name, err = getEnumName(e)
	if err != nil {
		return
	}
	name = name + "_vname"
	return name, nil
}

func getEnumVValueName(e *descriptor.Enum) (name string, err error) {
	name, err = getEnumName(e)
	if err != nil {
		return
	}
	name = name + "_vvalue"
	return name, nil
}

func getEnumVNameValues(e *descriptor.Enum) (values string, err error) {
	for _, v := range e.Value {
		n := common.GetVName(v)
		idx := v.GetNumber()
		values = values + fmt.Sprintf("%v:\"%v\",\n", idx, n)
	}
	return
}

func getEnumVValueValues(e *descriptor.Enum) (values string, err error) {
	for _, v := range e.Value {
		n := common.GetVName(v)
		idx := v.GetNumber()
		values = values + fmt.Sprintf("\"%v\":%v,\n", n, idx)
	}
	return
}

func getNormalizedEnumName(e *descriptor.Enum) (name string, err error) {
	name, err = getEnumName(e)
	if err != nil {
		return
	}
	name = name + "_normal"
	return name, nil
}

func getNormalizedEnum(e *descriptor.Enum) (values string, err error) {
	ret := make(map[string]string)
	for k := range e.Value {
		val := common.GetVName(e.Value[k])
		ret[val] = val
		if ret[val] != strings.ToLower(val) {
			ret[strings.ToLower(val)] = val
		}
	}
	keys := []string{}
	for k := range ret {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	for _, v := range keys {
		values = fmt.Sprintf("%s\"%s\":\"%s\",\n", values, v, ret[v])
	}
	return
}

func init() {
	cliTagRegex = regexp.MustCompile(`^[a-zA-Z0-9_\-]+$`)

	// Register Option Parsers
	common.RegisterOptionParsers()

	// Register Functions
	reg.RegisterFunc("getDbKey", getDbKey)
	reg.RegisterFunc("getURIKey", getURIKey)
	reg.RegisterFunc("getMsgURIKey", getMsgURIKey)
	reg.RegisterFunc("getServiceKey", getServiceKey)
	reg.RegisterFunc("getSvcParams", getSvcParams)
	reg.RegisterFunc("getPenctlCmdOptions", getPenctlCmdOptions)
	reg.RegisterFunc("getPenctlParentCmdOptions", getPenctlParentCmdOptions)
	reg.RegisterFunc("getPdsaGetGlobalOpts", getPdsaGetGlobalOpts)
	reg.RegisterFunc("getPdsaSetGlobalOpts", getPdsaSetGlobalOpts)
	reg.RegisterFunc("getRestSvcOptions", getRestSvcOptions)
	reg.RegisterFunc("getMethodParams", getMethodParams)
	reg.RegisterFunc("getCWD2", getCWD2)
	reg.RegisterFunc("getSwaggerFileName", getSwaggerFileName)
	reg.RegisterFunc("createDir", createDir)
	reg.RegisterFunc("genManifest", genManifest)
	reg.RegisterFunc("genNimbusManifest", genNimbusManifest)
	reg.RegisterFunc("genPkgManifest", genPkgManifest)
	reg.RegisterFunc("genSvcManifest", genServiceManifest)
	reg.RegisterFunc("genObjectURIs", genObjectURIs)
	reg.RegisterFunc("getSvcManifest", getServiceManifest)
	reg.RegisterFunc("getNimbusManifest", getNimbusManifest)
	reg.RegisterFunc("genSwaggerIndex", genSwaggerIndex)
	reg.RegisterFunc("getSwaggerMD", getSwaggerMD)
	reg.RegisterFunc("getAPIRefMD", getAPIRefMD)
	reg.RegisterFunc("addRelations", addRelations)
	reg.RegisterFunc("genRelMap", genRelMap)
	reg.RegisterFunc("genRelMapGo", genRelMapGo)
	reg.RegisterFunc("title", strings.Title)
	reg.RegisterFunc("detitle", detitle)
	reg.RegisterFunc("getInputType", getInputType)
	reg.RegisterFunc("getOutputType", getOutputType)
	reg.RegisterFunc("getListType", getListType)
	reg.RegisterFunc("getListTypeMsg", getListTypeMsg)
	reg.RegisterFunc("getWatchType", getWatchType)
	reg.RegisterFunc("getWatchTypeMsg", getWatchTypeMsg)
	reg.RegisterFunc("isAutoWatch", isAutoWatch)
	reg.RegisterFunc("isAutoList", isAutoList)
	reg.RegisterFunc("isWatchHelper", isWatchHelper)
	reg.RegisterFunc("isListHelper", isListHelper)
	reg.RegisterFunc("getPackageCrudObjects", getPackageCrudObjects)
	reg.RegisterFunc("getSvcCrudObjects", getSvcCrudObjects)
	reg.RegisterFunc("isAutoGenMethod", common.IsAutoGenMethod)
	reg.RegisterFunc("getAutoRestOper", getAutoRestOper)
	reg.RegisterFunc("isRestExposed", isRestExposed)
	reg.RegisterFunc("isRestMethod", isRestMethod)
	reg.RegisterFunc("isNestedMessage", isNestedMessage)
	reg.RegisterFunc("getNestedMsgName", getNestedMsgName)
	reg.RegisterFunc("isMapEntry", isMapEntry)
	reg.RegisterFunc("getFileName", getFileName)
	reg.RegisterFunc("getGrpcDestination", getGrpcDestination)
	reg.RegisterFunc("getValidatorManifest", getValidatorManifest)
	reg.RegisterFunc("getRequirementsManifest", getRequirementsManifest)
	reg.RegisterFunc("derefStr", derefStr)
	reg.RegisterFunc("getEnumStrMap", getEnumStrMap)
	reg.RegisterFunc("getEnumStrVMap", getEnumStrVMap)
	reg.RegisterFunc("getEnumStrNormalMap", getEnumStrNormalMap)
	reg.RegisterFunc("getStorageTransformersManifest", getStorageTransformersManifest)
	reg.RegisterFunc("isSpecStatusMessage", isSpecStatusMessage)
	reg.RegisterFunc("saveBool", scratch.setBool)
	reg.RegisterFunc("getBool", scratch.getBool)
	reg.RegisterFunc("saveInt", scratch.setInt)
	reg.RegisterFunc("getInt", scratch.getInt)
	reg.RegisterFunc("saveStr", scratch.setStr)
	reg.RegisterFunc("getStr", scratch.getStr)
	reg.RegisterFunc("isAPIServerServed", isAPIServerServed)
	reg.RegisterFunc("getSvcActionEndpoints", getSvcActionEndpoints)
	reg.RegisterFunc("getDefaulterManifest", getDefaulterManifest)
	reg.RegisterFunc("getRelPath", getRelPath)
	reg.RegisterFunc("getMsgMap", getMsgMap)
	reg.RegisterFunc("getEventTypes", getEventTypes)
	reg.RegisterFunc("genEventTypesJSON", genEventTypesJSON)
	reg.RegisterFunc("getFileCategory", getFileCategory)
	reg.RegisterFunc("isSvcWatch", isSvcWatch)
	reg.RegisterFunc("getAPIOperType", getAPIOperType)
	reg.RegisterFunc("getWatchHelperName", getWatchHelperName)
	reg.RegisterFunc("isStreaming", isStreaming)
	reg.RegisterFunc("isClientStreaming", isClientStreaming)
	reg.RegisterFunc("isTenanted", isTenanted)
	reg.RegisterFunc("isNamespaced", isNamespaced)
	reg.RegisterFunc("isObjTenanted", isObjTenanted)
	reg.RegisterFunc("isObjNamespaced", isObjNamespaced)
	reg.RegisterFunc("getProxyPaths", getProxyPaths)
	reg.RegisterFunc("HasSuffix", strings.HasSuffix)
	reg.RegisterFunc("TrimSuffix", strings.TrimSuffix)
	reg.RegisterFunc("TrimPrefix", strings.TrimPrefix)
	reg.RegisterFunc("ToLower", strings.ToLower)
	reg.RegisterFunc("ToUpper", strings.ToUpper)
	reg.RegisterFunc("CamelCase", gogen.CamelCase)
	reg.RegisterFunc("isActionMethod", isActionMethod)
	reg.RegisterFunc("getActionTarget", getActionTarget)
	reg.RegisterFunc("getCLIFlagMap", getCLIFlagMap)
	reg.RegisterFunc("splitSvcObj", splitSvcObj)
	reg.RegisterFunc("GetGolangTypeName", getGolangTypeName)
	reg.RegisterFunc("getRequirementPath", getRequirementPath)
	reg.RegisterFunc("joinFields", joinFields)
	reg.RegisterFunc("getPathsMap", getPathsMap)
	reg.RegisterFunc("getJSONTagByName", getJSONTagByName)
	reg.RegisterFunc("getMsgName", getMsgName)
	reg.RegisterFunc("isSrvBinStream", isSrvBinStream)
	reg.RegisterFunc("getEnumName", getEnumName)
	reg.RegisterFunc("getNormalizedEnum", getNormalizedEnum)
	reg.RegisterFunc("getNormalizedEnumName", getNormalizedEnumName)
	reg.RegisterFunc("genFileMetricsJSON", genFileMetricsJSON)
	reg.RegisterFunc("getGenParamsPrefix", getGenParamsPrefix)
	reg.RegisterFunc("getGenParamsPath", getGenParamsPath)
	reg.RegisterFunc("genMetricsManifest", genMetricsManifest)
	reg.RegisterFunc("getMsgMetricOptionsHdlr", getMsgMetricOptionsHdlr)
	reg.RegisterFunc("getFileMsgFieldMap", getFileMsgFieldMap)
	reg.RegisterFunc("getPdsaFieldOpt", getPdsaFieldOpt)
	reg.RegisterFunc("isPdsaFieldInTable", isPdsaFieldInTable)
	reg.RegisterFunc("getEnumVNameName", getEnumVNameName)
	reg.RegisterFunc("getEnumVNameValues", getEnumVNameValues)
	reg.RegisterFunc("getEnumVValueName", getEnumVValueName)
	reg.RegisterFunc("getEnumVValueValues", getEnumVValueValues)
	reg.RegisterFunc("getMetaswitchMibTablesInfo", getMetaswitchMibTablesInfo)
	reg.RegisterFunc("TypeIsMessage", typeIsMessage)
	reg.RegisterFunc("getPdsaCastSetFunc", getPdsaCastSetFunc)
	reg.RegisterFunc("getPdsaCastGetFunc", getPdsaCastGetFunc)
	reg.RegisterFunc("getCppTypeFieldFromProto", getCppTypeFieldFromProto)

	// Register request mutators
	reg.RegisterReqMutator("pensando", reqMutator)
}
