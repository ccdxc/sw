package vchub

import (
	"fmt"
	"strings"
	"time"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/utils/events/recorder"
)

func (v *VCHub) periodicTagSync() {
	defer v.Wg.Done()
	for v.Ctx.Err() == nil {
		v.tagSync()
		select {
		case <-v.Ctx.Done():
			return
		case <-time.After(v.tagSyncDelay):
		}
	}
}

// returns whether sync executed
func (v *VCHub) tagSync() {
	v.Log.Infof("VCHub tag sync running")
	count := 3
	for !v.probe.IsSessionReady() && count > 0 {
		select {
		case <-v.Ctx.Done():
			return
		case <-time.After(1 * time.Second):
			count--
		}
	}
	if count == 0 {
		v.Log.Errorf("Probe session isn't connected")
		return
	}
	// Verify base tags are written
	done := v.probe.SetupBaseTags() // Should this function be here?
	v.Log.Debugf("VCHub setup base tags returned %v", done)
	if v.Ctx.Err() != nil || !done {
		return
	}

	idToName := map[string]string{}

	managedObj := []types.ManagedObjectReference{}
	vlanTagMap := map[string]int{}
	v.DcMapLock.Lock()
	for _, dc := range v.DcMap {
		managedObj = append(managedObj, dc.dcRef)
		idToName[dc.dcRef.Value] = dc.Name
		dc.Lock()
		for _, dvs := range dc.DvsMap {
			managedObj = append(managedObj, dvs.DvsRef)
			idToName[dvs.DvsRef.Value] = dvs.DvsName
			dvs.Lock()
			for _, pg := range dvs.Pgs {
				managedObj = append(managedObj, pg.PgRef)
				idToName[pg.PgRef.Value] = pg.PgName
				nw, err := v.StateMgr.Controller().Network().Find(&pg.NetworkMeta)
				if err == nil {
					externalVlan := int(nw.Spec.VlanID)
					vlanTagMap[pg.PgRef.Value] = externalVlan
				} else {
					v.Log.Errorf("Couldn't tag PG %s with vlan tag since we couldn't find the network info: networkMeta %v, err %s", pg.PgName, pg.NetworkMeta, err)
				}
			}
			dvs.Unlock()
		}
		dc.Unlock()
	}
	v.DcMapLock.Unlock()

	// Get pensando tags on all managed objects
	tagMap, err := v.probe.GetPensandoTagsOnObjects(managedObj)
	if err != nil {
		v.Log.Errorf("Failed to get pensado tags, err %s", err)
		return
	}
	toTagAsManaged := []types.ManagedObjectReference{}
	toDel := map[string][]types.ManagedObjectReference{}

	// Generate list of tags to write/delete
	for _, obj := range managedObj {
		kind := obj.Type
		id := obj.Value
		kindMap, ok := tagMap[kind]
		if !ok {
			continue
		}

		tags, ok := kindMap[id]
		if !ok {
			continue
		}

		managed := false
		for _, tag := range tags {
			if v.probe.IsManagedTag(tag) {
				managed = true
			} else if strings.HasPrefix(tag, defs.VCTagManagedDefault) {
				objName := idToName[obj.Value]
				v.Log.Errorf("Found tag %s from another PSM on our object %s %s, raising event...", tag, objName, obj.Value)
				// Another Venice is/has managed this DC
				evt := eventtypes.ORCH_ALREADY_MANAGED
				msg := fmt.Sprintf("Found tag %s on object %s. This namespace might be managed by another PSM.", tag, objName)
				recorder.Event(evt, msg, v.State.OrchConfig)
			}
			vlan, ok := v.probe.IsVlanTag(tag)
			if ok {
				if kind != string(defs.DistributedVirtualPortgroup) {
					toDel[tag] = append(toDel[tag], obj)
				} else {
					expVlan, ok := vlanTagMap[id]
					if !ok {
						v.Log.Errorf("unable to set vlan tag for PG %s", expVlan)
					}
					if ok && vlan == vlanTagMap[id] {
						// Tag already there
						delete(vlanTagMap, id)
					} else {
						toDel[tag] = append(toDel[tag], obj)
					}
				}
			}
		}
		if !managed {
			toTagAsManaged = append(toTagAsManaged, obj)
		}
	}

	v.Log.Debugf("Tagging as managed %v", toTagAsManaged)
	// Pensando managed tag
	v.probe.TagObjsAsManaged(toTagAsManaged)

	// Write missing vlan tags
	for id, vlan := range vlanTagMap {
		ref := types.ManagedObjectReference{
			Type:  string(defs.DistributedVirtualPortgroup),
			Value: id,
		}
		err = v.probe.TagObjWithVlan(ref, vlan)
		if err != nil {
			v.Log.Errorf("tagging PG %s with vlan %d failed: %s", id, vlan, err)
		}
	}

	// Delete extra vlan tags
	for tag, refs := range toDel {
		// There shouldn't really ever be more than one item in this list
		// No need to write removeTagFromMultipleObjects function
		for _, r := range refs {
			err := v.probe.RemoveTag(r, tag)
			v.Log.Infof("Deleting tag %s on obj %s returned %s", tag, r.Value, err)
		}
	}
}
