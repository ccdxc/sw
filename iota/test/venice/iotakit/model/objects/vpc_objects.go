package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// MirrorSession represents mirrorsession
type Vpc struct {
	Obj *network.VirtualRouter
}

// MirrorSessionCollection is list of sessions
type VpcObjCollection struct {
	CollectionCommon
	//err      error
	Objs []*Vpc
}

func NewVPC(tenant string, name string, rmac string, vni uint32,
	client objClient.ObjClient, testbed *testbed.TestBed) *VpcObjCollection {
	return &VpcObjCollection{
		Objs: []*Vpc{
			{
				Obj: &network.VirtualRouter{
					TypeMeta:   api.TypeMeta{Kind: "VirtualRouter"},
					ObjectMeta: api.ObjectMeta{Name: name, Tenant: tenant},
					Spec: network.VirtualRouterSpec{
						Type:             "tenant",
						RouterMACAddress: rmac,
						VxLanVNI:         vni,
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
						DefaultIPAMPolicy: "",
					},
				},
			},
		},
		CollectionCommon: CollectionCommon{Testbed: testbed, Client: client},
	}
}

// Commit writes the VPC config to venice
func (voc *VpcObjCollection) Commit() error {
	if voc.HasError() {
		return voc.err
	}
	for _, vpc := range voc.Objs {
		err := voc.Client.CreateVPC(vpc.Obj)
		if err != nil {
			voc.err = err
			log.Infof("Creating or updating VPC failed %v", err)
			return err
		}

		log.Debugf("Created VPC : %#v", vpc.Obj)
	}

	return nil
}

// Delete deletes all VPCs in the collection
func (voc *VpcObjCollection) Delete() error {
	if voc.err != nil {
		return voc.err
	}

	// walk all sessions and delete them
	for _, v := range voc.Objs {
		err := voc.Client.DeleteVPC(v.Obj)
		if err != nil {
			return err
		}
	}

	return nil
}
