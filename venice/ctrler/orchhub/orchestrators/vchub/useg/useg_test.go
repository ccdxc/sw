package useg

import (
	"fmt"
	"testing"
	"time"

	tu "github.com/pensando/sw/venice/utils/testutils"
)

func TestPG(t *testing.T) {
	// Assign PGs till exhaustion
	// Free and reassign
	PGvlans := map[string][]int{}
	mgr, err := NewUsegAllocator()
	tu.AssertOk(t, err, "Failed to create useg mgr")
	// Set some before hand
	key := fmt.Sprintf("PG-%d", FirstUsegVlan-2)
	mgr.SetVlansForPG(key, FirstUsegVlan-2)
	PGvlans[key] = []int{FirstUsegVlan - 2, FirstUsegVlan - 1}
	key = fmt.Sprintf("PG-%d", FirstUsegVlan-4)
	// Testing passing in the secondary instead of primary
	mgr.SetVlansForPG(key, FirstUsegVlan-3)
	PGvlans[key] = []int{FirstUsegVlan - 4, FirstUsegVlan - 3}

	for index := 2; index < FirstUsegVlan-4; index += 2 {
		key := fmt.Sprintf("PG-%d", index)
		v1, v2, err := mgr.AssignVlansForPG(key)
		tu.AssertOk(t, err, "failed to assign vlans for %s", key)
		PGvlans[key] = []int{v1, v2}
	}
	// should be full, next assignment will fail
	_, _, err = mgr.AssignVlansForPG("PG-fail")
	tu.Assert(t, err != nil, "Assign PG should have failed")

	// Check the vlans
	for index := 2; index < FirstUsegVlan; index += 2 {
		key := fmt.Sprintf("PG-%d", index)
		v1, v2, err := mgr.GetVlansForPG(key)
		tu.AssertOk(t, err, "failed to get vlans for %s", key)
		tu.AssertEquals(t, PGvlans[key], []int{v1, v2}, "vlans did not match")
	}

	// Free them all
	for index := 2; index < FirstUsegVlan; index += 2 {
		key := fmt.Sprintf("PG-%d", index)
		err := mgr.ReleaseVlansForPG(key)
		tu.AssertOk(t, err, "failed to release vlans for %s", key)
	}

	// Should be able to reassign them all (no leaks)
	for index := 2; index < FirstUsegVlan; index += 2 {
		key := fmt.Sprintf("PG-%d", index)
		v1, v2, err := mgr.AssignVlansForPG(key)
		PGvlans[key] = []int{v1, v2}
		tu.AssertOk(t, err, "failed to assign vlans for %s", key)
	}
}

func TestVlans(t *testing.T) {
	// Free and reassign
	mgr, err := NewUsegAllocator()
	// Assign 1 host till exhaustion
	tu.AssertOk(t, err, "Failed to create useg mgr")

	doneCh := make(chan bool)

	assignAll := func(host string, usegVlans map[string]int) {
		for index := FirstUsegVlan; index < 4095; index++ {
			key := fmt.Sprintf("EP-%d", index)
			v1, err := mgr.AssignVlanForVnic(key, host)
			usegVlans[key] = v1
			tu.AssertOk(t, err, "failed to assign vlan for %s", key)
		}
		// should be full, next assignment will fail
		_, err := mgr.AssignVlanForVnic("EP-fail", host)
		tu.Assert(t, err != nil, "Assign EP should have failed")
	}

	setAll := func(host string, usegVlans map[string]int) {
		for index := FirstUsegVlan; index < 4095; index++ {
			key := fmt.Sprintf("EP-%d", index)
			err := mgr.SetVlanForVnic(key, host, index)
			usegVlans[key] = index
			tu.AssertOk(t, err, "failed to assign vlan for %s", key)
		}
		// should be full, next assignment will fail
		_, err := mgr.AssignVlanForVnic("EP-fail", host)
		tu.Assert(t, err != nil, "Assign EP should have failed")
	}

	checkAll := func(host string, usegVlans map[string]int) {
		for index := FirstUsegVlan; index < 4095; index++ {
			key := fmt.Sprintf("EP-%d", index)
			v1, err := mgr.GetVlanForVnic(key, host)
			tu.AssertOk(t, err, "failed to assign vlan for %s", key)
			tu.AssertEquals(t, usegVlans[key], v1, "vlans did not match")
		}
	}

	freeAll := func(host string, usegVlans map[string]int) {
		for index := FirstUsegVlan; index < 4095; index++ {
			key := fmt.Sprintf("EP-%d", index)
			err := mgr.ReleaseVlanForVnic(key, host)
			tu.AssertOk(t, err, "failed to assign vlan for %s", key)
		}
	}

	host1Useg := map[string]int{}
	host2Useg := map[string]int{}
	go func() {
		host := "host1"
		assignAll(host, host1Useg)
		doneCh <- true
	}()

	// Assign a second host till exhaustion
	go func() {
		host := "host2"
		setAll(host, host2Useg)
		doneCh <- true
	}()

	finishCount := 0
	for finishCount < 2 {
		select {
		case <-doneCh:
			finishCount++
		case <-time.After(5 * time.Second):
			t.Fatalf("go routines failed to complete. Only %d finished", finishCount)
		}
	}

	// Run get and clean for both hosts

	go func() {
		host := "host1"
		checkAll(host, host1Useg)
		freeAll(host, host1Useg)
		assignAll(host, host1Useg)
		doneCh <- true
	}()
	go func() {
		host := "host2"
		checkAll(host, host2Useg)
		freeAll(host, host2Useg)
		assignAll(host, host2Useg)
		doneCh <- true
	}()
	finishCount = 0
	for finishCount < 2 {
		select {
		case <-doneCh:
			finishCount++
		case <-time.After(5 * time.Second):
			t.Fatalf("go routines failed to complete. Only %d finished", finishCount)
		}
	}

}

func TestDebug(t *testing.T) {
	mgr, err := NewUsegAllocator()
	tu.AssertOk(t, err, "Failed to create useg mgr")

	PGvlans := map[string]int{}
	for index := 2; index < 6; index += 2 {
		key := fmt.Sprintf("PG-%d", index)
		v1, v2, err := mgr.AssignVlansForPG(key)
		tu.AssertOk(t, err, "failed to assign vlans for %s", key)
		k1, k2 := createPGKeys(key)
		PGvlans[k1] = v1
		PGvlans[k2] = v2
	}

	host1Useg := map[string]int{}
	for index := FirstUsegVlan; index < FirstUsegVlan+5; index++ {
		key := fmt.Sprintf("EP-%d", index)
		v1, err := mgr.AssignVlanForVnic(key, "host1")
		host1Useg[key] = v1
		tu.AssertOk(t, err, "failed to assign vlan for %s", key)
	}

	host2Useg := map[string]int{}
	for index := FirstUsegVlan; index < FirstUsegVlan+5; index++ {
		key := fmt.Sprintf("EP-%d", index)
		v1, err := mgr.AssignVlanForVnic(key, "host2")
		host2Useg[key] = v1
		tu.AssertOk(t, err, "failed to assign vlan for %s", key)
	}

	debugResp, err := mgr.Debug(nil)
	tu.AssertOk(t, err, "Failed to get debug info")
	debug := debugResp.(struct {
		PG    interface{}
		Hosts map[string]interface{}
	})
	tu.AssertEquals(t, PGvlans, debug.PG, "host2 debug info did not match")
	tu.AssertEquals(t, 2, len(debug.Hosts), "entries did not match")
	tu.AssertEquals(t, host1Useg, debug.Hosts["host1"], "host1 debug info did not match")
	tu.AssertEquals(t, host2Useg, debug.Hosts["host2"], "host2 debug info did not match")
}
