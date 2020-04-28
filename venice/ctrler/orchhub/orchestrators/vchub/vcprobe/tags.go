package vcprobe

import (
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe/session"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
)

const (
	renameDelay    = 30 * time.Second
	tagsPollDelay  = 10 * time.Second
	syncWriteDelay = 2 * time.Minute
)

type tagEntry struct {
	TagName string
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
	outbox chan<- defs.Probe2StoreMsg

	vmTagMapLock sync.Mutex
	vmTagMap     map[string]*DeltaStrSet // VM -> tags on VM

	tagMapLock sync.RWMutex         // Lock guarding tagMap and catMap
	tagMap     map[string]*tagEntry // tagID to tag info
	catMap     map[string]string    // catID to cat name

	writeTagInfo   map[string]string // Only for cat/tags we write to vc objects. Stores both Tag/cat Name to ID and ID -> tag/cat name
	writeLock      sync.Mutex        // Lock used for write related events
	managedTagName string
}

// newTagsProbe creates a new instance of tagsProbe
func (v *VCProbe) newTagsProbe() {
	tagName := defs.VCTagManagedDefault
	if len(v.State.ClusterID) != 0 {
		tagName = defs.CreateVCTagManagedTag(v.State.ClusterID)
	}
	v.TagsProbeInf = &tagsProbe{
		State:          v.State,
		Session:        v.Session,
		outbox:         v.outbox,
		vmTagMap:       map[string]*DeltaStrSet{},
		tagMap:         make(map[string]*tagEntry),
		catMap:         make(map[string]string),
		writeTagInfo:   make(map[string]string),
		managedTagName: tagName,
	}
}

func (t *tagsProbe) StartWatch() {
	t.Log.Infof("Tag Start Watch starting")
	t.pollTags()
	t.WatcherWg.Add(1)
	go func() {
		t.pollTagInfo()
		t.WatcherWg.Done()
	}()
}

func (t *tagsProbe) SetupBaseTags() bool {
	t.Log.Info("Setting up tags....")

	var tagCategory *tags.Category
	var err error
	retryUntilSuccessful := func(fn func() bool) bool {
		for t.Ctx.Err() == nil {
			if fn() {
				return true
			}
			select {
			case <-t.Ctx.Done():
				return false
			case <-time.After(tagsPollDelay):
			}
		}
		return false
	}
	// Create category
	completed := retryUntilSuccessful(func() bool {
		t.writeLock.Lock()
		defer t.writeLock.Unlock()
		ctx := t.ClientCtx
		if ctx == nil {
			return false
		}
		t.Log.Info("Creating tags category....")
		tc := t.GetTagClientWithRLock()
		defer t.ReleaseClientsRLock()
		tagCategory, err = tc.GetCategory(ctx, defs.VCTagCategory)
		if err == nil {
			t.writeTagInfo[defs.VCTagCategory] = tagCategory.ID
			t.writeTagInfo[tagCategory.ID] = defs.VCTagCategory
			return true
		}
		t.Log.Info("Tag category does not exist, creating default category")
		// Create category
		tagCategory = &tags.Category{
			Name:        defs.VCTagCategory,
			Description: defs.VCTagCategoryDescription,
			Cardinality: "MULTIPLE",
		}
		catID, err := tc.CreateCategory(ctx, tagCategory)
		if err == nil {
			t.Log.Info("Created tag category")
			t.writeTagInfo[defs.VCTagCategory] = catID
			t.writeTagInfo[catID] = defs.VCTagCategory
			tagCategory.ID = catID
			return true
		}
		t.Log.Errorf("Failed to create default category: %s", err)
		if t.IsREST401(err) {
			t.Log.Errorf("CHECK TAG SESSION", err)
			t.CheckTagSession = true
		}
		return false
	})
	if !completed {
		return false
	}

	// Get tags for category
	var tagObjs []tags.Tag
	completed = retryUntilSuccessful(func() bool {
		t.writeLock.Lock()
		defer t.writeLock.Unlock()
		t.Log.Info("Getting tags in category....")
		tc := t.GetTagClientWithRLock()
		defer t.ReleaseClientsRLock()
		var err error
		tagObjs, err = tc.GetTagsForCategory(t.ClientCtx, tagCategory.ID)
		if err == nil {
			t.Log.Infof("Got tags in Pensando category")
			return true
		}
		t.Log.Errorf("Failed to get tags for category %s: %s", tagCategory.ID, err)
		if t.IsREST401(err) {
			t.CheckTagSession = true
		}
		return false
	})

	if !completed {
		return false
	}
	tagInfo := map[string]string{}
	for _, tag := range tagObjs {
		tagInfo[tag.Name] = tag.ID
		tagInfo[tag.ID] = tag.Name
	}
	// merge category tags
	tagInfo[defs.VCTagCategory] = tagCategory.ID
	tagInfo[tagCategory.ID] = defs.VCTagCategory
	t.writeTagInfo = tagInfo

	// Create VCTagManaged
	completed = retryUntilSuccessful(func() bool {
		t.writeLock.Lock()
		defer t.writeLock.Unlock()
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
			CategoryID:  tagCategory.ID,
		}
		tagID, err := tc.CreateTag(t.ClientCtx, tag)
		if err == nil {
			t.writeTagInfo[t.managedTagName] = tagID
			t.writeTagInfo[tagID] = t.managedTagName
			t.Log.Infof("Pensando managed tag created")
			return true
		}
		t.Log.Errorf("Failed to create VCTagManaged for category: %s", err)
		if t.IsREST401(err) {
			t.CheckTagSession = true
		}
		return false
	})
	if !completed {
		return false
	}
	return true

}

func (t *tagsProbe) TagObjsAsManaged(refs []types.ManagedObjectReference) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()
	id, ok := t.writeTagInfo[t.managedTagName]
	if !ok {
		return fmt.Errorf("Tag pensando managed does not exist")
	}
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()

	var moRefs []mo.Reference
	for _, ref := range refs {
		moRefs = append(moRefs, mo.View{
			Self: ref,
		})
	}

	t.Log.Debugf("Tagging objects %v as pensando managed", refs)
	return tc.AttachTagToMultipleObjects(t.ClientCtx, id, moRefs)
}

// GetPensandoTagsOnObjects retreives the tags on the given objects.
// Returns map of kind -> objID -> tags
func (t *tagsProbe) GetPensandoTagsOnObjects(refs []types.ManagedObjectReference) (map[string]KindTagMapEntry, error) {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()

	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()

	t.Log.Debugf("Retrieving pensando tags on %d objects", len(refs))
	var moRefs []mo.Reference
	for _, ref := range refs {
		moRefs = append(moRefs, mo.View{
			Self: ref,
		})
	}

	attachedTags, err := tc.ListAttachedTagsOnObjects(t.ClientCtx, moRefs)
	if err != nil {
		return nil, err
	}

	// Convert tags
	// Map of kind -> objID -> tags
	resMap := map[string](KindTagMapEntry){}

	for _, attachedTag := range attachedTags {
		kind := attachedTag.ObjectID.Reference().Type
		id := attachedTag.ObjectID.Reference().Value

		kindMap, ok := resMap[kind]
		if !ok {
			kindMap = KindTagMapEntry{}
			resMap[kind] = kindMap
		}

		tags := []string{}
		for _, id := range attachedTag.TagIDs {
			if tagName, ok := t.writeTagInfo[id]; ok {
				tags = append(tags, tagName)
			} else {
				t.Log.Debugf("Skipping tag %s as its not a pensando tag", id)
			}
		}
		kindMap[id] = tags
	}
	t.Log.Debugf("Retrieved tags %+v", resMap)

	return resMap, nil
}

func (t *tagsProbe) IsManagedTag(tag string) bool {
	return t.managedTagName == tag
}

func (t *tagsProbe) IsVlanTag(tag string) (int, bool) {
	if !strings.HasPrefix(tag, defs.VCTagVlanPrefix) {
		return -1, false
	}
	s := strings.TrimPrefix(tag, defs.VCTagVlanPrefix)
	i, err := strconv.Atoi(s)
	if err != nil {
		t.Log.Errorf("Failed to parse vlan value while parsing tag %s", tag)
		return -1, false
	}
	return i, true
}

func (t *tagsProbe) TagObjAsManaged(ref types.ManagedObjectReference) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()
	id, ok := t.writeTagInfo[t.managedTagName]
	if !ok {
		return fmt.Errorf("Tag pensando managed does not exist")
	}
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Tagging object %s as pensando managed", ref.Value)
	return tc.AttachTag(t.ClientCtx, id, ref)
}

func (t *tagsProbe) RemoveTagObjManaged(ref types.ManagedObjectReference) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()
	id, ok := t.writeTagInfo[t.managedTagName]
	if !ok {
		return fmt.Errorf("Tag pensando managed does not exist")
	}
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing tag managed from object %s", ref.Value)
	return tc.DetachTag(t.ClientCtx, id, ref)
}

func (t *tagsProbe) TagObjWithVlan(ref types.ManagedObjectReference, vlanValue int) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()

	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Tagging object %s with vlan %d", ref.Value, vlanValue)
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
		t.writeTagInfo[tagID] = tagName
	}

	return tc.AttachTag(t.ClientCtx, tagID, ref)
}

func (t *tagsProbe) RemoveTag(ref types.ManagedObjectReference, tagName string) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()

	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing vlan tag from object %s", ref.Value)

	tagID, ok := t.writeTagInfo[tagName]
	if !ok {
		return fmt.Errorf("Failed to remove tag, no entry for tag %s", tagName)
	}
	return tc.DetachTag(t.ClientCtx, tagID, ref)
}

// RemoveTagObjVlan has worse performance than removeTag. Use RemoveTag if possible.
func (t *tagsProbe) RemoveTagObjVlan(ref types.ManagedObjectReference) error {
	t.writeLock.Lock()
	defer t.writeLock.Unlock()

	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing vlan tag from object %s", ref.Value)

	tags, err := tc.GetAttachedTags(t.ClientCtx, ref)
	if err != nil {
		return err
	}
	for _, tag := range tags {
		catID, ok := t.writeTagInfo[defs.VCTagCategory]
		if !ok {
			return fmt.Errorf("Pensando category is not created yet")
		}
		if tag.CategoryID == catID && strings.HasPrefix(tag.Name, defs.VCTagVlanPrefix) {
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
	t.writeLock.Lock()
	defer t.writeLock.Unlock()

	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()
	t.Log.Debugf("Removing all pensando tags from object %s", ref.Value)

	errs := []error{}

	tagIDs, err := tc.ListAttachedTags(t.ClientCtx, ref)
	if err != nil {
		errs = append(errs, err)
		return errs
	}

	for _, tag := range tagIDs {
		if _, ok := t.writeTagInfo[tag]; ok {
			// Tag is a pensando tag
			err = tc.DetachTag(t.ClientCtx, tag, ref)
			if err != nil {
				t.Log.Errorf("Removing tag %s failed: err %s", t.writeTagInfo[tag], err)
				errs = append(errs, err)
			}
		}
	}

	return errs
}

// pollTags polls the cis rest server for tag information
func (t *tagsProbe) pollTags() {
	t.Log.Infof("Poll tags starting")
	ctx := t.ClientCtx
	if ctx == nil {
		t.Log.Infof("Poll tags exiting")
		return
	}
	for {
		select {
		case <-ctx.Done():
			t.Log.Infof("Poll tags exiting")
			return
		case <-time.After(tagsPollDelay):
			t.fetchTags()
		}
	}
}

func (t *tagsProbe) pollTagInfo() {
	t.Log.Infof("Poll tag info starting")
	ctx := t.ClientCtx
	if ctx == nil {
		t.Log.Infof("Poll tags exiting")
		return
	}
	for {
		select {
		case <-ctx.Done():
			t.Log.Infof("Poll tags exiting")
			return
		case <-time.After(renameDelay):
			t.fetchTagInfo()
		}
	}
}

func (t *tagsProbe) fetchTags() {
	opts := api.ListWatchOptions{}
	ctx := t.ClientCtx
	if ctx == nil {
		return
	}
	workloads, err := t.StateMgr.Controller().Workload().List(ctx, &opts)
	if err != nil {
		t.Log.Errorf("Failed to get workload list. Err : %v", err)
	}
	vms := []mo.Reference{}
	for _, workload := range workloads {
		if utils.IsObjForOrch(workload.Labels, t.VcID, "") {
			// TODO: FIX ME - should use the utility function
			vmKey := fmt.Sprintf("%s", utils.ParseGlobalKey(t.OrchID, "", workload.Name))
			vm := mo.ExtensibleManagedObject{
				Self: types.ManagedObjectReference{
					Type:  string(defs.VirtualMachine),
					Value: vmKey,
				},
			}
			vms = append(vms, vm)
		}
	}
	t.Log.Debugf("Fetching tags for %d VMs", len(vms))

	// Recommended chunk size is 2000 objects per call
	chunkedVms := [][]mo.Reference{}
	chunkSize := 2000
	for i := 0; i < len(vms); i += chunkSize {
		end := i + chunkSize
		if end > len(vms) {
			end = len(vms)
		}

		chunkedVms = append(chunkedVms, vms[i:end])
	}

	for _, vmChunk := range chunkedVms {
		for retryCount := 3; retryCount > 0; retryCount-- {
			t.Log.Debugf("Getting attached tags on %d objects", len(vmChunk))
			tc := t.GetTagClientWithRLock()
			res, err := tc.ListAttachedTagsOnObjects(t.ClientCtx, vmChunk)
			ctx := t.ClientCtx
			t.ReleaseClientsRLock()
			if err != nil {
				t.Log.Errorf("Failed to get attached tags: %s", err)
				if t.IsREST401(err) {
					t.CheckTagSession = true
				}
				// Might be hitting rate limiting at scale
				select {
				case <-ctx.Done():
					return
				case <-time.After(500 * time.Millisecond):
				}
				continue
			}
			// Process res
			if len(res) != len(vmChunk) {
				presentMap := map[string]bool{}
				// Some items have no tags. Add in their references
				for _, item := range res {
					presentMap[item.ObjectID.Reference().Value] = true
				}

				for _, item := range vmChunk {
					if _, ok := presentMap[item.Reference().Value]; !ok {
						res = append(res, tags.AttachedTags{
							ObjectID: item,
							TagIDs:   []string{},
						})
					}
				}
			}
			t.processTags(res)
			break
		}
	}
}

// ResyncVMTags resends tag events to the probeCh for the given vm
func (t *tagsProbe) ResyncVMTags(vmID string) {
	t.vmTagMapLock.Lock()
	oldTags, ok := t.vmTagMap[vmID]
	t.vmTagMapLock.Unlock()
	if !ok {
		return
	}
	tagObj := tags.AttachedTags{
		TagIDs: oldTags.Items(),
	}
	t.generateTagEvent(vmID, tagObj)
}

func (t *tagsProbe) processTags(tags []tags.AttachedTags) {
	// For each object generate its diff
	for _, tagObj := range tags {
		t.Log.Debugf("processing %d tags", len(tagObj.TagIDs))
		vm := tagObj.ObjectID.Reference().Value
		// Do a diff with the current set to generate updates
		writeTags := false
		t.vmTagMapLock.Lock()
		oldTags, ok := t.vmTagMap[vm]
		if !ok {
			t.vmTagMap[vm] = NewDeltaStrSet([]string{})
			oldTags = t.vmTagMap[vm]
		}

		diffs := oldTags.Diff(tagObj.TagIDs)
		if diffs.Count() != 0 {
			// Tags changed
			writeTags = true
		}
		t.vmTagMapLock.Unlock()
		if writeTags {
			t.generateTagEvent(vm, tagObj)
		}
	}
}

func (t *tagsProbe) generateTagEvent(vm string, tagObj tags.AttachedTags) {
	t.vmTagMapLock.Lock()
	t.vmTagMap[vm] = NewDeltaStrSet(tagObj.TagIDs)
	t.vmTagMapLock.Unlock()

	workWg := sync.WaitGroup{}
	var tagEntries []defs.TagEntry
	jobs := make(chan int, 100)
	res := make(chan []defs.TagEntry, 100)
	workerFn := func() {
		defer workWg.Done()
		if t.ClientCtx.Err() != nil {
			return
		}
		var entries []defs.TagEntry
		for j := range jobs {
			id := tagObj.TagIDs[j]

			t.tagMapLock.RLock()
			tagEntry, ok := t.tagMap[id]
			if !ok {
				// Could be new tag
				t.tagMapLock.RUnlock()

				// No locks should be held when calling fetch tagInfo
				t.fetchSingleTagInfo(id)

				// Try re-reading
				t.tagMapLock.RLock()
				tagEntry, ok = t.tagMap[id]
				if !ok {
					// Skip since we failed to get info about this tag
					t.Log.Errorf("Failed to get info about tag ID %s, skipping", id)
					t.tagMapLock.RUnlock()
					// Don't mark the tag as written
					t.vmTagMapLock.Lock()
					t.vmTagMap[vm].RemoveSingle(id)
					t.vmTagMapLock.Unlock()
					continue
				}
			}
			tagName := tagEntry.TagName
			catID := tagEntry.CatID
			catName, ok := t.catMap[catID]
			t.tagMapLock.RUnlock()
			if !ok {
				// Skip since we failed to get info about this category
				t.Log.Errorf("Failed to get info about cat ID %s for tag %s ID %s, skipping", catID, tagName, id)
				t.vmTagMapLock.Lock()
				t.vmTagMap[vm].RemoveSingle(id)
				t.vmTagMapLock.Unlock()
				continue
			}

			entry := defs.TagEntry{
				Name:     tagName,
				Category: catName,
			}
			entries = append(entries, entry)
		}
		res <- entries
	}

	for w := 1; w <= 20; w++ {
		workWg.Add(1)
		go workerFn()
	}

	for j := 0; j < len(tagObj.TagIDs); j++ {
		jobs <- j
	}
	close(jobs)

	workWg.Wait()

	read := true
	for read {
		select {
		case entry := <-res:
			tagEntries = append(tagEntries, entry...)
		default:
			read = false
		}
	}

	m := defs.Probe2StoreMsg{
		MsgType: defs.VCEvent,
		Val: defs.VCEventMsg{
			VcObject: defs.VirtualMachine,
			Key:      vm,
			Changes: []types.PropertyChange{
				{
					Name: string(defs.VMPropTag),
					Op:   "set",
					Val:  defs.TagMsg{Tags: tagEntries},
				},
			},
			Originator: t.VcID,
		},
	}
	t.Log.Debugf("Sending tag message to store, key: %s, changes: %v", vm, tagEntries)
	if t.outbox != nil {
		t.outbox <- m
	}

}

func (t *tagsProbe) retry(fn func() (interface{}, error), operName string, delay time.Duration, count int) (interface{}, error) {
	if count < 1 {
		return nil, fmt.Errorf("Supplied invalid count")
	}
	i := 0
	var ret interface{}
	err := fmt.Errorf("Client Ctx cancelled")
	ctx := t.ClientCtx
	for i < count && ctx != nil && ctx.Err() == nil {
		ret, err = fn()
		if err == nil {
			return ret, nil
		}
		i++
		if i < count {
			t.Log.Errorf("tag %s failed: %s, retrying...", operName, err)
			if t.IsREST401(err) {
				t.CheckTagSession = true
			}
		}
		select {
		case <-ctx.Done():
			return nil, err
		case <-time.After(retryDelay):
		}
	}
	return nil, err
}

//
func (t *tagsProbe) fetchSingleTagInfo(tagID string) {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()

	fn := func() (interface{}, error) {
		return tc.GetTag(t.ClientCtx, tagID)
	}

	tagInf, err := t.retry(fn, "fetch single tag", 500*time.Millisecond, 3)
	if err != nil {
		t.Log.Errorf("Failed to fetch tag %s", err)
		return
	}
	tag := tagInf.(*tags.Tag)

	t.tagMapLock.Lock()
	currEntry, ok := t.tagMap[tagID]
	if ok && currEntry.TagName != tag.Name {
		// Tag name has changed
		t.resetTag(tag.ID)
	}
	t.tagMap[tag.ID] = &tagEntry{
		TagName: tag.Name,
		CatID:   tag.CategoryID,
	}

	_, ok = t.tagMap[tag.CategoryID]
	t.tagMapLock.Unlock()
	if ok {
		return
	}
	// Check if we have cat info
	fn = func() (interface{}, error) {
		return tc.GetCategory(t.ClientCtx, tag.CategoryID)
	}
	catInf, err := t.retry(fn, "fetch single cat", 500*time.Millisecond, 3)
	if err != nil {
		t.Log.Errorf("Failed to fetch tag %s", err)
		return
	}

	cat := catInf.(*tags.Category)
	t.tagMapLock.Lock()
	currName, ok := t.catMap[cat.ID]
	if ok && currName != cat.Name {
		// cat name has changed
		// All tags in this category need to be reset
		t.resetCategory(cat.ID, true)
	}
	t.catMap[cat.ID] = cat.Name
	t.tagMapLock.Unlock()

}

// fetchTagInfo gets the display names for all tags that we are currently using.
// If a tag or category is renamed, we iterate over all of vmTagMap and remove those entries
// The next time the poll runs, it will generate correct update events for them.
// This is an expensive operation, but tags/categories should not be renamed that often
func (t *tagsProbe) fetchTagInfo() {
	tc := t.GetTagClientWithRLock()
	defer t.ReleaseClientsRLock()

	// GetTags synchronously does GetTag for each return tag ID
	// Perform ListTag and then call the GetTag APIs concurrently ourselves.
	// tagIDs, err := tc.ListTags(t.ClientCtx)
	t.tagMapLock.Lock()
	tagIDs := []string{}
	for id := range t.tagMap {
		tagIDs = append(tagIDs, id)
	}
	t.tagMapLock.Unlock()

	workWg := sync.WaitGroup{}
	workerFn := func(jobs <-chan int) {
		defer workWg.Done()
		if t.ClientCtx.Err() != nil {
			return
		}
		for j := range jobs {
			id := tagIDs[j]

			fn := func() (interface{}, error) {
				return tc.GetTag(t.ClientCtx, id)
			}

			// Larger time out since this is only for renamed tags
			tagInf, err := t.retry(fn, "fetching tag", 500*time.Millisecond, 3)
			if err != nil {
				t.Log.Errorf("Failed to fetch tag %s", err)
				continue // Don't return, as this could make the jobs channel get blockedj
			}
			tag := tagInf.(*tags.Tag)

			t.tagMapLock.Lock()
			currEntry, ok := t.tagMap[id]
			if ok && currEntry.TagName != tag.Name {
				// Tag name has changed
				t.resetTag(tag.ID)
			}
			t.tagMap[tag.ID] = &tagEntry{
				TagName: tag.Name,
				CatID:   tag.CategoryID,
			}
			t.tagMapLock.Unlock()
		}
	}

	jobs := make(chan int, 100)
	// For 1000 tags
	// 1 worker   - 1 min, 30 seconds
	// 10 workers - 12 seconds
	// 20 workers - 9 seconds
	for w := 1; w <= 20; w++ {
		workWg.Add(1)
		go workerFn(jobs)
	}

	for j := 0; j < len(tagIDs); j++ {
		jobs <- j
	}
	close(jobs)

	workWg.Wait()

	catList, err := tc.GetCategories(t.ClientCtx)
	if err != nil {
		t.Log.Errorf("Tags client list categories failed, %s", err.Error())
		return
	}
	t.tagMapLock.Lock()
	defer t.tagMapLock.Unlock()
	for _, cat := range catList {
		currName, ok := t.catMap[cat.ID]
		if ok && currName != cat.Name {
			// cat name has changed
			// All tags in this category need to be reset
			t.resetCategory(cat.ID, true)
		}
		t.catMap[cat.ID] = cat.Name
	}
}

func (t *tagsProbe) resetCategory(id string, holdingLock bool) {
	if !holdingLock {
		t.tagMapLock.Lock()
		defer t.tagMapLock.Unlock()
	}
	for tagID, entry := range t.tagMap {
		if entry.CatID == id {
			t.resetTag(tagID)
		}
	}
}

func (t *tagsProbe) resetTag(id string) {
	t.vmTagMapLock.Lock()
	defer t.vmTagMapLock.Unlock()
	for _, tagSet := range t.vmTagMap {
		tagSet.RemoveSingle(id)
	}
}
