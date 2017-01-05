package etcd

import (
	"testing"
)

type Foo struct {
}

type ValidList1 struct {
	Items []Foo
}

type ValidList2 struct {
	Items []*Foo
}

type ValidList3 struct {
	Items *[]Foo
}

type EmptyList struct {
}

func TestValidObj(t *testing.T) {
	if err := validObjForDecode(Foo{}); err == nil {
		t.Fatalf("Non pointer object is deemed valid")
	}
	if err := validObjForDecode(&Foo{}); err != nil {
		t.Fatalf("Pointer object is deemed invalid")
	}
	var f1 *Foo
	if err := validObjForDecode(f1); err == nil {
		t.Fatalf("Nil pointer is deemed valid")
	}
	t.Logf("Valid object tests passed")
}

func TestValidListObj(t *testing.T) {
	if _, err := validListObjForDecode(&EmptyList{}); err == nil {
		t.Fatalf("Object without Items is deemed valid")
	}
	if _, err := validListObjForDecode(ValidList1{}); err == nil {
		t.Fatalf("Non pointer list object is deemed valid")
	}
	if _, err := validListObjForDecode(&ValidList1{}); err != nil {
		t.Fatalf("Valid list object is deemed invalid")
	}
	if _, err := validListObjForDecode(&ValidList2{}); err != nil {
		t.Fatalf("Valid list object is deemed invalid")
	}
	if _, err := validListObjForDecode(&ValidList3{}); err == nil {
		t.Fatalf("Nil object is deemed valid")
	}
	var l3 ValidList3
	l3.Items = &[]Foo{}
	if _, err := validListObjForDecode(&l3); err != nil {
		t.Fatalf("Valid object deemed invalid, error %v", err)
	}
	t.Logf("Valid object list tests passed")
}
