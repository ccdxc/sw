// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package utils

import (
	"testing"
)

func TestConvertMAC(t *testing.T) {
	mac := "00:15:CF:80:F8:13"
	dottedMAC := "0015.CF80.F813"
	if dottedMAC != ConvertMAC(mac) {
		t.Errorf("MAC conversion failed, Expected %s got %s", dottedMAC, ConvertMAC(mac))
	}
}
