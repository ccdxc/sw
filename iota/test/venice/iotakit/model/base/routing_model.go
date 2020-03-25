package base

import (
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
)

func (sm *SysModel) NewRoutingConfig(name string) *objects.RoutingConfigCollection {
	return objects.NewRoutingConfig(name, sm.ObjClient(), sm.Tb)
}

func (sm *SysModel) GetRoutingConfig(name string ) (*objects.RoutingConfig, error) {
	obj,err :=  sm.ObjClient().GetRoutingConfig (name)
	return &objects.RoutingConfig{RoutingObj: obj}, err
}

func (sm *SysModel) ListRoutingConfig() (*objects.RoutingConfigCollection, error) {
	
	objs,err := sm.ObjClient().ListRoutingConfig ()
	rcc := objects.RoutingConfigCollection{}

	for _, rc := range objs {
		rcc.RoutingObjs = append(rcc.RoutingObjs, &objects.RoutingConfig{RoutingObj: rc})
	}
	rcc.CollectionCommon.Client = sm.ObjClient()
	rcc.CollectionCommon.Testbed = sm.Tb
	return &rcc, err
}