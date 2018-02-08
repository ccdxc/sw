package vcprobe

import (
	"context"
	"net/url"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	tagsPollDelay = 10 * time.Second
	deleteDelay   = 500 * time.Millisecond
)

// tagsProbe is responsible for keeping the tag information
// in sync using the REST interface. It assumes single
// thread
type tagsProbe struct {
	vcID     string // unique identifier for the VC
	ctx      context.Context
	outBox   chan<- defs.StoreMsg // goes to store
	tc       *TagClient
	vmsOnTag map[string]*DeltaStrSet // vms that have this tagID
	vmInfo   map[string]*DeltaStrSet // tags on a VM
	tagName  map[string]string       // tagID to name
}

// newTagsProbe returns a new instance of tagsProbe
func newTagsProbe(vcURL *url.URL, hOutBox chan<- defs.StoreMsg) *tagsProbe {
	return &tagsProbe{
		tc:       NewTagClient(vcURL),
		vcID:     vcURL.Hostname() + ":" + vcURL.Port(),
		outBox:   hOutBox,
		vmsOnTag: make(map[string]*DeltaStrSet),
		vmInfo:   make(map[string]*DeltaStrSet),
		tagName:  make(map[string]string),
	}
}

// Start initializes client
func (t *tagsProbe) Start(ctx context.Context) error {
	// set up a cis session
	t.ctx = ctx
	err := t.tc.CreateSession(ctx)
	return err
}

// PollTags polls the cis rest server for tag information
func (t *tagsProbe) PollTags(wg *sync.WaitGroup) {
	wg.Add(1)
	defer wg.Done()

	for {
		select {
		case <-t.ctx.Done():
			// delete session using another ctx
			doneCtx, cancel := context.WithTimeout(context.Background(), deleteDelay)
			t.tc.SetContext(doneCtx)
			t.tc.DeleteSession()
			cancel()
			return
		case <-time.After(tagsPollDelay):
			t.fetchTags()
		}
	}
}

func (t *tagsProbe) fetchTags() error {
	tagList, err := t.tc.ListTags()
	if err != nil {
		if strings.Contains(err.Error(), "code: 401") { // auth error

			t.tc.CreateSession(t.ctx)
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
	vmList, err := t.tc.ListVMs(tagID)
	if err != nil {
		log.Errorf("tag: %s error: %v", tagID, err)
		return
	}

	vmSet := t.vmsOnTag[tagID]
	if vmSet == nil && len(vmList) == 0 {
		// this tag can be ignored
		log.Debugf("No vms on tag %s", tagID)
		return
	}

	// get the tagname
	tagName, _, err := t.tc.GetTag(tagID)
	if err != nil {
		log.Errorf("Error %v getting tagname %s", err, tagID)
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
