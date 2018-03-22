package apisrvpkg

import (
	"testing"

	"github.com/pensando/sw/venice/apiserver"
	mocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
)

// TestMethodKvWrite
// Test registering of methods to service and retrieval
func TestSvcAddMethod(t *testing.T) {
	svc := NewService("testSvc").(*ServiceHdlr)
	svc.AddMethod("Method1", mocks.NewFakeMethod(true))
	svc.AddMethod("Method2", mocks.NewFakeMethod(true))
	svc.AddMethod("AutoWatchMethod2", mocks.NewFakeMethod(true))

	if len(svc.Methods) != 3 {
		t.Errorf("Expecting [3] methods found [%v]", len(svc.Methods))
	}

	if svc.GetMethod("Method1") == nil {
		t.Errorf("Get method [Method1] failed")
	}
	if svc.GetMethod("Method2") == nil {
		t.Errorf("Get method [Method2] failed")
	}
	if svc.GetCrudService("Method2", apiserver.WatchOper) == nil {
		t.Errorf("Get method [Method2] failed")
	}

	m1 := svc.GetMethod("Method1").(*mocks.FakeMethod)
	svc.Disable()
	if m1.Enabled == true {
		t.Errorf("method should be Disabled")
	}
	svc.Enable()
	if m1.Enabled == false {
		t.Errorf("method should be Enabled")
	}
}
func TestGetCrudServiceName(t *testing.T) {
	// Test different options of GetCrudService
	for _, c := range []struct {
		oper   apiserver.APIOperType
		output string
	}{
		{oper: apiserver.CreateOper, output: "AutoAddTestMethod"},
		{oper: apiserver.UpdateOper, output: "AutoUpdateTestMethod"},
		{oper: apiserver.GetOper, output: "AutoGetTestMethod"},
		{oper: apiserver.DeleteOper, output: "AutoDeleteTestMethod"},
		{oper: apiserver.ListOper, output: "AutoListTestMethod"},
		{oper: apiserver.WatchOper, output: "AutoWatchTestMethod"},
		{oper: "junk", output: ""},
	} {
		o := apiserver.GetCrudServiceName("TestMethod", c.oper)
		if c.output != o {
			t.Errorf("Expected [%s] got [%s]", c.output, o)
		}
	}
}
