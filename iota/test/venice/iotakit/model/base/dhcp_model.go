package base

import (
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

func (sm *SysModel) NewIPAMPolicy(name string, tenant string, vrf string, ip string) *objects.IPAMPolicyCollection {
	return objects.NewIPAMPolicy(name, tenant, vrf, ip, sm.ObjClient(), sm.Tb)
}

func (sm *SysModel) ListIPAMPolicy(tenant string) (*objects.IPAMPolicyCollection, error) {

	objs, err := sm.ObjClient().ListIPAMPolicy(tenant)
	ipc := objects.IPAMPolicyCollection{}

	for _, ip := range objs {
		ipc.PolicyObjs = append(ipc.PolicyObjs, &objects.IPAMPolicy{PolicyObj: ip})
	}
	ipc.CollectionCommon.Client = sm.ObjClient()
	ipc.CollectionCommon.Testbed = sm.Tb
	return &ipc, err
}

func (sm *SysModel) GetIPAMPolicy(name string, tenant string) (*objects.IPAMPolicy, error) {
	obj, err := sm.ObjClient().GetIPAMPolicy(name, tenant)
	return &objects.IPAMPolicy{PolicyObj: obj}, err
}
