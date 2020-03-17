package vchub

import (
	"fmt"
	"sync"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/useg"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/vcprobe"
	"github.com/pensando/sw/venice/utils/events/recorder"
)

// PenDC represents an instance of a Datacenter
type PenDC struct {
	sync.Mutex
	*defs.State
	Name  string
	dcRef types.ManagedObjectReference
	// Map from dvs display name to PenDVS inside this DC
	DvsMap       map[string]*PenDVS
	probe        vcprobe.ProbeInf
	HostName2Key map[string]string
}

// NewPenDC creates a new penDC object
func (v *VCHub) NewPenDC(dcName, dcID string) (*PenDC, error) {
	v.DcMapLock.Lock()
	defer v.DcMapLock.Unlock()

	var dc *PenDC
	// TODO: DC name might change, in that case lookp using ID
	if dcExisting, ok := v.DcMap[dcName]; ok {
		dc = dcExisting
	} else {
		dcRef := types.ManagedObjectReference{
			Type:  string(defs.Datacenter),
			Value: dcID,
		}
		dc = &PenDC{
			State:        v.State,
			probe:        v.probe,
			Name:         dcName,
			dcRef:        dcRef,
			DvsMap:       map[string]*PenDVS{},
			HostName2Key: map[string]string{},
		}
		v.DcMap[dcName] = dc
		if v.DcID2NameMap == nil {
			v.DcID2NameMap = map[string]string{}
		}
		v.DcID2NameMap[dcID] = dcName
		err := v.probe.TagObjAsManaged(dcRef)
		if err != nil {
			v.Log.Errorf("Failed to tag DC as managed, %s", err)
			// Error isn't worth failing the operation for
		}
	}

	dvsName := CreateDVSName(dcName)

	// Create a pen dvs
	// Pvlan allocations on the dvs
	pvlanConfigSpecArray := []types.VMwareDVSPvlanConfigSpec{}

	// Setup all the pvlan allocations now
	// vlans 0 and 1 are reserved
	for i := useg.FirstPGVlan; i < useg.FirstUsegVlan; i += 2 {
		PvlanEntryProm := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "promiscuous",
			SecondaryVlanId: int32(i),
		}
		pvlanMapEntry := types.VMwareDVSPvlanMapEntry{
			PrimaryVlanId:   int32(i),
			PvlanType:       "isolated",
			SecondaryVlanId: int32(i + 1),
		}
		pvlanSpecProm := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: PvlanEntryProm,
			Operation:  "add",
		}
		pvlanSpec := types.VMwareDVSPvlanConfigSpec{
			PvlanEntry: pvlanMapEntry,
			Operation:  "add",
		}
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpecProm)
		pvlanConfigSpecArray = append(pvlanConfigSpecArray, pvlanSpec)
	}

	// TODO: Set number of uplinks
	var spec types.DVSCreateSpec
	spec.ConfigSpec = &types.VMwareDVSConfigSpec{
		PvlanConfigSpec: pvlanConfigSpecArray,
	}
	spec.ConfigSpec.GetDVSConfigSpec().Name = dvsName
	spec.ProductInfo = new(types.DistributedVirtualSwitchProductSpec)
	spec.ProductInfo.Version = "6.5.0"
	err := dc.AddPenDVS(&spec)
	if err != nil {
		evtMsg := fmt.Sprintf("Failed to create DVS in Datacenter %s. Network configuration cannot be pushed.", dcName)
		v.Log.Errorf(evtMsg)

		recorder.Event(eventtypes.ORCH_CONFIG_PUSH_FAILURE, evtMsg, v.State.OrchConfig)
		return nil, err
	}

	return dc, nil
}

// GetDC returns the DC by display name
func (v *VCHub) GetDC(name string) *PenDC {
	v.DcMapLock.Lock()
	defer v.DcMapLock.Unlock()
	dc, ok := v.DcMap[name]
	if !ok {
		return nil
	}
	return dc
}

// GetDCFromID returns the DC by vcenter DcID
func (v *VCHub) GetDCFromID(dcID string) *PenDC {
	v.DcMapLock.Lock()
	defer v.DcMapLock.Unlock()
	dcName, ok := v.DcID2NameMap[dcID]
	if !ok {
		return nil
	}
	if dc, ok := v.DcMap[dcName]; ok {
		return dc
	}
	return nil
}

// AddPG adds a PG to all DVS in this DC, unless dvsName is not blank
func (d *PenDC) AddPG(pgName string, networkMeta api.ObjectMeta, dvsName string) []error {
	d.Lock()
	defer d.Unlock()
	var errs []error
	for _, penDVS := range d.DvsMap {
		if dvsName == "" || dvsName == penDVS.DvsName {
			err := penDVS.AddPenPG(pgName, networkMeta)
			if err != nil {
				d.Log.Errorf("Create PG for dvs %s returned err %s", penDVS.DvsName, err)
				errs = append(errs, err)
			}
		}
	}
	return errs
}

// GetDVS returns the DVS with matching name
func (d *PenDC) GetDVS(dvsName string) *PenDVS {
	d.Lock()
	defer d.Unlock()
	dvs, ok := d.DvsMap[dvsName]
	if !ok {
		return nil
	}
	return dvs
}

// GetPG returns the pg with the matching name. Looks thorugh all
// DVS unless dvsName is supplied
func (d *PenDC) GetPG(pgName string, dvsName string) *PenPG {
	d.Lock()
	defer d.Unlock()
	for _, penDVS := range d.DvsMap {
		if dvsName == "" || dvsName == penDVS.DvsName {
			pg := penDVS.GetPenPG(pgName)
			if pg != nil {
				return pg
			}
		}
	}
	return nil
}

// GetPGByID returns the Pen PG object with the given vcenter ID
func (d *PenDC) GetPGByID(pgID string) *PenPG {
	d.Lock()
	defer d.Unlock()
	for _, penDVS := range d.DvsMap {
		pg := penDVS.GetPenPGByID(pgID)
		if pg != nil {
			return pg
		}
	}
	return nil
}

// RemovePG removes a PG from all DVS in this DC, unless dvsName is not blank
// Returns a map from dvs name to number of PG allocations still available
func (d *PenDC) RemovePG(pgName string, dvsName string) (map[string]int, []error) {
	d.Log.Infof("Removing PG %s...", pgName)
	d.Lock()
	defer d.Unlock()
	var errs []error
	for _, penDVS := range d.DvsMap {
		d.Log.Debugf("Removing PG %s in dvs %s", pgName, penDVS.DvsName)
		if dvsName == "" || dvsName == penDVS.DvsName {
			err := penDVS.RemovePenPG(pgName)
			if err != nil {
				d.Log.Errorf("Delete PG %s for dvs %s returned err %s", pgName, penDVS.DvsName, err)
				errs = append(errs, err)
				continue
			}
			err = penDVS.UsegMgr.ReleaseVlansForPG(pgName)
			d.Log.Errorf("Delete PG %s for dvs %s returned err %s", pgName, penDVS.DvsName, err)
			errs = append(errs, err)
		}
	}
	d.Log.Debugf("Removing PG %s completed with errs: %v", pgName, errs)
	remainingAlloc := d.getRemainingPGAllocations(dvsName)
	return remainingAlloc, errs
}

func (d *PenDC) getRemainingPGAllocations(dvsName string) map[string]int {
	ret := map[string]int{}
	for _, penDVS := range d.DvsMap {
		if dvsName == "" || dvsName == penDVS.DvsName {
			ret[penDVS.DvsName] = penDVS.UsegMgr.GetRemainingPGCount()
		}
	}
	return ret
}

func (d *PenDC) addHostNameKey(name, key string) {
	d.Lock()
	defer d.Unlock()
	d.HostName2Key[name] = key
}

func (d *PenDC) delHostNameKey(name string) {
	d.Lock()
	defer d.Unlock()
	if _, ok := d.HostName2Key[name]; ok {
		delete(d.HostName2Key, name)
	}
}

func (d *PenDC) findHostKeyByName(name string) (string, bool) {
	d.Lock()
	defer d.Unlock()
	key, ok := d.HostName2Key[name]
	return key, ok
}

func (d *PenDC) findHostNameByKey(key string) (string, bool) {
	d.Lock()
	defer d.Unlock()
	for name, k := range d.HostName2Key {
		if k == key {
			return name, true
		}
	}
	return "", false
}
