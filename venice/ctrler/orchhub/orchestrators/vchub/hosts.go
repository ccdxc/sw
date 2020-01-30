package vchub

import (
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
	v.Log.Debugf("Got handle host event")
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

	penDC := v.GetDC(m.DcName)
	var hConfig *types.HostConfigInfo

	for _, prop := range m.Changes {
		configProp, ok := prop.Val.(types.HostConfigInfo)
		if !ok {
			v.Log.Errorf("Failed to cast to HostConfigInfo. Prop Name: %s", prop.Name)
			return
		}
		hConfig = &configProp
	}
	if hConfig == nil {
		return
	}

	nwInfo := hConfig.Network
	if nwInfo == nil {
		v.Log.Errorf("Missing hConfig.Network")
		return
	}

	var penDVS *PenDVS
	for _, dvsProxy := range nwInfo.ProxySwitch {
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
	}

	if existingHost == nil {
		v.Log.Infof("Creating host %s", hostObj.Name)
		v.StateMgr.Controller().Host().Create(hostObj)
		v.pCache.RevalidateKind(workloadKind)
	}

	// TODO the host name is not correct here.. fix it
	v.syncHostVmkNics(penDC, penDVS, m.Key, hostObj.Name, hConfig)

	// If different, write to apiserver
	if reflect.DeepEqual(hostObj, existingHost) {
		// Nothing to do
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

func (v *VCHub) deleteHost(obj *cluster.Host) {
	// Delete from apiserver
	v.StateMgr.Controller().Host().Delete(obj)
	return
}

func (v *VCHub) findHostByName(hostName string) *cluster.Host {
	meta := &api.ObjectMeta{
		Name: hostName,
	}
	ctkitHost, err := v.StateMgr.Controller().Host().Find(meta)
	if err != nil {
		return nil
	}
	return &ctkitHost.Host
}
