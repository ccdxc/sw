package upggosdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

func registerUpgApp(client clientApi.Client, name string) {
	log.Infof("registerUpgApp called")
	app := upgrade.GetUpgApp(client, name)
	if app == nil {
		log.Infof("App %s not registered. Registering now.", name)
		app = &upgrade.UpgApp{
			Key: name,
		}
		client.SetObject(app)
		if app == nil {
			log.Infof("App %s not registered.", name)
			return
		}
	}
}

func upgAppRegInit(client clientApi.Client, name string) {
	log.Infof("upgAppRegInit called")
	upgrade.UpgAppMountKey(client, name, delphi.MountMode_ReadWriteMode)
}
