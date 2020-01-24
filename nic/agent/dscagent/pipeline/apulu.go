// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package pipeline

import (
	"context"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
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
	SubnetClient            halapi.SubnetSvcClient
	DeviceSvcClient         halapi.DeviceSvcClient
	SecurityPolicySvcClient halapi.SecurityPolicySvcClient
	DHCPRelayClient         halapi.DHCPSvcClient
	InterfaceClient         halapi.IfSvcClient
	EventClient             halapi.EventSvcClient
	PortClient              halapi.PortSvcClient
}

// NewPipelineAPI returns the implemetor of PipelineAPI
func NewPipelineAPI(infraAPI types.InfraAPI) (*ApuluAPI, error) {
	conn, err := utils.CreateNewGRPCClient("PDS_GRPC_PORT", types.HalGRPCDefaultPort)
	if err != nil {
		log.Errorf("Failed to create GRPC Connection to HAL. Err: %v", err)
		return nil, err
	}

	a := ApuluAPI{
		InfraAPI:                infraAPI,
		VPCClient:               halapi.NewVPCSvcClient(conn),
		SubnetClient:            halapi.NewSubnetSvcClient(conn),
		DeviceSvcClient:         halapi.NewDeviceSvcClient(conn),
		SecurityPolicySvcClient: halapi.NewSecurityPolicySvcClient(conn),
		DHCPRelayClient:         halapi.NewDHCPSvcClient(conn),
		InterfaceClient:         halapi.NewIfSvcClient(conn),
		PortClient:              halapi.NewPortSvcClient(conn),
		EventClient:             halapi.NewEventSvcClient(conn),
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
	//	profile := netproto.SecurityProfile{}
	//	if _, err := a.HandleSecurityProfile(types.Create, profile); err != nil {
	//		log.Error(err)
	//		return err
	//	}
	//	log.Infof("Apulu API: %s | %s", types.InfoPipelineInit, types.InfoSecurityProfileCreate)

	// initialize stream for Lif events
	a.initEventStream()
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
	err = apulu.HandleVPC(a.InfraAPI, a.VPCClient, oper, vrf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleNetwork handles CRUD Methods for Network Object
func (a *ApuluAPI) HandleNetwork(oper types.Operation, network netproto.Network) (networks []netproto.Network, err error) {
	a.Lock()
	defer a.Unlock()

	err = utils.ValidateMeta(oper, network.Kind, network.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Network
		)
		dat, err = a.InfraAPI.Read(network.Kind, network.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err)
		}
		networks = append(networks, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = a.InfraAPI.List(network.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var network netproto.Network
			err := proto.Unmarshal(o, &network)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err))
				continue
			}
			networks = append(networks, network)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if network.Status.NetworkID == 0 {
			network.Status.NetworkID = a.InfraAPI.AllocateID(types.NetworkID, types.NetworkOffSet)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingNetwork netproto.Network
		dat, err := a.InfraAPI.Read(network.Kind, network.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound)
		}
		err = existingNetwork.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&network.Spec, &existingNetwork.Spec) {
			//log.Infof("Network: %s | Info: %s ", network.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		network.Status.NetworkID = existingNetwork.Status.NetworkID
	case types.Delete:
		var existingNetwork netproto.Network
		dat, err := a.InfraAPI.Read(network.Kind, network.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingNetwork.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", network.GetKey(), err)
		}
		network = existingNetwork
	}
	// Perform object validations
	uplinkIDs, vrf, err := validator.ValidateNetwork(a.InfraAPI, network)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	log.Infof("Network: %v | Op: %s | %s", network, oper, types.InfoHandleObjBegin)
	defer log.Infof("Network: %v | Op: %s | %s", network, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = apulu.HandleSubnet(a.InfraAPI, a.SubnetClient, oper, network, vrf.Status.VrfID, uplinkIDs)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleEndpoint unimplemented
func (a *ApuluAPI) HandleEndpoint(oper types.Operation, ep netproto.Endpoint) ([]netproto.Endpoint, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Endpoint %s is not implemented by Apulu Pipeline", oper)
}

// HandleInterface handles CRUD Methods for Interface object
func (a *ApuluAPI) HandleInterface(oper types.Operation, intf netproto.Interface) (intfs []netproto.Interface, err error) {
	// Take a lock to ensure a single HAL API is active at any given point
	a.Lock()
	defer a.Unlock()

	err = utils.ValidateMeta(oper, intf.Kind, intf.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Interface
		)
		dat, err = a.InfraAPI.Read(intf.Kind, intf.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		intfs = append(intfs, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = a.InfraAPI.List(intf.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var intf netproto.Interface
			err := proto.Unmarshal(o, &intf)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
				continue
			}
			intfs = append(intfs, intf)
		}

		return
	case types.Create:
		// Allow only L3 and loopback interfaces be created
		if intf.Spec.Type != "L3" && intf.Spec.Type != "LOOPBACK" {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrInvalidInterfaceType))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrInvalidInterfaceType)
		}
	case types.Update:
		// Get to ensure that the object exists
		var existingIntf netproto.Interface
		dat, err := a.InfraAPI.Read(intf.Kind, intf.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound)
		}
		err = existingIntf.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&intf.Spec, &existingIntf.Spec) {
			//log.Infof("Interface: %s | Info: %s ", intf.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}
	case types.Delete:
		var existingIntf netproto.Interface

		// Allow only L3 and loopback interfaces be deleted
		if intf.Spec.Type != "L3" && intf.Spec.Type != "LOOPBACK" {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrInvalidInterfaceType))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrInvalidInterfaceType)
		}
		dat, err := a.InfraAPI.Read(intf.Kind, intf.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingIntf.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		intf = existingIntf
	}
	log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjBegin)
	defer log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjEnd)

	err = apulu.HandleInterface(a.InfraAPI, a.InterfaceClient, oper, intf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
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

// HandleIPAMPolicy handles CRUD methods for IPAMPolicy objects
func (a *ApuluAPI) HandleIPAMPolicy(oper types.Operation, policy netproto.IPAMPolicy) (policies []netproto.IPAMPolicy, err error) {
	// Take a lock to ensure a single HAL API is active at any given point
	a.Lock()
	defer a.Unlock()

	err = utils.ValidateMeta(oper, policy.Kind, policy.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.IPAMPolicy
		)
		dat, err = a.InfraAPI.Read(policy.Kind, policy.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err)
		}
		policies = append(policies, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = a.InfraAPI.List(policy.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var policy netproto.IPAMPolicy
			err := proto.Unmarshal(o, &policy)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err))
				continue
			}
			policies = append(policies, policy)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if policy.Status.IPAMPolicyID == 0 {
			policy.Status.IPAMPolicyID = a.InfraAPI.AllocateID(types.IPAMPolicyID, 0)
		}
	case types.Update:
		// Get to ensure that the object exists
		var existingPolicy netproto.IPAMPolicy
		dat, err := a.InfraAPI.Read(policy.Kind, policy.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound)
		}
		err = existingPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&policy.Spec, &existingPolicy.Spec) {
			return nil, nil
		}

		// Reuse ID from store
		policy.Status.IPAMPolicyID = existingPolicy.Status.IPAMPolicyID
	case types.Delete:
		var existingPolicy netproto.IPAMPolicy
		dat, err := a.InfraAPI.Read(policy.Kind, policy.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err)
		}
		policy = existingPolicy
	}
	log.Infof("IPAMPolicy: %v | Op: %s | %s", policy, oper, types.InfoHandleObjBegin)
	defer log.Infof("IPAMPolicy: %v | Op: %s | %s", policy, oper, types.InfoHandleObjEnd)

	err = apulu.HandleIPAMPolicy(a.InfraAPI, a.DHCPRelayClient, oper, policy)
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

//func (a *ApuluAPI)HandleTelemetry(oper types.Operation, tm *netproto.Telemetry) ([]*netproto.Telemetry, error) {
//	return nil, errors.Wrapf(types.ErrNotImplemented, "Telemetry %s is not implemented by Apulu Pipeline",  oper)
//}

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

func (a *ApuluAPI) initEventStream() {

	evtReqMsg := &halapi.EventRequest{
		Request: []*halapi.EventRequest_EventSpec{
			{
				EventId: halapi.EventId_EVENT_ID_PORT_CREATE,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_PORT_UP,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_PORT_DOWN,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_LIF_CREATE,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_LIF_UPDATE,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_LIF_UP,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
			{
				EventId: halapi.EventId_EVENT_ID_LIF_DOWN,
				Action:  halapi.EventOp_EVENT_OP_SUBSCRIBE,
			},
		},
	}

	// subscribe to event stream
	eventStream, err := a.EventClient.EventSubscribe(context.Background())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineEventListen, "Init: %v", err))
	}
	eventStream.Send(evtReqMsg)

	// get all the LIFs known at this time
	lifReqMsg := &halapi.LifGetRequest{}
	lifs, err := a.InterfaceClient.LifGet(context.Background(), lifReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineLifGet, "Init: %v", err))
	}

	// get all the ports known at this time
	portReqMsg := &halapi.PortGetRequest{
		Id: []uint32{},
	}
	ports, err := a.PortClient.PortGet(context.Background(), portReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelinePortGet, "Init: %v", err))
	}

	go func(stream halapi.EventSvc_EventSubscribeClient) {
		for {
			resp, err := stream.Recv()
			if err == io.EOF {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "PDS Event stream closed"))
				break
			}
			if err != nil {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "Init: %v", err))
				break
			}

			if resp.Status != halapi.ApiStatus_API_STATUS_OK {
				log.Error(errors.Wrapf(types.ErrDatapathHandling, "Iris: %v", err))
			}

			switch resp.EventId {
			case halapi.EventId_EVENT_ID_LIF_CREATE:
				lif := resp.GetLifEventInfo()
				intfID, _ := binary.Uvarint(lif.Spec.GetId())
				l := netproto.Interface{
					TypeMeta: api.TypeMeta{
						Kind: "Interface",
					},
					ObjectMeta: api.ObjectMeta{
						Tenant:    "default",
						Namespace: "default",
						Name:      fmt.Sprintf("%s%d", types.LifPrefix, lif.Spec.GetId()),
					},
					Spec: netproto.InterfaceSpec{
						Type: "LIF",
					},
					Status: netproto.InterfaceStatus{
						InterfaceID: intfID,
						IFHostStatus: netproto.InterfaceHostStatus{
							HostIfName: lif.Status.GetName(),
						},
						OperStatus: lif.Status.GetStatus().String(),
					},
				}
				b, _ := json.MarshalIndent(l, "", "   ")
				fmt.Println(string(b))
				dat, _ := l.Marshal()
				if err := a.InfraAPI.Store(l.Kind, l.GetKey(), dat); err != nil {
					log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Lif: %s | Lif: %v", l.GetKey(), err))
				}
			}
		}
	}(eventStream)

	// Store initial Lifs
	for _, lif := range lifs.Response {
		log.Infof("Processing lif get response. Resp: %v", lif)
		intfID, _ := binary.Uvarint(lif.Spec.GetId())
		l := netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("%s%d", types.LifPrefix, intfID),
			},
			Spec: netproto.InterfaceSpec{
				Type: "LIF",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: intfID,
				IFHostStatus: netproto.InterfaceHostStatus{
					HostIfName: lif.Status.GetName(),
				},
				OperStatus: lif.Status.GetStatus().String(),
			},
		}
		dat, _ := l.Marshal()
		if err := a.InfraAPI.Store(l.Kind, l.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Lif: %s | Lif: %v", l.GetKey(), err))
		}
	}

	// handle the ports
	for _, port := range ports.Response {
		log.Infof("Processing port get response. Resp: %v", port)
		portID := uint64(port.Spec.GetId())
		p := netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("%s%d", types.EthPrefix, portID),
			},
			Spec: netproto.InterfaceSpec{
				Type: "Eth",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: portID,
				OperStatus:  port.Status.GetLinkStatus().GetOperState().String(),
			},
		}
		dat, _ := p.Marshal()
		if err := a.InfraAPI.Store(p.Kind, p.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Port: %s | Port: %v", p.GetKey(), err))
		}
	}
}
