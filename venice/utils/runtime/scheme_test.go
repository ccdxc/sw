package runtime

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
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

func TestSchemaTypes(t *testing.T) {
	types := map[string]*api.Struct{
		"test.Type1": &api.Struct{
			Kind:     "TestKind1",
			APIGroup: "TestGroup1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": api.Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": &api.Struct{
			Kind:     "TestKind2",
			APIGroup: "TestGroup1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type3": &api.Struct{
			Kind:     "TestKind3",
			APIGroup: "TestGroup2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
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
	expKinds := map[string][]string{
		"TestGroup1": {"TestKind1", "TestKind2"},
		"TestGroup2": {"TestKind3"},
	}
	kinds := schema.Kinds()
	if len(kinds) != len(expKinds) {
		t.Fatalf("unexpected number of entries in Kinds() want: %d got: %d", len(expKinds), len(kinds))
	}
	for k, v := range kinds {
		v1, ok := expKinds[k]
		if !ok {
			t.Fatalf("key not expected %v", k)
		}
		if len(v1) != len(v) {
			t.Fatalf("unexpected number of entries for grou[%v] got: %v want:%v", k, len(v), len(v1))
		}
		for i := range v {
			for j := range v1 {
				if v[i] == v1[j] {
					v1[j] = "FOUND"
				}
			}
		}
		for i := range v1 {
			if v1[i] != "FOUND" {
				t.Fatalf("Not found [%v]", v1[i])
			}
		}
	}

	cases := map[string]string{
		"TestKind1": "test.Type1", "TestKind2": "test.Type2", "TestKind3": "test.Type3", "DummyUnknown": "",
	}
	for k, v := range cases {
		r := schema.Kind2SchemaType(k)
		if r != v {
			t.Errorf("returned Type did not match [%v] [%v]", r, v)
		}
	}

	cases = map[string]string{
		"TestKind1": "TestGroup1", "TestKind2": "TestGroup1", "TestKind3": "TestGroup2", "UnknownKind": "",
	}
	for k, v := range cases {
		r := schema.Kind2APIGroup(k)
		if r != v {
			t.Errorf("returned Group did not match [%v] [%v]", r, v)
		}
	}
	sCases := map[string]bool{
		"TestKind1": true, "TestKind2": true, "TestKind3": false,
	}
	for k, v := range sCases {
		s, err := schema.IsTenantScoped(k)
		if err != nil {
			t.Errorf("IsTenantScoped retured error (%s)", err)
		}
		if s != v {
			t.Errorf("IsTenantScoped mismatch expecting[%v] got: [%v]", v, s)
		}

		s, err = schema.IsClusterScoped(k)
		if err != nil {
			t.Errorf("IsTenantScoped retured error (%s)", err)
		}
		if s != !v {
			t.Errorf("IsTenantScoped mismatch expecting[%v] got: [%v]", !v, s)
		}
	}
}

func TestDupSchema(t *testing.T) {
	types := map[string]*api.Struct{
		"test.Type1": &api.Struct{
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": api.Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"test.Type2": &api.Struct{
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
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
