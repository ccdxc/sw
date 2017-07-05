package runtime

import (
	"testing"
)

func TestObjectMetaGet(t *testing.T) {
	obj1 := &TestObjectWithoutMeta{}
	if _, err := GetObjectMeta(obj1); err == nil || err != errNotAPIObject {
		t.Fatalf("GetObjectMeta failed for object without ObjectMeta")
	}

	obj2 := &TestObjectWithMeta{}
	if _, err := GetObjectMeta(obj2); err != nil {
		t.Fatalf("GetObjectMeta failed for object with ObjectMeta")
	}
}

func TestListMetaGet(t *testing.T) {
	obj1 := &TestObjectWithoutMeta{}
	if _, err := GetListMeta(obj1); err == nil || err != errNotListObject {
		t.Fatalf("GetListMeta failed for object without ListMeta")
	}

	obj2 := &TestListObjectWithMeta{}
	if _, err := GetListMeta(obj2); err != nil {
		t.Fatalf("GetListMeta failed for object with ListMeta")
	}
}
