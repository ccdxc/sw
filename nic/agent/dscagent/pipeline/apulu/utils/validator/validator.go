// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package validator

import (
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// Only TCP, UDP and ICMP are supported on cloud
func unsupportedApp(app netproto.App) bool {
	if app.Spec.ALG == nil {
		return false
	}
	if app.Spec.ALG.DNS != nil {
		return true
	}
	if app.Spec.ALG.SIP != nil {
		return true
	}
	if len(app.Spec.ALG.SUNRPC) > 0 {
		return true
	}
	if app.Spec.ALG.FTP != nil {
		return true
	}
	if len(app.Spec.ALG.MSRPC) > 0 {
		return true
	}
	if app.Spec.ALG.TFTP != nil {
		return true
	}
	if app.Spec.ALG.RTSP != nil {
		return true
	}
	return false
}

// ValidateNetworkSecurityPolicy validates network security policy for apulu pipeline
func ValidateNetworkSecurityPolicy(i types.InfraAPI, nsp netproto.NetworkSecurityPolicy) (err error) {
	var pdsRules int
	for _, r := range nsp.Spec.Rules {
		var dat []byte
		if len(r.AppName) > 0 {
			app := netproto.App{
				TypeMeta: api.TypeMeta{
					Kind: "App",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    nsp.Tenant,
					Namespace: nsp.Namespace,
					Name:      r.AppName,
				},
			}

			dat, err = i.Read(app.Kind, app.GetKey())
			if err != nil {
				log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
				return errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
			}
			err = app.Unmarshal(dat)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
				return errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
			}

			if unsupportedApp(app) {
				log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", r.AppName, types.ErrUnsupportedApp))
				return errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", r.AppName, types.ErrUnsupportedApp)
			}
		}
		srcAddresses := 1
		dstAddresses := 1
		srcProtoPorts := 1
		dstProtoPorts := 1
		if r.Src != nil {
			if len(r.Src.Addresses) > 0 {
				srcAddresses = len(r.Src.Addresses)
			}
			if len(r.Src.ProtoPorts) > 0 {
				srcProtoPorts = len(r.Src.ProtoPorts)
			}
		}
		if r.Dst != nil {
			if len(r.Dst.Addresses) > 0 {
				dstAddresses = len(r.Dst.Addresses)
			}
			if len(r.Dst.ProtoPorts) > 0 {
				dstProtoPorts = len(r.Dst.ProtoPorts)
			}
		}
		pdsRules += srcAddresses * dstAddresses * srcProtoPorts * dstProtoPorts
	}
	if pdsRules > types.MaxRulesPerSecurityPolicy {
		log.Error(errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v %v", nsp.GetKey(), types.ErrMaxRulesPerSecurityPolicyExceeded, pdsRules))
		return errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrMaxRulesPerSecurityPolicyExceeded)
	}
	return
}
