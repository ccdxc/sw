// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"net"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleMirrorSession handles crud operations on mirror session
func HandleMirrorSession(infraAPI types.InfraAPI, client halapi.MirrorSvcClient, oper types.Operation, mirror netproto.MirrorSession,
	vpcID uint64) error {
	switch oper {
	case types.Create:
		return createMirrorSessionHandler(infraAPI, client, mirror, vpcID)
	case types.Update:
		return updateMirrorSessionHandler(infraAPI, client, mirror, vpcID)
	case types.Delete:
		return deleteMirrorSessionHandler(infraAPI, client, mirror)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createMirrorSessionHandler(infraAPI types.InfraAPI, client halapi.MirrorSvcClient, mirror netproto.MirrorSession, vpcID uint64) error {
	mirrorReq := convertMirrorSession(infraAPI, mirror, vpcID)
	resp, err := client.MirrorSessionCreate(context.Background(), mirrorReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := mirror.Marshal()
	if err := infraAPI.Store(mirror.Kind, mirror.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	return nil
}

func updateMirrorSessionHandler(infraAPI types.InfraAPI, client halapi.MirrorSvcClient, mirror netproto.MirrorSession, vpcID uint64) error {
	mirrorReq := convertMirrorSession(infraAPI, mirror, vpcID)
	resp, err := client.MirrorSessionUpdate(context.Background(), mirrorReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", mirror.GetKind(), mirror.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := mirror.Marshal()

	if err := infraAPI.Store(mirror.Kind, mirror.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	return nil
}

func deleteMirrorSessionHandler(infraAPI types.InfraAPI, client halapi.MirrorSvcClient, mirror netproto.MirrorSession) error {
	mirrorDelReq := &halapi.MirrorSessionDeleteRequest{
		Id: []uint32{
			uint32(mirror.Status.MirrorSessionID),
		},
	}

	resp, err := client.MirrorSessionDelete(context.Background(), mirrorDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("MirrorSession: %s", mirror.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(mirror.Kind, mirror.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	return nil
}

func convertMirrorSession(infraAPI types.InfraAPI, mirror netproto.MirrorSession, vpcID uint64) *halapi.MirrorSessionRequest {
	var mirrorSpecs []*halapi.MirrorSessionSpec
	mgmtIP, _, _ := net.ParseCIDR(infraAPI.GetConfig().MgmtIP)
	for _, c := range mirror.Spec.Collectors {
		m := &halapi.MirrorSessionSpec{
			Id:      uint32(mirror.Status.MirrorSessionID),
			SnapLen: mirror.Spec.PacketSize,
			Mirrordst: &halapi.MirrorSessionSpec_ErspanSpec{
				ErspanSpec: &halapi.ERSpanSpec{
					VPCId: utils.ConvertID64(vpcID)[0],
					Erspandst: &halapi.ERSpanSpec_DstIP{
						DstIP: utils.ConvertIPAddresses(c.ExportCfg.Destination)[0],
					},
					SrcIP:  utils.ConvertIPAddresses(mgmtIP.String())[0],
					SpanId: uint32(mirror.Status.MirrorSessionID),
				},
			},
		}
		mirrorSpecs = append(mirrorSpecs, m)
	}
	return &halapi.MirrorSessionRequest{
		Request: mirrorSpecs,
	}
}
