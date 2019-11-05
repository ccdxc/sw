package vcprobe

import (
	"context"
	"strings"
	"time"

	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
)

const (
	tagsPollDelay = 10 * time.Second
	deleteDelay   = 500 * time.Millisecond
)

// tagsProbe is responsible for keeping the tag information
// in sync using the REST interface. It assumes single
// thread
type tagsProbe struct {
	vcInst
	tc       *tags.Manager
	outbox   chan<- defs.Probe2StoreMsg
	vmsOnTag map[string]*DeltaStrSet // vms that have this tagID
	vmInfo   map[string]*DeltaStrSet // tags on a VM
	tagName  map[string]string       // tagID to name
}

// newTagsProbe creates a new instance of tagsProbe
func (v *VCProbe) newTagsProbe(ctx context.Context) {
	restCl := rest.NewClient(v.client.Client)
	tagCl := tags.NewManager(restCl)
	v.tp = &tagsProbe{
		vcInst:   v.vcInst,
		outbox:   v.outbox,
		tc:       tagCl,
		vmsOnTag: make(map[string]*DeltaStrSet),
		vmInfo:   make(map[string]*DeltaStrSet),
		tagName:  make(map[string]string),
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
		if strings.Contains(err.Error(), "code: 401") { // auth error
			loginErr := t.tc.Login(t.ctx, t.vcURL.User)
			if loginErr != nil {
				t.Log.Errorf("Tags client attempted to re-login but failed, %s", loginErr.Error())
			}
		}
		return err
	}

	changedVMs := NewDeltaStrSet([]string{})
	for _, tagID := range tagList {
		t.fetchTagInfo(tagID, changedVMs)
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
	for _, obj := range objs[0].ObjectIDs {
		if obj.Reference().Type == "VirtualMachine" {
			vmList = append(vmList, obj.Reference().Value)
		}
	}

	vmSet := t.vmsOnTag[tagID]
	if vmSet == nil && len(vmList) == 0 {
		// this tag can be ignored
		t.Log.Debugf("No vms on tag %s", tagID)
		return
	}

	if vmSet == nil {
		// new tag
		vmSet = NewDeltaStrSet(vmList)
		t.vmsOnTag[tagID] = vmSet
		chSet.Add(vmList)
		t.updateVMInfo(tagID, vmSet)
		t.tagName[tagID] = tagName
		return
	}

	// if tagName changed, mark the whole vmList as changed
	if t.tagName[tagID] != tagName {
		chSet.Add(vmList)
		t.tagName[tagID] = tagName
	}

	// get the changeset on the tag
	diff := vmSet.Diff(vmList)

	// apply the change to vmsOnTag
	vmSet.ApplyDiff(diff)

	// update each VM's tag info
	t.updateVMInfo(tagID, diff)

	// add vms to changeset to enable notifications
	chSet.Add(diff.Items())
}

// updateVMInfo updates tags of vms in the change set
func (t *tagsProbe) updateVMInfo(tagID string, d *DeltaStrSet) {
	for _, vmKey := range d.Additions() {
		vm := t.vmInfo[vmKey]
		if vm == nil {
			vm = NewDeltaStrSet([]string{})
			t.vmInfo[vmKey] = vm
		}

		vm.AddSingle(tagID)
	}

	for _, vmKey := range d.Deletions() {
		vm := t.vmInfo[vmKey]
		if vm != nil {
			vm.RemoveSingle(tagID)
		}
	}
}

// genUpdates generates update notifications for changed VMs
func (t *tagsProbe) genUpdates(chSet *DeltaStrSet) {
}
