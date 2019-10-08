package store

import (
	"context"
	"net/http"
	"reflect"
	"sort"

	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch/vchub/defs"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

func (v *VCHStore) handleHost(m defs.StoreMsg) {
	meta := &api.ObjectMeta{
		// TODO: prefix the key
		Name: createGlobalKey(m.Originator, m.Key),
		// TODO: Don't use default tenant
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
	var hostObj *cluster.Host
	existingHost, err := v.apicl.ClusterV1().Host().Get(context.Background(), meta)
	if err != nil {
		// 404 object not found error is ok
		apiStatus := apierrors.FromError(err)
		if apiStatus.GetCode() != http.StatusNotFound {
			log.Errorf("Call to get workload failed: %v", apiStatus)
			// TODO: Add retry, maybe push back on to inbox channel
			return
		}
		existingHost = nil
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
				log.Errorf("Failed to cast to HostConfigInfo. Prop Name: %s", prop.Name)
				return
			}

			nwInfo := hConfig.Network
			if nwInfo == nil {
				log.Errorf("Missing hConfig.Network")
				return
			}

			var DSCs []cluster.DistributedServiceCardID
			for _, pnic := range nwInfo.Pnic {
				// TODO check for vendor field to identify Pensando NICs
				DSCs = append(DSCs, cluster.DistributedServiceCardID{
					MACAddress: pnic.Mac,
				})
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
		v.apicl.ClusterV1().Host().Delete(context.Background(), &hostObj.ObjectMeta)
		return
	}
	// If different, write to apiserver
	if reflect.DeepEqual(hostObj, existingHost) {
		// Nothing to do
		return
	}
	if existingHost == nil {
		v.apicl.ClusterV1().Host().Create(context.Background(), hostObj)
	} else {
		v.apicl.ClusterV1().Host().Update(context.Background(), hostObj)
	}

}
