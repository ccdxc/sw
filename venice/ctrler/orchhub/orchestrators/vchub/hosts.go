package vchub

import (
	"reflect"
	"sort"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/ref"
	conv "github.com/pensando/sw/venice/utils/strconv"
)

func (v *VCHub) handleHost(m defs.VCEventMsg) {
	meta := &api.ObjectMeta{
		Name: utils.CreateGlobalKey(m.Originator, m.DC, m.Key),
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

	toDelete := false
	for _, prop := range m.Changes {
		switch getStoreOp(prop.Op) {
		case defs.VCOpDelete:
			toDelete = true
			break
		case defs.VCOpSet:
			hConfig, ok := prop.Val.(types.HostConfigInfo)
			if !ok {
				v.Log.Errorf("Failed to cast to HostConfigInfo. Prop Name: %s", prop.Name)
				return
			}

			nwInfo := hConfig.Network
			if nwInfo == nil {
				v.Log.Errorf("Missing hConfig.Network")
				return
			}

			var DSCs []cluster.DistributedServiceCardID
			for _, pnic := range nwInfo.Pnic {
				// TODO check for vendor field to identify Pensando NICs
				macStr, err := conv.ParseMacAddr(pnic.Mac)
				if err != nil {
					continue
				}
				DSCs = append(DSCs, cluster.DistributedServiceCardID{
					MACAddress: macStr,
				})
				// TODO : Currently we do not allow more than one Pnics per host to be added to Venice.
				break
			}

			// Sort before storing, so that if we receive the Pnics
			// in a different order later we don't generate
			// an unnecessary update
			sort.Slice(DSCs, func(i, j int) bool {
				return DSCs[i].MACAddress < DSCs[j].MACAddress
			})
			hostObj.Spec.DSCs = DSCs
		}
	}

	if toDelete {
		if existingHost == nil {
			return
		}
		// Delete from apiserver
		v.StateMgr.Controller().Host().Delete(hostObj)
		return
	}
	// If different, write to apiserver
	if reflect.DeepEqual(hostObj, existingHost) {
		// Nothing to do
		return
	}

	if hostObj.Labels == nil {
		hostObj.Labels = make(map[string]string)
	}

	if v.OrchConfig != nil {
		utils.AddOrchNameLabel(hostObj.Labels, v.OrchConfig.Name)
	}

	if existingHost == nil {
		v.StateMgr.Controller().Host().Create(hostObj)
	} else {
		v.StateMgr.Controller().Host().Update(hostObj)
	}
}
