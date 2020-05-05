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

func (rcc *RoutingConfigCollection) SetPasswordOnNaples(passwd string) {
	log.Infof("Setting Naples password to %s", passwd)
	for _, v := range rcc.RoutingObjs {
		if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig {
			//Naples config
			for _, n := range v.RoutingObj.Spec.BGPConfig.Neighbors {
				if n.GetEnableAddressFamilies()[0] == "l2vpn-evpn" {
					n.Password = passwd
				}
			}
		}
	}
}

func (rcc *RoutingConfigCollection) SetPasswordOnRR(passwd string) {
	log.Infof("Setting RR password to %s", passwd)
	for _, v := range rcc.RoutingObjs {
		if v.RoutingObj.Spec.BGPConfig.DSCAutoConfig {
			//Naples config
			continue
		}
		found := false
		for _, n := range v.RoutingObj.Spec.BGPConfig.Neighbors {
			if n.DSCAutoConfig {
				found = true
				n.Password = passwd
			}
		}
		if !found {
			nbrs := v.RoutingObj.Spec.BGPConfig.GetNeighbors()

			newNbr := &network.BGPNeighbor{
				Shutdown:              false,
				IPAddress:             "",
				RemoteAS:              v.RoutingObj.Spec.BGPConfig.GetASNumber(),
				MultiHop:              10,
				EnableAddressFamilies: []string{"l2vpn-evpn"},
				Password:              passwd,
				DSCAutoConfig:         true,
			}
			nbrs = append(nbrs, newNbr)
			v.RoutingObj.Spec.BGPConfig.Neighbors = nbrs
		}
	}

}
