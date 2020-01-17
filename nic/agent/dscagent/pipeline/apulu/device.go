// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto"
)

// HandleDevice handles CRUD operations on device
func HandleDevice(oper types.Operation, client halapi.DeviceSvcClient) error {
	switch oper {
	case types.Create:
		return createDeviceHandler(client)
	case types.Update:
		return updateDeviceHandler(client)
	case types.Delete:
		return deleteDeviceHandler(client)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createDeviceHandler(client halapi.DeviceSvcClient) error {
	deviceRequest := &halapi.DeviceRequest{
		Request: &halapi.DeviceSpec{
			DevOperMode:     halapi.DeviceOperMode_DEVICE_OPER_MODE_HOST,
			Profile:         halapi.DeviceProfile_DEVICE_PROFILE_DEFAULT,
			BridgingEn:      true,
			LearningEn:      true,
			LearnAgeTimeout: 300,
		},
	}
	resp, err := client.DeviceCreate(context.Background(), deviceRequest)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create failed for Device")); err != nil {
			return err
		}
	}
	return nil
}

func updateDeviceHandler(client halapi.DeviceSvcClient) error {
	// TODO: devise a way to get updates or push all the logic from this file to HandleDevice in apulu.go
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
