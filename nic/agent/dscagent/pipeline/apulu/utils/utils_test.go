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

}
