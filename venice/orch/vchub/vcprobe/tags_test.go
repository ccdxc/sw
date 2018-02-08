package vcprobe

import (
	"context"
	"testing"

	"github.com/vmware/govmomi/vim25/soap"

	"github.com/pensando/sw/venice/orch/vchub/sim"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTags(t *testing.T) {
	sim.Setup()
	defer sim.TearDown()
	vc1, err := sim.Simulate("127.0.0.1:8990", 2, 2)
	if err != nil {
		t.Fatalf("Error %v simulating vCenter", err)
	}

	u, err := soap.ParseURL(vc1)
	if err != nil {
		t.Fatal(err)
	}

	tp := newTagsProbe(u, nil)
	AssertEventually(t, func() (bool, []interface{}) {
		err := tp.Start(context.Background())
		return (err == nil), nil
	}, "tagsProbe.Start", "5ms", "20s")

	err = tp.fetchTags()
	if err != nil {
		t.Error(err)
	}
	Assert(t, err == nil, "fatchTags filed")

	// add some tags and list again
	err = tp.tc.CreateTag("tagZone1")
	Assert(t, err == nil, "CreateTag failed")
	err = tp.fetchTags()
	Assert(t, err == nil, "fatchTags filed")
	err = tp.tc.CreateTag("tagZone2")
	Assert(t, err == nil, "CreateTag failed")
	err = tp.tc.CreateTag("tagZone3")
	Assert(t, err == nil, "CreateTag failed")
	err = tp.fetchTags()

	// Attach some VMs
	err = tp.tc.Attach("tagZone1", "vm-201")
	Assert(t, err == nil, "AttachTag failed")
	err = tp.tc.Attach("tagZone1", "vm-202")
	Assert(t, err == nil, "AttachTag failed")
	err = tp.tc.Attach("tagZone3", "vm-201")
	Assert(t, err == nil, "AttachTag failed")
	//delete session
	err = tp.tc.DeleteSession()
	Assert(t, err == nil, "DeleteSession failed")

	err = tp.fetchTags()
	Assert(t, err != nil, "Expected session error")
	err = tp.fetchTags()
	Assert(t, err == nil, "fetchTags failed")
	err = tp.tc.Attach("tagZone1", "vm-301")
	Assert(t, err == nil, "AttachTag failed")
	err = tp.fetchTags()
	Assert(t, err == nil, "fetchTags failed")

	// change tagZone1 to tagZone11
	id1, err := tp.tc.GetTagByName("tagZone1")
	Assert(t, err == nil, "GetTagByName failed")
	err = tp.tc.UpdateTag(id1, "tagZone11")
	Assert(t, err == nil, "UpdateTag failed")
	err = tp.fetchTags()
	Assert(t, err == nil, "fetchTags failed")

	ids201 := tp.vmInfo["vm-201"]
	Assert(t, ids201 != nil, "No tags found on vm-201")
	ids202 := tp.vmInfo["vm-202"]
	Assert(t, ids202 != nil, "No tags found on vm-202")
	names201 := NewDeltaStrSet([]string{})
	for _, id := range ids201.Items() {
		names201.AddSingle(tp.tagName[id])
	}

	Assert(t, names201.Diff([]string{"tagZone11", "tagZone3"}).Count() == 0, "Tags on vm-201 do not match")
	Assert(t, len(ids202.Additions()) == 1, "Tags on vm-202 do not match")
	ids301 := tp.vmInfo["vm-301"]
	items301 := ids301.Items()
	Assert(t, len(ids301.Additions()) == 1, "Tags on vm-301 do not match")
	err = tp.tc.Attach("tagZone2", "vm-301")
	Assert(t, err == nil, "AttachTag failed")
	err = tp.tc.Detach("tagZone11", "vm-301")
	Assert(t, err == nil, "DetachTag failed")
	err = tp.fetchTags()
	Assert(t, err == nil, "DetachTag failed")
	ids301 = tp.vmInfo["vm-301"]
	diff301 := ids301.Diff(items301)
	Assert(t, len(diff301.Additions()) == 1, "Tags on vm-301 do not match")
	Assert(t, len(diff301.Deletions()) == 1, "Tags on vm-301 do not match")
	Assert(t, err == nil, "fetchTags failed")

	err = tp.tc.DeleteTag("tagZone3")
	Assert(t, err == nil, "DeleteTag failed")
	ids201 = tp.vmInfo["vm-201"]
	err = tp.fetchTags()
	Assert(t, err == nil, "DetachTag failed")
	Assert(t, len(ids301.Items()) == 1, "Tags on vm-201 do not match")
}

func TestClient(t *testing.T) {
	sim.Setup()
	defer sim.TearDown()
	vc1, err := sim.Simulate("127.0.0.1:8990", 2, 2)
	if err != nil {
		t.Fatalf("Error %v simulating vCenter", err)
	}

	u, err := soap.ParseURL(vc1)
	if err != nil {
		t.Fatal(err)
	}

	tc := NewTagClient(u)

	AssertEventually(t, func() (bool, []interface{}) {
		err := tc.CreateSession(context.Background())
		return (err == nil), nil
	}, "tc.CreateSession", "5ms", "20s")

	// CreateTag
	err = tc.CreateTag("tagA")
	if err != nil {
		t.Fatal(err)
	}

	// GetTagByName
	id, err := tc.GetTagByName("tagA")
	if err != nil {
		t.Fatal(err)
	}

	if id == "" {
		t.Errorf("GetTagByName failed")
	}

	// UpdateTag
	err = tc.UpdateTag(id, "tagB")
	if err != nil {
		t.Fatal(err)
	}
	idB, err := tc.GetTagByName("tagB")
	if err != nil {
		t.Fatal(err)
	}

	if idB != id {
		t.Fatalf("Expected %s, got %s", id, idB)
	}
	err = tc.CreateTag("tagABC")
	if err != nil {
		t.Fatal(err)
	}

	tags, err := tc.ListTags()
	if err != nil {
		t.Fatal(err)
	}

	if len(tags) != 2 {
		t.Errorf("Expected two tags, got %d", len(tags))
	}

	err = tc.DeleteTag("tagB")
	if err != nil {
		t.Fatal(err)
	}

	// Attach
	err = tc.Attach("tagABC", "vm-101")
	if err != nil {
		t.Fatal(err)
	}

	idABC, err := tc.GetTagByName("tagABC")
	if err != nil {
		t.Fatal(err)
	}
	// ListVMs
	vms, err := tc.ListVMs(idABC)
	if err != nil {
		t.Fatal(err)
	}

	if len(vms) != 1 || vms[0] != "vm-101" {
		t.Fatalf("ListVMs failed %+v", vms)
	}
	err = tc.Attach("tagABC", "vm-102")
	if err != nil {
		t.Fatal(err)
	}

	// create another tag
	err = tc.CreateTag("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	err = tc.Attach("tagXYZ", "vm-103")
	if err != nil {
		t.Fatal(err)
	}

	idXYZ, err := tc.GetTagByName("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	// ListVMsMulti
	tags = []string{idABC, idXYZ}
	to, err := tc.ListVMsMulti(tags)
	if err != nil {
		t.Fatal(err)
	}

	if len(to) != 2 {
		t.Errorf("Expected 2 object sets, got %d",
			len(to))
	}

	// DeleteSession
	err = tc.DeleteSession()
	if err != nil {
		t.Fatal(err)
	}

	// Detach
	err = tc.Detach("tagABC", "vm-102")
	if err == nil {
		t.Fatal("Expected error, got none")
	}

	// Create session
	err = tc.CreateSession(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	// Detach
	err = tc.Detach("tagABC", "vm-102")
	if err != nil {
		t.Fatal(err)
	}
	err = tc.DeleteTag("tagXYZ")
	if err != nil {
		t.Fatal(err)
	}
	// ListVMsMulti
	tags = []string{idABC, idXYZ}
	to, err = tc.ListVMsMulti(tags)
	if err != nil {
		t.Fatal(err)
	}

	found := false
	for _, obj := range to {
		if len(obj.ObjIDs) != 0 {
			if obj.ObjIDs[0].ID == "vm-101" {
				found = true
			}
			break
		}
	}

	if !found {
		t.Errorf("Error listing %+v", to)
	}
}
