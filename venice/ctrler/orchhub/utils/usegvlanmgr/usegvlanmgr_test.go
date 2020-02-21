package usegvlanmgr

import (
	"fmt"
	"math/rand"
	"testing"
	"time"

	tu "github.com/pensando/sw/venice/utils/testutils"
)

type testCase struct {
	name          string
	startingVlans map[int]string
	input         interface{}
	output        interface{}
	expVlans      map[int]string
	expOwnerMap   map[string]int
	errMsg        string
}

func TestVlanManager(t *testing.T) {
	vMgr := NewVlanManager(500, 3596, false, 0)
	tu.AssertEquals(t, 3096, vMgr.GetFreeVlanCount(), "Remaining vlan count did not match")

	// Dummy release shouldn't affect count
	vMgr.ReleaseVlan(0)
	doneCh := make(chan bool)
	// Have 4 go threads assign and free vlans
	assignAndFree := func(start, count int) {
		ownerMap := make(map[string]int)
		for index := start; index < start+count; index++ {
			owner := fmt.Sprintf("owner-%d", index)
			vlan, err := vMgr.AssignVlan(owner)

			tu.AssertOk(t, err, "assigning next vlan failed for %s", owner)
			ownerMap[owner] = vlan
		}
		// Free up the owners
		for index := start; index < start+count; index++ {
			owner := fmt.Sprintf("owner-%d", index)
			if index%2 == 0 {
				vlan := ownerMap[owner]
				err := vMgr.ReleaseVlan(vlan)
				tu.AssertOk(t, err, "releasing vlan %d for owner %s failed", vlan, owner)
			} else {
				err := vMgr.ReleaseVlanOwner(owner)
				tu.AssertOk(t, err, "releasing owner %s failed", owner)
			}
		}
		doneCh <- true
	}

	go assignAndFree(0, 1000)
	go assignAndFree(1000, 1000)
	go assignAndFree(2000, 1000)

	finishCount := 0
	for finishCount < 3 {
		select {
		case <-doneCh:
			finishCount++
		case <-time.After(5 * time.Second):
			t.Fatalf("go routines failed to complete. Only %d finished", finishCount)
		}
	}

	tu.AssertEquals(t, 3096, vMgr.GetFreeVlanCount(), "Remaining vlan count did not match")

	// Assign till exhaustion
	// Specifically assign 1000-2000 first
	for index := 1000; index < 2000; index++ {
		owner := fmt.Sprintf("owner-%d", index)
		err := vMgr.SetVlanOwner(owner, index)
		tu.AssertOk(t, err, "assigning next vlan failed for %s", owner)
	}
	tu.AssertEquals(t, 2096, vMgr.GetFreeVlanCount(), "Remaining vlan count did not match")

	for index := 0; index < 2000; index++ {
		owner := fmt.Sprintf("owner1-%d", index)
		_, err := vMgr.AssignVlan(owner)
		tu.AssertOk(t, err, "assigning next vlan failed for %s", owner)
	}

	tu.AssertEquals(t, 96, vMgr.GetFreeVlanCount(), "Remaining vlan count did not match")

	for index := 0; index < 96; index++ {
		owner := fmt.Sprintf("owner2-%d", index)
		_, err := vMgr.AssignVlan(owner)
		tu.AssertOk(t, err, "assigning next vlan failed for %s", owner)
	}

	tu.AssertEquals(t, 0, vMgr.GetFreeVlanCount(), "Remaining vlan count did not match")

	_, err := vMgr.AssignVlan("overflow")
	tu.AssertEquals(t, "no vlans available", err.Error(), "Allocating more than 4096 vlans should have failed.")

	err = vMgr.ReleaseVlan(3095)
	tu.AssertOk(t, err, "releasing vlan should have passed")

	_, err = vMgr.AssignVlan("owner-4095")
	tu.AssertOk(t, err, "Failed to assign vlan")

}

func TestSetVlanOwner(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name:          "basic case 0 vlan",
			startingVlans: map[int]string{},
			input:         []interface{}{"default", 0},
			expVlans: map[int]string{
				0: "default",
			},
		},
		{
			name:          "basic case 4095 vlan",
			startingVlans: map[int]string{},
			input:         []interface{}{"default", 4095},
			expVlans: map[int]string{
				4095: "default",
			},
		},
		{
			name: "vlan no-op assignment",
			startingVlans: map[int]string{
				10: "default",
			},
			input: []interface{}{"default", 10},
			expVlans: map[int]string{
				10: "default",
			},
		},
		{
			name: "vlan already assigned",
			startingVlans: map[int]string{
				10: "vnic1",
			},
			input: []interface{}{"default", 10},
			expVlans: map[int]string{
				10: "vnic1",
			},
			errMsg: "vlan 10 (index 10) is already taken by owner vnic1",
		},
		{
			name: "owner already assigned",
			startingVlans: map[int]string{
				10: "default",
			},
			input: []interface{}{"default", 12},
			expVlans: map[int]string{
				10: "default",
			},
			errMsg: "owner default is already assigned to vlan 10",
		},
		{
			name:          "above vlan limit",
			startingVlans: map[int]string{},
			input:         []interface{}{"default", 4096},
			expVlans:      map[int]string{},
			errMsg:        "supplied vlan 4096 is an invalid vlan value",
		},
		{
			name:          "below vlan limit",
			startingVlans: map[int]string{},
			input:         []interface{}{"default", -1},
			expVlans:      map[int]string{},
			errMsg:        "supplied vlan -1 is an invalid vlan value",
		},
		{
			name:          "invalid owner",
			startingVlans: map[int]string{},
			input:         []interface{}{"", 10},
			expVlans:      map[int]string{},
			errMsg:        "owner cannot be empty",
		},
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		args := tc.input.([]interface{})

		err := vMgr.SetVlanOwner(args[0].(string), args[1].(int))
		return err
	}

	testHelper(t, testCases, testFunction, validateVlans, forceTestName)
}

func TestAssignVlan(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name:          "basic case empty array",
			startingVlans: map[int]string{},
			input:         "vnic1",
			expOwnerMap: map[string]int{
				"vnic1": -1,
			},
		},
		{
			name: "basic case non-empty array",
			startingVlans: map[int]string{
				0: "vnic0",
				2: "vnic2",
			},
			input:  "vnic1",
			output: 1,
			// Assignments will be random, so vlans here
			// should be ignored
			expOwnerMap: map[string]int{
				"vnic0": -1,
				"vnic1": -1,
				"vnic2": -1,
			},
		},
		// Exhausting vlans is tested in TestVlanManager
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		_, err := vMgr.AssignVlan(tc.input.(string))
		return err
	}

	testHelper(t, testCases, testFunction, validateAssignments, forceTestName)
}

func TestGetOwner(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name: "happy case",
			startingVlans: map[int]string{
				0: "vnic1",
			},
			input:  0,
			output: "vnic1",
			expVlans: map[int]string{
				0: "vnic1",
			},
		},
		{
			name: "invalid vlan",
			startingVlans: map[int]string{
				0: "vnic1",
			},
			input: -1,
			expVlans: map[int]string{
				0: "vnic1",
			},
			errMsg: "supplied vlan -1 is an invalid vlan value",
		},
		{
			name:          "owner of unassigned vlan",
			startingVlans: map[int]string{},
			input:         0,
			output:        "",
			expVlans:      map[int]string{},
		},
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		out, err := vMgr.GetOwner(tc.input.(int))

		if tc.output != nil {
			tu.AssertEquals(t, tc.output.(string), out, "Output did not match for test %s", tc.name)
		}
		return err
	}

	testHelper(t, testCases, testFunction, validateVlans, forceTestName)
}

func TestGetVlan(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name: "happy case",
			startingVlans: map[int]string{
				0: "vnic1",
			},
			input: "vnic1",
			expVlans: map[int]string{
				0: "vnic1",
			},
		},
		{
			name:          "unassigned vlan",
			startingVlans: map[int]string{},
			input:         "vnic1",
			expVlans:      map[int]string{},
			errMsg:        "owner vnic1 has no vlan assignment",
		},
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		out, err := vMgr.GetVlan(tc.input.(string))

		if tc.output != nil {
			tu.AssertEquals(t, tc.output.(int), out, "Output did not match for test %s", tc.name)
		}
		return err
	}

	testHelper(t, testCases, testFunction, validateVlans, forceTestName)
}

func TestReleaseVlanOwner(t *testing.T) {

	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name: "happy case",
			startingVlans: map[int]string{
				0: "vnic1",
			},
			input:       "vnic1",
			expVlans:    map[int]string{},
			expOwnerMap: map[string]int{},
		},
		{
			name: "unassigned owner",
			startingVlans: map[int]string{
				0: "vnic1",
			},
			input: "vnic2",
			expVlans: map[int]string{
				0: "vnic1",
			},
			expOwnerMap: map[string]int{
				"vnic1": 0,
			},
			errMsg: "owner vnic2 has no vlan assignment",
		},
		{
			name:          "invalid owner",
			startingVlans: map[int]string{},
			input:         "",
			expVlans:      map[int]string{},
			expOwnerMap:   map[string]int{},
			errMsg:        "owner  has no vlan assignment",
		},
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		err := vMgr.ReleaseVlanOwner(tc.input.(string))
		return err
	}

	testHelper(t, testCases, testFunction, validateVlans, forceTestName)
}

func TestReleaseVlan(t *testing.T) {
	// If supplied, will only run the test with the matching name
	forceTestName := ""
	testCases := []testCase{
		{
			name: "release vlan",
			startingVlans: map[int]string{
				10: "vnic1",
			},
			input:       10,
			expVlans:    map[int]string{},
			expOwnerMap: map[string]int{},
		},
		{
			name: "invalid vlan",
			startingVlans: map[int]string{
				10: "vnic1",
			},
			input: -1,
			expVlans: map[int]string{
				10: "vnic1",
			},
			expOwnerMap: map[string]int{
				"vnic1": 10,
			},
			errMsg: "supplied vlan -1 is an invalid vlan value",
		},
	}

	testFunction := func(vMgr *VlanMgr, tc testCase) error {
		err := vMgr.ReleaseVlan(tc.input.(int))
		return err
	}

	testHelper(t, testCases, testFunction, validateVlans, forceTestName)
}

func TestDebug(t *testing.T) {
	vMgr := NewVlanManager(0, 4000, false, 0)

	ownerMap := make(map[string]int)
	for index := 0; index < 10; index++ {
		owner := fmt.Sprintf("owner-%d", index)
		vlan, err := vMgr.AssignVlan(owner)

		tu.AssertOk(t, err, "assigning next vlan failed for %s", owner)
		ownerMap[owner] = vlan
	}

	debugMap, err := vMgr.Debug(nil)
	tu.AssertOk(t, err, "Failed to get debug info")
	tu.AssertEquals(t, ownerMap, debugMap, "debug info didn't match expected")
}

func validateVlans(t *testing.T, tc testCase, vMgr *VlanMgr) {
	// Validate vlan array
	for i, val := range vMgr.vlans {
		expVal, ok := tc.expVlans[i]
		if !ok && len(val) != 0 {
			t.Errorf("Unexpected entry for vlan %d for test %s", i, tc.name)
		}
		if ok {
			tu.Assert(t, expVal == val, "Expected %s but got %s for vlan %d for test %s", expVal, val, i, tc.name)
		}
	}
	if tc.expOwnerMap != nil {
		tu.AssertEquals(t, tc.expOwnerMap, vMgr.ownerMap, "vmgr ownerMap did not match expected")
	}
}

func validateAssignments(t *testing.T, tc testCase, vMgr *VlanMgr) {
	// Validate every owner maps to the correct vlan and vice versa
	// Also validate that the list of owners is correct
	expOwners := []string{}
	for k := range tc.expOwnerMap {
		expOwners = append(expOwners, k)
	}

	for i, owner := range vMgr.vlans {
		if len(owner) != 0 {
			vlan, ok := vMgr.ownerMap[owner]
			tu.Assert(t, ok, "Owner map didn't have owner %s", owner)
			tu.AssertEquals(t, vlan, i+vMgr.StartingVlan, "vlans were not equal")
			tu.AssertOneOf(t, owner, expOwners)
		}
	}
	for owner := range tc.expOwnerMap {
		_, ok := vMgr.ownerMap[owner]
		tu.Assert(t, ok, "Owner %s was not in owner map", owner)
	}
}

func testHelper(t *testing.T, testCases []testCase, testFunction func(*VlanMgr, testCase) error, validateFn func(*testing.T, testCase, *VlanMgr), forceTestName string) {
	for _, tc := range testCases {
		if len(forceTestName) != 0 && tc.name != forceTestName {
			continue
		}
		if tc.startingVlans == nil {
			tc.startingVlans = map[int]string{}
		}
		// Setup starting map data
		arr := make([]string, 4096)
		ownerMap := make(map[string]int)
		for i, val := range tc.startingVlans {
			arr[i] = val
			ownerMap[val] = i
		}
		var randSeed = time.Now().Unix()
		var r = rand.New(rand.NewSource(randSeed))
		vMgr := &VlanMgr{
			vlans:      arr,
			ownerMap:   ownerMap,
			EndingVlan: 4096,
			randSource: r,
		}
		vMgr.genItemList()

		err := testFunction(vMgr, tc)
		checkError(t, tc, err)

		validateFn(t, tc, vMgr)
	}

	tu.Assert(t, len(forceTestName) == 0, "focus test flag should not be checked in")
}

func checkError(t *testing.T, tc testCase, err error) {
	if err != nil {
		tu.AssertEquals(t, tc.errMsg, err.Error(), "Error msg did not match for test %s.", tc.name)
	} else if len(tc.errMsg) != 0 {
		t.Errorf("Expected errmsg but gone none, test %s", tc.name)
	}
}
