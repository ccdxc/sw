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

// HandleInterface handles crud operations on intf
func HandleInterface(infraAPI types.InfraAPI, client halapi.InterfaceClient, oper types.Operation, intf netproto.Interface) error {
	switch oper {
	case types.Create:
		return createInterfaceHandler(infraAPI, client, intf)
	case types.Update:
		return updateInterfaceHandler(infraAPI, client, intf)
	case types.Delete:
		return deleteInterfaceHandler(infraAPI, client, intf)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createInterfaceHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, intf netproto.Interface) error {
	intfReqMsg := convertInterface(intf)
	resp, err := client.InterfaceCreate(context.Background(), intfReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", intf.GetKind(), intf.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := intf.Marshal()

	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Interface: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Interface: %v", intf.GetKey(), err)
	}
	return nil
}

func updateInterfaceHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, intf netproto.Interface) error {
	intfReqMsg := convertInterface(intf)
	resp, err := client.InterfaceUpdate(context.Background(), intfReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", intf.GetKind(), intf.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := intf.Marshal()

	if err := infraAPI.Store(intf.Kind, intf.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Interface: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Interface: %s | Interface: %v", intf.GetKey(), err)
	}
	return nil
}

func deleteInterfaceHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, intf netproto.Interface) error {
	intfDelReq := &halapi.InterfaceDeleteRequestMsg{
		Request: []*halapi.InterfaceDeleteRequest{
			{
				KeyOrHandle: convertIfKeyHandles(0, intf.Status.InterfaceID)[0],
			},
		},
	}

	resp, err := client.InterfaceDelete(context.Background(), intfDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("Interface: %s", intf.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(intf.Kind, intf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err)
	}
	return nil
}

func convertInterface(intf netproto.Interface) *halapi.InterfaceRequestMsg {
	var txMirrorSessionhandles []*halapi.MirrorSessionKeyHandle
	var rxMirrorSessionhandles []*halapi.MirrorSessionKeyHandle
	switch strings.ToLower(intf.Spec.Type) {
	case "uplink_eth":
		return &halapi.InterfaceRequestMsg{
			Request: []*halapi.InterfaceSpec{
				{
					KeyOrHandle: convertIfKeyHandles(0, intf.Status.InterfaceID)[0],
					Type:        halapi.IfType_IF_TYPE_UPLINK,
					IfInfo: &halapi.InterfaceSpec_IfUplinkInfo{
						IfUplinkInfo: &halapi.IfUplinkInfo{
							PortNum: intf.Status.IFUplinkStatus.PortID,
						},
					},
					TxMirrorSessions: txMirrorSessionhandles,
					RxMirrorSessions: rxMirrorSessionhandles,
				},
			},
		}
	case "uplink_mgmt":
		return &halapi.InterfaceRequestMsg{
			Request: []*halapi.InterfaceSpec{
				{
					KeyOrHandle: convertIfKeyHandles(0, intf.Status.InterfaceID)[0],
					Type:        halapi.IfType_IF_TYPE_UPLINK,
					IfInfo: &halapi.InterfaceSpec_IfUplinkInfo{
						IfUplinkInfo: &halapi.IfUplinkInfo{
							PortNum:         intf.Status.IFUplinkStatus.PortID,
							IsOobManagement: true,
						},
					},
					TxMirrorSessions: txMirrorSessionhandles,
					RxMirrorSessions: rxMirrorSessionhandles,
				},
			},
		}
	}
	return nil
}

func convertIfKeyHandles(vlanID uint32, intfIDs ...uint64) (ifKeyHandles []*halapi.InterfaceKeyHandle) {
	if vlanID == types.UntaggedCollVLAN {
		return
	}

	for _, id := range intfIDs {
		ifKeyHandle := halapi.InterfaceKeyHandle{
			KeyOrHandle: &halapi.InterfaceKeyHandle_InterfaceId{
				InterfaceId: id,
			},
		}
		ifKeyHandles = append(ifKeyHandles, &ifKeyHandle)
	}
	return
}
