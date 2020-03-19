package vcprobe

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/session"
)

const (
	tagsPollDelay = 10 * time.Second
	deleteDelay   = 500 * time.Millisecond
)

type tagEntry struct {
	TagName string
	CatName string
	CatID   string
}

type itemKey struct {
	tagID string
	catID string
}

// tagsProbe is responsible for keeping the tag information
// in sync using the REST interface. It assumes single
// thread
type tagsProbe struct {
	*defs.State
	*session.Session
	outbox         chan<- defs.Probe2StoreMsg
	vmsOnKey       map[itemKey]*DeltaStrSet // vms that have catID:tagID
	vmInfo         map[string]*DeltaStrSet  // tags on a VM
	tagMap         map[string]*tagEntry     // catID:tagID to tag info
	catMap         map[string]string        // catID to cat info
	writeTagInfo   map[string]string        // Tag/cat Name to ID. These cat/tags are objects we write to vc objects
	managedTagName string
}

// newTagsProbe creates a new instance of tagsProbe
func (v *VCProbe) newTagsProbe() {
	tagName := defs.VCTagManagedDefault
	if len(v.State.ClusterID) != 0 {
		tagName = defs.CreateVCTagManagedTag(v.State.ClusterID)
	}
	v.tp = &tagsProbe{
		State:          v.State,
		Session:        v.Session,
		outbox:         v.outbox,
		vmsOnKey:       make(map[itemKey]*DeltaStrSet),
		vmInfo:         make(map[string]*DeltaStrSet),
		tagMap:         make(map[string]*tagEntry),
		catMap:         make(map[string]string),
		writeTagInfo:   make(map[string]string),
		managedTagName: tagName,
	}
}

func (t *tagsProbe) StartWatch() {
	t.Log.Debugf("Tag Start Watch starting")
	t.pollTags()
}

// SetupVCTags continually tries to create tags until ctx is killed
// Ctx should be derived from ClientCtx
// Returns whether all tags were set up successfully
func (t *tagsProbe) SetupVCTags(ctx context.Context) bool {
	t.Log.Info("Setting up tags....")
	var tagCategory *tags.Category
	var err error
	retryUntilSuccessful := func(fn func() bool) bool {
		for ctx.Err() == nil {
			if fn() {
				return true
			}
			select {
			case <-ctx.Done():
				return false
			case <-time.After(tagsPollDelay):
			}
		}
		return false
	}
	// Create category
	completed := retryUntilSuccessful(func() bool {
		t.Log.Info("Creating tags category....")
		tc := t.GetTagClientWithRLock()
		defer t.ReleaseClientsRLock()
		tagCategory, err = tc.GetCategory(t.ClientCtx, defs.VCTagCategory)
		if err == nil {
			t.writeTagInfo[defs.VCTagCategory] = tagCategory.ID
			return true
		}
		t.Log.Info("Tag category does not exist, creating default category")
		// Create category
		tagCategory = &tags.Category{
			Name:        defs.VCTagCategory,
			Description: defs.VCTagCategoryDescription,
			Cardinality: "MULTIPLE",
		}
		catID, err := tc.CreateCategory(t.ClientCtx, tagCategory)
		if err == nil {
			t.Log.Info("Created tag category")
			t.writeTagInfo[defs.VCTagCategory] = catID
			return true
		}
		t.Log.Errorf("Failed to create default category: %s", err)
		return false
	})
	if !completed {
		return false
	}

	// Get tags for category
	var tagObjs []tags.Tag
	completed = retryUntilSuccessful(func() bool {
		t.Log.Info("Getting tags in category....")
		tc := t.GetTagClientWithRLock()
		defer t.ReleaseClientsRLock()
		var err error
		tagObjs, err = tc.GetTagsForCategory(t.ClientCtx, tagCategory.Name)
		if err == nil {
			t.Log.Infof("Got tags in Pensando category")
			return true
		}
		t.Log.Errorf("Failed to get tags for category %s: %s", tagCategory.Name, err)
		return false
	})

	if !completed {
		return false
	}
	for _, tag := range tagObjs {
		t.writeTagInfo[tag.Name] = tag.ID
	}

	// Create VCTagManaged
	completed = retryUntilSuccessful(func() bool {
		t.Log.Info("Creating tag managed....")
		tc := t.GetTagClientWithRLock()
		defer t.ReleaseClientsRLock()
		if _, ok := t.writeTagInfo[t.managedTagName]; ok {
			t.Log.Infof("Pensando managed tag already exists")
			return true
		}
		tag := &tags.Tag{
			Name:        t.managedTagName,
			Description: defs.VCTagManagedDescription,
			CategoryID:  tagCategory.Name,
		}
		tagID, err := tc.CreateTag(t.ClientCtx, tag)
		if err == nil {
			t.writeTagInfo[t.managedTagName] = tagID
			t.Log.Infof("Pensando managed tag created")
			return true
		}
		t.Log.Errorf("Failed to create VCTagManaged for category: %s", err)
		return false
	})
	if !completed {
		return false
	}
	return true
}

func (t *tagsProbe) TagObjAsManaged(ref types.ManagedObjectReference) error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Infof("Tagging object %s as pensando managed", ref.Value)
	return tc.AttachTag(t.ClientCtx, t.writeTagInfo[t.managedTagName], ref)
}

func (t *tagsProbe) RemoveTagObjManaged(ref types.ManagedObjectReference) error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Infof("Removing tag managed from object %s", ref.Value)
	return tc.DetachTag(t.ClientCtx, t.writeTagInfo[t.managedTagName], ref)
}

func (t *tagsProbe) TagObjWithVlan(ref types.ManagedObjectReference, vlanValue int) error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Infof("Tagging object %s with vlan %d", ref.Value, vlanValue)
	tagName := fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, vlanValue)
	var tagID string
	var ok bool
	if tagID, ok = t.writeTagInfo[tagName]; !ok {
		var err error
		catID, ok := t.writeTagInfo[defs.VCTagCategory]
		if !ok {
			return fmt.Errorf("Cateogry tag hasn't been created yet")
		}
		// Create tag
		tagReq := &tags.Tag{
			Name:        tagName,
			Description: defs.VCTagVlanDescription,
			CategoryID:  catID,
		}
		tagID, err = tc.CreateTag(t.ClientCtx, tagReq)
		if err != nil {
			t.Log.Errorf("Failed to create vlan tag %s for obj %s, err %s", tagName, ref.Value, err)
			return err
		}
		t.writeTagInfo[tagName] = tagID
	}

	return tc.AttachTag(t.ClientCtx, tagID, ref)
}

func (t *tagsProbe) RemoveTagObjVlan(ref types.ManagedObjectReference) error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing vlan tag from object %s", ref.Value)

	tags, err := tc.GetAttachedTags(t.ClientCtx, ref)
	if err != nil {
		return err
	}
	for _, tag := range tags {
		if tag.CategoryID == t.writeTagInfo[defs.VCTagCategory] && strings.HasPrefix(tag.Name, defs.VCTagVlanPrefix) {
			// Tag is a vlan tag
			// There should never be more than one vlan tag
			// so we can return after the first one we find
			return tc.DetachTag(t.ClientCtx, tag.ID, ref)
		}
	}

	t.Log.Infof("Received tag remove for %s, but it didn't have a vlan tag", ref.Value)

	return nil
}

func (t *tagsProbe) RemovePensandoTags(ref types.ManagedObjectReference) []error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing all pensando tags from object %s", ref.Value)

	errs := []error{}

	tags, err := tc.GetAttachedTags(t.ClientCtx, ref)
	if err != nil {
		errs = append(errs, err)
		return errs
	}
	for _, tag := range tags {
		if tag.CategoryID == t.writeTagInfo[defs.VCTagCategory] {
			// Tag is a pensando tag
			err = tc.DetachTag(t.ClientCtx, tag.ID, ref)
			if err != nil {
				t.Log.Errorf("Removing tag %s failed: err %s", tag.Name, err)
				errs = append(errs, err)
			}
		}
	}

	return errs
}

// pollTags polls the cis rest server for tag information
func (t *tagsProbe) pollTags() {
	for {
		select {
		case <-t.ClientCtx.Done():
			return
		case <-time.After(tagsPollDelay):
			t.fetchTags()
		}
	}
}

func (t *tagsProbe) fetchTags() error {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()

	tagList, err := tc.ListTags(t.ClientCtx)
	if err != nil {
		t.Log.Errorf("Tags client list tags failed, %s", err.Error())
		if strings.Contains(err.Error(), "code: 401") { // auth error
			loginErr := tc.Login(t.ClientCtx, t.VcURL.User)
			if loginErr != nil {
				t.Log.Errorf("Tags client attempted to re-login but failed, %s", loginErr.Error())
				t.CheckSession = true
			}
		}
		return err
	}
	catList, err := tc.GetCategories(t.ClientCtx)
	if err != nil {
		t.Log.Errorf("Tags client list categories failed, %s", err.Error())
		return err
	}
	for _, cat := range catList {
		t.catMap[cat.ID] = cat.Name
	}

	changedVMs := NewDeltaStrSet([]string{})
	for _, tagID := range tagList {
		t.fetchTagInfo(tagID, changedVMs)
	}

	// Check if any tags have been deleted
	tagsInUse := make([]string, 0, len(t.vmsOnKey))
	for key := range t.vmsOnKey {
		tagsInUse = append(tagsInUse, key.tagID)
	}
	tagSet := NewDeltaStrSet(tagsInUse)
	diffs := tagSet.Diff(tagList)
	tagsToRemove := diffs.Deletions()
	for _, rt := range tagsToRemove {
		entry := t.tagMap[rt]
		id := createKey(rt, entry.CatID)
		vms := t.vmsOnKey[id]
		vms.MarkAllForDeletion()
		// Remove tag from vms
		t.updateVMInfo(id, vms)
		changedVMs.Add(vms.Items())
	}

	t.genUpdates(changedVMs)

	return nil
}

func (t *tagsProbe) fetchTagInfo(tagID string, chSet *DeltaStrSet) {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	objs, err := tc.GetAttachedObjectsOnTags(t.ClientCtx, []string{tagID})
	if err != nil {
		t.Log.Errorf("tag: %s error: %v", tagID, err)
		return
	}

	var vmList []string
	if len(objs) == 0 {
		// No objects on tag
		return
	}
	tagName := objs[0].Tag.Name
	catID := objs[0].Tag.CategoryID
	catName := t.catMap[catID]
	key := createKey(tagID, catID)
	for _, obj := range objs[0].ObjectIDs {
		if obj.Reference().Type == "VirtualMachine" {
			vmList = append(vmList, obj.Reference().Value)
		}
	}

	vmSet := t.vmsOnKey[key]
	if vmSet == nil && len(vmList) == 0 {
		// this tag can be ignored
		return
	}

	if vmSet == nil {
		// new tag
		vmSet = NewDeltaStrSet(vmList)
		t.vmsOnKey[key] = vmSet
		chSet.Add(vmList)
		t.updateVMInfo(key, vmSet)
		t.tagMap[tagID] = &tagEntry{
			TagName: tagName,
			CatName: catName,
			CatID:   catID,
		}
		return
	}

	// if tagName or catName changed, mark the whole vmList as changed
	if t.tagMap[tagID].TagName != tagName || t.tagMap[tagID].CatName != catName {
		chSet.Add(vmList)
		t.tagMap[tagID] = &tagEntry{
			TagName: tagName,
			CatName: catName,
			CatID:   catID,
		}
	}

	// get the changeset on the tag
	diff := vmSet.Diff(vmList)

	// apply the change to vmsOnKey
	vmSet.ApplyDiff(diff)

	// update each VM's tag info
	t.updateVMInfo(key, diff)

	// add vms to changeset to enable notifications
	chSet.Add(diff.Items())
}

// updateVMInfo updates tags of vms in the change set
func (t *tagsProbe) updateVMInfo(id itemKey, d *DeltaStrSet) {
	for _, vmKey := range d.Additions() {
		vm := t.vmInfo[vmKey]
		if vm == nil {
			vm = NewDeltaStrSet([]string{})
			t.vmInfo[vmKey] = vm
		}

		vm.AddSingle(id.tagID)
	}

	for _, vmKey := range d.Deletions() {
		vm := t.vmInfo[vmKey]
		if vm != nil {
			vm.RemoveSingle(id.tagID)
		}
	}
}

// genUpdates generates update notifications for changed VMs
func (t *tagsProbe) genUpdates(chSet *DeltaStrSet) {
	changedVms := chSet.Items()
	for _, vm := range changedVms {
		tagSet, ok := t.vmInfo[vm]
		if !ok {
			continue
		}

		tags := tagSet.Items()
		var tagEntries []defs.TagEntry
		for _, id := range tags {
			entry := defs.TagEntry{
				Name:     t.tagMap[id].TagName,
				Category: t.catMap[t.tagMap[id].CatID],
			}
			tagEntries = append(tagEntries, entry)
		}

		pc := []types.PropertyChange{
			{
				Name: string(defs.VMPropTag),
				Op:   "set",
				Val:  defs.TagMsg{Tags: tagEntries},
			},
		}
		m := defs.Probe2StoreMsg{
			MsgType: defs.VCEvent,
			Val: defs.VCEventMsg{
				VcObject:   defs.VirtualMachine,
				Key:        vm,
				Changes:    pc,
				Originator: t.VcID,
			},
		}
		t.Log.Debugf("Sending tag message to store, key: %s, changes: %v", vm, pc)
		if t.outbox != nil {
			t.outbox <- m
		}
	}
}

func createKey(tagID, catID string) itemKey {
	return itemKey{
		tagID: tagID,
		catID: catID,
	}
}
