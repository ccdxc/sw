package vchub

import (
	"context"
	"fmt"
	"reflect"
	"sort"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ref"
	conv "github.com/pensando/sw/venice/utils/strconv"
)

func (v *VCHub) handleHost(m defs.VCEventMsg) {
	v.Log.Infof("Got handle host event for host %s in DC %s", m.Key, m.DcID)
	penDC := v.GetDC(m.DcName)
	if penDC == nil {
		return
	}

	meta := &api.ObjectMeta{
		Name: createHostName(m.Originator, m.DcID, m.Key),
	}
	var existingHost, hostObj *cluster.Host
	ctkitHost, err := v.StateMgr.Controller().Host().Find(meta)
	if err != nil {
		existingHost = nil
	} else {
		existingHost = &ctkitHost.Host
	}

	if existingHost == nil {
		hostObj = &cluster.Host{
			TypeMeta: api.TypeMeta{
				Kind:       "Host",
				APIVersion: "v1",
			},
			ObjectMeta: *meta,
		}
	} else {
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

	var hConfig *types.HostConfigInfo
	var dispName string
	for _, prop := range m.Changes {
		switch defs.VCProp(prop.Name) {
		case defs.HostPropConfig:
			propConfig, _ := prop.Val.(types.HostConfigInfo)
			hConfig = &propConfig
		case defs.HostPropName:
			dispName = prop.Val.(string)
			// Try to update vmkworkloadname incase we did not get the name info
			// during create
			penDC.addHostNameKey(dispName, m.Key)
			v.updateVmkWorkloadLabels(hostObj.Name, m.DcName, dispName)
		default:
			v.Log.Errorf("host prop change %s - not handled", prop.Name)
		}
	}
	// TODO: check if name changed - need to find and delete old Name2Key entry
	// Name of a host cannot change easily (used for DNS resolution etc), not
	// handled rightnow
	if hConfig == nil {
		// cover the case where config was received first followed by name property
		if dispName != "" && existingHost != nil {
			if existingHost.Labels == nil {
				existingHost.Labels = make(map[string]string)
			}
			v.Log.Infof("Adding display-name label %s on host %s", dispName, existingHost.Name)
			addNameLabel(existingHost.Labels, dispName)
			v.StateMgr.Controller().Host().Update(existingHost)
		}
		v.Log.Debugf("No Config change for the host")
		return
	}

	if dispName == "" {
		// see if received it before config property
		dispName, _ = penDC.findHostNameByKey(m.Key)
	}

	nwInfo := hConfig.Network
	if nwInfo == nil {
		v.Log.Errorf("Missing hConfig.Network")
		return
	}

	var penDVS *PenDVS
	for _, dvsProxy := range nwInfo.ProxySwitch {
		v.Log.Debugf("Proxy switch %s", dvsProxy.DvsName)
		penDVS = penDC.GetDVS(dvsProxy.DvsName)
		if penDVS != nil {
			break
		}
	}
	if penDVS == nil {
		// This host in not on pen-dvs, ignore it
		v.Log.Infof("Host %s is not added to pensando DVS - ignored", m.Key)
		return
	}

	var DSCs []cluster.DistributedServiceCardID
	// sort pnics based on MAC address to find/use correct
	// MAC for DSC idenitification. the management MAC is numerically highest MAC addr
	// DSC objects use the base (lowest) mac address
	sort.Slice(nwInfo.Pnic, func(i, j int) bool {
		return nwInfo.Pnic[i].Mac < nwInfo.Pnic[j].Mac
	})

	for _, pnic := range nwInfo.Pnic {
		macStr, err := conv.ParseMacAddr(pnic.Mac)
		if err != nil {
			v.Log.Errorf("Failed to parse Mac, %s", err)
			continue
		}
		if !netutils.IsPensandoMACAddress(macStr) {
			continue
		}
		DSCs = append(DSCs, cluster.DistributedServiceCardID{
			MACAddress: macStr,
		})
		// TODO : Currently we do not allow more than one DSC per host to be added to Venice.
		// Currently hConfig.Network.ProxySwitch[].Pnic[] is not checked to see if the host is
		// connected to penDVS or not. When multiple DSCs are present, EP->DSC mapping gets tricky
		// it depends on how PG's uplink teaming is configured
		break
	}

	if len(DSCs) == 0 {
		// Not a pensando host
		v.Log.Infof("Host %s is ignored - not a Pensando host", m.Key)
		return
	}

	// Sort before storing, so that if we receive the Pnics
	// in a different order later we don't generate
	// an unnecessary update
	sort.Slice(DSCs, func(i, j int) bool {
		return DSCs[i].MACAddress < DSCs[j].MACAddress
	})
	hostObj.Spec.DSCs = DSCs

	if hostObj.Labels == nil {
		hostObj.Labels = make(map[string]string)
	}
	if existingHost == nil && v.OrchConfig != nil {
		utils.AddOrchNameLabel(hostObj.Labels, v.OrchConfig.Name)
		if hostObj.Labels == nil {
			hostObj.Labels = make(map[string]string)
		}
		if dispName == "" {
			v.Log.Infof("Host %s is created without name property", hostObj.Name)
		}
	}
	if dispName != "" {
		addNameLabel(hostObj.Labels, dispName)
	}
	utils.AddOrchNamespaceLabel(hostObj.Labels, penDC.Name)

	if existingHost == nil {
		v.Log.Infof("Creating host %s", hostObj.Name)
		err := v.StateMgr.Controller().Host().Create(hostObj)
		if err != nil {
			err = v.fixStaleHost(hostObj)
		}
		if err == nil {
			v.pCache.RevalidateKind(workloadKind)
		} else {
			v.Log.Infof("Host create failed for %s - %s", hostObj.Name, err)
		}
	}

	v.syncHostVmkNics(penDC, penDVS, dispName, m.Key, hConfig)

	// If different, write to apiserver
	if reflect.DeepEqual(hostObj, existingHost) {
		// Nothing to do
		v.Log.Debugf("host event ignored - nothing changed")
		return
	}

	if existingHost != nil {
		v.Log.Infof("Updating host %s", hostObj.Name)
		v.StateMgr.Controller().Host().Update(hostObj)
		// Revalidate kind call is not needed here.
		// Only thing that can be updated in a host is the labels
		// once it is written
	}
}

func (v *VCHub) fixStaleHost(host *cluster.Host) error {
	// check if there is another host with the same MACAddr (DSC)
	// If that host belongs to this VC it is likely that vcenter host-id changed for some
	// reason, delete that host and create new one.
	// TODO: If host was moved from one VCenter to another, we can just check if it has
	// some VC association and do the same?? (linked VC case)
	// List hosts
	opts := api.ListWatchOptions{}
	hosts, err := v.StateMgr.Controller().Host().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("fixStaleHost Failed to get host list. Err : %v", err)
		return err
	}
	var hostFound *cluster.Host
searchHosts:
	for _, eh := range hosts {
		for i, dsc := range eh.Spec.DSCs {
			if i >= len(host.Spec.DSCs) {
				continue
			}
			if dsc.MACAddress == host.Spec.DSCs[i].MACAddress {
				hostFound = &eh.Host
				break searchHosts
			}
		}
	}
	if hostFound == nil {
		return fmt.Errorf("No duplicate Host entry found")
	}

	vcID, ok := hostFound.Labels[utils.OrchNameKey]
	if !ok {
		return fmt.Errorf("Duplicate Host %s is being used by non-VC application", hostFound.Name)
	}

	if !utils.IsObjForOrch(hostFound.Labels, v.VcID, "") {
		// host found, but not used by this VC
		v.Log.Infof("Deleting host that belonged to another VC %s", vcID)
	}
	v.deleteHost(hostFound)
	err = v.StateMgr.Controller().Host().Create(host)
	return err
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
	hostName, ok := obj.Labels[NameKey]
	if penDC != nil && ok {
		if hKey, ok := penDC.findHostKeyByName(hostName); ok {
			// Delete vmkworkload
			vmkWlName := createVmkWorkloadName(v.VcID, penDC.dcRef.Value, hKey)
			v.deleteWorkloadByName(vmkWlName)
			penDC.delHostNameKey(hostName)
		}
	}
	v.Log.Infof("Deleting host %s", obj.Name)
	// Delete from apiserver
	if err := v.StateMgr.Controller().Host().Delete(obj); err != nil {
		v.Log.Errorf("Could not delete host from Venice %s", obj.Name)
	}
	return
}

// DeleteHosts deletes all host objects from API server for this VCHub instance
func (v *VCHub) DeleteHosts() {
	// List hosts
	opts := api.ListWatchOptions{}
	hosts, err := v.StateMgr.Controller().Host().List(context.Background(), &opts)
	if err != nil {
		v.Log.Errorf("Failed to get host list. Err : %v", err)
	}
	for _, host := range hosts {
		if !utils.IsObjForOrch(host.Labels, v.VcID, "") {
			// Filter out hosts not for this Orch
			v.Log.Debugf("Skipping host %s", host.Name)
			continue
		}
		// Delete host
		v.deleteHost(&host.Host)
	}
}

func (v *VCHub) findHost(hostName string) *cluster.Host {
	meta := &api.ObjectMeta{
		Name: hostName,
	}
	ctkitHost, err := v.StateMgr.Controller().Host().Find(meta)
	if err != nil {
		return nil
	}
	return &ctkitHost.Host
}

func (v *VCHub) getDCNameForHost(hostName string) string {
	hostObj := v.findHost(hostName)
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
