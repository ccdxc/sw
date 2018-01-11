package plugin

import (
	"testing"

	"github.com/gogo/protobuf/proto"
	gogodescriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
)

func parseStringOptions(val interface{}) (interface{}, error) {
	v := val.(*string)
	return *v, nil
}

func TestFileGetExtension(t *testing.T) {
	in := `

	name:"service.proto"
	package:"example"

	options:<[plugin.testFileExt]:"FileStr1234">
	syntax:"proto3"
	`
	var fd gogodescriptor.FileDescriptorProto
	if err := proto.UnmarshalText(in, &fd); err != nil {
		t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", in, err)
	}
	RegisterOptionParser("plugin.testFileExt", parseStringOptions)
	var file descriptor.File
	file.FileDescriptorProto = &fd
	var o interface{}
	var err error
	if o, err = GetExtension("plugin.testFileExt", &file); err != nil {
		t.Fatalf("Failed to get Extension for File (%v)", err)
	}
	if o.(string) != "FileStr1234" {
		t.Fatalf("retrieved extension does not match [%s/%s]", o.(string), "FileStr1234")
	}

	if o, err = GetExtension("DummyExtension", &file); err == nil {
		t.Fatalf("Could retrieve a dummy extension for File")
	}
}

func TestServiceGetExtension(t *testing.T) {
	in := `

	name:"TestSvc"

	options:<[plugin.testServiceExt]:"testServiceExtStr">
	`
	var svc gogodescriptor.ServiceDescriptorProto
	if err := proto.UnmarshalText(in, &svc); err != nil {
		t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", in, err)
	}
	RegisterOptionParser("plugin.testServiceExt", parseStringOptions)
	var service descriptor.Service
	service.ServiceDescriptorProto = &svc
	var o interface{}
	var err error
	if o, err = GetExtension("plugin.testServiceExt", &service); err != nil {
		t.Fatalf("Failed to get Extension for Service (%v)", err)
	}
	if o.(string) != "testServiceExtStr" {
		t.Fatalf("retrieved extension does not match [%s/%s]", o.(string), "testServiceExtStr")
	}

	if o, err = GetExtension("DummyExtension", &service); err == nil {
		t.Fatalf("Could retrieve a dummy extension for File")
	}
}

func TestMethodGetExtension(t *testing.T) {
	in := `

	name:"TestMethod"
	input_type:".example.testmsg"
	output_type:".example.testmsg"
	options:<[plugin.testMethodExt]:"testMethodExtStr" >
	`
	var m gogodescriptor.MethodDescriptorProto
	if err := proto.UnmarshalText(in, &m); err != nil {
		t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", in, err)
	}
	RegisterOptionParser("plugin.testMethodExt", parseStringOptions)
	var method descriptor.Method
	method.MethodDescriptorProto = &m
	var o interface{}
	var err error
	if o, err = GetExtension("plugin.testMethodExt", &method); err != nil {
		t.Fatalf("Failed to get Extension for Service (%v)", err)
	}
	if o.(string) != "testMethodExtStr" {
		t.Fatalf("retrieved extension does not match [%s/%s]", o.(string), "testMethodExtStr")
	}

	if o, err = GetExtension("DummyExtension", &method); err == nil {
		t.Fatalf("Could retrieve a dummy extension for File")
	}
}

func TestMessageGetExtension(t *testing.T) {
	in := `

	name: 'testmsg'
	field <
		name: 'test_field'
		label: LABEL_OPTIONAL
		type: TYPE_STRING
		number: 1
	>
	options:<[plugin.testMsgExt]:"testMsgExtStr" >
	`
	var m gogodescriptor.DescriptorProto
	if err := proto.UnmarshalText(in, &m); err != nil {
		t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", in, err)
	}
	RegisterOptionParser("plugin.testMsgExt", parseStringOptions)
	var message descriptor.Message
	message.DescriptorProto = &m
	var o interface{}
	var err error
	if o, err = GetExtension("plugin.testMsgExt", &message); err != nil {
		t.Fatalf("Failed to get Extension for Service (%v)", err)
	}
	if o.(string) != "testMsgExtStr" {
		t.Fatalf("retrieved extension does not match [%s/%s]", o.(string), "testMsgExtStr")
	}

	if o, err = GetExtension("DummyExtension", &message); err == nil {
		t.Fatalf("Could retrieve a dummy extension for File")
	}
}
