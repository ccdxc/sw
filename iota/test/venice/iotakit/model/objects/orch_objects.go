package objects

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
)

//Orchestrator Return orchestrator
type Orchestrator struct {
	Name     string
	IP       string
	Username string
	Password string
	License  string
	DC       string
	orch     *orchestration.Orchestrator
	client   objClient.ObjClient
}

func createOrchestrator(name, ip, user, password string) *orchestration.Orchestrator {
	return &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		TypeMeta: api.TypeMeta{
			Kind: "Orchestrator",
		},
		Spec: orchestration.OrchestratorSpec{
			Type: "vcenter",
			URI:  ip,
			Credentials: &monitoring.ExternalCred{
				AuthType: "username-password",
				UserName: user,
				Password: password,
			},
		},
		Status: orchestration.OrchestratorStatus{
			Status: "unknown",
		},
	}
}

//NewOrchestrator create orchestrator.
func NewOrchestrator(client objClient.ObjClient, dcname, name, ip, user, password string) *Orchestrator {

	orch := createOrchestrator(name, ip, user, password)

	return &Orchestrator{
		Name:     name,
		IP:       ip,
		Username: user,
		Password: password,
		DC:       dcname,
		orch:     orch,
		client:   client,
	}
}

//Commit commit the orchestration object
func (orch *Orchestrator) Commit() error {
	//orch.client.

	orch.orch.Labels = make(map[string]string)
	orch.orch.Labels["force-dc-names"] = orch.DC
	return orch.client.CreateOrchestration(orch.orch)
}

//Delete deletes orch config
func (orch *Orchestrator) Delete() error {

	return orch.client.DeleteOrchestration(orch.orch)

}
