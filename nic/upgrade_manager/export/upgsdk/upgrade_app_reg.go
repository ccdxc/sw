package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

func registerUpgApp(client gosdk.Client, name string) {
	log.Infof("registerUpgApp called")
	app := upgrade.GetUpgApp(client, name)
	if app == nil {
		log.Infof("App %s not registered. Registering now.", name)
		app = upgrade.NewUpgAppWithKey(client, name)
		if app == nil {
			log.Infof("App %s not registered.", name)
			return
		}
	}
}

func upgAppRegInit(client gosdk.Client) {
	log.Infof("upgAppRegInit called")
	//TODO
	//upgrade.UpgAppMount(client, delphi.MountMode_ReadWriteMode)
	upgrade.UpgAppMount(client, delphi.MountMode_ReadMode)
}
