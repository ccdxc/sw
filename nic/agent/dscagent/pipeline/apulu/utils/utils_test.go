// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package utils

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/nic/agent/protos/netproto"
)

func TestRDToBytes(t *testing.T) {
	rd := netproto.RouteDistinguisher{
		Type:          "type0",
		AdminValue:    100,
		AssignedValue: 100,
	}
	r := RTToBytes(&rd)
	t.Logf("RD is [%v]", r)
	exp := []byte{0, 2, 0, 100, 0, 0, 0, 100}
	if !reflect.DeepEqual(r, exp) {
		t.Errorf("unexpected RD [%v][%v]", r, exp)
	}
	rd.Type = "Type0"
	r = RTToBytes(&rd)
	t.Logf("RD is [%v]", r)
	exp = []byte{0, 2, 0, 100, 0, 0, 0, 100}
	if !reflect.DeepEqual(r, exp) {
		t.Errorf("unexpected RD [%v][%v]", r, exp)
	}
	rd.Type = "Type2"
	r = RTToBytes(&rd)
	t.Logf("RD is [%v]", r)
	exp = []byte{2, 2, 0, 0, 0, 100, 0, 100}
	if !reflect.DeepEqual(r, exp) {
		t.Errorf("unexpected RD [%v][%v]", r, exp)
	}

	rd.Type = "Type1"
	r = RTToBytes(&rd)
	t.Logf("RD is [%v]", r)
	exp = []byte{1, 2, 0, 0, 0, 100, 0, 100}
	if !reflect.DeepEqual(r, exp) {
		t.Errorf("unexpected RD [%v][%v]", r, exp)
	}

	for _, c := range []struct {
		in  uint32
		out string
	}{
		{0x0A010101, "10.1.1.1"},
		{0, "0.0.0.0"},
		{1, "0.0.0.1"},
		{0x0A0A0101, "10.10.1.1"},
	} {
		if c.out != ConvertIntIPtoStr(c.in) {
			t.Errorf("expecting [%v] [ got [%v]", c.out, ConvertIntIPtoStr(c.in))
		}
	}
}
