package ref

import (
	"reflect"
	"testing"
)

type TestCopy struct {
	TypeMeta
	ObjectMeta
	Spec TestCopySpec
}

type TestCopySpec struct {
	String         string
	SliceString    []string
	MapString      map[string]string
	SliceStruct    []SGRule
	Int            int
	PtrStruct      *Permission
	MapStruct      map[string]Policy
	MapStructPtr   map[string]*Policy
	Uint64         uint64
	Uint32         uint32
	Int32          int32
	PtrString      *string
	SlicePtrString []*string
	SlicePtrStruct []*IPOpt
	Bool           bool
}

func TestDeepCopy(t *testing.T) {
	sip := "sip"
	mac1 := "0x010203040506"
	mac2 := "0x010203040507"
	tc1 := TestCopy{
		TypeMeta: TypeMeta{
			Kind:       "user",
			APIVersion: "v1",
		},
		ObjectMeta: ObjectMeta{
			Tenant:          "default",
			Name:            "u1",
			Namespace:       "default",
			ResourceVersion: "100",
			UUID:            "0000-0000-0000-0000",
			Labels: map[string]string{
				"label1": "value1",
				"label2": "value2",
			},
		},
		Spec: TestCopySpec{
			String:      "tc2",
			SliceString: []string{"admin", "operator"},
			MapString: map[string]string{
				"label1": "value1",
				"label2": "value2",
			},
			SliceStruct: []SGRule{{Ports: "8000", Action: "allow", PeerGroup: "bar"}, {Ports: "80", Action: "allow", PeerGroup: "bar"}},
			Int:         100,
			PtrStruct: &Permission{
				ToObj: "network1",
				RWX:   "rwx",
			},
			MapStruct: map[string]Policy{
				"in": {
					ToGroup:   "foo",
					FromGroup: "bar",
				},
				"out": {
					ToGroup:   "bar",
					FromGroup: "foo",
				},
			},
			MapStructPtr: map[string]*Policy{
				"in": {
					ToGroup:   "foo",
					FromGroup: "bar",
				},
				"out": {
					ToGroup:   "bar",
					FromGroup: "foo",
				},
			},
			Uint64:         123123123123,
			Uint32:         65536,
			Int32:          65536,
			PtrString:      &sip,
			SlicePtrString: []*string{&mac1, &mac2},
			SlicePtrStruct: []*IPOpt{{Version: "v4", Protocol: "udp"}, {Version: "v4", Protocol: "tcp"}},
			Bool:           true,
		},
	}
	tc2 := DeepCopy(tc1)
	t.Logf("Original version: %+v", tc1)
	t.Logf("Copied version: %+v", tc2)
	if !reflect.DeepEqual(tc1, tc2) {
		t.Fatalf("DeepEqual didn't match DeepCopy")
	}
}
