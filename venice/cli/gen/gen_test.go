package gen

import (
	"testing"
)

func TestAddFlags(t *testing.T) {
	cf := GetInfo()

	flags1 := []CliFlag{{ID: "field1", Type: "String"}, {ID: "field2", Type: "StringSlice"}}
	flags2 := []CliFlag{{ID: "field1", Type: "Bool"}, {ID: "field2", Type: "String"}}
	flags3 := []CliFlag{{ID: "field1", Type: "Int"}, {ID: "field2", Type: "Uint"}}

	// populate newpkg.One
	cf.AddCliInfo("newpkg.One", "create", flags1)
	cf.AddRemoveObjOperFunc("newpkg.One", dummyRemoveObjOperFunc)
	cf.AddRestPostFunc("newpkg.One", "v1", dummyRestPostFunc)
	cf.AddRestDeleteFunc("newpkg.One", "v1", dummyRestDeleteFunc)
	cf.AddRestPutFunc("newpkg.One", "v1", dummyRestPutFunc)
	cf.AddRestGetFunc("newpkg.One", "v1", dummyRestGetFunc)

	cf.AddCliInfo("newpkg.Two", "create", flags2)
	cf.AddRemoveObjOperFunc("newpkg.Two", dummyRemoveObjOperFunc)
	cf.AddRestPostFunc("newpkg.Two", "v1", dummyRestPostFunc)
	cf.AddRestDeleteFunc("newpkg.Two", "v1", dummyRestDeleteFunc)
	cf.AddRestPutFunc("newpkg.Two", "v1", dummyRestPutFunc)
	cf.AddRestGetFunc("newpkg.Two", "v1", dummyRestGetFunc)

	cf.AddCliInfo("newpkg.Three", "create", flags3)
	cf.AddRemoveObjOperFunc("newpkg.Three", dummyRemoveObjOperFunc)
	cf.AddRestPostFunc("newpkg.Three", "v1", dummyRestPostFunc)
	cf.AddRestDeleteFunc("newpkg.Three", "v1", dummyRestDeleteFunc)
	cf.AddRestPutFunc("newpkg.Three", "v1", dummyRestPutFunc)
	cf.AddRestGetFunc("newpkg.Three", "v1", dummyRestGetFunc)

	newFlags1, err := cf.FindFlags("two", "create")
	if err != nil {
		t.Fatalf("error finding create flags for label")
	}
	if len(newFlags1) != 2 {
		t.Fatalf("invalid newFlags: %+v", newFlags1)
	}
	if newFlags1[0].ID != "field1" || newFlags1[0].Type != "Bool" {
		t.Fatalf("invalid newFlags: %+v", newFlags1)
	}
	if _, err := cf.FindFlags("two", "delete"); err == nil {
		t.Fatalf("found flags for delete oper")
	}

	keys := cf.GetAllKeys()
	if len(keys) != 3 {
		t.Fatalf("invalid keys: %+v", keys)
	}

	objName, objListName, err := cf.FindSvcName("three")
	if err != nil || objName != "newpkg.Three" || objListName != "newpkg.ThreeList" {
		t.Fatalf("err = %s, objName = %s, objListName = %s", err, objName, objListName)
	}

	stringVar := ""
	strPtr := &stringVar
	getRemoveObjFunc, err := cf.FindRemoveObjOperFunc("two")
	if err != nil {
		t.Fatalf("error %s finding function cf.FindGetObjFromListFunc", err)
	}
	if getRemoveObjFunc(strPtr) != nil && *strPtr != "dummyRemoveObjFunc" {
		t.Fatalf("getRemoveObjFunc returned invalid value: %s", *strPtr)
	}

	restGetFunc, err := cf.FindRestGetFunc("two", "v1")
	if err != nil {
		t.Fatalf("error %s finding function cf.FindRestGetFunc", err)
	}
	if restGetFunc("", "", "", strPtr) != nil && *strPtr != "dummyRestGetFunc" {
		t.Fatalf("restGetFunc returned invalid value: %s", *strPtr)
	}

	restDeleteFunc, err := cf.FindRestDeleteFunc("three", "v1")
	if err != nil {
		t.Fatalf("error %s finding function cf.FindRestDeleteFunc", err)
	}
	if restDeleteFunc("", "", strPtr) != nil && *strPtr != "dummyRestDeleteFunc" {
		t.Fatalf("restDeleteFunc returned invalid value: %s", *strPtr)
	}

	restPostFunc, err := cf.FindRestPostFunc("two", "v1")
	if err != nil {
		t.Fatalf("error %s finding function cf.FindRestPostFunc", err)
	}
	if restPostFunc("", "", strPtr) != nil && *strPtr != "dummyRestPostFunc" {
		t.Fatalf("restPostFunc returned invalid value: %s", *strPtr)
	}

	restPutFunc, err := cf.FindRestPutFunc("two", "v1")
	if err != nil {
		t.Fatalf("error %s finding function cf.FindRestPutFunc", err)
	}
	if restPutFunc("", "", strPtr) != nil && *strPtr != "dummyRestPutFunc" {
		t.Fatalf("restPutFunc returned invalid value: %s", *strPtr)
	}

	if _, _, err := cf.FindSvcName("invalid"); err == nil {
		t.Fatalf("nil error on invalid service")
	}

	if _, err := cf.FindRemoveObjOperFunc("invalid"); err == nil {
		t.Fatalf("nil error on invalid service")
	}

	if _, err := cf.FindRestGetFunc("two", "invalid"); err == nil {
		t.Fatalf("nil error on invalid version")
	}

	if _, err := cf.FindRestGetFunc("invalid", "v1"); err == nil {
		t.Fatalf("nil error on invalid service")
	}

	if _, err := cf.FindRestDeleteFunc("invalid", "v1"); err == nil {
		t.Fatalf("nil error on invalid service")
	}

	if _, err := cf.FindRestPostFunc("invalid", "v1"); err == nil {
		t.Fatalf("nil error on invalid service")
	}

	if _, err := cf.FindRestPutFunc("invalid", "v1"); err == nil {
		t.Fatalf("nil error on invalid service")
	}
}

func dummyRemoveObjOperFunc(obj interface{}) error {
	strPtr := obj.(*string)
	*strPtr = "dummyRemoveObjOperFunc"
	return nil
}

func dummyRestGetFunc(hostname, tenant, token string, obj interface{}) error {
	strPtr := obj.(*string)
	*strPtr = "dummyRestGetFunc"
	return nil
}
func dummyRestDeleteFunc(hostname, token string, obj interface{}) error {
	strPtr := obj.(*string)
	*strPtr = "dummyRestDeleteFunc"
	return nil
}
func dummyRestPostFunc(hostname, token string, obj interface{}) error {
	strPtr := obj.(*string)
	*strPtr = "dummyRestPostFunc"
	return nil
}
func dummyRestPutFunc(hostname, token string, obj interface{}) error {
	strPtr := obj.(*string)
	*strPtr = "dummyRestPutFunc"
	return nil
}
