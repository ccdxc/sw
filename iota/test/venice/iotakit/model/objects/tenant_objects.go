package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

//TODO Make tenant a collection

type Tenant struct {
	CollectionCommon
	Tenant *cluster.Tenant
}

func NewTenant(name string, client objClient.ObjClient, testbed *testbed.TestBed) *Tenant {

	return &Tenant{
		CollectionCommon: CollectionCommon{Client: client, Testbed: testbed},
		Tenant: &cluster.Tenant{
			TypeMeta:   api.TypeMeta{Kind: "Tenant", APIVersion: "v1"},
			ObjectMeta: api.ObjectMeta{Name: name},
		},
	}
}

// Commit writes the VPC config to venice
func (ten *Tenant) Commit() error {
	if ten.HasError() {
		return ten.err
	}

	err := ten.Client.CreateTenant(ten.Tenant)
	if err != nil {
		ten.err = err
		log.Infof("Creating or updating Tenant failed %v", err)
		return err
	}

	log.Debugf("Created VPC : %#v", ten)

	return nil
}

// Delete deletes tenant
func (ten *Tenant) Delete() error {
	if ten.err != nil {
		return ten.err
	}

	err := ten.Client.DeleteTenant(ten.Tenant)

	if err != nil {
		ten.err = err
	}

	return err
}