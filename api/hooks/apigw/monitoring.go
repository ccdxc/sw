package impl

import (
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

func registerMonitoringHooks(svc apigw.APIGatewayService, l log.Logger) error {
	if err := registerArchiveHooks(svc, l); err != nil {
		return err
	}
	return registerAlertHooks(svc, l)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("monitoring.MonitoringV1", registerMonitoringHooks)
}
