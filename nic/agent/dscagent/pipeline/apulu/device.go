// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleDevice handles CRUD operations on device
func HandleDevice(oper types.Operation, client halapi.DeviceSvcClient, lbip *halapi.IPAddress) error {
	switch oper {
	case types.Create:
		return createDeviceHandler(client, lbip)
	case types.Update:
		return updateDeviceHandler(client, lbip)
	case types.Delete:
		return deleteDeviceHandler(client)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createDeviceHandler(client halapi.DeviceSvcClient, lbip *halapi.IPAddress) error {
	deviceRequest := convertDevice(lbip)
	resp, err := client.DeviceCreate(context.Background(), deviceRequest)
	log.Infof("createDeviceHandler Response: %v. Err : %v", resp, err)
	if err == nil {
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create failed for Device")); err != nil {
				return err
			}
		}
	}
	return nil
}

func updateDeviceHandler(client halapi.DeviceSvcClient, lbip *halapi.IPAddress) error {
	deviceRequest := convertDevice(lbip)
	resp, err := client.DeviceUpdate(context.Background(), deviceRequest)
	log.Infof("update DeviceHandler Response: %v. Err : %v", resp, err)
	if err == nil {
		if resp != nil {
			if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update failed for Device")); err != nil {
				return err
			}
		}
	}
	return nil
}

func deleteDeviceHandler(client halapi.DeviceSvcClient) error {
	deviceDeleteRequest := &halapi.DeviceDeleteRequest{}
	resp, err := client.DeviceDelete(context.Background(), deviceDeleteRequest)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus, err, fmt.Sprintf("Delete failed for Device")); err != nil {
			return err
		}
	}
	return nil
}

func convertDevice(lbip *halapi.IPAddress) *halapi.DeviceRequest {
	return &halapi.DeviceRequest{
		Request: &halapi.DeviceSpec{
			DevOperMode:         halapi.DeviceOperMode_DEVICE_OPER_MODE_HOST,
			MemoryProfile:       halapi.MemoryProfile_MEMORY_PROFILE_DEFAULT,
			BridgingEn:          true,
			LearningEn:          true,
			LearnAgeTimeout:     300,
			OverlayRoutingEn:    true,
			IPAddr:              lbip,
			FwPolicyXposnScheme: halapi.FwPolicyXposn_FW_POLICY_XPOSN_ANY_DENY,
		},
	}
}
