package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

//SvcRole upgrade service application role
type SvcRole int

const (
	//Agent role agent
	Agent SvcRole = 0
	//NonAgent role non-agent
	NonAgent SvcRole = 1
)

type upgSdk struct {
	svcName   string
	svcRole   SvcRole
	sdkClient gosdk.Client
}

// UpgSdk is the main Upgrade SDK API
type UpgSdk interface {

	//API used initiate upgrade request
	StartUpgrade() error
}

//NewUpgSdk API is used to init upgrade sdk
func NewUpgSdk(name string, client gosdk.Client) (UpgSdk, error) {
	log.Infof("NewUpgSdk called for %s\n", name)
	upgsdk := &upgSdk{
		svcName:   name,
		sdkClient: client,
	}
	upgrade.UpgReqMount(client, delphi.MountMode_ReadWriteMode)
	UpgRespInit(client)
	return upgsdk, nil
}

func (u *upgSdk) StartUpgrade() error {
	upgreq := upgrade.NewUpgReq(u.sdkClient)
	upgreq.SetKey(10)
	upgreq.SetUpgReqCmd(upgrade.UpgReqType_UpgStart)
	return nil
}
