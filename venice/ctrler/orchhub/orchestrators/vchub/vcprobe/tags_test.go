package vcprobe

import (
	"testing"
	"time"

	"github.com/vmware/govmomi/vapi/tags"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTags(t *testing.T) {
	vcID := "user:pass@127.0.0.1:8990"
	s, err := sim.NewVcSim(sim.Config{Addr: vcID})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC("dc1")
	AssertOk(t, err, "Failed to create dc")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "Failed to create host")
	vm1, err := dc.AddVM("vm1", "host1")
	AssertOk(t, err, "Failed to create vm1")
	vm2, err := dc.AddVM("vm2", "host1")
	AssertOk(t, err, "Failed to create vm2")
	vm3, err := dc.AddVM("vm3", "host1")
	AssertOk(t, err, "Failed to create vm3")

	u := s.GetURL()

	storeCh := make(chan defs.Probe2StoreMsg, 24)
	probeCh := make(chan defs.Store2ProbeMsg, 24)

	logConfig := log.GetDefaultConfig("tags_test")
	logConfig.LogToStdout = true
	logger := log.SetConfig(logConfig)

	vcp := NewVCProbe(vcID, u, storeCh, probeCh, nil, logger)
	vcp.Start()
	defer vcp.Stop()

	// Give time for the probe to connect
	time.Sleep(time.Second)
	tp := vcp.tp

	err = tp.fetchTags()
	if err != nil {
		t.Error(err)
	}
	AssertOk(t, err, "fetchTags failed")

	// add some tags and list again
	tagZone1 := &tags.Tag{
		Name:        "tagZone1",
		Description: "desc",
		CategoryID:  "default",
	}
	tagZone2 := &tags.Tag{
		Name:        "tagZone2",
		Description: "desc",
		CategoryID:  "default",
	}
	tagZone3 := &tags.Tag{
		Name:        "tagZone3",
		Description: "desc",
		CategoryID:  "default",
	}
	defaultCat := &tags.Category{
		Name:            "default",
		Description:     "default",
		Cardinality:     "10",
		AssociableTypes: []string{"VirtualMachine"},
	}
	tp.tc.CreateCategory(tp.ctx, defaultCat)
	_, err = tp.tc.CreateTag(tp.ctx, tagZone1)
	AssertOk(t, err, "CreateTag failed")
	_, err = tp.tc.CreateTag(tp.ctx, tagZone2)
	AssertOk(t, err, "CreateTag failed")
	_, err = tp.tc.CreateTag(tp.ctx, tagZone3)
	AssertOk(t, err, "CreateTag failed")

	// Attach some VMs
	err = tp.tc.AttachTag(tp.ctx, "tagZone1", vm1.Reference())
	AssertOk(t, err, "AttachTag failed")
	err = tp.tc.AttachTag(tp.ctx, "tagZone1", vm2.Reference())
	AssertOk(t, err, "AttachTag failed")
	err = tp.tc.AttachTag(tp.ctx, "tagZone3", vm1.Reference())
	AssertOk(t, err, "AttachTag failed")

	err = tp.tc.AttachTag(tp.ctx, "tagZone1", vm3.Reference())
	AssertOk(t, err, "AttachTag failed")

	// change tagZone1 to tagZone11
	tagZone1, err = tp.tc.GetTag(tp.ctx, "tagZone1")
	AssertOk(t, err, "GetTagByName failed")
	tagZone1.Name = "tagZone11"
	err = tp.tc.UpdateTag(tp.ctx, tagZone1)
	AssertOk(t, err, "UpdateTag failed")

	err = tp.fetchTags()
	AssertOk(t, err, "fetchTags failed")

	idsvm1 := tp.vmInfo[vm1.Self.Value]
	Assert(t, idsvm1 != nil, "No tags found on vm-vm1")
	idsvm2 := tp.vmInfo[vm2.Self.Value]
	Assert(t, idsvm2 != nil, "No tags found on vm-vm2")
	namesvm1 := NewDeltaStrSet([]string{})
	for _, id := range idsvm1.Items() {
		namesvm1.AddSingle(tp.tagName[id])
	}

	Assert(t, namesvm1.Diff([]string{"tagZone11", "tagZone3"}).Count() == 0, "Tags on vm-vm1 do not match")
	Assert(t, len(idsvm2.Additions()) == 1, "Tags on vm-vm2 do not match")
	idsvm3 := tp.vmInfo[vm3.Self.Value]
	itemsvm3 := idsvm3.Items()
	Assert(t, len(idsvm3.Additions()) == 1, "Tags on vm-vm3 do not match")
	err = tp.tc.AttachTag(tp.ctx, "tagZone2", vm3.Reference())
	AssertOk(t, err, "AttachTag failed")
	err = tp.tc.DetachTag(tp.ctx, "tagZone11", vm3.Reference())
	AssertOk(t, err, "DetachTag failed")
	err = tp.fetchTags()
	AssertOk(t, err, "FetchTag failed")
	idsvm3 = tp.vmInfo[vm3.Self.Value]
	diffvm3 := idsvm3.Diff(itemsvm3)
	Assert(t, len(diffvm3.Additions()) == 1, "Tags on vm-vm3 do not match")
	Assert(t, len(diffvm3.Deletions()) == 1, "Tags on vm-vm3 do not match")
	AssertOk(t, err, "fetchTags failed")

	tagZone3, err = tp.tc.GetTag(tp.ctx, "tagZone3")
	AssertOk(t, err, "Failed to get tag tagZone3")
	err = tp.tc.DeleteTag(tp.ctx, tagZone3)
	AssertOk(t, err, "DeleteTag failed")
	idsvm1 = tp.vmInfo[vm2.Self.Value]
	err = tp.fetchTags()
	AssertOk(t, err, "DetachTag failed")
	Assert(t, len(idsvm3.Items()) == 1, "Tags on vm-vm1 do not match")
}
