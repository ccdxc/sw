package utils

import "testing"

func TestIsRunningOnVeniceAppl(t *testing.T) {
	if IsRunningOnVeniceAppl() != false {
		t.Fatalf("expecting Test to be running on non-appliance but got appliance")
	}
}