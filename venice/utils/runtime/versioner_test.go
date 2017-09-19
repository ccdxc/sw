package runtime

import (
	"testing"

	"github.com/pensando/sw/api"
)

type TestObjectWithoutMeta struct {
	api.TypeMeta
}

type TestObjectWithMeta struct {
	api.TypeMeta
	api.ObjectMeta
}

type TestListObjectWithMeta struct {
	api.TypeMeta
	api.ListMeta
}

func TestNonAPIObject(t *testing.T) {
	obj := &TestObjectWithoutMeta{}
	v := NewObjectVersioner()
	if err := v.SetVersion(obj, 10); err == nil || err != errNotAPIObject {
		t.Fatalf("SetVersion didn't fail on a non API object, error: %v", err)
	}
	if _, err := v.GetVersion(obj); err == nil || err != errNotAPIObject {
		t.Fatalf("GetVersion didn't fail on a non API object, error: %v", err)
	}
}

func TestAPIObj(t *testing.T) {
	obj := &TestObjectWithMeta{}
	v := NewObjectVersioner()
	if err := v.SetVersion(obj, 10); err != nil {
		t.Fatalf("SetVersion failed on API object, error: %v", err)
	}
	if ver, err := v.GetVersion(obj); err != nil || ver != 10 {
		t.Fatalf("GetVersion failed on API object, error: %v, version %d", err, ver)
	}
}

func TestNonListObject(t *testing.T) {
	obj := &TestObjectWithoutMeta{}
	v := NewListVersioner()
	if err := v.SetVersion(obj, 10); err == nil || err != errNotListObject {
		t.Fatalf("SetVersion didn't fail on a non List object, error: %v", err)
	}
	if _, err := v.GetVersion(obj); err == nil || err != errNotListObject {
		t.Fatalf("GetVersion didn't fail on a non List object, error: %v", err)
	}
}

func TestListObj(t *testing.T) {
	obj := &TestListObjectWithMeta{}
	v := NewListVersioner()
	if err := v.SetVersion(obj, 10); err != nil {
		t.Fatalf("SetVersion failed on List object, error: %v", err)
	}
	if ver, err := v.GetVersion(obj); err != nil || ver != 10 {
		t.Fatalf("GetVersion failed on List object, error: %v, version %d", err, ver)
	}
}
