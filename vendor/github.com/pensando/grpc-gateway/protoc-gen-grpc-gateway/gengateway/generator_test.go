package gengateway

import (
	"strings"
	"testing"

	"github.com/gogo/protobuf/proto"
	protodescriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
)

func TestGenerateServiceWithoutBindings(t *testing.T) {
	msgdesc := &protodescriptor.DescriptorProto{
		Name: proto.String("ExampleMessage"),
	}
	msg := &descriptor.Message{
		DescriptorProto: msgdesc,
	}
	msg1 := &descriptor.Message{
		DescriptorProto: msgdesc,
		File: &descriptor.File{
			GoPkg: descriptor.GoPackage{
				Path: "github.com/gogo/protobuf/ptypes/empty",
				Name: "empty",
			},
		},
	}
	meth := &protodescriptor.MethodDescriptorProto{
		Name:       proto.String("Example"),
		InputType:  proto.String("ExampleMessage"),
		OutputType: proto.String("ExampleMessage"),
	}
	meth1 := &protodescriptor.MethodDescriptorProto{
		Name:       proto.String("ExampleWithoutBindings"),
		InputType:  proto.String("empty.Empty"),
		OutputType: proto.String("empty.Empty"),
	}
	svc := &protodescriptor.ServiceDescriptorProto{
		Name:   proto.String("ExampleService"),
		Method: []*protodescriptor.MethodDescriptorProto{meth, meth1},
	}
	file := descriptor.File{
		FileDescriptorProto: &protodescriptor.FileDescriptorProto{
			Name:        proto.String("example.proto"),
			Package:     proto.String("example"),
			Dependency:  []string{"a.example/b/c.proto", "a.example/d/e.proto"},
			MessageType: []*protodescriptor.DescriptorProto{msgdesc},
			Service:     []*protodescriptor.ServiceDescriptorProto{svc},
		},
		GoPkg: descriptor.GoPackage{
			Path: "example.com/path/to/example/example.pb",
			Name: "example_pb",
		},
		Messages: []*descriptor.Message{msg},
		Services: []*descriptor.Service{
			{
				ServiceDescriptorProto: svc,
				Methods: []*descriptor.Method{
					{
						MethodDescriptorProto: meth,
						RequestType:           msg,
						ResponseType:          msg,
						Bindings: []*descriptor.Binding{
							{
								HTTPMethod: "GET",
								Body:       &descriptor.Body{FieldPath: nil},
							},
						},
					},
					{
						MethodDescriptorProto: meth1,
						RequestType:           msg1,
						ResponseType:          msg1,
					},
				},
			},
		},
	}
	g := &generator{}
	got, err := g.generate(crossLinkFixture(&file))
	if err != nil {
		t.Errorf("generate(%#v) failed with %v; want success", file, err)
		return
	}
	if notwanted := `"github.com/gogo/protobuf/ptypes/empty"`; strings.Contains(got, notwanted) {
		t.Errorf("generate(%#v) = %s; does not want to contain %s", file, got, notwanted)
	}
}

// TestDynPath
// Tests generating of dynamic paths from template list definition when using
// {PACKAGE} and {FILE} annotations.
func TestDynPath(t *testing.T) {
	name := "testFile.proto"
	protofile := protodescriptor.FileDescriptorProto{
		Name: &name,
	}
	file := descriptor.File{
		GoPkg: descriptor.GoPackage{
			Name: "testPkg",
		},
	}
	file.FileDescriptorProto = &protofile

	s := getDynPath(&file, "generated/{PACKAGE}/{FILE}_a.go")
	if s != "generated/testPkg/testFile_a.go" {
		t.Errorf("Expecting [generated/testPkg/testFile_a.go] got [%s]", s)
	}
	s = getDynPath(&file, "generated/{PACKAGE }/{FILE}_a.go")
	if s != "generated/{PACKAGE }/testFile_a.go" {
		t.Errorf("Expecting [generated/{PACKAGE }/testFile_a.go] got [%s]", s)
	}
	s = getDynPath(&file, "generated/FILE}_a.go")
	if s != "generated/FILE}_a.go" {
		t.Errorf("Expecting [generated/FILE}_a.go] got [%s]", s)
	}
	s = getDynPath(&file, "generated/{FILE}_a.go")
	if s != "generated/testFile_a.go" {
		t.Errorf("Expecting [generated/testFile_a.go] got [%s]", s)
	}
	name = "testFile.pro"
	s = getDynPath(&file, "generated/{PACKAGE}/{FILE}_a.go")
	if s != "generated/testPkg/testFile.pro_a.go" {
		t.Errorf("Expecting [generated/testPkg/testFile.pro_a.go] got [%s]", s)
	}
}
