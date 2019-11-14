package vcprobe

import (
	"fmt"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"github.com/vmware/govmomi/simulator"
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
	th := &testHelper{
		t:       t,
		tp:      vcp.tp,
		storeCh: storeCh,
	}

	th.fetchTags()
	expMap := map[string][]string{}
	th.verifyTags(expMap)

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
	th.tp.tc.CreateCategory(th.tp.ctx, defaultCat)
	_, err = th.tp.tc.CreateTag(th.tp.ctx, tagZone1)
	AssertOk(t, err, "CreateTag failed")
	_, err = th.tp.tc.CreateTag(th.tp.ctx, tagZone2)
	AssertOk(t, err, "CreateTag failed")
	_, err = th.tp.tc.CreateTag(th.tp.ctx, tagZone3)
	AssertOk(t, err, "CreateTag failed")

	// Attach some VMs
	th.attachTag("tagZone1", vm1)
	th.attachTag("tagZone1", vm2)
	th.attachTag("tagZone1", vm3)
	th.attachTag("tagZone3", vm1)

	expMap[vm1.Self.Value] = []string{"default:tagZone1", "default:tagZone3"}
	expMap[vm2.Self.Value] = []string{"default:tagZone1"}
	expMap[vm3.Self.Value] = []string{"default:tagZone1"}
	th.verifyTags(expMap)

	// Check tag renaming event
	th.renameTag("tagZone1", "tagZone11")

	expMap[vm1.Self.Value] = []string{"default:tagZone11", "default:tagZone3"}
	expMap[vm2.Self.Value] = []string{"default:tagZone11"}
	expMap[vm3.Self.Value] = []string{"default:tagZone11"}
	th.verifyTags(expMap)

	// Check tag removal basic
	th.detachTag("tagZone11", vm3)
	expMap = map[string][]string{
		vm3.Self.Value: []string{},
	}
	th.verifyTags(expMap)

	// Add tag to vm with no tag
	th.attachTag("tagZone2", vm3)
	expMap = map[string][]string{
		vm3.Self.Value: []string{"default:tagZone2"},
	}
	th.verifyTags(expMap)

	// Remove tag that only exists on one vm
	th.detachTag("tagZone3", vm1)

	expMap = map[string][]string{
		vm1.Self.Value: []string{"default:tagZone11"},
	}
	th.verifyTags(expMap)

	// Rename category should generate events
	th.renameCategory("default", "default1")
	expMap = map[string][]string{
		vm1.Self.Value: []string{"default1:tagZone11"},
		vm2.Self.Value: []string{"default1:tagZone11"},
		vm3.Self.Value: []string{"default1:tagZone2"},
	}
	th.verifyTags(expMap)

	// Delete tag that isn't used shouldn't generate new events
	th.deleteTag("tagZone3")
	expMap = map[string][]string{}
	th.verifyTags(expMap)

	// Delete tag that is being used
	th.deleteTag("tagZone11")
	expMap = map[string][]string{
		vm1.Self.Value: []string{},
		vm2.Self.Value: []string{},
	}
	th.verifyTags(expMap)

}

type testHelper struct {
	t       *testing.T
	tp      *tagsProbe
	storeCh chan defs.Probe2StoreMsg
}

func (h *testHelper) attachTag(tagID string, vm *simulator.VirtualMachine) {
	err := h.tp.tc.AttachTag(h.tp.ctx, tagID, vm.Reference())
	AssertOk(h.t, err, "AttachTag failed")
}

func (h *testHelper) detachTag(tagID string, vm *simulator.VirtualMachine) {
	err := h.tp.tc.DetachTag(h.tp.ctx, tagID, vm.Reference())
	AssertOk(h.t, err, "AttachTag failed")
}

func (h *testHelper) deleteTag(tagID string) {
	tag, err := h.tp.tc.GetTag(h.tp.ctx, tagID)
	AssertOk(h.t, err, "GetTagByName failed")
	err = h.tp.tc.DeleteTag(h.tp.ctx, tag)
	AssertOk(h.t, err, "deleteTag failed")
}

func (h *testHelper) renameTag(tagName string, newName string) {
	tag, err := h.tp.tc.GetTag(h.tp.ctx, tagName)
	AssertOk(h.t, err, "GetTagByName failed")
	tag.Name = newName
	err = h.tp.tc.UpdateTag(h.tp.ctx, tag)
	AssertOk(h.t, err, "UpdateTag failed")
}

func (h *testHelper) renameCategory(catName string, newName string) {
	cat, err := h.tp.tc.GetCategory(h.tp.ctx, catName)
	AssertOk(h.t, err, "GetTagByName failed")
	cat.Name = newName
	err = h.tp.tc.UpdateCategory(h.tp.ctx, cat)
	AssertOk(h.t, err, "UpdateTag failed")
}

func (h *testHelper) verifyTags(expMap map[string][]string) {
	h.fetchTags()
	items := h.getTagMsgsFromStore()

	AssertEquals(h.t, len(expMap), len(items), "%s : Expected msgs did not match, %+v", getCaller(), items)
	for _, item := range items {
		expTags, ok := expMap[item.Key]
		Assert(h.t, ok, "received event for unexpected key %s", item.Key)
		for _, change := range item.Changes {
			tagMsg := change.Val.(defs.TagMsg)
			for _, tag := range tagMsg.Tags {
				AssertOneOf(h.t, fmt.Sprintf(("%s:%s"), tag.Category, tag.Name), expTags)
			}
		}
	}
}

func (h *testHelper) fetchTags() {
	err := h.tp.fetchTags()
	AssertOk(h.t, err, "Fetch tags failed")
}

func (h *testHelper) getTagMsgsFromStore() []defs.Probe2StoreMsg {
	var items []defs.Probe2StoreMsg
	hasItems := true
	for hasItems {
		select {
		case item := <-h.storeCh:
			for _, change := range item.Changes {
				if change.Name == string(defs.VMPropTag) {
					items = append(items, item)
				}
			}
		default:
			hasItems = false
		}
	}
	return items
}

func getCaller() string {
	_, file, line, _ := runtime.Caller(2)
	return fmt.Sprintf("%s:%d", filepath.Base(file), line)
}
