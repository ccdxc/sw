package vcprobe

import (
	"context"
	"strings"
	"time"

	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
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
	vcInst
	tc       *tags.Manager
	outbox   chan<- defs.Probe2StoreMsg
	vmsOnKey map[itemKey]*DeltaStrSet // vms that have catID:tagID
	vmInfo   map[string]*DeltaStrSet  // tags on a VM
	tagMap   map[string]*tagEntry     // catID:tagID to tag info
	catMap   map[string]string        // catID to cat info
}

// newTagsProbe creates a new instance of tagsProbe
func (v *VCProbe) newTagsProbe(ctx context.Context) {
	restCl := rest.NewClient(v.client.Client)
	tagCl := tags.NewManager(restCl)
	v.tp = &tagsProbe{
		vcInst:   v.vcInst,
		outbox:   v.outbox,
		tc:       tagCl,
		vmsOnKey: make(map[itemKey]*DeltaStrSet),
		vmInfo:   make(map[string]*DeltaStrSet),
		tagMap:   make(map[string]*tagEntry),
		catMap:   make(map[string]string),
	}
}

// Start starts the client
func (t *tagsProbe) Start() {
	err := t.tc.Login(t.ctx, t.vcURL.User)
	if err != nil {
		t.Log.Errorf("Tags client failed to login, %s", err.Error())
		return
	}
	t.pollTags()
}

// pollTags polls the cis rest server for tag information
func (t *tagsProbe) pollTags() {
	t.wg.Add(1)
	defer t.wg.Done()

	for {
		select {
		case <-t.ctx.Done():
			// delete session using another ctx
			doneCtx, cancel := context.WithTimeout(context.Background(), deleteDelay)
			t.tc.Logout(doneCtx)
			cancel()
			return
		case <-time.After(tagsPollDelay):
			t.fetchTags()
		}
	}
}

func (t *tagsProbe) fetchTags() error {
	tagList, err := t.tc.ListTags(t.ctx)
	if err != nil {
		t.Log.Errorf("Tags client list tags failed, %s", err.Error())
		if strings.Contains(err.Error(), "code: 401") { // auth error
			loginErr := t.tc.Login(t.ctx, t.vcURL.User)
			if loginErr != nil {
				t.Log.Errorf("Tags client attempted to re-login but failed, %s", loginErr.Error())
			}
		}
		return err
	}
	catList, err := t.tc.GetCategories(t.ctx)
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
	objs, err := t.tc.GetAttachedObjectsOnTags(t.ctx, []string{tagID})
	if err != nil {
		t.Log.Errorf("tag: %s error: %v", tagID, err)
		return
	}

	var vmList []string
	if len(objs) == 0 {
		t.Log.Debugf("No objects on tag %s", tagID)
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
		t.Log.Debugf("No vms on %s", key)
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
			VcObject:   defs.VirtualMachine,
			Key:        vm,
			Changes:    pc,
			Originator: t.VcID,
		}
		t.Log.Debugf("Sending tag message to store, key: %s, changes: %v", vm, pc)
		t.outbox <- m
	}
}

func createKey(tagID, catID string) itemKey {
	return itemKey{
		tagID: tagID,
		catID: catID,
	}
}
