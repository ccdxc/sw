package common

import (
	"errors"
	"reflect"
	"strings"
	"sync"
	"testing"

	govldtr "github.com/asaskevich/govalidator"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gwplugins "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
)

var once sync.Once

func doOnce() {
	RegisterOptionParsers()
}

func TestParsers(t *testing.T) {
	dummyval1 := "should not pass"
	dummyval2 := false
	{
		in := "test"
		out, err := parseStringOptions(&in)
		if err != nil {
			t.Fatalf("parseStringOptions failed (%s)", err)
		}
		if out.(string) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseStringOptions(&dummyval2); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := []string{"test", "test1"}
		out, err := parseStringSliceOptions(in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]string), in) {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseStringSliceOptions(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := int32(10)
		out, err := parseInt32Options(&in)
		if err != nil {
			t.Errorf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(int32) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseInt32Options(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := false
		out, err := parseBoolOptions(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(bool) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseBoolOptions(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := venice.ObjectRln{
			Type: "NamedRef",
			To:   "group/destination",
		}
		out, err := parseObjRelation(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(venice.ObjectRln) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseObjRelation(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := []*venice.RestEndpoint{
			&venice.RestEndpoint{
				Object:  "test",
				Method:  []string{"put", "get"},
				Pattern: "/a/a/a/",
			},
		}
		out, err := parseRestServiceOption(in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.RestEndpoint), in) {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := &googapi.HttpRule{Body: "test body"}
		out, err := parseGoogleAPIHTTP(in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*googapi.HttpRule), in) {
			t.Errorf("parseGoogleAPIHTTP returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseGoogleAPIHTTP passed")
		}
	}
	{
		in := &venice.PenCtlCmd{Cmd: "XYZ", HelpStr: "Test strr", RootCmd: " Root cmd"}
		out, err := parsePenctlCmd(in)
		if err != nil {
			t.Fatalf("parsePenctlCmd failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*venice.PenCtlCmd), in) {
			t.Errorf("parsePenctlCmd returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parsePenctlCmd passed")
		}
	}
	{
		in := []*venice.PenCtlCmd{{Cmd: "XYZ", HelpStr: "Test strr", RootCmd: " Root cmd"}}
		out, err := parsePenctlParentCmd(in)
		if err != nil {
			t.Fatalf("parsePenctlParentCmd failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.PenCtlCmd), in) {
			t.Errorf("parsePenctlParentCmd returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parsePenctlParentCmd passed")
		}
	}
	{
		in := []*venice.RestEndpoint{{Object: "TestObj", Pattern: "TestPattern", Prefix: "TestPrefix"}}
		out, err := parseNaplesRestService(in)
		if err != nil {
			t.Fatalf("parseNaplesRestService failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.RestEndpoint), in) {
			t.Errorf("parseNaplesRestService returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseNaplesRestService passed")
		}
	}
	{
		in := []*venice.ActionEndpoint{{Action: "TestActon", Request: "Test Request", Response: "Test Response"}}
		out, err := parseAPIActions(in)
		if err != nil {
			t.Fatalf("parseAPIActions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.ActionEndpoint), in) {
			t.Errorf("parseAPIActions returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseAPIActions passed")
		}
	}
	{
		in := &venice.ObjectPrefix{Path: "TestPath"}
		out, err := parseObjectPrefix(in)
		if err != nil {
			t.Fatalf("parseObjectPrefix failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*venice.ObjectPrefix), in) {
			t.Errorf("parseObjectPrefix returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseObjectPrefix passed")
		}
	}
	{
		in := []*venice.ProxyEndpoint{{Path: "TestPath"}}
		out, err := parseProxyEndpoint(in)
		if err != nil {
			t.Fatalf("parseProxyEndpoint failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.ProxyEndpoint), in) {
			t.Errorf("parseProxyEndpoint returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseProxyEndpoint passed")
		}
	}
	{
		in := []*venice.ProxyEndpoint{{Path: "TestPath"}}
		out, err := parseProxyEndpoint(in)
		if err != nil {
			t.Fatalf("parseProxyEndpoint failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]*venice.ProxyEndpoint), in) {
			t.Errorf("parseProxyEndpoint returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseProxyEndpoint passed")
		}
	}
	{
		in := &venice.MetricInfo{DisplayName: "Display Name", Description: "Test"}
		out, err := parseMetricInfo(in)
		if err != nil {
			t.Fatalf("parseMetricInfo failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*venice.MetricInfo), in) {
			t.Errorf("parseMetricInfo returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseMetricInfo passed")
		}
	}
	{
		in := &venice.MetricFieldInfo{DisplayName: "Display Name", Description: "Test"}
		out, err := parseMetricFieldInfo(in)
		if err != nil {
			t.Fatalf("parseMetricFieldInfo failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*venice.MetricFieldInfo), in) {
			t.Errorf("parseMetricFieldInfo returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseMetricFieldInfo passed")
		}
	}
	{
		in := &venice.GlobalOpts{FillFn: "test"}
		out, err := parseGlobalOpts(in)
		if err != nil {
			t.Fatalf("parseGlobalOpts failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(*venice.GlobalOpts), in) {
			t.Errorf("parseGlobalOpts returned value wrong [%v]", out)
		}
	}
	{
		in := &venice.PdsaFields{Field: "test"}
		out, err := parsePdsaFieldOptions(in)
		if err != nil {
			t.Fatalf("parsePdsaFieldOptions failed (%s)", err)
		}
		if out.(venice.PdsaFields).Field != in.Field {
			t.Errorf("parsePdsaFieldOptions returned value wrong [%v]", out)
		}
	}
}

func TestParseValidator(t *testing.T) {
	scratchMap := ValidatorArgMap
	ValidatorArgMap = map[string][]CheckArgs{
		"OneStr": {IsString},
		"MulStr": {IsString, IsString, IsString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {IsString, govldtr.IsInt},
		"NoArgs": {},
	}

	cases := []struct {
		str string
		res bool
		ret ValidateField
	}{
		{str: "v1:OneStr(arg1)", res: true, ret: ValidateField{Fn: "OneStr", Ver: "v1", Args: []string{"arg1"}}},
		{str: "MulStr(arg1, arg2, arg3)", res: true, ret: ValidateField{Fn: "MulStr", Ver: "all", Args: []string{"arg1", "arg2", "arg3"}}},
		{str: "MulStr(arg1, arg2, arg3)", res: true, ret: ValidateField{Fn: "MulStr", Ver: "all", Args: []string{"arg1", "arg2", "arg3"}}},
		{str: "MulStr(arg1, arg2)", res: false, ret: ValidateField{}},
		{str: "all:OneInt(arg1)", res: false, ret: ValidateField{}},
		{str: "all:OneInt(1)", res: true, ret: ValidateField{Fn: "OneInt", Ver: "all", Args: []string{"1"}}},
		{str: "*:MulInt(1,3)", res: true, ret: ValidateField{Fn: "MulInt", Ver: "all", Args: []string{"1", "3"}}},
		{str: "all:Unknown(arg1)", res: false, ret: ValidateField{}},
		{str: "NoArgs()", res: true, ret: ValidateField{Fn: "NoArgs", Ver: "all", Args: []string{}}},
		{str: "v1:NoArgs()", res: true, ret: ValidateField{Fn: "NoArgs", Ver: "v1", Args: []string{}}},
		{str: "Unknown(arg1)", res: false, ret: ValidateField{}},
	}
	for _, c := range cases {
		v, err := ParseValidator(c.str)
		if c.res && !(err == nil) {
			t.Errorf("[%s] - Expecing result [%v] got [%v], (%v)", c.str, c.res, (err == nil), err)
		}
		if !reflect.DeepEqual(c.ret, v) {
			t.Errorf("[%s] - returned validate does not match [%+v]/[%+v]", c.str, c.ret, v)
		}
	}
	ValidatorArgMap = scratchMap
}

func TestParseDefaulters(t *testing.T) {
	cases := []struct {
		in  string
		ver string
		val string
		err error
	}{
		{
			in:  "DefaultOnly",
			ver: "all",
			val: "DefaultOnly",
			err: nil,
		},
		{
			in:  ":DefaultOnly",
			ver: "all",
			val: ":DefaultOnly",
			err: nil,
		},
		{
			in:  "v1:DefaultOnly",
			ver: "v1",
			val: "DefaultOnly",
			err: nil,
		},
		{
			in:  "v1\\:DefaultOnly",
			ver: "all",
			val: "v1\\:DefaultOnly",
			err: nil,
		},
		{
			in:  "DefaultOnly:",
			ver: "DefaultOnly",
			val: "",
			err: nil,
		},
	}
	for _, c := range cases {
		ver, val, err := parseDefault(c.in)
		if ver != c.ver || val != c.val || c.err != err {
			t.Errorf("Does not match [%s][ %s / %s / %v ]", c.in, ver, val, err)
		}
	}
}

func TestDefaulters(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'TestMsg'
			field <
				name: 'no_default_msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				number: 1
			>
			field <
				name: 'msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				options:<[venice.default]: "false" [venice.default]: "v2:true">
				number: 1
			>
			field <
				name: 'bool_field'
				type: TYPE_BOOL
				options:<[venice.default]: "false" [venice.default]: "v2:true">
				number: 1
			>
			field <
				name: 'enum_field'
				type: TYPE_ENUM
				type_name: '.example.Enum1'
				options:<[venice.default]: "EnumAll" [venice.default]: "v2:EnumONE">
				number: 2
			>
			field <
				name: 'int32_field'
				type: TYPE_INT32
				options:<[venice.default]: "10" [venice.default]: "v3:42">
				number: 3
			>
			field <
				name: 'int64_field'
				type: TYPE_INT64
				options:<[venice.default]: "1000000000000" [venice.default]: "v3:42000000000000">
				number: 4
			>
			field <
				name: 'sint32_field'
				type: TYPE_SINT32
				options:<[venice.default]: "-22" [venice.default]: "v3:-42">
				number: 5
			>
			field <
				name: 'sint64_field'
				type: TYPE_SINT64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:-42000000000000">
				number: 6
			>
			field <
				name: 'sfixed32_field'
				type: TYPE_SFIXED32
				options:<[venice.default]: "-22" [venice.default]: "v3:-42">
				number: 7
			>
			field <
				name: 'sfixed64_field'
				type: TYPE_SFIXED64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:-42000000000000">
				number: 8
			>
			field <
				name: 'uint32_field'
				type: TYPE_UINT32
				options:<[venice.default]: "10" [venice.default]: "v3:42">
				number: 9
			>
			field <
				name: 'uint64_field'
				type: TYPE_UINT64
				options:<[venice.default]: "1000000000000" [venice.default]: "v3:42000000000000">
				number: 10
			>
			field <
				name: 'fixed32_field'
				type: TYPE_FIXED32
				options:<[venice.default]: "22" [venice.default]: "v3:42">
				number: 11
			>
			field <
				name: 'fixed64_field'
				type: TYPE_FIXED64
				options:<[venice.default]: "22000000000000" [venice.default]: "v3:42000000000000">
				number: 12
			>
			field <
				name: 'float_field'
				type: TYPE_FLOAT
				options:<[venice.default]: "3.1415" [venice.default]: "v3:2.71828">
				number: 13
			>
			field <
				name: 'double_field'
				type: TYPE_DOUBLE
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:2000000000000.71828">
				number: 14
			>
			field <
				name: 'string_field'
				type: TYPE_STRING
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:unknown value">
				number: 15
			>
			field <
				name: 'str_enum_field'
				type: TYPE_STRING
				options:<[venice.default]: "Value1" [venice.default]: "v3:unknown value" [venice.check]: "StrEnum(TestMsg.Enum1)" [venice.check]: "v2:StrEnum(Enum2)">
				number: 16
			>
			field <
				name: 'str_enum_nodef_field'
				type: TYPE_STRING
				options:<[venice.check]: "StrEnum(TestMsg.Enum1))" [venice.check]: "v3:StrEnum(.example.Enum2)">
				number: 17
			>
			field <
				name: 'no_default_msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				number: 18
			>
			field <
				name: 'no_default_int32_field'
				type: TYPE_INT32
				number: 19
			>
			field <
				name: 'bad_bool_field'
				type: TYPE_BOOL
				options:<[venice.default]: "unknown" [venice.default]: "v2:aaaa">
				number: 21
			>
			field <
				name: 'bad_int32_field'
				type: TYPE_INT32
				options:<[venice.default]: "10" [venice.default]: "v3:1000000000000">
				number: 23
			>
			field <
				name: 'bad_int64_field'
				type: TYPE_INT64
				options:<[venice.default]: "1000000000000a" [venice.default]: "v3:42000000000000">
				number: 24
			>
			field <
				name: 'bad_sint32_field'
				type: TYPE_SINT32
				options:<[venice.default]: "-22" [venice.default]: "v3:">
				number: 25
			>
			field <
				name: 'bad_sint64_field'
				type: TYPE_SINT64
				options:<[venice.default]: "true" [venice.default]: "v3:xyze">
				number: 26
			>
			field <
				name: 'bad_sfixed32_field'
				type: TYPE_SFIXED32
				options:<[venice.default]: "-22" [venice.default]: "v3:42000000000000.12333">
				number: 27
			>
			field <
				name: 'bad_sfixed64_field'
				type: TYPE_SFIXED64
				options:<[venice.default]: "-aa22000000000000" [venice.default]: "v3:-42000000000000">
				number: 28
			>
			field <
				name: 'bad_uint32_field'
				type: TYPE_UINT32
				options:<[venice.default]: "thousand" [venice.default]: ":">
				number: 29
			>
			field <
				name: 'bad_uint64_field'
				type: TYPE_UINT64
				options:<[venice.default]: "" [venice.default]: "v3:42000000000000">
				number: 210
			>
			field <
				name: 'bad_fixed32_field'
				type: TYPE_FIXED32
				options:<[venice.default]: "22" [venice.default]: "v3:0xaa">
				number: 211
			>
			field <
				name: 'bad_fixed64_field'
				type: TYPE_FIXED64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:42000000000000">
				number: 212
			>
			field <
				name: 'bad_float_field'
				type: TYPE_FLOAT
				options:<[venice.default]: "a.1415" [venice.default]: "v3:2.71828">
				number: 213
			>
			field <
				name: 'bad_double_field'
				type: TYPE_DOUBLE
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3\\:2000000000000.71828">
				number: 214
			>
			field <
				name: 'dup_str_enum_field'
				type: TYPE_STRING
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:unknown value" [venice.check]: "StrEnum(TestMsg.Enum1)" [venice.check]: "v2:StrEnum(.example.Enum2)" [venice.default]: "v3:unknown value2">
				number: 215
			>
			enum_type <
				name: "Enum1"
				value <name: "value1", number: 0>
				value <name: "value2", number: 1>
			>
		>
		message_type <
			name: 'another_message'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
		>
		enum_type <
			name:"Enum2"
			value:<name:"Value3" number:0 >
			value:<name:"Value4" number:1 >
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	testerr := errors.New("error")
	RegisterOptionParsers()
	expected := map[string]struct {
		ret   Defaults
		err   error
		found bool
	}{
		"msg_field":              {err: testerr, found: true, ret: Defaults{}},
		"bool_field":             {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "false", "v2": "true"}}},
		"enum_field":             {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "EnumAll", "v2": "EnumONE"}}},
		"int32_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "10", "v3": "42"}}},
		"int64_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "1000000000000", "v3": "42000000000000"}}},
		"sint32_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22", "v3": "-42"}}},
		"sint64_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22000000000000", "v3": "-42000000000000"}}},
		"sfixed32_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22", "v3": "-42"}}},
		"sfixed64_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22000000000000", "v3": "-42000000000000"}}},
		"uint32_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "10", "v3": "42"}}},
		"uint64_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "1000000000000", "v3": "42000000000000"}}},
		"fixed32_field":          {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "22", "v3": "42"}}},
		"fixed64_field":          {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "22000000000000", "v3": "42000000000000"}}},
		"float_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "3.1415", "v3": "2.71828"}}},
		"double_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "3000000000000.1415", "v3": "2000000000000.71828"}}},
		"string_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"3000000000000.1415\"", "v3": "\"unknown value\""}}},
		"str_enum_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"Value1\"", "v2": "\"value3\"", "v3": "\"unknown value\""}}},
		"str_enum_nodef_field":   {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"value1\"", "v3": "\"value3\""}}},
		"bad_bool_field":         {err: testerr, found: true, ret: Defaults{}},
		"bad_enum_field":         {err: testerr, found: true, ret: Defaults{}},
		"bad_int32_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_int64_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_sint32_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_sint64_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_sfixed32_field":     {err: testerr, found: true, ret: Defaults{}},
		"bad_sfixed64_field":     {err: testerr, found: true, ret: Defaults{}},
		"bad_uint32_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_uint64_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_fixed32_field":      {err: testerr, found: true, ret: Defaults{}},
		"bad_fixed64_field":      {err: testerr, found: true, ret: Defaults{}},
		"bad_float_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_double_field":       {err: testerr, found: true, ret: Defaults{}},
		"dup_str_enum_field":     {err: testerr, found: true, ret: Defaults{}},
		"no_default_msg_field":   {err: nil, found: false, ret: Defaults{}},
		"no_default_int32_field": {err: nil, found: false, ret: Defaults{}},
	}
	msg := file.Messages[0]
	for i := range msg.Fields {
		name := *msg.Fields[i].Name
		c := expected[name]
		if v, found, err := ParseDefaults(file, msg.Fields[i]); (err == nil) != (c.err == nil) {
			t.Errorf("errors dont match for %s (%v/%v)[%+v]", name, err, c.err, v)
		} else if err == nil {
			if found != c.found {
				t.Errorf("[%v]found flag does not match[%v/%v]", name, found, c.found)
			}
			if found {
				if !reflect.DeepEqual(v, c.ret) {
					t.Errorf("[%v]returned value does not match expected [ %v]/ [%v]", name, v, c.ret)
				}
			}
		}
	}
}

func TestFieldProfiles(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'TestMsg'
			enum_type:<
				name:"Enum1"
				value:<name:"Value1" number:0 >
				value:<name:"Value2" number:1 >
			>
			field <
				name: 'enum_field1'
				type: TYPE_STRING
				options:<[venice.check]: "StrEnum(TestMsg.Enum1)">
				number: 1
			>
			field <
				name: 'enum_field2'
				type: TYPE_STRING
				options:<[venice.check]: "StrEnum(Enum2)">
				number: 2
			>
			field <
				name: 'enum_field3'
				type: TYPE_STRING
				options:<[venice.check]: "StrEnum(.example.Enum2)">
				number: 3
			>
			field <
				name: 'str_field1'
				type: TYPE_STRING
				options:<[venice.check]: "StrLen(4,30)" [venice.check]: "v1:StrLen(5,31)">
				number: 4
			>
			field <
				name: 'str_field2'
				type: TYPE_STRING
				options:<[venice.check]: "EmptyOr(StrLen(4,30))" [venice.check]: "v1:StrLen(5,31)">
				number: 5
			>
			field <
				name: 'str_field3'
				type: TYPE_STRING
				options:<[venice.check]: "StrLen(4,-1)" [venice.check]: "v1:StrLen(5,31)">
				number: 6
			>
			field <
				name: 'str_field4'
				type: TYPE_STRING
				options:<[venice.check]: "IPAddr()">
				number: 7
			>
			field <
				name: 'str_field5'
				type: TYPE_STRING
				options:<[venice.check]: "HostAddr()">
				number: 8
			>
			field <
				name: 'str_field6'
				type: TYPE_STRING
				options:<[venice.check]: "MacAddr()">
				number: 9
			>
			field <
				name: 'str_field7'
				type: TYPE_STRING
				options:<[venice.check]: "URI()">
				number: 10 
			>
			field <
				name: 'str_field8'
				type: TYPE_STRING
				options:<[venice.check]: "UUID()">
				number: 11
			>
			field <
				name: 'str_field9'
				type: TYPE_STRING
				options:<[venice.check]: "IPv4()">
				number: 12
			>
			field <
				name: 'int_field1'
				type: TYPE_STRING
				options:<[venice.check]: "IntRange(3, 1000)">
				number: 13
			>
			field <
				name: 'int_field2'
				type: TYPE_STRING
				options:<[venice.check]: "IntMin(5)">
				number: 14
			>
			field <
				name: 'regexp_field1'
				type: TYPE_STRING
				options:<[venice.check]: "RegExp(name)">
				number: 15
			>
			field <
				name: 'regexp_field2'
				type: TYPE_STRING
				options:<[venice.check]: "EmptyOr(RegExp(name))">
				number: 16
			>
			field <
				name: 'duration1'
				type: TYPE_STRING
				options:<[venice.check]: "Duration(4m,30m)">
				number: 17
			>
			field <
				name: 'duration2'
				type: TYPE_STRING
				options:<[venice.check]: "Duration(4m,0)">
				number: 18
			>
			field <
				name: 'duration3'
				type: TYPE_STRING
				options:<[venice.check]: "Duration(0,30m)">
				number: 19
			>
			field <
				name: 'duration4'
				type: TYPE_STRING
				options:<[venice.check]: "EmptyOr(Duration(4m,30m))">
				number: 20
			>
		>
		enum_type <
			name:"Enum2"
			value:<name:"Value3" number:0 >
			value:<name:"Value4" number:1 >
		>
		source_code_info:<
		location:<path:4 path:0 path:4 path: 0 path:2 path:0 leading_comments:"val1 comments\n ui-hint: hint1" >
		location:<path:4 path:0 path:4 path: 0 path:2 path:1 leading_comments:"ui-hint: hint2" >
		location:<path:5 path:0 path:2 path:0 trailing_comments:"   ui-hint: hint3" >
		location:<path:5 path:0 path:2 path:1 leading_comments:"msg1 field 2 comments" >
		>
					`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	cases := map[string]FieldProfile{
		"enum_field1": FieldProfile{
			Enum: map[string][]string{
				"all": []string{"value1", "value2"},
			},
			EnumHints: map[string]map[string]string{
				"all": map[string]string{
					"value1": "hint1",
					"value2": "hint2",
				},
			},
			Required: map[string]bool{"all": true},
		},
		"enum_field2": FieldProfile{
			Enum: map[string][]string{
				"all": []string{"value3", "value4"},
			},
			EnumHints: map[string]map[string]string{
				"all": map[string]string{
					"value3": "hint3",
				},
			},
			Required: map[string]bool{"all": true},
		},
		"enum_field3": FieldProfile{
			Enum: map[string][]string{
				"all": []string{"value3", "value4"},
			},
			Required: map[string]bool{"all": true},
		},
		"str_field1": FieldProfile{
			MinLen:     map[string]int64{"all": int64(4), "v1": int64(5)},
			MaxLen:     map[string]int64{"all": int64(30), "v1": int64(31)},
			DocStrings: map[string][]string{"all": []string{"length of string should be between 4 and 30"}, "v1": []string{"length of string should be between 5 and 31"}},
			Required:   map[string]bool{"all": true},
		},
		"str_field2": FieldProfile{
			MinLen:     map[string]int64{"all": int64(4), "v1": int64(5)},
			MaxLen:     map[string]int64{"all": int64(30), "v1": int64(31)},
			DocStrings: map[string][]string{"all": []string{"length of string should be between 4 and 30"}, "v1": []string{"length of string should be between 5 and 31"}},
			Required:   map[string]bool{"all": false},
		},
		"str_field3": FieldProfile{
			MinLen:     map[string]int64{"all": int64(4), "v1": int64(5)},
			MaxLen:     map[string]int64{"v1": int64(31)},
			DocStrings: map[string][]string{"all": []string{"length of string should be at least 4"}, "v1": []string{"length of string should be between 5 and 31"}},
			Required:   map[string]bool{"all": true},
		},
		"str_field4": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"IP address"}},
			Example:    map[string]string{"all": "10.1.1.1"},
			Required:   map[string]bool{"all": true},
		},
		"str_field5": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"IP address or hostname"}},
			Example:    map[string]string{"all": "10.1.1.1"},
			Required:   map[string]bool{"all": true},
		},
		"str_field6": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"MAC address"}},
			Example:    map[string]string{"all": "0000"},
			Required:   map[string]bool{"all": true},
		},
		"str_field7": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"valid URI"}},
			Example:    map[string]string{"all": "https://"},
			Required:   map[string]bool{"all": true},
		},
		"str_field8": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"valid UUID"}},
			Example:    map[string]string{"all": "49943a2c-9d76-11e7-abc4-cec278b6b50a"},
			Required:   map[string]bool{"all": true},
		},
		"str_field9": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"IPv4 address"}},
			Example:    map[string]string{"all": "10.1.1.1"},
			Required:   map[string]bool{"all": true},
		},
		"int_field1": FieldProfile{
			MinInt:     map[string]int64{"all": int64(3)},
			MaxInt:     map[string]int64{"all": int64(1000)},
			DocStrings: map[string][]string{"all": []string{"value should be between 3 and 1000"}},
			Required:   map[string]bool{"all": true},
		},
		"int_field2": FieldProfile{
			MinInt:     map[string]int64{"all": int64(5)},
			DocStrings: map[string][]string{"all": []string{"value should be at least 5"}},
			Required:   map[string]bool{"all": true},
		},
		"regexp_field1": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"must start and end with alpha numeric and can have alphanumeric, -, _, ."}},
			Pattern:    map[string]string{"all": `^[a-zA-Z0-9][\w\-\.]*[a-zA-Z0-9]$`},
			Required:   map[string]bool{"all": true},
		},
		"regexp_field2": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"must start and end with alpha numeric and can have alphanumeric, -, _, ."}},
			Pattern:    map[string]string{"all": `^[a-zA-Z0-9][\w\-\.]*[a-zA-Z0-9]$`},
			Required:   map[string]bool{"all": false},
		},
		"duration1": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"should be a valid time duration"}},
			Required:   map[string]bool{"all": true},
		},
		"duration2": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"should be a valid time duration of at least 4m0s"}},
			Required:   map[string]bool{"all": true},
		},
		"duration3": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"should be a valid time duration of at most 30m0s"}},
			Required:   map[string]bool{"all": true},
		},
		"duration4": FieldProfile{
			DocStrings: map[string][]string{"all": []string{"should be a valid time duration between 4m0s and 30m0s"}},
			Required:   map[string]bool{"all": false},
		},
	}
	msg, err := r.LookupMsg("", ".example.TestMsg")
	if err != nil {
		t.Fatalf("Could not find msg")
	}
	for _, fld := range msg.Fields {
		if c, ok := cases[*fld.Name]; ok {
			cks, err := gwplugins.GetExtension("venice.check", fld)
			if err == nil {
				profile := FieldProfile{}
				profile.Init()
				for _, v := range cks.([]string) {
					fldv, err := ParseValidator(v)
					if err != nil {
						t.Fatalf("could not parse options for [%s](%s)", *fld.Name, err)
					}
					ValidatorProfileMap[fldv.Fn](fld, r, fldv.Ver, fldv.Args, &profile)
					if fldv.AllowEmpty {
						profile.Required[fldv.Ver] = false
					}
				}
				if len(c.Enum) > 0 {
					if !reflect.DeepEqual(c.Enum, profile.Enum) {
						t.Fatalf("Enum does not match for [%v], exp[%v] got [%v]", *fld.Name, c.Enum, profile.Enum)
					}
				}
				if len(c.EnumHints) > 0 {
					if !reflect.DeepEqual(c.EnumHints, profile.EnumHints) {
						t.Fatalf("Enum does not match for [%v], exp[%v] got [%v]", *fld.Name, c.EnumHints, profile.EnumHints)
					}
				}
				if len(profile.MinInt) != len(c.MinInt) {
					t.Fatalf("lengths of MinInts do not match for [%v]", *fld.Name)
				}
				if len(profile.MinInt) > 0 {
					if !reflect.DeepEqual(profile.MinInt, c.MinInt) {
						t.Fatalf("MinInt profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MinInt, profile.MinInt)
					}
				}
				if len(profile.MaxInt) != len(c.MaxInt) {
					t.Fatalf("lengths of MaxInts do not match for [%v]", *fld.Name)
				}
				if len(profile.MaxInt) > 0 {
					if !reflect.DeepEqual(profile.MaxInt, c.MaxInt) {
						t.Fatalf("MaxInt profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MaxInt, profile.MaxInt)
					}
				}
				if len(profile.MinLen) != len(c.MinLen) {
					t.Fatalf("lengths of MinLens do not match for [%v]", *fld.Name)
				}
				if len(profile.MinLen) > 0 {
					if !reflect.DeepEqual(profile.MinLen, c.MinLen) {
						t.Fatalf("MinLen profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MinLen, profile.MinLen)
					}
				}
				if len(profile.MaxLen) != len(c.MaxLen) {

					t.Fatalf("lengths of MaxLens do not match for [%v], %v, %v", *fld.Name, profile.MaxLen, c.MaxLen)
				}
				if len(profile.MaxLen) > 0 {
					if !reflect.DeepEqual(profile.MaxLen, c.MaxLen) {
						t.Fatalf("MaxLen profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MaxLen, profile.MaxLen)
					}
				}
				if len(c.DocStrings) > 0 {
					for k, v := range c.DocStrings {
						expDocStrings := strings.Join(v, ". ")
						docStrings := strings.Join(profile.DocStrings[k], ". ")
						if !strings.Contains(docStrings, expDocStrings) {
							t.Fatalf("docstring does not match for [%v] got [%v] should contain [%v]", *fld.Name, docStrings, expDocStrings)
						}
					}
				}
				if len(c.Example) > 0 {
					for k, v := range c.Example {
						if !strings.Contains(profile.Example[k], v) {
							t.Fatalf("Example does not match for [%v] got [%v] should contain [%v]", *fld.Name, profile.Example[k], v)
						}
					}
				}
				if len(c.Pattern) > 0 {
					for k, v := range c.Pattern {
						if !strings.Contains(profile.Pattern[k], v) {
							t.Fatalf("Pattern does not match for [%v] got [%v] should contain [%v]", *fld.Name, profile.Pattern[k], v)
						}
					}
				}
				if len(c.Required) > 0 {
					for k, v := range c.Required {
						if !profile.Required[k] == v {
							t.Fatalf("Required does not match for [%v] got [%v] should contain [%v]", *fld.Name, profile.Required[k], v)
						}
					}
				}
			}
		}
	}
}

func TestGetLocation(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'msg1'
			field <
				name: 'msg1fld1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'msg1fld2'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		message_type <
			name: 'msg2'
			field <
				name: 'msg2fld1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'msg2fld2'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		service <
			name: 'svc1'
			method: <
				name: 'svc1method1'
				input_type: '.example.msg1'
				output_type: '.example.msg1'
			>
			method: <
				name: 'svc1method2'
				input_type: '.example.msg2'
				output_type: '.example.msg2'
			>
		>
		service <
			name: 'svc2'
			method: <
				name: 'svc2method1'
				input_type: '.example.msg1'
				output_type: '.example.msg1'
			>
			method: <
				name: 'svc2method2'
				input_type: '.example.msg2'
				output_type: '.example.msg2'
			>
		>
		source_code_info:<
		location:<path:4 path:0 leading_comments:"msg1 comments" >
		location:<path:4 path:0 path:2 path:0 leading_comments:"msg1 field 1 comments" >
		location:<path:4 path:0 path:2 path:1 leading_comments:"msg1 field 2 comments" >
		location:<path:4 path:1 leading_comments:"msg2 comments" >
		location:<path:4 path:1 path:2 path:0 leading_comments:"msg2 field 1 comments" >
		location:<path:4 path:1 path:2 path:1 leading_comments:"msg2 field 2 comments" >
		location:<path:6 path:0 leading_comments:"svc 1 comments" >
		location:<path:6 path:0 path:2 path:0 leading_comments:"svc1 method 1 comments" >
		location:<path:6 path:0 path:2 path:1 leading_comments:"svc1 method 2 comments" >
		location:<path:6 path:1 leading_comments:"svc 2 comments" >
		location:<path:6 path:1 path:2 path:0 leading_comments:"svc2 method 1 comments" >
		location:<path:6 path:1 path:2 path:1 leading_comments:"svc2 method 2 comments" >
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	req.FileToGenerate = []string{"example.proto"}

	file := req.GetProtoFile()[0]
	cases := []struct {
		name     string
		found    bool
		paths    []int
		comments string
		err      error
	}{
		{name: "case1", found: true, paths: []int{4, 0}, comments: "msg1 comments", err: nil},
		{name: "case2", found: true, paths: []int{4, 0, 2, 0}, comments: "msg1 field 1 comments", err: nil},
		{name: "case3", found: true, paths: []int{4, 0, 2, 1}, comments: "msg1 field 2 comments", err: nil},
		{name: "case4", found: true, paths: []int{4, 1}, comments: "msg2 comments", err: nil},
		{name: "case5", found: true, paths: []int{4, 1, 2, 0}, comments: "msg2 field 1 comments", err: nil},
		{name: "case6", found: true, paths: []int{4, 1, 2, 1}, comments: "msg2 field 2 comments", err: nil},
		{name: "case7", found: true, paths: []int{6, 0}, comments: "svc 1 comments", err: nil},
		{name: "case8", found: true, paths: []int{6, 0, 2, 0}, comments: "svc1 method 1 comments", err: nil},
		{name: "case9", found: true, paths: []int{6, 0, 2, 1}, comments: "svc1 method 2 comments", err: nil},
		{name: "case10", found: true, paths: []int{6, 1}, comments: "svc 2 comments", err: nil},
		{name: "case11", found: false, paths: []int{6, 3}, comments: "", err: errors.New("")},
		{name: "case12", found: false, paths: []int{6, 1, 2}, comments: "", err: errors.New("")},
		{name: "case13", found: false, paths: []int{6, 1, 2, 3}, comments: "", err: errors.New("")},
		{name: "case14", found: false, paths: []int{4, 1, 2, 3}, comments: "", err: errors.New("")},
		{name: "case15", found: false, paths: []int{7, 1, 2, 3}, comments: "", err: errors.New("")},
	}
	for _, c := range cases {
		loc, err := GetLocation(file.GetSourceCodeInfo(), c.paths)
		if (c.err == nil) != (err == nil) {
			t.Fatalf("[%s] errors dont match exp: %v got %v", c.name, (c.err == nil), (err == nil))
		}
		if err == nil && c.found && loc.GetLeadingComments() != c.comments {
			t.Fatalf("[%s]comments dont match exp [%s] got [%s]", c.name, c.comments, loc.GetLeadingComments())
		}
	}
}

func TestGetFieldExtensions(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'msg1'
				field <
					name: 'msg1fld1'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					options:<[gogoproto.embed]: true, [venice.mutable]: false>
					number: 1
				>
				field <
					name: 'msg1fld2'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					options:<[gogoproto.embed]: true, [gogoproto.jsontag]: "testTag">
					number: 2
				>
				field <
				name: 'msg1fld3'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.msg2'
				options:<[gogoproto.embed]: true, [gogoproto.jsontag]: "testTag,inline">
				number: 3
				>
				field <
				name: 'msg1fld4'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.msg2'
				options:<[gogoproto.jsontag]: "testTag,inline">
				number: 4
				>
			>
			message_type <
				name: 'msg2'
				field <
					name: 'msg2fld1'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
				>
				field <
					name: 'msg2fld2'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 2
				>
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	RegisterOptionParsers()
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	msg, err := r.LookupMsg("", "example.msg1")
	if err != nil {
		t.Fatalf("could not find message (%s)", err)
	}
	for _, f := range msg.Fields {
		switch *f.Name {
		case "msg1fld1":
			if IsEmbed(f) {
				t.Fatalf("returned Embed as true")
			}
			if GetJSONTag(f) != "" {
				t.Fatalf("got wrong JSON tag [%v]", GetJSONTag(f))
			}
			if IsInline(f) {
				t.Fatalf("expecting inline to be false")
			}
			if IsMutable(f) {
				t.Fatalf("expecting mutable to be false")
			}
		case "msg1fld2":
			if !IsEmbed(f) {
				t.Fatalf("expecting Embed to be true")
			}
			if GetJSONTag(f) != "testTag" {
				t.Fatalf("got wrong JSON tag [%v]", GetJSONTag(f))
			}
			if IsInline(f) {
				t.Fatalf("expecting inline to be false")
			}
			if !IsMutable(f) {
				t.Fatalf("expecting mutable to be true")
			}
		case "msg1fld3":
			if !IsEmbed(f) {
				t.Fatalf("expecting Embed to be true")
			}
			if GetJSONTag(f) != "testTag" {
				t.Fatalf("got wrong JSON tag [%v]", GetJSONTag(f))
			}
			if !IsInline(f) {
				t.Fatalf("expecting inline to be true")
			}
			if !IsMutable(f) {
				t.Fatalf("expecting mutable to be true")
			}
		case "msg1fld4":
			if IsEmbed(f) {
				t.Fatalf("expecting Embed to be false")
			}
			if GetJSONTag(f) != "testTag" {
				t.Fatalf("got wrong JSON tag [%v]", GetJSONTag(f))
			}
			if !IsInline(f) {
				t.Fatalf("expecting inline to be true")
			}
			if !IsMutable(f) {
				t.Fatalf("expecting mutable to be true")
			}
		}

	}

}

func TestGetParams(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
			field <
				name: 'str_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 3
			>
			options:<[venice.objectPrefix]:{Collection:"prefix-{str_field}", Path:"/qual{nest1_field.embedded_field}"}>
		>
		message_type <
			name: 'testmsg'
			field <
				name: 'real_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
			field <
				name: 'leaf_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 3
			>
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 4
			>
			options:<[venice.objectPrefix]:{Collection:"prefix1", Path:"/{leaf_field}"}>
		>
		message_type <
			name: 'Auto_ListNest1'
			field <
				name: 'meta'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
			field <
				name: 'Items'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
		>
		message_type <
		name: 'ObjectMeta'
		field <
			name: 'Name'
			label: LABEL_OPTIONAL
			type: TYPE_STRING
			number: 2
		>
	>
		service <
			name: 'full_crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1">
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc_create'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"create" [venice.methodAutoGen]: true [google.api.http]:<selector:"" post:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_update'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"update" [venice.methodAutoGen]: true [google.api.http]:<selector:"" put:"/prefix/{str_field}">>
			>
			method: <
				name: 'noncrudsvc_get'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"get" [venice.methodAutoGen]: true [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_delete'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"delete" [venice.methodAutoGen]: true [google.api.http]:<selector:"" delete:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_list'
				input_type: '.example.Nest1'
				output_type: '.example.Auto_ListNest1'
				options:<[venice.methodOper]:"list" [venice.methodAutoGen]: true [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_action'
				input_type: '.example.Nest1'
				output_type: '.example.testmsg'
				options:<[venice.methodOper]:"create" [venice.methodAutoGen]: true [venice.methodAutoGen]: true [venice.methodActionObject]: "Nest1" [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_watch'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"watch" [venice.methodAutoGen]: true>
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post" ], Pattern: "/testpattern"}>
		>
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'Nest2'
			field <
				name: 'embedded_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
		>
		service <
			name: 'crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest2">
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	once.Do(doOnce)
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	for _, svc := range file.Services {
		if *svc.Name != "hybrid_crudservice" {
			continue
		}
		sparams, err := GetSvcParams(svc)
		if err != nil {
			t.Errorf("error getting service params")
		}
		t.Logf("Service [%s] got [%+v]", *svc.Name, sparams)
		if sparams.Version != "v1" || sparams.Prefix != "example" {
			t.Errorf("[%s] Did not get expected service params %+v", *svc.Name, sparams)
		}
	}
}

func TestGetProxyPaths(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'Nest1'
				field <
					name: 'nest1_field'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.Nest2'
					number: 1
				>
			>
			service <
				name: 'crudservice'
				method: <
					name: 'TestMethod'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
				>
				options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.proxyPrefix]:{PathPrefix: "/backapi1", Path: "/test1", Backend: "localhost:9999", Response: ".example.Nest1", FormParams:  [{Name: "file", Type: "file", Required: true, Description: "file to upload"}, {Name: "something", Type: "string", Required: false, Description: "another field"}]} [venice.proxyPrefix]:{PathPrefix: "/backapi2", Path: "/test2", Backend: "resolved-svc", Response: "Nest1"} >
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	once.Do(doOnce)
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	svc := file.Services[0]
	pp, err := GetProxyPaths(svc)
	if err != nil {
		t.Fatalf("failed to get proxy paths (%s)", err)
	}
	exp := []ProxyPath{
		{Prefix: "/backapi1", TrimPath: "/configs/example/v1/", Path: "test1", FullPath: "/configs/example/v1/test1", Backend: "localhost:9999", Response: ".example.Nest1", FormParams: []FormParam{{Name: "file", Type: "file", Required: true, Description: "file to upload"}, {Name: "something", Type: "string", Required: false, Description: "another field"}}},
		{Prefix: "/backapi2", TrimPath: "/configs/example/v1/", Path: "test2", FullPath: "/configs/example/v1/test2", Backend: "resolved-svc", Response: ".example.Nest1"},
	}
	if !reflect.DeepEqual(pp, exp) {
		t.Fatalf("Proxy paths does not match exp[%+v] got [%+v]", exp, pp)
	}
}
