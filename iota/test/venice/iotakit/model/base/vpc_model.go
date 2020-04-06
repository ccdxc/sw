package base

import "github.com/pensando/sw/iota/test/venice/iotakit/model/objects"

// NewVPC creates a new VPC
func (sm *SysModel) NewVPC(tenant string, name string, rmac string, vni uint32, ipam string) *objects.VpcObjCollection {
	return objects.NewVPC(tenant, name, rmac, vni, ipam, sm.ObjClient(), sm.Tb)
}

func (sm *SysModel) GetVPC(name string, tenant string) (*objects.Vpc, error) {
	obj, err := sm.ObjClient().GetVPC(name, tenant)

	return &objects.Vpc{Obj: obj}, err
}
