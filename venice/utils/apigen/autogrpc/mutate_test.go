package autogrpc

import (
	"errors"
	"fmt"
	"reflect"
	"sort"
	"strings"
	"testing"

	_ "github.com/gogo/protobuf/gogoproto"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gwplugins "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"

	_ "github.com/pensando/sw/venice/utils/apigen/annotations"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
	"github.com/pensando/sw/venice/utils/ref"
)

func TestMutator(t *testing.T) {
	var req plugin.CodeGeneratorRequest
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
			options:<[venice.objectPrefix]:{Collection:"nest1"}>
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
			options:<[venice.objectPrefix]:{Collection:"testmsg"}>
		>
		message_type <
			name: 'singletonmsg'
			field <
				name: 'real_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
			options:<[venice.objectPrefix]:{Singleton:"singletonmsg"}>
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post", "label" ], Pattern: "/testpattern"}>
		>
		service <
			name: 'full_crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1">
		>
		service <
			name: 'action_service'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiAction]: {Collection: "Nest1", Action:"testaction", Request:"Nest1", Response:"Nest1"} [venice.apiAction]: {Object: "Nest1", Action:"testObjaction", Request:"Nest1", Response:"Nest1"}>

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
	req.FileToGenerate = []string{"example.proto"}
	type svccount struct {
		methodcount int
		autoCreate  int
		autoUpdate  int
		autoDelete  int
		autoGet     int
		autoList    int
		autoWatch   int
		autoLabel   int
	}
	type counts struct {
		msgcount  int
		autoList  int
		autoWatch int
		svcs      map[string]*svccount
	}
	expected := make(map[string]*counts)

	expected["example.proto"] = &counts{
		svcs:      make(map[string]*svccount),
		msgcount:  7,
		autoList:  2,
		autoWatch: 2,
	}
	expected["example.proto"].svcs["hybrid_crudservice"] = &svccount{
		methodcount: 16,
		autoCreate:  2,
		autoUpdate:  2,
		autoDelete:  2,
		autoGet:     2,
		autoList:    2,
		autoWatch:   3,
		autoLabel:   2,
	}
	expected["example.proto"].svcs["full_crudservice"] = &svccount{
		methodcount: 8,
		autoCreate:  1,
		autoUpdate:  1,
		autoDelete:  1,
		autoGet:     1,
		autoList:    1,
		autoWatch:   2,
		autoLabel:   1,
	}
	expected["example.proto"].svcs["action_service"] = &svccount{
		methodcount: 10,
		autoCreate:  1,
		autoUpdate:  1,
		autoDelete:  1,
		autoGet:     1,
		autoList:    1,
		autoWatch:   2,
		autoLabel:   1,
	}
	expected["another.proto"] = &counts{
		svcs:      make(map[string]*svccount),
		msgcount:  1,
		autoList:  0,
		autoWatch: 0,
	}
	expected["another.proto"].svcs["crudservice"] = &svccount{
		methodcount: 0,
		autoCreate:  0,
		autoUpdate:  0,
		autoDelete:  0,
		autoGet:     0,
		autoList:    0,
		autoWatch:   0,
		autoLabel:   0,
	}
	AddAutoGrpcEndpoints(&req)
	found := make(map[string]*counts)
	found["example.proto"] = &counts{
		svcs: make(map[string]*svccount),
	}
	found["another.proto"] = &counts{
		svcs: make(map[string]*svccount),
	}
	for _, file := range req.ProtoFile {
		c := found[*file.Name]
		c.msgcount = len(file.MessageType)
		for _, msgs := range file.MessageType {
			if strings.Contains(*msgs.Name, "List") {
				c.autoList++
			}
			if strings.Contains(*msgs.Name, "WatchHelper") {
				c.autoWatch++
			}
			if strings.Contains(*msgs.Name, "singleton") {
				if s, err := isSingleton(msgs); err != nil || !s {
					t.Errorf("Single message %v did not return true in singleton check", *msgs.Name)
				}
			}
		}
		for _, svcs := range file.Service {
			if _, ok := c.svcs[*svcs.Name]; !ok {
				c.svcs[*svcs.Name] = &svccount{}
			}
			s := c.svcs[*svcs.Name]
			s.methodcount = len(svcs.Method)
			for _, method := range svcs.Method {
				if strings.Contains(*method.Name, "AutoAdd") {
					s.autoCreate++
				}
				if strings.Contains(*method.Name, "AutoUpdate") {
					s.autoUpdate++
				}
				if strings.Contains(*method.Name, "AutoGet") {
					s.autoGet++
				}
				if strings.Contains(*method.Name, "AutoDelete") {
					s.autoDelete++
				}
				if strings.Contains(*method.Name, "AutoList") {
					s.autoList++
				}
				if strings.Contains(*method.Name, "AutoWatch") {
					s.autoWatch++
				}
				if strings.Contains(*method.Name, "AutoLabel") {
					s.autoLabel++
				}
			}
			methStr := fmt.Sprintf("%v", svcs.Method)
			sort.Slice(svcs.Method, func(x, y int) bool {
				return *svcs.Method[x].Name < *svcs.Method[y].Name
			})
			if methStr != fmt.Sprintf("%v", svcs.Method) {
				t.Errorf("Methods list is not sorted")
			}
		}
		msgStr := fmt.Sprintf("%v", file.MessageType)
		sort.Slice(file.MessageType, func(x, y int) bool {
			return *file.MessageType[x].Name < *file.MessageType[y].Name
		})
		if msgStr != fmt.Sprintf("%v", file.MessageType) {
			t.Errorf("Messages list is not sorted")
		}
		svcStr := fmt.Sprintf("%v", file.Service)
		sort.Slice(file.Service, func(x, y int) bool {
			return *file.Service[x].Name < *file.Service[y].Name
		})
		if svcStr != fmt.Sprintf("%v", file.Service) {
			t.Errorf("Services list is not sorted")
		}
	}
	if !reflect.DeepEqual(expected, found) {
		for _, protoName := range []string{"example.proto", "another.proto"} {
			t.Logf("===> %s", protoName)
			t.Logf("expected: %+v\nfound: %+v", expected[protoName], found[protoName])
			t.Logf("full_crud expected: %+v\nfull_crud found %+v",
				expected[protoName].svcs["full_crudservice"], found[protoName].svcs["full_crudservice"])
			t.Logf("hybrid_crud expected: %+v\nhybrid_crud found %+v",
				expected[protoName].svcs["hybrid_crudservice"], found[protoName].svcs["hybrid_crudservice"])
			t.Logf("action_service expected: %+v\naction_service found %+v",
				expected[protoName].svcs["action_service"], found[protoName].svcs["action_service"])

		}
		t.Fatalf("expected and found do not match")
	}
}

func TestSourceCodeInfo(t *testing.T) {
	var req plugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		enum_type:<
			name:"Enum1"
			value:<name:"Value1" number:0 >
			value:<name:"Value2" number:1 >
		>
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
			enum_type:<
				name:"msg1Enum1"
				value:<name:"Value1" number:0 >
				value:<name:"Value2" number:1 >
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
		location:<path:5 path:0 leading_comments:"Enum1 comments" >
		location:<path:5 path:0 path:2 path:0 leading_comments:"Enum1 field 1 comments" >
		location:<path:5 path:0 path:2 path:1>
		location:<path:4 path:0 leading_comments:"msg1 comments" >
		location:<path:4 path:0 path:2 path:0 leading_comments:"msg1 field 1 comments" >
		location:<path:4 path:0 path:2 path:1 leading_comments:"msg1 field 2 comments" >
		location:<path:4 path:0 path:4 path:0 leading_comments:"msg1Enum1 comments" >
		location:<path:4 path:0 path:4 path:0 path:2 path:0 leading_comments:"msg1Enum1 field 1 comments" >
		location:<path:4 path:0 path:4 path:0 path:2  path:1>
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
	sci := saveSrcCodeInfo(file)
	expsci := srcCodeInfo{
		msgs: map[string]msgSrcCodeInfo{
			"msg1": msgSrcCodeInfo{
				codeInfo: codeInfo{comments: "msg1 comments"},
				fields: map[string]codeInfo{
					"msg1fld1": codeInfo{comments: "msg1 field 1 comments"},
					"msg1fld2": codeInfo{comments: "msg1 field 2 comments"},
				},
				nestedMsgs: map[string]msgSrcCodeInfo{},
				enums: map[string]enumSrcCodeInfo{
					"msg1Enum1": enumSrcCodeInfo{
						codeInfo: codeInfo{comments: "msg1Enum1 comments"},
						values: map[string]codeInfo{
							"Value1": codeInfo{comments: "msg1Enum1 field 1 comments"},
							"Value2": codeInfo{comments: ""},
						},
					},
				},
			},
			"msg2": msgSrcCodeInfo{
				codeInfo: codeInfo{comments: "msg2 comments"},
				fields: map[string]codeInfo{
					"msg2fld1": codeInfo{comments: "msg2 field 1 comments"},
					"msg2fld2": codeInfo{comments: "msg2 field 2 comments"},
				},
				nestedMsgs: map[string]msgSrcCodeInfo{},
				enums:      map[string]enumSrcCodeInfo{},
			},
		},
		services: map[string]svcSrcCodeInfo{
			"svc1": svcSrcCodeInfo{
				codeInfo: codeInfo{comments: "svc 1 comments"},
				methods: map[string]codeInfo{
					"svc1method1": codeInfo{comments: "svc1 method 1 comments"},
					"svc1method2": codeInfo{comments: "svc1 method 2 comments"},
				},
			},
			"svc2": svcSrcCodeInfo{
				codeInfo: codeInfo{comments: "svc 2 comments"},
				methods: map[string]codeInfo{
					"svc2method1": codeInfo{comments: "svc2 method 1 comments"},
					"svc2method2": codeInfo{comments: "svc2 method 2 comments"},
				},
			},
		},
		enums: map[string]enumSrcCodeInfo{
			"Enum1": enumSrcCodeInfo{
				codeInfo: codeInfo{comments: "Enum1 comments"},
				values: map[string]codeInfo{
					"Value1": codeInfo{comments: "Enum1 field 1 comments"},
					"Value2": codeInfo{comments: ""},
				},
			},
		},
	}
	if !reflect.DeepEqual(&sci, &expsci) {
		diff, _ := ref.ObjDiff(sci, expsci)
		t.Fatalf("save src code info does not match at %v exp/got\n%+v\n%+v", diff.List(), expsci, sci.services)
	}
	// Restore the source code info.
	restoreScrCodeInfo(file, sci)
	cases := []struct {
		name     string
		paths    []int
		comments string
		err      error
	}{
		{name: "case1", paths: []int{4, 0}, comments: "msg1 comments", err: nil},
		{name: "case2", paths: []int{4, 0, 2, 0}, comments: "msg1 field 1 comments", err: nil},
		{name: "case3", paths: []int{4, 0, 2, 1}, comments: "msg1 field 2 comments", err: nil},
		{name: "case4", paths: []int{4, 1}, comments: "msg2 comments", err: nil},
		{name: "case5", paths: []int{4, 1, 2, 0}, comments: "msg2 field 1 comments", err: nil},
		{name: "case6", paths: []int{4, 1, 2, 1}, comments: "msg2 field 2 comments", err: nil},
		{name: "case7", paths: []int{6, 0}, comments: "svc 1 comments", err: nil},
		{name: "case8", paths: []int{6, 0, 2, 0}, comments: "svc1 method 1 comments", err: nil},
		{name: "case9", paths: []int{6, 0, 2, 1}, comments: "svc1 method 2 comments", err: nil},
		{name: "case10", paths: []int{6, 1}, comments: "svc 2 comments", err: nil},
		{name: "case11", paths: []int{4, 0, 4, 0}, comments: "msg1Enum1 comments", err: nil},
		{name: "case12", paths: []int{4, 0, 4, 0, 2, 0}, comments: "msg1Enum1 field 1 comments", err: nil},
		{name: "case13", paths: []int{4, 0, 4, 0, 2, 1}, comments: "", err: nil},
		{name: "case14", paths: []int{5, 0}, comments: "Enum1 comments", err: nil},
		{name: "case15", paths: []int{5, 0, 2, 0}, comments: "Enum1 field 1 comments", err: nil},
		{name: "case16", paths: []int{5, 0, 2, 1}, comments: "", err: nil},
		{name: "case51", paths: []int{6, 3}, comments: "", err: errors.New("")},
		{name: "case52", paths: []int{6, 1, 2}, comments: "", err: errors.New("")},
		{name: "case53", paths: []int{6, 1, 2, 3}, comments: "", err: errors.New("")},
		{name: "case54", paths: []int{4, 1, 2, 3}, comments: "", err: errors.New("")},
		{name: "case55", paths: []int{7, 1, 2, 3}, comments: "", err: errors.New("")},
	}
	for _, c := range cases {
		loc, err := common.GetLocation(file.GetSourceCodeInfo(), c.paths)
		if (c.err == nil) != (err == nil) {
			t.Fatalf("[%s] errors dont match exp: %v got %v", c.name, (c.err == nil), (err == nil))
		}
		if err == nil && loc.GetLeadingComments() != c.comments {
			t.Fatalf("[%s]comments dont match exp [%s] got [%s]", c.name, c.comments, loc.GetLeadingComments())
		}
	}

}

func parseBoolOptions(val interface{}) (interface{}, error) {
	v, ok := val.(*bool)
	if !ok {
		return nil, fmt.Errorf("could not parse")
	}
	return *v, nil
}

func TestInsertFileDefaults(t *testing.T) {
	params := make(map[string]string)
	var req plugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example1.proto'
		package: 'example'
		syntax: 'proto3'
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileGrpcDest]: "xyz">
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	gwplugins.RegisterOptionParser("gogoproto.goproto_enum_stringer_all", parseBoolOptions)
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example1.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file1, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file1")
	}
	file2, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file2")
	}
	// With no option
	if err = insertFileDefaults(params, file1.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file1) {
		t.Fatalf("not expecting extension to be added")
	}
	if err = insertFileDefaults(params, file2.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file2) {
		t.Fatalf("not expecting extension to be added")
	}

	// With bad option
	params[customEnumStringParam] = "junkvalue"
	if err = insertFileDefaults(params, file1.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file1) {
		t.Fatalf("not expecting extension to be added")
	}
	if err = insertFileDefaults(params, file2.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file2) {
		t.Fatalf("not expecting extension to be added")
	}
	// With proper option but false
	params[customEnumStringParam] = "false"
	if err = insertFileDefaults(params, file1.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file1) {
		t.Fatalf("not expecting extension to be added")
	}
	if err = insertFileDefaults(params, file2.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file2) {
		t.Fatalf("not expecting extension to be added")
	}
	// with proper option and true
	params[customEnumStringParam] = "true"
	if err = insertFileDefaults(params, file1.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if !gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file1) {
		t.Fatalf("expecting extension to be added")
	}
	if err = insertFileDefaults(params, file2.FileDescriptorProto); err != nil {
		t.Fatalf("insertFileDefaults failed for file1")
	}
	if !gwplugins.HasExtension("gogoproto.goproto_enum_stringer_all", file2) {
		t.Fatalf("expecting extension to be added")
	}
}

func TestParseReqParams(t *testing.T) {
	cases := []struct {
		str string
		exp map[string]string
	}{
		{
			str: "custom_enumstr=true,Mgoogle/protobuf/timestamp.proto=github.com/gogo/protobuf/types,Mgoogle/protobuf/any.proto=github.com/gogo/protobuf/types",
			exp: map[string]string{"custom_enumstr": "true", "Mgoogle/protobuf/timestamp.proto": "github.com/gogo/protobuf/types", "Mgoogle/protobuf/any.proto": "github.com/gogo/protobuf/types"},
		},
		{
			str: "test1,test2=abc,test3=abc=xyx,test4:aaa",
			exp: map[string]string{"test2": "abc", "test3": "abc=xyx"},
		},
	}

	for _, c := range cases {
		got := make(map[string]string)
		err := parseReqParam(c.str, got)
		if err != nil {
			t.Fatalf("failed to parse parameters (%s)", err)
		}
		if !reflect.DeepEqual(got, c.exp) {
			t.Fatalf("did not match \n[%+v]\n[%+v]", got, c.exp)
		}
	}
}
