package runtime

import (
	"reflect"
	"testing"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
)

type TestObj struct {
	api.TypeMeta
	foo string
	Bar string
}

type NonProtoObj struct {
	Bar string
}

func (n *NonProtoObj) GetObjectKind() string       { return "NonProtoObj" }
func (n *NonProtoObj) GetObjectAPIVersion() string { return "v1" }

func newCodec(codec string) Codec {
	s := NewScheme()
	switch codec {
	case "json":
		s.AddKnownTypes(&TestObj{})
		return NewJSONCodec(s)
	case "proto":
		s.AddKnownTypes(&cmd.Cluster{})
		return NewProtoCodec(s)
	}
	return nil
}

func TestJSONEncode(t *testing.T) {
	test := TestObj{foo: "abc", Bar: "def"}
	expectedOut := "{\"Bar\":\"def\"}"

	codec := newCodec("json")

	out, err := codec.Encode(&test)
	if err != nil || string(out) != expectedOut {
		t.Fatalf("Encode test failed, err %v, out %v", err, string(out))
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
	test := cmd.Cluster{ObjectMeta: api.ObjectMeta{Name: "testCluster"}}
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
	expectedObj := cmd.Cluster{
		TypeMeta:   api.TypeMeta{Kind: "Cluster"},
		ObjectMeta: api.ObjectMeta{Name: "testCluster"},
	}
	var obj cmd.Cluster
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
