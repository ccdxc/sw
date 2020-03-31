package vcprobe

import (
	"context"
	"fmt"
	"net/url"
	"path/filepath"
	"runtime"
	"sync"
	"testing"
	"time"

	"github.com/vmware/govmomi/simulator"
	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
)

func TestTags(t *testing.T) {
	config := log.GetDefaultConfig("vcprobe_test_tags")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	vcID := "127.0.0.1:8990"
	user := "user"
	password := "pass"

	u := &url.URL{
		Scheme: "https",
		Host:   vcID,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, password)

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC("dc1")
	AssertOk(t, err, "Failed to create dc")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "Failed to create host")
	vm1, err := dc.AddVM("vm1", "host1", []sim.VNIC{})
	AssertOk(t, err, "Failed to create vm1")
	vm2, err := dc.AddVM("vm2", "host1", []sim.VNIC{})
	AssertOk(t, err, "Failed to create vm2")
	vm3, err := dc.AddVM("vm3", "host1", []sim.VNIC{})
	AssertOk(t, err, "Failed to create vm3")

	storeCh := make(chan defs.Probe2StoreMsg, 100)

	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. ERR : %v", err)
	ctx, cancel := context.WithCancel(context.Background())

	orchConfig := smmock.GetOrchestratorConfig(vcID, user, password)
	orchConfig.Status.OrchID = 1
	err = sm.Controller().Orchestrator().Create(orchConfig)
	orchID := fmt.Sprintf("orch%d", orchConfig.Status.OrchID)

	state := &defs.State{
		VcURL:      u,
		VcID:       vcID,
		OrchID:     orchID,
		Ctx:        ctx,
		Log:        logger,
		StateMgr:   sm,
		OrchConfig: orchConfig,
		Wg:         &sync.WaitGroup{},
	}
	vcp := NewVCProbe(storeCh, nil, state)
	vcp.Start(false)

	defer func() {
		cancel()
		vcp.WatcherWg.Wait()
		state.Wg.Wait()
	}()

	// Start test
	vcp.StartWatchers()

	time.Sleep(3 * time.Second)

	th := &testHelper{
		t:       t,
		tp:      vcp.TagsProbeInf.(*tagsProbe),
		storeCh: storeCh,
	}

	labels := map[string]string{}
	labels["io.pensando.namespace"] = ""
	labels["io.pensando.orch-name"] = orchConfig.Name
	// Write VMs into stateManager
	vm1Name := utils.CreateGlobalKey(orchID, "", vm1.Self.Value)
	vm2Name := utils.CreateGlobalKey(orchID, "", vm2.Self.Value)
	vm3Name := utils.CreateGlobalKey(orchID, "", vm3.Self.Value)
	th.createWorkload(sm, vm1Name, labels)
	th.createWorkload(sm, vm2Name, labels)
	th.createWorkload(sm, vm3Name, labels)

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
	tc := th.tp.GetTagClientWithRLock()
	tc.CreateCategory(th.tp.Ctx, defaultCat)
	_, err = tc.CreateTag(th.tp.Ctx, tagZone1)
	AssertOk(t, err, "CreateTag failed")
	_, err = tc.CreateTag(th.tp.Ctx, tagZone2)
	AssertOk(t, err, "CreateTag failed")
	_, err = tc.CreateTag(th.tp.Ctx, tagZone3)
	AssertOk(t, err, "CreateTag failed")
	th.tp.ReleaseClientsRLock()

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
	th.tp.fetchTagInfo()

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
	th.tp.fetchTagInfo()
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

func TestTagWriting(t *testing.T) {
	config := log.GetDefaultConfig("vcprobe_test_tags")
	config.LogToStdout = true
	config.Filter = log.AllowAllFilter
	logger := log.SetConfig(config)

	vcID := "127.0.0.1:8990"
	user := "user"
	password := "pass"

	u := &url.URL{
		Scheme: "https",
		Host:   vcID,
		Path:   "/sdk",
	}
	u.User = url.UserPassword(user, password)

	tsdb.Init(context.Background(), &tsdb.Opts{})
	defer tsdb.Cleanup()

	s, err := sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	defer s.Destroy()
	dc, err := s.AddDC("dc1")
	AssertOk(t, err, "Failed to create dc")
	_, err = dc.AddHost("host1")
	AssertOk(t, err, "Failed to create host")
	vm1, err := dc.AddVM("vm1", "host1", []sim.VNIC{})
	AssertOk(t, err, "Failed to create vm1")
	vm2, err := dc.AddVM("vm2", "host1", []sim.VNIC{})
	AssertOk(t, err, "Failed to create vm1")

	storeCh := make(chan defs.Probe2StoreMsg, 100)

	sm, _, err := smmock.NewMockStateManager()
	AssertOk(t, err, "Failed to create state manager. ERR : %v", err)
	ctx, cancel := context.WithCancel(context.Background())

	orchConfig := smmock.GetOrchestratorConfig(vcID, user, password)
	orchConfig.Status.OrchID = 1
	err = sm.Controller().Orchestrator().Create(orchConfig)
	orchID := fmt.Sprintf("orch%d", orchConfig.Status.OrchID)

	labels := map[string]string{}
	labels["io.pensando.namespace"] = ""
	labels["io.pensando.orch-name"] = orchConfig.Name
	// Write VMs into stateManager
	vm1Name := utils.CreateGlobalKey(orchID, "", vm1.Self.Value)
	vm2Name := utils.CreateGlobalKey(orchID, "", vm2.Self.Value)

	state := &defs.State{
		VcURL:      u,
		VcID:       vcID,
		OrchID:     orchID,
		ClusterID:  "testCluster",
		Ctx:        ctx,
		Log:        logger,
		StateMgr:   sm,
		OrchConfig: orchConfig,
		Wg:         &sync.WaitGroup{},
	}
	vcp := NewVCProbe(storeCh, nil, state)
	vcp.Start(false)

	defer func() {
		cancel()
		vcp.WatcherWg.Wait()
		state.Wg.Wait()
	}()

	// Start test
	// vcp.StartWatchers()

	time.Sleep(3 * time.Second)

	th := &testHelper{
		t:       t,
		tp:      vcp.TagsProbeInf.(*tagsProbe),
		storeCh: storeCh,
	}
	th.createWorkload(sm, vm1Name, labels)
	th.createWorkload(sm, vm2Name, labels)

	vcp.SetupBaseTags()
	th.fetchTags()
	expMap := map[string][]string{}
	th.verifyTags(expMap)

	// Verify Venice default tags have been created
	tc := th.tp.GetTagClientWithRLock()
	cat, err := tc.GetCategory(th.tp.Ctx, defs.VCTagCategory)
	AssertOk(t, err, "Failed to get category")
	AssertEquals(t, defs.VCTagCategoryDescription, cat.Description, "Description for category did not match")

	tagObjs, err := tc.GetTagsForCategory(th.tp.Ctx, cat.ID)
	AssertOk(t, err, "Failed to get tags for category")
	AssertEquals(t, 1, len(tagObjs), "Category should have had two tags in it, had %v", tagObjs)
	th.tp.ReleaseClientsRLock()

	tagsMap := map[string]tags.Tag{}
	for _, t := range tagObjs {
		tagsMap[t.Name] = t
	}

	tag, ok := tagsMap[defs.CreateVCTagManagedTag(state.ClusterID)]
	Assert(t, ok, "tagsObj did not have tag VCTagManaged")
	AssertEquals(t, defs.VCTagManagedDescription, tag.Description, "tag description did not match")

	// Tag objects
	err = th.tp.TagObjAsManaged(vm1.Reference())
	AssertOk(t, err, "failed to tag vm1 as managed")

	err = th.tp.TagObjWithVlan(vm1.Reference(), 1000)
	AssertOk(t, err, "failed to tag vm1 with vlan tag")

	vlanTag := fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 1000)

	expMap = map[string][]string{
		vm1.Self.Value: []string{fmt.Sprintf("%s:%s", defs.VCTagCategory, defs.CreateVCTagManagedTag(state.ClusterID)),
			fmt.Sprintf("%s:%s%d", defs.VCTagCategory, defs.VCTagVlanPrefix, 1000)},
	}
	th.verifyTags(expMap)

	err = th.tp.TagObjsAsManaged([]types.ManagedObjectReference{vm1.Reference(), vm2.Reference()})
	AssertOk(t, err, "failed to tag vm1 & vm2 as managed")

	expMap = map[string][]string{
		vm2.Self.Value: []string{fmt.Sprintf("%s:%s", defs.VCTagCategory, defs.CreateVCTagManagedTag(state.ClusterID))},
	}
	th.verifyTags(expMap)

	tagRes, err := th.tp.GetPensandoTagsOnObjects([]types.ManagedObjectReference{vm1.Reference(), vm2.Reference()})
	AssertOk(t, err, "failed to get pensando managed tags")
	kingTagEntry := tagRes[string(defs.VirtualMachine)]
	AssertEquals(t, 2, len(kingTagEntry[vm1.Self.Value]), "Should have found 2 tags")
	AssertEquals(t, 1, len(kingTagEntry[vm2.Self.Value]), "Should have found 1 tag")

	// Remove tags
	err = th.tp.RemoveTag(vm1.Reference(), vlanTag)
	AssertOk(t, err, "failed to remove tag vlan ")

	expMap = map[string][]string{
		vm1.Self.Value: []string{fmt.Sprintf("%s:%s", defs.VCTagCategory, defs.CreateVCTagManagedTag(state.ClusterID))},
	}
	th.verifyTags(expMap)

	err = th.tp.RemoveTagObjManaged(vm1.Reference())
	AssertOk(t, err, "failed to remove tag managed")

	expMap = map[string][]string{
		vm1.Self.Value: []string{},
	}
	th.verifyTags(expMap)

	// Test remove pensando tags
	err = th.tp.TagObjAsManaged(vm1.Reference())
	AssertOk(t, err, "failed to tag vm1 as managed")

	err = th.tp.TagObjWithVlan(vm1.Reference(), 1000)
	AssertOk(t, err, "failed to tag vm1 with vlan tag")

	errs := th.tp.RemovePensandoTags(vm1.Reference())
	AssertEquals(t, 0, len(errs), "failed to remove all tags, errs %v", errs)

	Assert(t, th.tp.IsManagedTag(defs.CreateVCTagManagedTag(state.ClusterID)), "isManagedTag utility function failed")
	vlan, ok := th.tp.IsVlanTag(vlanTag)
	Assert(t, ok, "isVlanTag utility function failed")
	AssertEquals(t, 1000, vlan, "isVlanTag utility function failed")

}

type testHelper struct {
	t       *testing.T
	tp      *tagsProbe
	storeCh chan defs.Probe2StoreMsg
}

func (h *testHelper) attachTag(tagID string, vm *simulator.VirtualMachine) {
	tc := h.tp.GetTagClientWithRLock()
	err := tc.AttachTag(h.tp.Ctx, tagID, vm.Reference())
	h.tp.ReleaseClientsRLock()
	AssertOk(h.t, err, "AttachTag failed")
}

func (h *testHelper) detachTag(tagID string, vm *simulator.VirtualMachine) {
	tc := h.tp.GetTagClientWithRLock()
	err := tc.DetachTag(h.tp.Ctx, tagID, vm.Reference())
	h.tp.ReleaseClientsRLock()
	AssertOk(h.t, err, "AttachTag failed")
}

func (h *testHelper) deleteTag(tagID string) {
	tc := h.tp.GetTagClientWithRLock()
	tag, err := tc.GetTag(h.tp.Ctx, tagID)
	AssertOk(h.t, err, "GetTagByName failed")
	err = tc.DeleteTag(h.tp.Ctx, tag)
	AssertOk(h.t, err, "deleteTag failed")
	h.tp.ReleaseClientsRLock()
}

func (h *testHelper) renameTag(tagName string, newName string) {
	tc := h.tp.GetTagClientWithRLock()
	tag, err := tc.GetTag(h.tp.Ctx, tagName)
	AssertOk(h.t, err, "GetTagByName failed")
	tag.Name = newName
	err = tc.UpdateTag(h.tp.Ctx, tag)
	AssertOk(h.t, err, "UpdateTag failed")
	h.tp.ReleaseClientsRLock()
}

func (h *testHelper) renameCategory(catName string, newName string) {
	tc := h.tp.GetTagClientWithRLock()
	cat, err := tc.GetCategory(h.tp.Ctx, catName)
	AssertOk(h.t, err, "GetTagByName failed")
	cat.Name = newName
	err = tc.UpdateCategory(h.tp.Ctx, cat)
	AssertOk(h.t, err, "UpdateTag failed")
	h.tp.ReleaseClientsRLock()
}

func (h *testHelper) verifyTags(expMap map[string][]string) {
	h.fetchTags()
	items := h.getTagMsgsFromStore()

	AssertEquals(h.t, len(expMap), len(items), "%s : Expected msgs did not match, %+v", getCaller(), items)
	for _, item := range items {

		m, ok := item.Val.(defs.VCEventMsg)
		if !ok {
			continue
		}
		expTags, ok := expMap[m.Key]
		Assert(h.t, ok, "received event for unexpected key %s", m.Key)
		for _, change := range m.Changes {
			tagMsg := change.Val.(defs.TagMsg)
			for _, tag := range tagMsg.Tags {
				AssertOneOf(h.t, fmt.Sprintf(("%s:%s"), tag.Category, tag.Name), expTags)
			}
		}
	}
}

func (h *testHelper) fetchTags() {
	h.tp.fetchTags()
}

func (h *testHelper) getTagMsgsFromStore() []defs.Probe2StoreMsg {
	var items []defs.Probe2StoreMsg
	hasItems := true
	for hasItems {
		select {
		case item := <-h.storeCh:
			m, ok := item.Val.(defs.VCEventMsg)
			if !ok {
				continue
			}
			for _, change := range m.Changes {
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

func (h *testHelper) createWorkload(sm *smmock.Statemgr, name string, labels map[string]string) {
	err := sm.Controller().Workload().Create(&workload.Workload{
		TypeMeta: api.TypeMeta{
			Kind:       "Workload",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Labels:    labels,
		},
	},
	)
	AssertOk(h.t, err, "Failed to create workload")
}

func getCaller() string {
	_, file, line, _ := runtime.Caller(2)
	return fmt.Sprintf("%s:%d", filepath.Base(file), line)
}
