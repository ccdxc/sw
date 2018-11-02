package runtime

import (
	"testing"
)

func TestObjectMetaGet(t *testing.T) {
	obj1 := &TestObjectWithoutMeta{}
	if _, err := GetObjectMeta(obj1); err != errNotAPIObject {
		t.Fatalf("GetObjectMeta failed for object without ObjectMeta")
	}

	obj2 := &TestObjectWithMeta{}
	obj2Meta, err := GetObjectMeta(obj2)
	if err != nil {
		t.Fatalf("GetObjectMeta failed for object with ObjectMeta")
	}

	obj2Meta.Name = "obj2"
	if obj2.ObjectMeta.Name != "obj2" {
		t.Fatalf("Unable to modify object: obj %+v obj-meta %+v", obj2, obj2Meta)
	}
}

func TestListMetaGet(t *testing.T) {
	obj1 := &TestObjectWithoutMeta{}
	if _, err := GetListMeta(obj1); err != errNotListObject {
		t.Fatalf("GetListMeta failed for object without ListMeta")
	}

	obj2 := &TestListObjectWithMeta{}
	obj2Meta, err := GetListMeta(obj2)
	if err != nil {
		t.Fatalf("GetListMeta failed for object with ListMeta")
	}

	obj2Meta.ResourceVersion = "obj2-ver1"
	if obj2.ListMeta.ResourceVersion != "obj2-ver1" {
		t.Fatalf("Unable to modify object: obj %+v obj-meta %+v", obj2, obj2Meta)
	}
}
