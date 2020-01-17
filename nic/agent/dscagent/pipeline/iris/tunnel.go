// +build iris

package iris

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleTunnel handles crud operations on tunnel
func HandleTunnel(infraAPI types.InfraAPI, client halapi.InterfaceClient, oper types.Operation, tunnel netproto.Tunnel, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createTunnelHandler(infraAPI, client, tunnel, vrfID)
	case types.Update:
		return updateTunnelHandler(infraAPI, client, tunnel, vrfID)
	case types.Delete:
		return deleteTunnelHandler(infraAPI, client, tunnel)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createTunnelHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, tunnel netproto.Tunnel, vrfID uint64) error {
	tunnelReqMsg := convertTunnel(tunnel, vrfID)
	resp, err := client.InterfaceCreate(context.Background(), tunnelReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", tunnel.GetKind(), tunnel.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := tunnel.Marshal()

	if err := infraAPI.Store(tunnel.Kind, tunnel.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Tunnel: %v", tunnel.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Tunnel: %v", tunnel.GetKey(), err)
	}
	return nil
}

func updateTunnelHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, tunnel netproto.Tunnel, vrfID uint64) error {
	tunnelReqMsg := convertTunnel(tunnel, vrfID)
	resp, err := client.InterfaceUpdate(context.Background(), tunnelReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", tunnel.GetKind(), tunnel.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := tunnel.Marshal()

	if err := infraAPI.Store(tunnel.Kind, tunnel.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Tunnel: %s | Tunnel: %v", tunnel.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Tunnel: %s | Tunnel: %v", tunnel.GetKey(), err)
	}
	return nil
}

func deleteTunnelHandler(infraAPI types.InfraAPI, client halapi.InterfaceClient, tunnel netproto.Tunnel) error {

	tunnelDelReq := &halapi.InterfaceDeleteRequestMsg{
		Request: []*halapi.InterfaceDeleteRequest{
			{
				KeyOrHandle: convertIfKeyHandles(0, tunnel.Status.TunnelID)[0],
			},
		},
	}

	resp, err := client.InterfaceDelete(context.Background(), tunnelDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("Tunnel: %s", tunnel.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(tunnel.Kind, tunnel.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
	}
	return nil
}

func convertTunnel(tunnel netproto.Tunnel, vrfID uint64) *halapi.InterfaceRequestMsg {
	return &halapi.InterfaceRequestMsg{
		Request: []*halapi.InterfaceSpec{
			{
				KeyOrHandle: convertIfKeyHandles(0, tunnel.Status.TunnelID)[0],
				Type:        halapi.IfType_IF_TYPE_TUNNEL,
				AdminStatus: halapi.IfStatus_IF_STATUS_UP,
				IfInfo: &halapi.InterfaceSpec_IfTunnelInfo{
					IfTunnelInfo: &halapi.IfTunnelInfo{
						EncapType: halapi.IfTunnelEncapType_IF_TUNNEL_ENCAP_TYPE_GRE,
						EncapInfo: &halapi.IfTunnelInfo_GreInfo{
							GreInfo: &halapi.IfTunnelGREInfo{
								Source:      utils.ConvertIPAddresses(tunnel.Spec.Src)[0],
								Destination: utils.ConvertIPAddresses(tunnel.Spec.Dst)[0],
								Mtu:         types.TunnelMTU,
								Ttl:         types.TunnelTTL,
							},
						},
						VrfKeyHandle: convertVrfKeyHandle(vrfID),
					},
				},
			},
		},
	}
}
