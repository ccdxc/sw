// +build iris

package iris

import (
	"fmt"

	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"

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
	mirrorKey := fmt.Sprintf("%s/%s", mirror.Kind, mirror.GetKey())
	for _, c := range mirror.Spec.Collectors {
		sessionID := infraAPI.AllocateID(types.MirrorSessionID, 0)
		colName := fmt.Sprintf("%s-%d", mirrorKey, sessionID)
		// Create collector
		col := buildCollector(colName, sessionID, c, mirror.Spec.PacketSize, mirror.Spec.SpanID)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Create, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorCreate, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return errors.Wrapf(types.ErrCollectorCreate, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}

		// Populate the MirrorDestToIDMapping
		MirrorDestToIDMapping[mirrorKey] = append(MirrorDestToIDMapping[mirrorKey], sessionID)
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
		collectorMap := make(map[uint64]int)
		err = validator.ValidateInterface(infraAPI, intf, collectorMap, MirrorDestToIDMapping)
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
	mirrorKey := fmt.Sprintf("%s/%s", mirror.Kind, mirror.GetKey())
	sessionIDs := MirrorDestToIDMapping[mirrorKey]
	for idx, c := range mirror.Spec.Collectors {
		sessionID := sessionIDs[idx]
		colName := fmt.Sprintf("%s-%d", mirrorKey, sessionID)
		// Delete collector to HAL
		col := buildCollector(colName, sessionID, c, mirror.Spec.PacketSize, mirror.Spec.SpanID)
		if err := HandleCol(infraAPI, telemetryClient, intfClient, epClient, types.Delete, col, vrfID); err != nil {
			log.Error(errors.Wrapf(types.ErrCollectorDelete, "InterfaceMirrorSession: %s | Err: %v", mirror.GetKey(), err))
		}
	}

	// Clean up MirrorDestToIDMapping for mirror key
	delete(MirrorDestToIDMapping, mirrorKey)

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
