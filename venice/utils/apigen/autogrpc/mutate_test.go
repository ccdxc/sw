package autogrpc

import (
	"fmt"
	"reflect"
	"sort"
	"strings"
	"testing"

	_ "github.com/gogo/protobuf/gogoproto"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"

	_ "github.com/pensando/sw/venice/utils/apigen/annotations"
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
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post" ], Pattern: "/testpattern"}>
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
		methodcount: 13,
		autoCreate:  2,
		autoUpdate:  2,
		autoDelete:  2,
		autoGet:     2,
		autoList:    2,
		autoWatch:   2,
	}
	expected["example.proto"].svcs["full_crudservice"] = &svccount{
		methodcount: 6,
		autoCreate:  1,
		autoUpdate:  1,
		autoDelete:  1,
		autoGet:     1,
		autoList:    1,
		autoWatch:   1,
	}
	expected["example.proto"].svcs["action_service"] = &svccount{
		methodcount: 8,
		autoCreate:  1,
		autoUpdate:  1,
		autoDelete:  1,
		autoGet:     1,
		autoList:    1,
		autoWatch:   1,
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
			t.Logf("action_service expected: %+v\action_service found %+v",
				expected[protoName].svcs["action_service"], found[protoName].svcs["action_service"])

		}
		t.Fatalf("expected and found do not match")
	}
}
