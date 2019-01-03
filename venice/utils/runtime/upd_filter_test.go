package runtime_test

import (
	"encoding/json"
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type filterUpdateTestcase struct {
	ref            interface{}
	upd            interface{}
	ignoreFields   []string
	forceFields    []string
	expectedResult bool
}

func TestUpdateFilterBaseCases(t *testing.T) {
	type emptyStruct struct{}

	type nestedStruct struct {
		IntF       int
		StrF       string
		IntPtrF    *int
		MapF       map[string]int
		StructPtrF *nestedStruct
	}

	type compType struct { // composite type
		IntF          int
		StrF          string
		IntPtrF       *int
		MapF          map[string]int
		NestedStructF nestedStruct
	}

	es1 := emptyStruct{}
	es2 := emptyStruct{}

	zero := int(0)
	one := int(1)
	two := int(2)

	tcs := []filterUpdateTestcase{
		// negative test-cases -- not valid, just make sure they are handled
		{"", 0, nil, nil, false},      // type mismatch
		{"1", 1, nil, nil, false},     // type mismatch
		{&es1, &one, nil, nil, false}, // type mismatch
		{nil, 0, nil, nil, false},     // nil ref object
		// scalar types
		{nil, nil, nil, nil, true},
		{2, 2, nil, nil, true},
		{1, 2, nil, nil, false},
		{0, 2, nil, nil, false},
		{"2", "2", nil, nil, true},
		{"1", "2", nil, nil, false},
		{"0", "2", nil, nil, false},
		{&one, &one, nil, nil, true},
		{&one, &two, nil, nil, false},
		{nil, &one, nil, nil, false},
		{map[string]int{}, map[string]int{}, nil, nil, true},
		{nil, map[string]int{"1": 1}, nil, nil, false},
		{map[string]int{"1": 2}, map[string]int{"1": 1}, nil, nil, false},
		// scalar type zero values -- no updates
		{1, 0, nil, nil, true},
		{"hello", "", nil, nil, true},
		{&es1, nil, nil, nil, true},
		{&es1, &es2, nil, nil, true},
		{&one, &zero, nil, nil, true},
		{map[string]int{}, nil, nil, nil, true},
		// struct fields
		{compType{IntF: 1}, compType{}, nil, nil, true},
		{compType{IntF: 1}, compType{IntF: 2}, nil, nil, false},
		{compType{IntPtrF: &one}, compType{IntPtrF: &zero}, nil, nil, true},
		{compType{IntPtrF: &zero}, compType{IntPtrF: &one}, nil, nil, false},
		{compType{IntPtrF: &one}, compType{IntPtrF: &two}, nil, nil, false},
		{compType{MapF: map[string]int{"1": 1}}, compType{}, nil, nil, true},
		{compType{MapF: map[string]int{"1": 1}}, compType{MapF: map[string]int{"1": 1}}, nil, nil, true},
		{compType{MapF: map[string]int{"1": 1}}, compType{MapF: map[string]int{}}, nil, nil, false},
		{compType{MapF: map[string]int{"1": 1}}, compType{MapF: map[string]int{"2": 2}}, nil, nil, false},
		// nested fields
		{compType{NestedStructF: nestedStruct{IntF: 1}}, compType{NestedStructF: nestedStruct{IntF: 0}}, nil, nil, true},
		{compType{NestedStructF: nestedStruct{IntF: 1}}, compType{}, nil, nil, true},
		{compType{NestedStructF: nestedStruct{IntF: 1}}, compType{NestedStructF: nestedStruct{IntF: 2}}, nil, nil, false},
		{compType{NestedStructF: nestedStruct{IntPtrF: &one}}, compType{NestedStructF: nestedStruct{IntPtrF: &zero}}, nil, nil, true},
		{compType{NestedStructF: nestedStruct{IntPtrF: &zero}}, compType{NestedStructF: nestedStruct{IntPtrF: &one}}, nil, nil, false},
		{compType{NestedStructF: nestedStruct{IntPtrF: &one}}, compType{NestedStructF: nestedStruct{IntPtrF: &two}}, nil, nil, false},
		{compType{NestedStructF: nestedStruct{StructPtrF: &nestedStruct{StrF: "1"}}}, compType{NestedStructF: nestedStruct{StructPtrF: &nestedStruct{StrF: ""}}}, nil, nil, true},
		{compType{NestedStructF: nestedStruct{StructPtrF: &nestedStruct{StrF: "1"}}}, compType{NestedStructF: nestedStruct{StructPtrF: &nestedStruct{StrF: "2"}}}, nil, nil, false},
		{compType{NestedStructF: nestedStruct{MapF: map[string]int{"1": 1}}}, compType{NestedStructF: nestedStruct{MapF: map[string]int{"1": 1}}}, nil, nil, true},
		{compType{NestedStructF: nestedStruct{MapF: map[string]int{"1": 1}}}, compType{NestedStructF: nestedStruct{MapF: map[string]int{"1": 2}}}, nil, nil, false},
		// ignored fields (filter update even if they differ)
		{compType{IntF: 1}, compType{IntF: 2}, []string{"IntF"}, nil, true},
		{compType{IntPtrF: &one}, compType{IntPtrF: &two}, []string{"IntPtrF"}, nil, true},
		{compType{StrF: "1", IntPtrF: &one}, compType{StrF: "2", IntPtrF: &two}, []string{"StrF", "IntPtrF"}, nil, true},
		{compType{IntF: 1, NestedStructF: nestedStruct{IntF: 3}}, compType{IntF: 2, NestedStructF: nestedStruct{IntF: 4}}, []string{"IntF"}, nil, true},
		{compType{MapF: map[string]int{"1": 1}}, compType{MapF: map[string]int{"2": 2}}, []string{"MapF"}, nil, true},
		// forced fields (compare field even if update value is zero value)
		{compType{IntF: 1}, compType{IntF: 0}, nil, []string{"IntF"}, false},
		{compType{IntPtrF: &one}, compType{IntPtrF: &zero}, nil, []string{"IntPtrF"}, false},
		{compType{StrF: "1", IntPtrF: &one}, compType{StrF: "0", IntPtrF: &zero}, nil, []string{"StrF", "IntPtrF"}, false},
		{compType{IntF: 1, NestedStructF: nestedStruct{IntF: 3}}, compType{}, nil, []string{"IntF"}, false},
		{compType{MapF: map[string]int{"1": 1}}, compType{}, nil, []string{"MapF"}, false},
		// forced fields should be ignored if present in the ignore list
		{compType{IntF: 1}, compType{IntF: 2}, []string{"IntF"}, []string{"IntF"}, true},
		{compType{IntF: 1}, compType{IntF: 0}, []string{"IntF"}, []string{"IntF"}, true},
	}

	for i, tc := range tcs {
		result := runtime.FilterUpdate(tc.ref, tc.upd, tc.ignoreFields, tc.forceFields)
		Assert(t, result == tc.expectedResult, fmt.Sprintf("Testcase %d: %+v, unexpected result. Have: %v, want: %v", i, tc, result, tc.expectedResult))
	}
}

func TestUpdateFilterWithObjects(t *testing.T) {

	nicJSON := []byte(`{"kind":"SmartNIC","api-version":"v1","meta":{"name":"44:44:44:44:00:02","generation-id":"10","resource-version":"114790","uuid":"ec38f54d-c4d5-4709-9860-26f6fe24dfb1","creation-time":"2018-12-19T23:56:41.743152915Z","mod-time":"2018-12-20T17:59:19.218143088Z","self-link":"/configs/cluster/v1/smartnics/44:44:44:44:00:02"},"spec":{"admit":true,"hostname":"naples1-host-new","mgmt-mode":"NETWORK"},"status":{"admission-phase":"ADMITTED","conditions":[{"type":"HEALTHY","status":"TRUE","last-transition-time":"2018-12-20T17:59:18Z"}],"serial-num":"0x0123456789ABCDEFghijk","primary-mac":"44:44:44:44:00:02"}}`)

	// create identical but independent instances of the NIC object to start from
	var nicObj, nicObjCopy, updateObj, unhealthyNICObj cluster.SmartNIC
	for _, obj := range []*cluster.SmartNIC{&nicObj, &nicObjCopy, &updateObj, &unhealthyNICObj} {
		err := json.Unmarshal(nicJSON, obj)
		AssertOk(t, err, "Error unmarshaling nicJSON")
	}

	// modify LastTransitionTime only
	updateObj.Status.Conditions[0].LastTransitionTime = "2018-12-20T17:59:19Z"

	// modify LastTransitionTime and health status
	unhealthyNICObj.Status.Conditions[0].Status = "unknown"
	unhealthyNICObj.Status.Conditions[0].LastTransitionTime = "2018-12-20T17:59:19Z"

	// this is an incremental update similar to what agent would send
	healthyUpdateDelta := cluster.SmartNIC{
		Status: cluster.SmartNICStatus{
			Conditions: []cluster.SmartNICCondition{
				{
					LastTransitionTime: "2018-12-20T17:59:19Z",
				},
			},
		},
	}

	// same as healthyUpdate, except for health status
	unhealthyUpdateDelta := cluster.SmartNIC{
		Status: cluster.SmartNICStatus{
			Conditions: []cluster.SmartNICCondition{
				{
					LastTransitionTime: "2018-12-20T17:59:19Z",
					Status:             "unknown",
				},
			},
		},
	}

	tcs := []filterUpdateTestcase{
		// full object update cases
		{nicObj, nicObjCopy, nil, nil, true},                                  // identical, no update
		{nicObj, updateObj, nil, nil, false},                                  // LastTransitionTime is different and not ignored
		{nicObj, updateObj, []string{"LastTransitionTime"}, nil, true},        // LastTransitionTime is different but ignored
		{nicObj, unhealthyNICObj, []string{"LastTransitionTime"}, nil, false}, // health status is different, need to update
		// delta update cases
		{nicObj, cluster.SmartNIC{}, nil, nil, true},                               // empty update
		{nicObj, healthyUpdateDelta, nil, nil, false},                              // LastTransitionTime is different and not ignored
		{nicObj, healthyUpdateDelta, []string{"LastTransitionTime"}, nil, true},    // LastTransitionTime is different but ignored
		{nicObj, unhealthyUpdateDelta, []string{"LastTransitionTime"}, nil, false}, // health status is different, need to update
	}

	for i, tc := range tcs {
		result := runtime.FilterUpdate(tc.ref, tc.upd, tc.ignoreFields, tc.forceFields)
		Assert(t, result == tc.expectedResult, fmt.Sprintf("Testcase %d: %+v, unexpected result. Have: %v, want: %v", i, tc, result, tc.expectedResult))
	}
}
