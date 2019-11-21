// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"errors"

	"github.com/pensando/sw/api/generated/network"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/log"
)

type networkHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (h *networkHooks) validateIPAMPolicyConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	cfg, ok := i.(network.IPAMPolicy)

	if ok == false {
		return []error{errors.New("Invalid input configuration")}
	}

	// in the current implementation only one dhcp server configuration is supported even though the model
	// defines it as a slice for future enhancements
	if len(cfg.Spec.DHCPRelay.Servers) > 1 {
		return []error{errors.New("Only one DHCP server configuration is supported")}
	}
	return nil
}

func registerNetworkHooks(svc apiserver.Service, logger log.Logger) {
	hooks := networkHooks{}
	hooks.svc = svc
	hooks.logger = logger
	logger.InfoLog("Service", "NetworkV1", "msg", "registering networkAction hook")
	svc.GetCrudService("IPAMPolicy", apiintf.CreateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
	svc.GetCrudService("IPAMPolicy", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("network.NetworkV1", registerNetworkHooks)
}
