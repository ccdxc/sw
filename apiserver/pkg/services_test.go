package apisrvpkg

import "testing"

// TestMethodKvWrite
// Test registering of methods to service and retrieval
func TestSvcAddMethod(t *testing.T) {
	svc := NewService("testSvc").(*ServiceHdlr)
	svc.AddMethod("Method1", newFakeMethod(true))
	svc.AddMethod("Method2", newFakeMethod(true))

	if len(svc.Methods) != 2 {
		t.Errorf("Expecting [2] methods found [%v]", len(svc.Methods))
	}

	if svc.GetMethod("Method1") == nil {
		t.Errorf("Get method [Method1] failed")
	}
	if svc.GetMethod("Method2") == nil {
		t.Errorf("Get method [Method2] failed")
	}

	m1 := svc.GetMethod("Method1").(*fakeMethod)
	svc.Disable()
	if m1.enabled == true {
		t.Errorf("method should be Disabled")
	}
	svc.Enable()
	if m1.enabled == false {
		t.Errorf("method should be Enabled")
	}

}
