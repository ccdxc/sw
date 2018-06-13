package upgsdk

import (
	"errors"

	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

//SvcRole upgrade service application role
type SvcRole int

const (
	//AgentRole role agent
	AgentRole SvcRole = 0
	//NonAgentRole role non-agent
	NonAgentRole SvcRole = 1
)

type upgSdk struct {
	svcName   string
	svcRole   SvcRole
	sdkClient gosdk.Client
}

//AgentHandlers agents to implement this
type AgentHandlers interface {
	UpgSuccessful()
	UpgFailed()
}

// UpgSdk is the main Upgrade SDK API
type UpgSdk interface {

	//API used initiate upgrade request
	StartUpgrade() error
}

//NewUpgSdk API is used to init upgrade sdk
func NewUpgSdk(name string, client gosdk.Client, role SvcRole, agentHdlrs AgentHandlers) (UpgSdk, error) {
	log.Infof("NewUpgSdk called for %s\n", name)
	upgsdk := &upgSdk{
		svcName:   name,
		sdkClient: client,
		svcRole:   role,
	}
	upgrade.UpgReqMount(client, delphi.MountMode_ReadWriteMode)
	if role == AgentRole {
		UpgRespInit(client, agentHdlrs)
	}
	return upgsdk, nil
}

func (u *upgSdk) StartUpgrade() error {
	if u.svcRole != AgentRole {
		return errors.New("Svc not of role Agent")
	}
	upgreq := upgrade.NewUpgReq(u.sdkClient)
	upgreq.SetKey(10)
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_UpgStart)
	return nil
}
