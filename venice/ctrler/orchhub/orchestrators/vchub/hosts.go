package vchub

import (
	"fmt"
	"sort"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ref"
	conv "github.com/pensando/sw/venice/utils/strconv"
)

func (v *VCHub) penDVSForVcHost(dcName string, hConfig *types.HostConfigInfo) (*PenDVS, []string) {
	var pnics []string
	if hConfig == nil || hConfig.Network == nil {
		return nil, pnics
	}
	penDC := v.GetDC(dcName)
	if penDC == nil {
		return nil, pnics
	}
	var penDVS *PenDVS
	for _, dvsProxy := range hConfig.Network.ProxySwitch {
		v.Log.Debugf("Proxy switch %s", dvsProxy.DvsName)
		penDVS = penDC.GetDVS(dvsProxy.DvsName)
		if penDVS != nil {
			pnics = dvsProxy.Pnic
			break
		}
	}
	return penDVS, pnics
}

func (v *VCHub) handleHost(m defs.VCEventMsg) {
	v.Log.Infof("Got handle host event for host %s in DC %s", m.Key, m.DcID)
	penDC := v.GetDC(m.DcName)
	if penDC == nil {
		return
	}

	meta := &api.ObjectMeta{
		Name: v.createHostName(m.DcID, m.Key),
	}
	existingHost := v.pCache.GetHost(meta)
	var hostObj *cluster.Host

	if existingHost == nil {
		v.Log.Debugf("This is a new host - %s", meta.Name)
		hostObj = &cluster.Host{
			TypeMeta: api.TypeMeta{
				Kind:       "Host",
				APIVersion: "v1",
			},
			ObjectMeta: *meta,
		}
	} else {
		// Copying to prevent modifying of ctkit state
		temp := ref.DeepCopy(*existingHost).(cluster.Host)
		hostObj = &temp
	}

	if m.UpdateType == types.ObjectUpdateKindLeave {
		// Object was deleted
		if existingHost == nil {
			// Don't have object we received delete for
			return
		}
		v.deleteHost(hostObj)
		return
	}

	if hostObj.Labels == nil {
		hostObj.Labels = map[string]string{}
	}

	utils.AddOrchNameLabel(hostObj.Labels, v.OrchConfig.Name)
	utils.AddOrchNamespaceLabel(hostObj.Labels, m.DcName)

	var hConfig *types.HostConfigInfo
	var dispName string
	for _, prop := range m.Changes {
		switch defs.VCProp(prop.Name) {
		case defs.HostPropConfig:
			propConfig, _ := prop.Val.(types.HostConfigInfo)
			hConfig = &propConfig
			v.processHostConfig(prop, m.DcID, m.DcName, hostObj)
		case defs.HostPropName:
			dispName = prop.Val.(string)
			// Try to update vmkworkloadname incase we did not get the name info
			// during create
			penDC.addHostNameKey(dispName, m.Key)
			v.updateVmkWorkloadLabels(hostObj.Name, m.DcName, dispName)
			addNameLabel(hostObj.Labels, dispName)
		default:
			v.Log.Errorf("host prop change %s - not handled", prop.Name)
		}
	}

	if existingHost != nil && len(hostObj.Spec.DSCs) == 0 {
		// host was removed from Pen-DVS, cleanup any workloads on it and
		// delete the host
		v.Log.Errorf("Removing host %s and all workloads on it", hostObj.Name)
		v.hostRemovedFromDVS(existingHost)
	}

	if dispName == "" {
		// see if received it before config property
		dispName, _ = penDC.findHostNameByKey(m.Key)
	}

	if existingHost == nil {
		v.Log.Infof("Creating host %s", hostObj.Name)
		// Check if there are any stale hosts with the same DSC
		v.fixStaleHost(hostObj)
		v.pCache.Set(string(cluster.KindHost), hostObj)
		v.pCache.RevalidateKind(string(workload.KindWorkload))
	}

	penDVS, _ := v.penDVSForVcHost(m.DcName, hConfig)

	v.syncHostVmkNics(penDC, penDVS, dispName, m.Key, hConfig)

	if existingHost != nil {
		v.Log.Infof("Updating host %s", hostObj.Name)
		v.pCache.Set(string(cluster.KindHost), hostObj)
		// Revalidate kind call is not needed here.
		// Only thing that can be updated in a host is the labels
		// once it is written
	}
}

func (v *VCHub) validateHost(in interface{}) (bool, bool) {
	obj, ok := in.(*cluster.Host)
	if !ok {
		return false, false
	}
	if len(obj.Spec.DSCs) == 0 {
		v.Log.Errorf("host %s has no DSC", obj.GetObjectMeta().Name)
		return false, false
	}
	return true, true
}

func (v *VCHub) processHostConfig(prop types.PropertyChange, dcID string, dcName string, hostObj *cluster.Host) {
	propConfig, _ := prop.Val.(types.HostConfigInfo)
	hConfig := &propConfig

	hostObj.Spec.DSCs = []cluster.DistributedServiceCardID{}

	penDVS, pNicsUsed := v.penDVSForVcHost(dcName, hConfig)
	if penDVS == nil {
		// This host in not on pen-dvs, ignore it
		v.Log.Infof("Host %s is not added to pensando DVS", hostObj.Name)
		return
	}

	nwInfo := hConfig.Network
	if nwInfo == nil {
		v.Log.Errorf("Missing hConfig.Network")
		return
	}
	var DSCs []cluster.DistributedServiceCardID
	// sort pnics based on MAC address to find/use correct
	// MAC for DSC idenitification. the management MAC is numerically highest MAC addr
	// DSC objects use the base (lowest) mac address
	sort.Slice(nwInfo.Pnic, func(i, j int) bool {
		return nwInfo.Pnic[i].Mac < nwInfo.Pnic[j].Mac
	})

	naplesPnicUsed := false
	for _, pnic := range nwInfo.Pnic {
		macStr, err := conv.ParseMacAddr(pnic.Mac)
		if err != nil {
			v.Log.Errorf("Failed to parse Mac, %s", err)
			continue
		}
		if !netutils.IsPensandoMACAddress(macStr) {
			continue
		}
		if len(DSCs) == 0 {
			// Only one DSC is supported per host.. for now it must be the first DSC
			// TODO: support for multiple DSCs per host
			DSCs = append(DSCs, cluster.DistributedServiceCardID{
				MACAddress: macStr,
			})
		}
		for _, pn := range pNicsUsed {
			if pn == pnic.Key {
				naplesPnicUsed = true
				break
			}
		}
	}

	if len(DSCs) == 0 {
		v.Log.Infof("Host %s is ignored - not a Pensando host", hostObj.Name)
		return
	}

	if !naplesPnicUsed {
		v.Log.Infof("Host %s is ignored - Naples Pnic is not connected to Pensando DVS", hostObj.Name)
		return
	}

	// Sort before storing, so that if we receive the Pnics
	// in a different order later we don't generate
	// an unnecessary update
	sort.Slice(DSCs, func(i, j int) bool {
		return DSCs[i].MACAddress < DSCs[j].MACAddress
	})
	hostObj.Spec.DSCs = DSCs
}

func (v *VCHub) fixStaleHost(host *cluster.Host) error {
	// check if there is another host with the same MACAddr (DSC)
	// If that host belongs to this VC it is likely that vcenter host-id changed for some
	// reason, delete that host and create new one.
	// TODO: If host was moved from one VCenter to another, we can just check if it has
	// some VC association and do the same?? (linked VC case)
	// List hosts
	hosts := v.pCache.ListHosts(v.Ctx)
	var hostFound *cluster.Host
searchHosts:
	for _, eh := range hosts {
		for i, dsc := range eh.Spec.DSCs {
			if i >= len(host.Spec.DSCs) {
				continue
			}
			if dsc.MACAddress == host.Spec.DSCs[i].MACAddress {
				hostFound = eh
				break searchHosts
			}
		}
	}
	if hostFound == nil {
		v.Log.Infof("No duplicate host found")
		return nil
	}
	var vcID string
	ok := false
	if hostFound.Labels != nil {
		vcID, ok = hostFound.Labels[utils.OrchNameKey]
	}
	if !ok {
		err := fmt.Errorf("Duplicate Host %s is being used by non-VC application", hostFound.Name)
		v.Log.Infof("%s", err)
		return err
	}

	if !utils.IsObjForOrch(hostFound.Labels, v.VcID, "") {
		// host found, but not used by this VC
		v.Log.Infof("Deleting host that belonged to another VC %s", vcID)
	} else if hostFound.Name == host.Name {
		// Host we found matches our state. Nothing to do.
		return nil
	}
	v.deleteHost(hostFound)
	return nil
}

func (v *VCHub) hostRemovedFromDVS(host *cluster.Host) {
	v.Log.Infof("Host %s Removed from Pen-DVS", host.Name)
	opts := api.ListWatchOptions{}
	wlList, err := v.StateMgr.Controller().Workload().List(v.Ctx, &opts)
	if err == nil {
		for _, wl := range wlList {
			if wl.Spec.HostName != host.Name {
				continue
			}
			v.deleteWorkload(&wl.Workload)
		}
	}
	v.deleteHost(host)
}

func (v *VCHub) deleteHost(obj *cluster.Host) {
	if obj.Labels == nil {
		// all hosts created from orchhub will have labels
		v.Log.Debugf("deleteHost - no lables")
		return
	}
	dcName, ok := obj.Labels[utils.NamespaceKey]
	if !ok {
		v.Log.Debugf("deleteHost - no namespace")
		return
	}
	penDC := v.GetDC(dcName)
	v.deleteHostFromDc(obj, penDC)

	return
}

func (v *VCHub) deleteHostFromDc(obj *cluster.Host, penDC *PenDC) {
	if obj.Labels == nil {
		// all hosts created from orchhub will have labels
		v.Log.Debugf("deleteHostFromDc - no lables")
		return
	}
	hostName, ok := obj.Labels[NameKey]
	if penDC != nil && ok {
		if hKey, ok := penDC.findHostKeyByName(hostName); ok {
			// Delete vmkworkload
			vmkWlName := v.createVmkWorkloadName(penDC.dcRef.Value, hKey)
			v.deleteWorkloadByName(vmkWlName)
			penDC.delHostNameKey(hostName)
		}
	}
	v.Log.Infof("Deleting host %s", obj.Name)
	// Delete from apiserver, but not from pcache so that we still have
	// display name in case it is re-added to dvs
	if err := v.StateMgr.Controller().Host().Delete(obj); err != nil {
		v.Log.Errorf("Could not delete host from Venice %s", obj.Name)
	}
}

// DeleteHosts deletes all host objects from API server for this VCHub instance
func (v *VCHub) DeleteHosts() {
	// List hosts
	hosts := v.pCache.ListHosts(v.Ctx)
	for _, host := range hosts {
		if !utils.IsObjForOrch(host.Labels, v.VcID, "") {
			// Filter out hosts not for this Orch
			v.Log.Debugf("Skipping host %s", host.Name)
			continue
		}
		// Delete host
		v.deleteHost(host)
	}
}

func (v *VCHub) getDCNameForHost(hostName string) string {
	hostObj := v.pCache.GetHostByName(hostName)
	if hostObj == nil {
		v.Log.Errorf("Host %s not found", hostName)
		return ""
	}
	if hostObj.Labels == nil {
		v.Log.Errorf("Host %s has no labels", hostName)
		return ""
	}
	if dcName, ok := hostObj.Labels[utils.NamespaceKey]; ok {
		return dcName
	}
	v.Log.Errorf("Host %s has no namespace label", hostName)
	return ""
}

func (v *VCHub) updateVmkWorkloadLabels(hostName, dcName, dispName string) {
	wlName := createVmkWorkloadNameFromHostName(hostName)
	workloadObj := v.getWorkload(wlName)
	if workloadObj == nil {
		return
	}
	v.addWorkloadLabels(workloadObj, dispName, dcName)
}
