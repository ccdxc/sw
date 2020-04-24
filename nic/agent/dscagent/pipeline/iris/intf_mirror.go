// +build iris

package iris

import (
	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"

	commonUtils "github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleInterfaceMirrorSession handles crud operations on mirror session
func HandleInterfaceMirrorSession(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, oper types.Operation, mirror netproto.InterfaceMirrorSession, vrfID uint64) error {
	switch oper {
	case types.Create:
		return createInterfaceMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	case types.Update:
		return updateInterfaceMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	case types.Delete:
		return deleteInterfaceMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createInterfaceMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.InterfaceMirrorSession, vrfID uint64) error {
	for _, c := range mirror.Spec.Collectors {
		dstIP := c.ExportCfg.Destination
		// Create the unique key for collector dest IP
		destKey := commonUtils.BuildDestKey(mirror.Spec.VrfName, dstIP)
		// Create collector
		col := buildCollector(mirror.Name+"-"+destKey, mirror.Spec.VrfName, dstIP, c, mirror.Spec.PacketSize)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Create, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorCreate, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return errors.Wrapf(types.ErrCollectorCreate, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
	}

	dat, _ := mirror.Marshal()

	if err := infraAPI.Store(mirror.Kind, mirror.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "InterfaceMirrorSession: %s | InterfaceMirrorSession: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "InterfaceMirrorSession: %s | InterfaceMirrorSession: %v", mirror.GetKey(), err)
	}
	return nil
}

func updateInterfaceMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.InterfaceMirrorSession, vrfID uint64) error {
	var existingMirror netproto.InterfaceMirrorSession
	dat, err := infraAPI.Read(mirror.Kind, mirror.GetKey())
	if err != nil {
		return err
	}
	err = existingMirror.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrUnmarshal, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	if err := deleteInterfaceMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, existingMirror, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionDeleteDuringUpdate, "InterfaceMirrorSession: %s | InterfaceMirrorSession: %v", existingMirror.GetKey(), err))
	}
	if err := createInterfaceMirrorSessionHandler(infraAPI, telemetryClient, intfClient, epClient, mirror, vrfID); err != nil {
		log.Error(errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "InterfaceMirrorSession: %s | InterfaceMirrorSession: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrMirrorSessionCreateDuringUpdate, "InterfaceMirrorSession: %s | InterfaceMirrorSession: %v", mirror.GetKey(), err)
	}
	data, err := infraAPI.List("Interface")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: | Err: %v", types.ErrObjNotFound))
	}

	for _, o := range data {
		var intf netproto.Interface
		err := proto.Unmarshal(o, &intf)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			continue
		}
		if !usingMirrorSession(intf, mirror.GetKey()) {
			continue
		}
		collectorMap := make(map[string]int)
		err = validator.ValidateInterface(infraAPI, intf, collectorMap)
		if err != nil {
			log.Error(err)
			continue
		}
		if err := HandleInterface(infraAPI, intfClient, types.Update, intf, collectorMap); err != nil {
			log.Error(errors.Wrapf(types.ErrInterfaceUpdateDuringInterfaceMirrorSessionUpdate, "Interface: %s | Err: %v", intf.GetKey(), err))
		}
	}
	return nil
}

func deleteInterfaceMirrorSessionHandler(infraAPI types.InfraAPI, telemetryClient halapi.TelemetryClient, intfClient halapi.InterfaceClient, epClient halapi.EndpointClient, mirror netproto.InterfaceMirrorSession, vrfID uint64) error {
	for _, c := range mirror.Spec.Collectors {
		dstIP := c.ExportCfg.Destination
		// Create the unique key for collector dest IP
		destKey := commonUtils.BuildDestKey(mirror.Spec.VrfName, dstIP)
		_, ok := MirrorDestToIDMapping[destKey]

		if !ok {
			log.Error(errors.Wrapf(types.ErrDeleteReceivedForNonExistentCollector, "InterfaceMirrorSession: %s | collectorKey: %s", mirror.GetKey(), destKey))
			return errors.Wrapf(types.ErrDeleteReceivedForNonExistentCollector, "InterfaceMirrorSession: %s | collectorKey: %s", mirror.GetKey(), destKey)
		}
		// Try to delete collector if ref count is 0
		col := buildCollector(mirror.Name+"-"+destKey, mirror.Spec.VrfName, dstIP, c, mirror.Spec.PacketSize)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Delete, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorDelete, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
		}
	}

	if err := infraAPI.Delete(mirror.Kind, mirror.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err)
	}
	return nil
}

func usingMirrorSession(intf netproto.Interface, mirror string) bool {
	for _, m := range intf.Spec.MirrorSessions {
		if m == mirror {
			return true
		}
	}
	return false
}
