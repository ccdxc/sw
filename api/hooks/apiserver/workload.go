package impl

import (
	"fmt"

	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/log"
)

type wlHooks struct {
	logger log.Logger
}

// errInvalidMacConfig returns error associated with invalid mac-address
func (wl *wlHooks) errInvalidMacConfig(mac string) error {
	return fmt.Errorf("mis-configured workload policy, invalid mac address: %s", mac)
}

// Validate the Workload config
func (wl *wlHooks) validateWorkloadConfig(i interface{}, ver string, ignStatus bool) []error {
	var err []error
	obj, ok := i.(workload.Workload)
	if !ok {
		return []error{fmt.Errorf("incorrect object type, expected workload object")}
	}

	// validate the mac address in the interface spec
	for mac := range obj.Spec.Interfaces {
		if vldtor.MacAddr(mac) == false {
			wl.logger.Errorf("Invalid mac: %s", mac)
			err = append(err, wl.errInvalidMacConfig(mac))
		}
	}

	return err
}

func registerWorkloadHooks(svc apiserver.Service, logger log.Logger) {
	r := wlHooks{}
	r.logger = logger.WithContext("Service", "WorkloadHooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("Workload", apiserver.CreateOper).GetRequestType().WithValidate(r.validateWorkloadConfig)
	svc.GetCrudService("Workload", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateWorkloadConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("workload.WorkloadV1", registerWorkloadHooks)
}
