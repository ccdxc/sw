// +build linux

package sysif

import (
	"reflect"
	"testing"
)

func TestGetInfo(t *testing.T) {
	info, err := GetNodeInfo()
	if err != nil {
		t.Fatalf("GetNodeInfo call failed, err %v", err)
	}
	if info.MachineID == "" {
		t.Fatalf("GetNodeInfo - Got Null MachineID")
	}
	if info.NumCPU <= 0 {
		t.Fatalf("GetNodeInfo - Invalid number of CPU")
	}
	if info.NumCore <= 0 {
		t.Fatalf("GetNodeInfo - Invalid number of CPU cores")
	}
	if info.NumPhysicalCPU <= 0 {
		t.Fatalf("GetNodeInfo - Invalid number of Physical CPU ")
	}
	if info.CPUModelName == "" {
		t.Fatalf("GetNodeInfo - Invalid CPU ModelName")
	}
	if info.CPUMHz <= 0 {
		t.Fatal("GetNodeInfo - Invalid CPU MHz", info.CPUMHz)
	}
	t.Logf("%+v\n", info)
}

func TestMarshall(t *testing.T) {
	info, err := GetNodeInfo()
	if err != nil {
		t.Fatalf("GetNodeInfo call failed, err %v", err)
	}
	m := MarshallToStringMap(info)
	info2 := UnmarshallFromStringMap(m)
	if !reflect.DeepEqual(*info, info2) {
		t.Fatalf("marshall/unmarshall changed data. before:%v after:%v", info, info2)
	}
}
