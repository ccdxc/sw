package base

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

type NetworkParams struct {
	NwName string
	Ip     string
	Gw     string
	Vni    uint32
	Vpc    string
	Tenant string
}

// Networks returns a list of subnets
func (sm *SysModel) Networks() *objects.NetworkCollection {
	snc := objects.NetworkCollection{}
	nws, err := sm.CfgModel.ListNetwork("")
	if err != nil {
		log.Errorf("Error listing networks %v", err)
		return nil
	}
	for _, sn := range nws {
		snc.AddSubnet(&objects.Network{VeniceNetwork: sn})
	}

	return &snc
}

func (sm *SysModel) NewNetwork(nwp *NetworkParams) *objects.NetworkCollection {

	//Create Network object and call addsubnet
	snc := &objects.NetworkCollection{
		CollectionCommon: objects.CollectionCommon{Testbed: sm.Tb, Client: sm.ObjClient()},
	}

	sn := &objects.Network{
		Name: nwp.NwName,
		VeniceNetwork: &network.Network{
			TypeMeta:   api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{Name: nwp.NwName, Tenant: nwp.Tenant},
			Spec: network.NetworkSpec{
				Type:          "routed",
				IPv4Subnet:    nwp.Ip,
				IPv4Gateway:   nwp.Gw,
				VxlanVNI:      nwp.Vni,
				VirtualRouter: nwp.Vpc,
				RouteImportExport: &network.RDSpec{
					AddressFamily: "l2vpn-evpn",
					RDAuto:        true,
					RD:            nil,
					ExportRTs: []*network.RouteDistinguisher{
						{
							Type:          "type2",
							AdminValue:    200,
							AssignedValue: 100,
						},
						{
							Type:          "type2",
							AdminValue:    200,
							AssignedValue: 101,
						},
					},
					ImportRTs: []*network.RouteDistinguisher{
						{
							Type:          "type2",
							AdminValue:    100,
							AssignedValue: 100,
						},
					},
				},
			},
		},
	}

	snc.AddSubnet(sn)

	return snc
}

func (sm *SysModel) GetNetwork(tenant string, name string) (*objects.Network, error) {
	nw, err := sm.ObjClient().GetNetwork(tenant, name)

	return &objects.Network{VeniceNetwork: nw}, err
}
