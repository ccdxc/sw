// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package pipeline

import (
	"context"
	"sync"

	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ApuluAPI implements PipelineAPI for Apulu pipeline
type ApuluAPI struct {
	sync.Mutex
	InfraAPI                types.InfraAPI
	VPCClient               halapi.VPCSvcClient
	DeviceSvcClient         halapi.DeviceSvcClient
	SecurityPolicySvcClient halapi.SecurityPolicySvcClient
}

// NewPipelineAPI returns the implemetor of PipelineAPI
func NewPipelineAPI(infraAPI types.InfraAPI) (*ApuluAPI, error) {
	conn, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Errorf("Failed to create GRPC Connection to HAL. Err: %v", err)
		return nil, err
	}

	a := ApuluAPI{
		InfraAPI:                infraAPI,
		VPCClient:               halapi.NewVPCSvcClient(conn),
		DeviceSvcClient:         halapi.NewDeviceSvcClient(conn),
		SecurityPolicySvcClient: halapi.NewSecurityPolicySvcClient(conn),
	}

	if err := a.PipelineInit(); err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineInit, "Apulu Init: %v", err))
		return nil, errors.Wrapf(types.ErrPipelineInit, "Apulu Init: %v", err)
	}

	return &a, nil
}

// ############################################### PipelineAPI Methods  ###############################################

// PipelineInit does Apulu Pipeline init. Creating device, default security profile
func (a *ApuluAPI) PipelineInit() error {
	log.Infof("Apulu API: %s", types.InfoPipelineInit)
	if err := a.HandleDevice(types.Create); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Apulu API: %s | %s", types.InfoPipelineInit, types.InfoDeviceCreate)
	profile := netproto.SecurityProfile{}
	if _, err := a.HandleSecurityProfile(types.Create, profile); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Apulu API: %s | %s", types.InfoPipelineInit, types.InfoSecurityProfileCreate)
	return nil
}

// HandleDevice handles CRUD methods for Device objects
func (a *ApuluAPI) HandleDevice(oper types.Operation) error {
	a.Lock()
	defer a.Unlock()
	log.Infof("Device Op: %s | %s", oper, types.InfoHandleObjBegin)
	defer log.Infof("Device Op: %s | %s", oper, types.InfoHandleObjEnd)
	return apulu.HandleDevice(oper, a.DeviceSvcClient)
}

// HandleVrf handles CRUD Methods for Vrf Object
func (a *ApuluAPI) HandleVrf(oper types.Operation, vrf netproto.Vrf) (vrfs []netproto.Vrf, err error) {
	a.Lock()
	defer a.Unlock()

	err = utils.ValidateMeta(oper, vrf.Kind, vrf.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Vrf
		)
		dat, err = a.InfraAPI.Read(vrf.Kind, vrf.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}
		vrfs = append(vrfs, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = a.InfraAPI.List(vrf.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var vrf netproto.Vrf
			err := proto.Unmarshal(o, &vrf)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err))
				continue
			}
			vrfs = append(vrfs, vrf)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if vrf.Status.VrfID == 0 {
			vrf.Status.VrfID = a.InfraAPI.AllocateID(types.VrfID, types.VrfOffSet)
		}
	case types.Update:
		// Get to ensure that the object exists
		var existingVrf netproto.Vrf
		dat, err := a.InfraAPI.Read(vrf.Kind, vrf.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound)
		}
		err = existingVrf.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&vrf.Spec, &existingVrf.Spec) {
			//log.Infof("Vrf: %s | Info: %s ", vrf.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		vrf.Status.VrfID = existingVrf.Status.VrfID
	case types.Delete:
		var existingVrf netproto.Vrf
		dat, err := a.InfraAPI.Read(vrf.Kind, vrf.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingVrf.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}
		vrf = existingVrf
	}
	log.Infof("Vrf: %v | Op: %s | %s", vrf, oper, types.InfoHandleObjBegin)
	defer log.Infof("Vrf: %v | Op: %s | %s", vrf, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = apulu.HandleVPC(a.InfraAPI, a.VPCClient, oper, vrf) // TODO Change this to VPC Client
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleNetwork unimplemented
func (a *ApuluAPI) HandleNetwork(oper types.Operation, network netproto.Network) ([]netproto.Network, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Network %s is not implemented by Apulu Pipeline", oper)
}

// HandleEndpoint unimplemented
func (a *ApuluAPI) HandleEndpoint(oper types.Operation, ep netproto.Endpoint) ([]netproto.Endpoint, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Endpoint %s is not implemented by Apulu Pipeline", oper)
}

// HandleInterface unimplemented
func (a *ApuluAPI) HandleInterface(oper types.Operation, intf netproto.Interface) ([]netproto.Interface, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Interface %s is not implemented by Apulu Pipeline", oper)
}

// HandleTunnel unimplemented
func (a *ApuluAPI) HandleTunnel(oper types.Operation, tun netproto.Tunnel) ([]netproto.Tunnel, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Tunnel %s is not implemented by Apulu Pipeline", oper)
}

// HandleApp unimplemented
func (a *ApuluAPI) HandleApp(oper types.Operation, app netproto.App) ([]netproto.App, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "App %s is not implemented by Apulu Pipeline", oper)
}

// HandleNetworkSecurityPolicy unimplemented
func (a *ApuluAPI) HandleNetworkSecurityPolicy(oper types.Operation, nsp netproto.NetworkSecurityPolicy) ([]netproto.NetworkSecurityPolicy, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "NetworkSecurityPolicy %s is not implemented by Apulu Pipeline", oper)
}

// HandleSecurityProfile handles CRUD methods for SecurityProfile objects
func (a *ApuluAPI) HandleSecurityProfile(oper types.Operation, profile netproto.SecurityProfile) (profiles []netproto.SecurityProfile, err error) {
	a.Lock()
	defer a.Unlock()

	err = utils.ValidateMeta(oper, profile.Kind, profile.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.SecurityProfile
		)
		dat, err = a.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}
		profiles = append(profiles, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = a.InfraAPI.List(profile.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var profile netproto.SecurityProfile
			err := proto.Unmarshal(o, &profile)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
				continue
			}
			profiles = append(profiles, profile)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if profile.Status.SecurityProfileID == 0 {
			profile.Status.SecurityProfileID = a.InfraAPI.AllocateID(types.SecurityProfileID, types.SecurityProfileOffSet)
		}
	case types.Update:
		// Get to ensure that the object exists
		var existingSecurityProfile netproto.SecurityProfile
		dat, err := a.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "SecurityProfile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}
		err = existingSecurityProfile.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&profile.Spec, &existingSecurityProfile.Spec) {
			//log.Infof("SecurityProfile: %s | Info: %s ", profile.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		profile.Status.SecurityProfileID = existingSecurityProfile.Status.SecurityProfileID
	case types.Delete:
		var existingSecurityProfile netproto.SecurityProfile
		dat, err := a.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingSecurityProfile.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}
		profile = existingSecurityProfile
	}
	log.Infof("SecurityProfile: %v | Op: %s | %s", profile, oper, types.InfoHandleObjBegin)
	defer log.Infof("SecurityProfile: %v | Op: %s | %s", profile, oper, types.InfoHandleObjEnd)

	// Perform object validations
	// Currently security profile is singleton and not associated with any VPC
	_, err = validator.ValidateSecurityProfile(a.InfraAPI, profile)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	// Take a lock to ensure a single HAL API is active at any given point
	err = apulu.HandleSecurityProfile(a.InfraAPI, a.SecurityPolicySvcClient, oper, profile)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleMirrorSession unimplemented
func (a *ApuluAPI) HandleMirrorSession(oper types.Operation, mirror netproto.MirrorSession) (mirrors []netproto.MirrorSession, err error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Mirror Session not implemented by Apulu Pipeline")
}

// HandleFlowExportPolicy unimplemented
func (a *ApuluAPI) HandleFlowExportPolicy(oper types.Operation, netflow netproto.FlowExportPolicy) (netflows []netproto.FlowExportPolicy, err error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Mirror Session not implemented by Apulu Pipeline")
}

// ReplayConfigs unimplemented
func (a *ApuluAPI) ReplayConfigs() error {
	return errors.Wrapf(types.ErrNotImplemented, "Config Replays not implemented by Apulu Pipeline")
}

// PurgeConfigs unimplemented
func (a *ApuluAPI) PurgeConfigs() error {
	return errors.Wrapf(types.ErrNotImplemented, "Config Purges not implemented by Apulu Pipeline")
}

// GetWatchOptions returns the options to be used while establishing a watch from this agent.
func (a *ApuluAPI) GetWatchOptions(ctx context.Context, kind string) (ret api.ListWatchOptions) {
	switch kind {
	case "Endpoint":
		ret.Name = a.InfraAPI.GetDscName()
	}
	return ret
}
