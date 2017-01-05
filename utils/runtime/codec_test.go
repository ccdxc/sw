package runtime

import (
	"testing"

	"github.com/pensando/sw/api"
)

type TestObj struct {
	api.TypeMeta
	foo string
	Bar string
}

func newCodec() *JSONCodec {
	s := NewScheme()
	s.AddKnownTypes(&TestObj{})
	return NewJSONCodec(s)
}

func TestEncode(t *testing.T) {
	test := TestObj{foo: "abc", Bar: "def"}
	expectedOut := "{\"Bar\":\"def\"}"

	codec := newCodec()

	out, err := codec.Encode(&test)
	if err != nil || string(out) != expectedOut {
		t.Fatalf("Encode test failed, err %v, out %v", err, string(out))
	}
}

func TestDecode(t *testing.T) {
	expectedObj := TestObj{Bar: "def"}
	var obj TestObj
	input := "{\"Bar\":\"def\"}"

	codec := newCodec()

	if _, err := codec.Decode([]byte(input), &obj); err != nil || obj != expectedObj {
		t.Fatalf("Decode test failed, err %v, obj %v", err, obj)
	}
}
