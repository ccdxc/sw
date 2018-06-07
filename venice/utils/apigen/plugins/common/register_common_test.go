package common

import (
	"errors"
	"reflect"
	"strings"
	"testing"

	govldtr "github.com/asaskevich/govalidator"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"

	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gwplugins "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
)

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
			Type: "test",
			To:   "destination",
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
		in := venice.RestEndpoint{
			Object:  "test",
			Method:  []string{"put", "get"},
			Pattern: "/a/a/a/",
		}
		out, err := parseRestServiceOption(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(venice.RestEndpoint), in) {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
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
		"str_enum_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"Value1\"", "v2": "\"Value3\"", "v3": "\"unknown value\""}}},
		"str_enum_nodef_field":   {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"value1\"", "v3": "\"Value3\""}}},
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
				options:<[venice.check]: "IPAddr()">
				number: 5
			>
			field <
				name: 'str_field3'
				type: TYPE_STRING
				options:<[venice.check]: "HostAddr()">
				number: 6
			>
			field <
				name: 'str_field4'
				type: TYPE_STRING
				options:<[venice.check]: "MacAddr()">
				number: 7
			>
			field <
				name: 'str_field5'
				type: TYPE_STRING
				options:<[venice.check]: "URI()">
				number: 8
			>
			field <
				name: 'str_field6'
				type: TYPE_STRING
				options:<[venice.check]: "UUID()">
				number: 9
			>
			field <
				name: 'str_field7'
				type: TYPE_STRING
				options:<[venice.check]: "IPv4()">
				number: 9
			>
			field <
				name: 'int_field1'
				type: TYPE_STRING
				options:<[venice.check]: "IntRange(3, 1000)">
				number: 9
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
				"all": []string{"Value1", "Value2"},
			},
			EnumHints: map[string]map[string]string{
				"all": map[string]string{
					"Value1": "hint1",
					"Value2": "hint2",
				},
			},
		},
		"enum_field2": FieldProfile{
			Enum: map[string][]string{
				"all": []string{"Value3", "Value4"},
			},
			EnumHints: map[string]map[string]string{
				"all": map[string]string{
					"Value3": "hint3",
				},
			},
		},
		"enum_field3": FieldProfile{
			Enum: map[string][]string{
				"all": []string{"Value3", "Value4"},
			},
		},
		"str_field1": FieldProfile{
			MinLen:    map[string]int64{"all": int64(4), "v1": int64(5)},
			MaxLen:    map[string]int64{"all": int64(30), "v1": int64(31)},
			DocString: map[string]string{"all": "length of string should be between 4 and 30", "v1": "length of string should be between 5 and 31"},
		},
		"str_field2": FieldProfile{
			DocString: map[string]string{"all": "IP address"},
			Example:   map[string]string{"all": "10.1.1.1"},
		},
		"str_field3": FieldProfile{
			DocString: map[string]string{"all": "IP address or hostname"},
			Example:   map[string]string{"all": "10.1.1.1"},
		},
		"str_field4": FieldProfile{
			DocString: map[string]string{"all": "MAC address"},
			Example:   map[string]string{"all": "00:00"},
		},
		"str_field5": FieldProfile{
			DocString: map[string]string{"all": "valid URI"},
			Example:   map[string]string{"all": "https://"},
		},
		"str_field6": FieldProfile{
			DocString: map[string]string{"all": "valid UUID"},
			Example:   map[string]string{"all": "49943a2c-9d76-11e7-abc4-cec278b6b50a"},
		},
		"str_field7": FieldProfile{
			DocString: map[string]string{"all": "IPv4 address"},
			Example:   map[string]string{"all": "10.1.1.1"},
		},
		"int_field1": FieldProfile{
			MinInt:    map[string]int64{"all": int64(3)},
			MaxInt:    map[string]int64{"all": int64(1000)},
			DocString: map[string]string{"all": "value should be between 3 and 1000"},
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
					t.Fatalf("lenghts of MinInts do not match for [%v]", *fld.Name)
				}
				if len(profile.MinInt) > 0 {
					if !reflect.DeepEqual(profile.MinInt, c.MinInt) {
						t.Fatalf("MinInt profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MinInt, profile.MinInt)
					}
				}
				if len(profile.MaxInt) != len(c.MaxInt) {
					t.Fatalf("lenghts of MaxInts do not match for [%v]", *fld.Name)
				}
				if len(profile.MaxInt) > 0 {
					if !reflect.DeepEqual(profile.MaxInt, c.MaxInt) {
						t.Fatalf("MaxInt profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MaxInt, profile.MaxInt)
					}
				}
				if len(profile.MinLen) != len(c.MinLen) {
					t.Fatalf("lenghts of MinLens do not match for [%v]", *fld.Name)
				}
				if len(profile.MinLen) > 0 {
					if !reflect.DeepEqual(profile.MinLen, c.MinLen) {
						t.Fatalf("MinLen profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MinLen, profile.MinLen)
					}
				}
				if len(profile.MaxLen) != len(c.MaxLen) {
					t.Fatalf("lenghts of MaxLens do not match for [%v]", *fld.Name)
				}
				if len(profile.MaxLen) > 0 {
					if !reflect.DeepEqual(profile.MaxLen, c.MaxLen) {
						t.Fatalf("MaxLen profiles mismatch for [%v], exp[%v] got[%v]", *fld.Name, c.MaxLen, profile.MaxLen)
					}
				}
				if len(c.DocString) > 0 {
					for k, v := range c.DocString {
						if !strings.Contains(profile.DocString[k], v) {
							t.Fatalf("docstring does not match for [%v] got [%v] should contain [%v]", *fld.Name, profile.DocString[k], v)
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
