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
