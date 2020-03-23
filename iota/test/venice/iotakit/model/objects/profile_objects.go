package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/venice/utils/log"
)

//DscProfile wrapper for DSC profile
type DscProfile struct {
	Profile *cluster.DSCProfile
	client  objClient.ObjClient
}

//NewDscProfileInsertion  creates insertion profile
func NewDscProfileInsertion(client objClient.ObjClient, name string) *DscProfile {

	return &DscProfile{
		client: client,
		Profile: &cluster.DSCProfile{
			TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
			ObjectMeta: api.ObjectMeta{
				Name:      name,
				Namespace: "",
				Tenant:    "",
			},
			Spec: cluster.DSCProfileSpec{
				FwdMode:        "INSERTION",
				FlowPolicyMode: "ENFORCED",
			},
		},
	}
}

//NewDscProfileBasenet creates basenet profile
func NewDscProfileBasenet(client objClient.ObjClient, name string) *DscProfile {

	return &DscProfile{
		client: client,
		Profile: &cluster.DSCProfile{
			TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
			ObjectMeta: api.ObjectMeta{
				Name:      name,
				Namespace: "",
				Tenant:    "",
			},
			Spec: cluster.DSCProfileSpec{
				FwdMode:        "TRANSPARENT",
				FlowPolicyMode: "BASENET",
			},
		},
	}
}

//Commit commit the orchestration object
func (p *DscProfile) Commit() error {
	return p.client.CreateDscProfile(p.Profile)
}

//Delete deletes orch config
func (p *DscProfile) Delete() error {
	return p.client.DeleteDscProfile(p.Profile)
}

//AttachNaples attach profile to naples
func (p *DscProfile) AttachNaples(naples *NaplesCollection) error {

	var err error
	for _, node := range naples.Nodes {
		node.SmartNic, err = p.client.GetSmartNIC(node.SmartNic.ObjectMeta.Name)
		if err != nil {
			log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
			return err
		}
		node.SmartNic.Spec.DSCProfile = p.Profile.Name
		err := p.client.UpdateSmartNIC(node.SmartNic)
		if err != nil {
			log.Errorf("Error updating smartnic with profile %v", err.Error())
			return err
		}
	}

	for _, node := range naples.FakeNodes {
		node.SmartNic, err = p.client.GetSmartNIC(node.SmartNic.ObjectMeta.Name)
		if err != nil {
			log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
			return err
		}
		node.SmartNic.Spec.DSCProfile = p.Profile.Name
		err := p.client.UpdateSmartNIC(node.SmartNic)
		if err != nil {
			log.Errorf("Error updating smartnic with profile %v", err.Error())
			return err
		}
	}

	return nil
}
