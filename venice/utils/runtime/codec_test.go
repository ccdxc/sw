package runtime

import (
	"reflect"
	"testing"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime/test"
)

type TestObj struct {
	api.TypeMeta
	foo string
	Bar string
}

// Clone clones the object
func (m *TestObj) Clone(interface{}) (interface{}, error) { return nil, nil }

type NonProtoObj struct {
	Bar string
}

func (n *NonProtoObj) GetObjectKind() string                  { return "NonProtoObj" }
func (n *NonProtoObj) GetObjectAPIVersion() string            { return "v1" }
func (n *NonProtoObj) Clone(interface{}) (interface{}, error) { return nil, nil }

func newCodec(codec string) Codec {
	s := NewScheme()
	switch codec {
	case "json":
		s.AddKnownTypes(&TestObj{})
		return NewJSONCodec(s)
	case "proto":
		s.AddKnownTypes(&runtimetest.TestProtoMessage{})
		return NewProtoCodec(s)
	}
	return nil
}

func TestJSONEncode(t *testing.T) {
	test := TestObj{foo: "abc", Bar: "def"}
	expectedOut := "{\"kind\":\"\",\"Bar\":\"def\"}"

	codec := newCodec("json")

	out, err := codec.Encode(&test)
	if err != nil || string(out) != expectedOut {
		t.Fatalf("Encode test failed, err %v, out %v, expected: %v", err, string(out), expectedOut)
	}
}

func TestJSONDecode(t *testing.T) {
	expectedObj := TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, Bar: "def"}
	var obj TestObj
	input := "{\"Kind\": \"TestObj\", \"Bar\":\"def\"}"

	codec := newCodec("json")

	if _, err := codec.Decode([]byte(input), &obj); err != nil || obj != expectedObj {
		t.Fatalf("Decode test failed, err %v, obj %v", err, obj)
	}
	if obj, err := codec.Decode([]byte(input), nil); err != nil || obj == nil {
		t.Fatalf("Decode with nil obj failed, error: %v", err)
	}
}

func TestInvalidProtoEncode(t *testing.T) {
	if _, err := newCodec("proto").Encode(&NonProtoObj{Bar: "def"}); err == nil {
		t.Fatalf("Proto encode passed with non proto object")
	}
}

func TestProtoEncode(t *testing.T) {
	test := runtimetest.TestProtoMessage{
		TypeMeta:   api.TypeMeta{Kind: "testType"},
		ObjectMeta: api.ObjectMeta{Name: "TestObject"},
		Field1:     "Data field",
	}
	expectedOut, err := proto.Marshal(&test)
	if err != nil {
		t.Fatalf("Failed to marshal proto, error %v", err)
	}

	codec := newCodec("proto")

	out, err := codec.Encode(&test)
	if err != nil || string(out) != string(expectedOut) {
		t.Fatalf("Encode test failed, err %v, out %v", err, string(out))
	}
}

func TestProtoDecode(t *testing.T) {
	expectedObj := runtimetest.TestProtoMessage{
		TypeMeta:   api.TypeMeta{Kind: "TestProtoMessage"},
		ObjectMeta: api.ObjectMeta{Name: "TestObject"},
		Field1:     "Data field",
	}
	var obj runtimetest.TestProtoMessage
	input, err := proto.Marshal(&expectedObj)
	if err != nil {
		t.Fatalf("Failed to marshal proto, error %v", err)
	}

	codec := newCodec("proto")

	if _, err := codec.Decode(input, &obj); err != nil || !reflect.DeepEqual(obj, expectedObj) {
		t.Fatalf("Decode test failed, err %v, obj %v", err, obj)
	}
	if obj, err := codec.Decode([]byte(input), nil); err != nil || obj == nil {
		t.Fatalf("Decode with nil obj failed, error: %v", err)
	}
}
