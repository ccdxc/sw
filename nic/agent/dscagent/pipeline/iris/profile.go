// +build iris

package iris

import (
	"context"
	"fmt"
	"strings"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleProfile handles crud operations on profile
func HandleProfile(infraAPI types.InfraAPI, client halapi.SystemClient, oper types.Operation, profile netproto.Profile) error {
	switch oper {
	case types.Create:
		fallthrough
	case types.Delete:
		fallthrough
	case types.Update:
		return updateProfileHandler(infraAPI, client, profile)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func updateProfileHandler(infraAPI types.InfraAPI, client halapi.SystemClient, profile netproto.Profile) error {

	// Based on the modes unwatch TODO
	sysSpec := convertProfile(profile)
	//Unwatch, purge configs
	resp, err := client.SysSpecUpdate(context.Background(), sysSpec)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", profile.GetKind(), profile.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := profile.Marshal()

	if err := infraAPI.Store(profile.Kind, profile.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Profile: %s | Err: %v", profile.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Profile: %s | Err: %v", profile.GetKey(), err)
	}
	return nil
}

func convertProfile(profile netproto.Profile) *halapi.SysSpec {
	return &halapi.SysSpec{
		FwdMode:    convertFwdMode(profile.Spec.FwdMode),
		PolicyMode: convertPolicyMode(profile.Spec.PolicyMode),
	}
}

func convertFwdMode(fwdMode string) halapi.ForwardMode {
	switch strings.ToLower(fwdMode) {
	case strings.ToLower(netproto.ProfileSpec_TRANSPARENT.String()):
		return halapi.ForwardMode_FWD_MODE_TRANSPARENT
	case strings.ToLower(netproto.ProfileSpec_INSERTION.String()):
		return halapi.ForwardMode_FWD_MODE_MICROSEG
	default:
		return halapi.ForwardMode_FWD_MODE_NONE
	}
}
func convertPolicyMode(policyMode string) halapi.PolicyMode {
	switch strings.ToLower(policyMode) {
	case strings.ToLower(netproto.ProfileSpec_BASENET.String()):
		return halapi.PolicyMode_POLICY_MODE_BASE_NET
	case strings.ToLower(netproto.ProfileSpec_FLOWAWARE.String()):
		return halapi.PolicyMode_POLICY_MODE_FLOW_AWARE
	case strings.ToLower(netproto.ProfileSpec_ENFORCED.String()):
		return halapi.PolicyMode_POLICY_MODE_ENFORCE
	default:
		return halapi.PolicyMode_POLICY_MODE_NONE
	}
}
