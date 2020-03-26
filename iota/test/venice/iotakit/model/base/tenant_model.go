package base

import "github.com/pensando/sw/iota/test/venice/iotakit/model/objects"


func (sm *SysModel) NewTenant(name string) *objects.Tenant {
	return objects.NewTenant(name, sm.ObjClient(), sm.Tb)
}

func (sm *SysModel) GetTenant(name string) (*objects.Tenant, error) {
	obj, err := sm.ObjClient().GetTenant(name)

	return &objects.Tenant{Tenant:obj}, err
}
