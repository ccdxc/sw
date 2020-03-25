package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// RoutingConfig
type RoutingConfig struct {
	RoutingObj *network.RoutingConfig
}

type RoutingConfigCollection struct {
	CollectionCommon
	err         error
	RoutingObjs []*RoutingConfig
}

func NewRoutingConfig(name string, client objClient.ObjClient, testbed *testbed.TestBed) *RoutingConfigCollection {
	return &RoutingConfigCollection{
		RoutingObjs: []*RoutingConfig{
			{
				RoutingObj: &network.RoutingConfig{
					TypeMeta:   api.TypeMeta{Kind: "RoutingConfig"},
					ObjectMeta: api.ObjectMeta{Name: name},
					Spec: network.RoutingConfigSpec{
						BGPConfig: &network.BGPConfig{
							RouterId: "0.0.0.0",
							ASNumber: 100,
							Neighbors: []*network.BGPNeighbor{
								{
									IPAddress: "0.0.0.0",
									RemoteAS:  600,
									EnableAddressFamilies: []string{
										"ipv4-unicast",
									},
								},
								{
									IPAddress: "0.0.0.0",
									RemoteAS:  100,
									MultiHop:  10,
									EnableAddressFamilies: []string{
										"l2vpn-evpn",
									},
								},
							},
						},
					},
				},
			},
		},
		CollectionCommon: CollectionCommon{Testbed: testbed, Client: client},
	}
}

// Commit writes the RoutingConfig to Venice
func (rcc *RoutingConfigCollection) Commit() error {
	if rcc.err != nil {
		return rcc.err
	}

	for _, v := range rcc.RoutingObjs {
		err := rcc.Client.CreateRoutingConfig(v.RoutingObj)
		if err != nil {
			rcc.err = err
			log.Infof("Creating Routing Config failed %v", err)
			return err
		}
	}
	return nil
}

// Delete deletes all Routing Configs in the collection
func (rcc *RoutingConfigCollection) Delete() error {
	if rcc.err != nil {
		return rcc.err
	}

	//walk all configs and delete them
	for _, v := range rcc.RoutingObjs {
		err := rcc.Client.DeleteRoutingConfig(v.RoutingObj)
		if err != nil {
			return err
		}
	}
	return nil
}
