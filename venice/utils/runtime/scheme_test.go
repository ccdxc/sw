package runtime

import (
	"reflect"
	"testing"
)

func TestScheme(t *testing.T) {
	s := NewScheme()
	s.AddKnownTypes(&TestObj{})
	if _, err := s.New("TestObj"); err != nil {
		t.Fatalf("New failed, error: %v", err)
	}
	if _, err := s.New("testobj2"); err == nil {
		t.Fatalf("New passed when it should have failed")
	}
}

func TestSchemaTypes(t *testing.T) {
	types := map[string]*Struct{
		"test.Type1": &Struct{
			Fields: map[string]Field{
				"Fld1": Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": &Struct{
			Fields: map[string]Field{
				"Fld1": Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
	}
	schema := NewScheme()
	schema.AddSchema(types)

	n := schema.GetSchema("test.Dummy")
	if n != nil {
		t.Fatalf("found invalid Schema")
	}
	n = schema.GetSchema("test.Type1")
	if n == nil {
		t.Fatalf("Schema not found")
	}
	if fld, ok := n.FindField("Fld1"); !ok {
		t.Fatalf("Failed to find Fld1")
	} else {
		if fld.Name != "Fld1" || fld.JSONTag != "fld1" {
			t.Fatalf("wrong schema node")
		}
	}
	if fld, ok := n.FindFieldByJSONTag("fld1"); !ok {
		t.Fatalf("Failed to find fld1")
	} else {
		if fld.Name != "Fld1" || fld.JSONTag != "fld1" {
			t.Fatalf("wrong schema node")
		}
	}
	if _, ok := n.FindField("Dummy1"); ok {
		t.Fatalf("Found field Dummy1")
	}
	if _, ok := n.FindFieldByJSONTag("dummy1"); ok {
		t.Fatalf("Found field dummy1")
	}
}

func TestDupSchema(t *testing.T) {
	types := map[string]*Struct{
		"test.Type1": &Struct{
			Fields: map[string]Field{
				"Fld1": Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": &Struct{
			Fields: map[string]Field{
				"Fld1": Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
	}
	schema := NewScheme()
	schema.AddSchema(types)
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
	}()
	// Adding same types agains should panic
	schema.AddSchema(types)
}

type testObj struct {
	Test string
}

func (f testObj) GetObjectKind() string                       { return "testObj" }
func (f testObj) GetObjectAPIVersion() string                 { return "" }
func (f testObj) Clone(into interface{}) (interface{}, error) { return into, nil }

func TestSchemaNewEmpty(t *testing.T) {
	var in Object
	if out, err := NewEmpty(in); err == nil || out != nil {
		t.Errorf("expecting to fail")
	}

	cases := []struct {
		in  Object
		cmp Object
	}{
		{in: &TestObj{}, cmp: &TestObj{}},
		{in: &TestObj{foo: "abc", Bar: "xyz"}, cmp: &TestObj{}},
		{in: testObj{}, cmp: testObj{}},
		{in: testObj{Test: "foo"}, cmp: testObj{}},
		{in: &testObj{}, cmp: &testObj{}},
		{in: &testObj{Test: "foo"}, cmp: &testObj{}},
	}
	for _, c := range cases {
		if out, err := NewEmpty(c.in); err != nil || out == nil {
			t.Errorf("expecting to pass")
		} else {
			if !reflect.DeepEqual(out, c.cmp) {
				t.Errorf("Not empty TestObj object as expected got [%v] want [%v]", out, c.cmp)
			}
		}
	}
}
