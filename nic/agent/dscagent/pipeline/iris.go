// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build iris

package pipeline

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/proto"
	protoTypes "github.com/gogo/protobuf/types"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris"
	irisUtils "github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// IrisAPI implements PipelineAPI for Iris Pipeline
type IrisAPI struct {
	sync.Mutex
	InfraAPI        types.InfraAPI
	VrfClient       halapi.VrfClient
	L2SegmentClient halapi.L2SegmentClient
	EpClient        halapi.EndpointClient
	IntfClient      halapi.InterfaceClient
	NspClient       halapi.NwSecurityClient
	EventClient     halapi.EventClient
	PortClient      halapi.PortClient
	TelemetryClient halapi.TelemetryClient
}

// NewPipelineAPI returns the implementer or PipelineAPI for Iris Pipeline
func NewPipelineAPI(infraAPI types.InfraAPI) (*IrisAPI, error) {

	conn, err := utils.CreateNewGRPCClient("HAL_GRPC_PORT", types.HalGRPCDefaultPort)
	if err != nil {
		log.Errorf("Failed to create GRPC Connection to HAL. Err: %v", err)
		return nil, err
	}

	i := IrisAPI{
		InfraAPI:        infraAPI,
		VrfClient:       halapi.NewVrfClient(conn),
		L2SegmentClient: halapi.NewL2SegmentClient(conn),
		EpClient:        halapi.NewEndpointClient(conn),
		IntfClient:      halapi.NewInterfaceClient(conn),
		NspClient:       halapi.NewNwSecurityClient(conn),
		EventClient:     halapi.NewEventClient(conn),
		PortClient:      halapi.NewPortClient(conn),
		TelemetryClient: halapi.NewTelemetryClient(conn),
	}

	if err := i.PipelineInit(); err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineInit, "Iris Init: %v", err))
		return nil, errors.Wrapf(types.ErrPipelineInit, "Iris Init: %v", err)
	}

	return &i, nil
}

// ############################################### PipelineAPI Methods  ###############################################

// PipelineInit does Iris Pipeline init. Creating default Vrfs, Untagged collector network and uplinks
func (i *IrisAPI) PipelineInit() error {
	log.Infof("Iris API: %s", types.InfoPipelineInit)
	c, _ := protoTypes.TimestampProto(time.Now())
	defaultVrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
		Spec: netproto.VrfSpec{
			VrfType: "CUSTOMER",
		},
		Status: netproto.VrfStatus{
			VrfID: 65,
		},
	}
	if _, err := i.HandleVrf(types.Create, defaultVrf); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Iris API: %s | %s", types.InfoPipelineInit, types.InfoDefaultVrfCreate)

	defaultNetwork := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      types.InternalDefaultUntaggedNetwork,
			CreationTime: api.Timestamp{
				Timestamp: *c,
			},
			ModTime: api.Timestamp{
				Timestamp: *c,
			},
		},
		Spec: netproto.NetworkSpec{
			VlanID: types.UntaggedCollVLAN, // Untagged
		},
		Status: netproto.NetworkStatus{NetworkID: types.UntaggedCollVLAN},
	}

	if _, err := i.HandleNetwork(types.Create, defaultNetwork); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Iris API: %s | %s", types.InfoPipelineInit, types.InfoDefaultUntaggedNetworkCreate)

	i.initLifStream()

	if err := i.createPortsAndUplinks(); err != nil {
		log.Error(err)
		return err
	}

	return nil
}

// HandleVrf handles CRUD Methods for Vrf Object
func (i *IrisAPI) HandleVrf(oper types.Operation, vrf netproto.Vrf) (vrfs []netproto.Vrf, err error) {
	i.Lock()
	defer i.Unlock()

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
		dat, err = i.InfraAPI.Read(vrf.Kind, vrf.GetKey())
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
		dat, err = i.InfraAPI.List(vrf.Kind)
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
			vrf.Status.VrfID = i.InfraAPI.AllocateID(types.VrfID, types.VrfOffSet)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingVrf netproto.Vrf
		dat, err := i.InfraAPI.Read(vrf.Kind, vrf.GetKey())
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
		dat, err := i.InfraAPI.Read(vrf.Kind, vrf.GetKey())
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
	err = iris.HandleVrf(i.InfraAPI, i.VrfClient, oper, vrf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleNetwork handles CRUD Methods for Network Object
func (i *IrisAPI) HandleNetwork(oper types.Operation, network netproto.Network) (networks []netproto.Network, err error) {
	i.Lock()
	defer i.Unlock()

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
		dat, err = i.InfraAPI.Read(network.Kind, network.GetKey())
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
		dat, err = i.InfraAPI.List(network.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", network.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var network netproto.Network
			err = proto.Unmarshal(o, &network)
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
			network.Status.NetworkID = i.InfraAPI.AllocateID(types.NetworkID, types.NetworkOffSet)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingNetwork netproto.Network
		dat, err := i.InfraAPI.Read(network.Kind, network.GetKey())
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
		dat, err := i.InfraAPI.Read(network.Kind, network.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingNetwork.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", network.GetKey(), err)
		}
		network = existingNetwork
	}
	// Perform object validations
	uplinkIDs, vrf, err := validator.ValidateNetwork(i.InfraAPI, network)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	log.Infof("Network: %v | Op: %s | %s", network, oper, types.InfoHandleObjBegin)
	defer log.Infof("Network: %v | Op: %s | %s", network, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleL2Segment(i.InfraAPI, i.L2SegmentClient, oper, network, vrf.Status.VrfID, uplinkIDs)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleEndpoint handles CRUD Methods for Endpoint Object
func (i *IrisAPI) HandleEndpoint(oper types.Operation, endpoint netproto.Endpoint) (endpoints []netproto.Endpoint, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, endpoint.Kind, endpoint.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Endpoint
		)
		dat, err = i.InfraAPI.Read(endpoint.Kind, endpoint.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}
		endpoints = append(endpoints, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(endpoint.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var endpoint netproto.Endpoint
			err := proto.Unmarshal(o, &endpoint)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
				continue
			}
			endpoints = append(endpoints, endpoint)
		}

		return
	case types.Create:
		if i.isLocalEP(endpoint.Spec.NodeUUID) {
			endpoint.Status.EnicID = i.InfraAPI.AllocateID(types.EnicID, types.EnicOffset)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingEndpoint netproto.Endpoint
		dat, err := i.InfraAPI.Read(endpoint.Kind, endpoint.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Endpoint: %s | Err: %v", endpoint.GetKey(), types.ErrObjNotFound)
		}
		err = existingEndpoint.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&endpoint.Spec, &existingEndpoint.Spec) {
			//log.Infof("Endpoint: %s | Info: %s ", endpoint.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		if i.isLocalEP(endpoint.Spec.NodeUUID) {
			endpoint.Status.EnicID = existingEndpoint.Status.EnicID
		}

	case types.Delete:
		var existingEndpoint netproto.Endpoint
		dat, err := i.InfraAPI.Read(endpoint.Kind, endpoint.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingEndpoint.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}
		endpoint = existingEndpoint
	}

	// Perform object validations
	network, vrf, err := validator.ValidateEndpoint(i.InfraAPI, endpoint)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("Endpoint: %v | Op: %s | %s", endpoint, oper, types.InfoHandleObjBegin)
	defer log.Infof("Endpoint: %v | Op: %s | %s", endpoint, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleEndpoint(i.InfraAPI, i.EpClient, i.IntfClient, oper, endpoint, vrf.Status.VrfID, network.Status.NetworkID)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleInterface handles CRUD Methods for Interface Object
func (i *IrisAPI) HandleInterface(oper types.Operation, intf netproto.Interface) (intfs []netproto.Interface, err error) {
	i.Lock()
	defer i.Unlock()

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
		dat, err = i.InfraAPI.Read(intf.Kind, intf.GetKey())
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
		dat, err = i.InfraAPI.List(intf.Kind)
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
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays.
		if intf.Status.InterfaceID == 0 {
			intf.Status.InterfaceID = i.InfraAPI.AllocateID(types.InterfaceID, types.UplinkOffset)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingInterface netproto.Interface
		dat, err := i.InfraAPI.Read(intf.Kind, intf.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound)
		}
		err = existingInterface.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&intf.Spec, &existingInterface.Spec) {
			//log.Infof("Interface: %s | Info: %s ", intf.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		intf.Status.InterfaceID = existingInterface.Status.InterfaceID
	case types.Delete:
		var existingInterface netproto.Interface
		dat, err := i.InfraAPI.Read(intf.Kind, intf.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingInterface.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		intf = existingInterface
	}
	// Perform object validations
	err = validator.ValidateInterface(intf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjBegin)
	defer log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleInterface(i.InfraAPI, i.IntfClient, oper, intf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleTunnel handles CRUD Methods for Tunnel Object
func (i *IrisAPI) HandleTunnel(oper types.Operation, tunnel netproto.Tunnel) (tunnels []netproto.Tunnel, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, tunnel.Kind, tunnel.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Tunnel
		)
		dat, err = i.InfraAPI.Read(tunnel.Kind, tunnel.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}
		tunnels = append(tunnels, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(tunnel.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var tunnel netproto.Tunnel
			err := tunnel.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
				continue
			}
			tunnels = append(tunnels, tunnel)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if tunnel.Status.TunnelID == 0 {
			tunnel.Status.TunnelID = i.InfraAPI.AllocateID(types.TunnelID, types.TunnelOffset)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingTunnel netproto.Tunnel
		dat, err := i.InfraAPI.Read(tunnel.Kind, tunnel.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Tunnel: %s | Err: %v", tunnel.GetKey(), types.ErrObjNotFound)
		}
		err = existingTunnel.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&tunnel.Spec, &existingTunnel.Spec) {
			//log.Infof("Tunnel: %s | Info: %s ", tunnel.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		tunnel.Status.TunnelID = existingTunnel.Status.TunnelID
	case types.Delete:
		var existingTunnel netproto.Tunnel
		dat, err := i.InfraAPI.Read(tunnel.Kind, tunnel.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingTunnel.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}
		tunnel = existingTunnel
	}

	// Perform object validations
	vrf, err := validator.ValidateTunnel(i.InfraAPI, tunnel)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("Tunnel: %v | Op: %s | %s", tunnel, oper, types.InfoHandleObjBegin)
	defer log.Infof("Tunnel: %v | Op: %s | %s", tunnel, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleTunnel(i.InfraAPI, i.IntfClient, oper, tunnel, vrf.Status.VrfID)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleApp handles CRUD Methods for App Object
func (i *IrisAPI) HandleApp(oper types.Operation, app netproto.App) (apps []netproto.App, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, app.Kind, app.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}
	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.App
		)
		dat, err = i.InfraAPI.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}
		apps = append(apps, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(app.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var app netproto.App
			err := proto.Unmarshal(o, &app)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
				continue
			}
			apps = append(apps, app)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if app.Status.AppID == 0 {
			app.Status.AppID = i.InfraAPI.AllocateID(types.AppID, 0)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingApp netproto.App
		dat, err := i.InfraAPI.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
		}
		err = existingApp.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&app.Spec, &existingApp.Spec) {
			//log.Infof("App: %s | Info: %s ", app.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		app.Status.AppID = existingApp.Status.AppID
	case types.Delete:
		var existingApp netproto.App
		dat, err := i.InfraAPI.Read(app.Kind, app.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingApp.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
		}
		app = existingApp
	}
	// Perform object validations
	err = validator.ValidateApp(app)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("App: %v | Op: %s | %s", app, oper, types.InfoHandleObjBegin)
	defer log.Infof("App: %v | Op: %s | %s", app, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleApp(i.InfraAPI, oper, app)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	// TODO Trigger this from NPM's OnAppUpdate method
	if oper == types.Update {
		//
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List("NetworkSecurityPolicy")
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Failed to list NetworkSecurityPolicies | Err: %v", err))
		}

		for _, o := range dat {
			var nsp netproto.NetworkSecurityPolicy
			err := nsp.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
				continue
			}
			for _, r := range nsp.Spec.Rules {
				if r.AppName == app.Name {
					// Update NetworkSecurity Policy here
					vrf, ruleIDToAppMapping, err := validator.ValidateNetworkSecurityPolicy(i.InfraAPI, nsp)
					if err != nil {
						break
					}
					if err := iris.HandleNetworkSecurityPolicy(i.InfraAPI, i.NspClient, types.Update, nsp, vrf.Status.VrfID, ruleIDToAppMapping); err == nil {
						break
					}
				}
			}
		}
	}

	return
}

// HandleNetworkSecurityPolicy handles CRUD Methods for NetworkSecurityPolicy Object
func (i *IrisAPI) HandleNetworkSecurityPolicy(oper types.Operation, nsp netproto.NetworkSecurityPolicy) (netSecPolicies []netproto.NetworkSecurityPolicy, err error) {
	i.Lock()
	defer i.Unlock()
	err = utils.ValidateMeta(oper, nsp.Kind, nsp.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.NetworkSecurityPolicy
		)
		dat, err = i.InfraAPI.Read(nsp.Kind, nsp.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}
		netSecPolicies = append(netSecPolicies, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(nsp.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var nsp netproto.NetworkSecurityPolicy
			err := nsp.Unmarshal(o)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
				continue
			}
			netSecPolicies = append(netSecPolicies, nsp)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if nsp.Status.NetworkSecurityPolicyID == 0 {
			nsp.Status.NetworkSecurityPolicyID = i.InfraAPI.AllocateID(types.NetworkSecurityPolicyID, 0)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingNetworkSecurityPolicy netproto.NetworkSecurityPolicy
		dat, err := i.InfraAPI.Read(nsp.Kind, nsp.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrObjNotFound)
		}

		err = existingNetworkSecurityPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}

		if proto.Equal(&nsp.Spec, &existingNetworkSecurityPolicy.Spec) {
			return nil, nil
		}

		// Reuse ID from store
		nsp.Status.NetworkSecurityPolicyID = existingNetworkSecurityPolicy.Status.NetworkSecurityPolicyID
		log.Infof("Existing NetworkSecurityPolicy : %v", existingNetworkSecurityPolicy)
		log.Infof("New NetworkSecurityPolicy: %v", nsp)

	case types.Delete:
		var existingNetworkSecurityPolicy netproto.NetworkSecurityPolicy
		dat, err := i.InfraAPI.Read(nsp.Kind, nsp.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingNetworkSecurityPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}
		nsp = existingNetworkSecurityPolicy

	}

	// Perform object validations
	vrf, ruleIDToAppMapping, err := validator.ValidateNetworkSecurityPolicy(i.InfraAPI, nsp)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("NetworkSecurityPolicy: %v | Op: %s | %s", nsp.GetKey(), oper, types.InfoHandleObjBegin)
	defer log.Infof("NetworkSecurityPolicy: %v | Op: %s | %s", nsp.GetKey(), oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleNetworkSecurityPolicy(i.InfraAPI, i.NspClient, oper, nsp, vrf.Status.VrfID, ruleIDToAppMapping)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleSecurityProfile handles CRUD Methods for SecurityProfile Object
func (i *IrisAPI) HandleSecurityProfile(oper types.Operation, profile netproto.SecurityProfile) (profiles []netproto.SecurityProfile, err error) {
	i.Lock()
	defer i.Unlock()

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
		dat, err = i.InfraAPI.Read(profile.Kind, profile.GetKey())
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
		dat, err = i.InfraAPI.List(profile.Kind)
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
			profile.Status.SecurityProfileID = i.InfraAPI.AllocateID(types.SecurityProfileID, types.SecurityProfileOffSet)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingSecurityProfile netproto.SecurityProfile
		dat, err := i.InfraAPI.Read(profile.Kind, profile.GetKey())
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
		dat, err := i.InfraAPI.Read(profile.Kind, profile.GetKey())
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
	vrf, err := validator.ValidateSecurityProfile(i.InfraAPI, profile)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleSecurityProfile(i.InfraAPI, i.NspClient, i.VrfClient, oper, profile, vrf)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleMirrorSession handles CRUD Methods for MirrorSession Object
func (i *IrisAPI) HandleMirrorSession(oper types.Operation, mirror netproto.MirrorSession) (mirrors []netproto.MirrorSession, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, mirror.Kind, mirror.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.MirrorSession
		)
		dat, err = i.InfraAPI.Read(mirror.Kind, mirror.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		mirrors = append(mirrors, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(mirror.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var mirror netproto.MirrorSession
			err := proto.Unmarshal(o, &mirror)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
				continue
			}
			mirrors = append(mirrors, mirror)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if mirror.Status.MirrorSessionID == 0 {
			mirror.Status.MirrorSessionID = i.InfraAPI.AllocateID(types.MirrorSessionID, 0)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingMirrorSession netproto.MirrorSession
		dat, err := i.InfraAPI.Read(mirror.Kind, mirror.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrObjNotFound)
		}
		err = existingMirrorSession.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&mirror.Spec, &existingMirrorSession.Spec) {
			//log.Infof("MirrorSession: %s | Info: %s ", mirror.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		mirror.Status.MirrorSessionID = existingMirrorSession.Status.MirrorSessionID
	case types.Delete:
		var existingMirrorSession netproto.MirrorSession
		dat, err := i.InfraAPI.Read(mirror.Kind, mirror.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingMirrorSession.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		mirror = existingMirrorSession
	}
	log.Infof("MirrorSession: %v | Op: %s | %s", mirror, oper, types.InfoHandleObjBegin)
	defer log.Infof("MirrorSession: %v | Op: %s | %s", mirror, oper, types.InfoHandleObjEnd)

	// Perform object validations
	vrf, err := validator.ValidateMirrorSession(i.InfraAPI, mirror)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	// Update Mirror session ID to be under 8. TODO remove this once HAL doesn't rely on agents to provide its hardware ID
	mirror.Status.MirrorSessionID = mirror.Status.MirrorSessionID % types.MaxMirrorSessions
	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleMirrorSession(i.InfraAPI, i.TelemetryClient, i.IntfClient, i.EpClient, oper, mirror, vrf.Status.VrfID)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleFlowExportPolicy handles CRUD Methods for FlowExportPolicy Object
func (i *IrisAPI) HandleFlowExportPolicy(oper types.Operation, netflow netproto.FlowExportPolicy) (netflows []netproto.FlowExportPolicy, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, netflow.Kind, netflow.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.FlowExportPolicy
		)
		dat, err = i.InfraAPI.Read(netflow.Kind, netflow.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		netflows = append(netflows, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(netflow.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var netflow netproto.FlowExportPolicy
			err := proto.Unmarshal(o, &netflow)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
				continue
			}
			netflows = append(netflows, netflow)
		}

		return
	case types.Create:
		// Alloc ID if ID field is empty. This will be pre-populated in case of config replays
		if netflow.Status.FlowExportPolicyID == 0 {
			netflow.Status.FlowExportPolicyID = i.InfraAPI.AllocateID(types.FlowExportPolicyID, 0)
		}

	case types.Update:
		// Get to ensure that the object exists
		var existingFlowExportPolicy netproto.FlowExportPolicy
		dat, err := i.InfraAPI.Read(netflow.Kind, netflow.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), types.ErrObjNotFound)
		}
		err = existingFlowExportPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&netflow.Spec, &existingFlowExportPolicy.Spec) {
			//log.Infof("FlowExportPolicy: %s | Info: %s ", netflow.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		netflow.Status.FlowExportPolicyID = existingFlowExportPolicy.Status.FlowExportPolicyID
	case types.Delete:
		var existingFlowExportPolicy netproto.FlowExportPolicy
		dat, err := i.InfraAPI.Read(netflow.Kind, netflow.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingFlowExportPolicy.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		netflow = existingFlowExportPolicy
	}
	log.Infof("FlowExportPolicy: %v | Op: %s | %s", netflow, oper, types.InfoHandleObjBegin)
	defer log.Infof("FlowExportPolicy: %v | Op: %s | %s", netflow, oper, types.InfoHandleObjEnd)

	// Perform object validations
	vrf, err := validator.ValidateFlowExportPolicy(i.InfraAPI, netflow)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleFlowExportPolicy(i.InfraAPI, i.TelemetryClient, i.IntfClient, i.EpClient, oper, netflow, vrf.Status.VrfID)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleIPAMPolicy handles CRUD methods for IPAMPolicy
func (i *IrisAPI) HandleIPAMPolicy(oper types.Operation, policy netproto.IPAMPolicy) (policies []netproto.IPAMPolicy, err error) {
	return nil, err
}

// ReplayConfigs replays last known configs from boltDB
func (i *IrisAPI) ReplayConfigs() error {
	// Replay Network Object
	networks, err := i.InfraAPI.List("Network")
	if err == nil {
		for _, o := range networks {
			var network netproto.Network
			err := network.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Network. Err: %v", err)
				continue
			}
			creator, ok := network.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Network objects")
				if _, err := i.HandleNetwork(types.Create, network); err != nil {
					log.Errorf("Failed to recreate Network: %v. Err: %v", network.GetKey(), err)
				}
			}
		}
	}

	// Replay Endpoint Object
	endpoints, err := i.InfraAPI.List("Endpoint")
	if err == nil {
		for _, o := range endpoints {
			var endpoint netproto.Endpoint
			err := endpoint.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Endpoint. Err: %v", err)
				continue
			}
			creator, ok := endpoint.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Endpoint objects")
				if _, err := i.HandleEndpoint(types.Create, endpoint); err != nil {
					log.Errorf("Failed to recreate Endpoint: %v. Err: %v", endpoint.GetKey(), err)
				}
			}
		}
	}

	// Replay Tunnel Object
	tunnels, err := i.InfraAPI.List("Tunnel")
	if err == nil {
		for _, o := range tunnels {
			var tunnel netproto.Tunnel
			err := tunnel.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Tunnel. Err: %v", err)
				continue
			}
			creator, ok := tunnel.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Tunnel objects")
				if _, err := i.HandleTunnel(types.Create, tunnel); err != nil {
					log.Errorf("Failed to recreate Tunnel: %v. Err: %v", tunnel.GetKey(), err)
				}
			}
		}
	}

	// Replay NetworkSecurityPolicy Object
	policies, err := i.InfraAPI.List("NetworkSecurityPolicy")
	if err == nil {
		for _, o := range policies {
			var sgp netproto.NetworkSecurityPolicy
			err := sgp.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to NetworkSecurityPolicy. Err: %v", err)
				continue
			}
			creator, ok := sgp.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted NetworkSecurityPolicy objects")
				if _, err := i.HandleNetworkSecurityPolicy(types.Create, sgp); err != nil {
					log.Errorf("Failed to recreate NetworkSecurityPolicy: %v. Err: %v", sgp.GetKey(), err)
				}
			}
		}
	}
	return nil
}

// PurgeConfigs deletes all configs on Naples Decommission
func (i *IrisAPI) PurgeConfigs() error {
	// Apps, SGPolicies, Endpoints,  Networks
	a := netproto.App{TypeMeta: api.TypeMeta{Kind: "App"}}
	apps, _ := i.HandleApp(types.List, a)
	for _, app := range apps {
		if _, err := i.HandleApp(types.Delete, app); err != nil {
			log.Errorf("Failed to purge the App. Err: %v", err)
		}
	}

	s := netproto.NetworkSecurityPolicy{TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"}}
	policies, _ := i.HandleNetworkSecurityPolicy(types.List, s)
	for _, policy := range policies {
		if _, err := i.HandleNetworkSecurityPolicy(types.Delete, policy); err != nil {
			log.Errorf("Failed to purge the NetworkSecurityPolicy. Err: %v", err)
		}
	}

	e := netproto.Endpoint{TypeMeta: api.TypeMeta{Kind: "Endpoint"}}
	endpoints, _ := i.HandleEndpoint(types.List, e)
	for _, endpoint := range endpoints {
		if strings.Contains(endpoint.Name, "_internal") {
			continue
		}
		if _, err := i.HandleEndpoint(types.Delete, endpoint); err != nil {
			log.Errorf("Failed to purge the Endpoint. Err: %v", err)
		}
	}

	n := netproto.Network{TypeMeta: api.TypeMeta{Kind: "Network"}}
	networks, _ := i.HandleNetwork(types.List, n)
	for _, network := range networks {
		if strings.Contains(network.Name, "_internal") {
			continue
		}
		if _, err := i.HandleNetwork(types.Delete, network); err != nil {
			log.Errorf("Failed to purge the Network. Err: %v", err)
		}
	}

	return nil
}

// GetWatchOptions returns the options to be used while establishing a watch from this agent.
func (i *IrisAPI) GetWatchOptions(ctx context.Context, kind string) (ret api.ListWatchOptions) {
	switch kind {
	case "Endpoint":
		ret.Name = i.InfraAPI.GetDscName()
	}
	return ret
}

// ############################################### Helper Methods  ###############################################
func (i *IrisAPI) initLifStream() {

	lifReqMsg := &halapi.LifGetRequestMsg{
		Request: []*halapi.LifGetRequest{
			{},
		},
	}
	evtReqMsg := &halapi.EventRequest{
		EventId:        halapi.EventId_EVENT_ID_LIF_ADD_UPDATE,
		EventOperation: halapi.EventOp_EVENT_OP_SUBSCRIBE,
	}

	lifs, err := i.IntfClient.LifGet(context.Background(), lifReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineLifGet, "Iris Init: %v", err))
	}
	log.Infof("Iris API: %s | %s", types.InfoPipelineInit, types.InfoSingletonLifGet)

	lifStream, err := i.EventClient.EventListen(context.Background(), evtReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineEventListen, "Iris Init: %v", err))
	}

	go func(stream halapi.Event_EventListenClient) {
		for {
			resp, err := stream.Recv()
			if err != io.EOF {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "Iris init: %v", err))
				break
			}

			if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Error(errors.Wrapf(types.ErrDatapathHandling, "Iris Init: %v", err))
			}

			lif := resp.GetLifEvent()

			l := netproto.Interface{
				TypeMeta: api.TypeMeta{
					Kind: "Interface",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    "default",
					Namespace: "default",
					Name:      fmt.Sprintf("%s%d", types.LifPrefix, lif.Spec.KeyOrHandle.GetLifId()),
				},
				Spec: netproto.InterfaceSpec{
					Type: "LIF",
				},
				Status: netproto.InterfaceStatus{
					InterfaceID: lif.Spec.KeyOrHandle.GetLifId(),
					IFHostStatus: netproto.InterfaceHostStatus{
						HostIfName: lif.Spec.GetName(),
					},
					OperStatus: lif.Status.GetLifStatus().String(),
				},
			}
			b, _ := json.MarshalIndent(l, "", "   ")
			fmt.Println(string(b))
			dat, _ := l.Marshal()
			if err := i.InfraAPI.Store(l.Kind, l.GetKey(), dat); err != nil {
				log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Lif: %s | Lif: %v", l.GetKey(), err))
			}

		}

	}(lifStream)

	// Store initial Lifs
	for _, lif := range lifs.Response {
		l := netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("%s%d", types.LifPrefix, lif.Spec.KeyOrHandle.GetLifId()),
			},
			Spec: netproto.InterfaceSpec{
				Type: "LIF",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: lif.Spec.KeyOrHandle.GetLifId(),
				IFHostStatus: netproto.InterfaceHostStatus{
					HostIfName: lif.Spec.GetName(),
				},
				OperStatus: lif.Status.GetLifStatus().String(),
			},
		}
		dat, _ := l.Marshal()
		if err := i.InfraAPI.Store(l.Kind, l.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Lif: %s | Lif: %v", l.GetKey(), err))
		}
	}

}

// TODO Remove PortCreates once the linkmgr changes are stable
func (i *IrisAPI) createPortsAndUplinks() error {
	portReqMsg := &halapi.PortInfoGetRequestMsg{
		Request: []*halapi.PortInfoGetRequest{{}},
	}

	ports, err := i.PortClient.PortInfoGet(context.Background(), portReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelinePortGet, "Iris Init: %v", err))
		return errors.Wrapf(types.ErrPipelinePortGet, "Iris Init: %v", err)
	}

	var (
		pReq     []*halapi.PortSpec
		numLanes uint32
		uReq     []*halapi.InterfaceSpec
		uplinks  []netproto.Interface
	)

	for idx, port := range ports.Response {
		var (
			portSpeed             halapi.PortSpeed
			autoNeg, isMgmtUplink bool
			fecType               halapi.PortFecType
			uplinkType            string
		)

		// Set appropriate types for ports
		if port.Spec.PortType == halapi.PortType_PORT_TYPE_MGMT {
			portSpeed = halapi.PortSpeed_PORT_SPEED_1G
			autoNeg = false
			fecType = halapi.PortFecType_PORT_FEC_TYPE_NONE
			uplinkType = "UPLINK_MGMT"
			isMgmtUplink = true
		} else {
			portSpeed = halapi.PortSpeed_PORT_SPEED_100G
			autoNeg = true
			fecType = halapi.PortFecType_PORT_FEC_TYPE_RS
			uplinkType = "UPLINK_ETH"
			isMgmtUplink = false
		}

		portID := 1 + numLanes
		numLanes += port.Spec.NumLanes

		p := &halapi.PortSpec{
			KeyOrHandle: &halapi.PortKeyHandle{
				KeyOrHandle: &halapi.PortKeyHandle_PortId{
					PortId: portID,
				},
			},
			PortType:      port.Spec.PortType,
			AdminState:    port.Spec.AdminState,
			PortSpeed:     portSpeed,
			NumLanes:      port.Spec.NumLanes,
			FecType:       fecType,
			AutoNegEnable: autoNeg,
			Pause:         halapi.PortPauseType_PORT_PAUSE_TYPE_LINK,
			TxPauseEnable: true,
			RxPauseEnable: true,
		}
		uplinkID := uint64(types.UplinkOffset + idx + 1)
		uplink := netproto.Interface{
			TypeMeta: api.TypeMeta{
				Kind: "Interface",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      fmt.Sprintf("%s%d", types.UplinkPrefix, uplinkID),
			},
			Spec: netproto.InterfaceSpec{
				Type:        uplinkType,
				AdminStatus: "UP",
			},
			Status: netproto.InterfaceStatus{
				InterfaceID: uplinkID,
				IFUplinkStatus: netproto.InterfaceUplinkStatus{
					PortID: portID,
				},
			},
		}

		u := &halapi.InterfaceSpec{
			KeyOrHandle: &halapi.InterfaceKeyHandle{
				KeyOrHandle: &halapi.InterfaceKeyHandle_InterfaceId{
					InterfaceId: uplinkID,
				},
			},
			Type: halapi.IfType_IF_TYPE_UPLINK,
			IfInfo: &halapi.InterfaceSpec_IfUplinkInfo{
				IfUplinkInfo: &halapi.IfUplinkInfo{
					PortNum:         uplink.Status.IFUplinkStatus.PortID,
					IsOobManagement: isMgmtUplink,
				},
			},
		}

		pReq = append(pReq, p)
		uReq = append(uReq, u)
		uplinks = append(uplinks, uplink)
	}

	portCreateReqMsg := &halapi.PortRequestMsg{
		Request: pReq,
	}

	uplinkCreateReqMsg := &halapi.InterfaceRequestMsg{
		Request: uReq,
	}

	pResp, err := i.PortClient.PortCreate(context.Background(), portCreateReqMsg)
	if pResp != nil {
		if err := irisUtils.HandleErr(types.Create, pResp.Response[0].ApiStatus, err, fmt.Sprintf("Create failed for Ports. Err: %v", err)); err != nil {
			return err
		}
	}

	uResp, err := i.IntfClient.InterfaceCreate(context.Background(), uplinkCreateReqMsg)
	if uResp != nil {
		if err := irisUtils.HandleErr(types.Create, uResp.Response[0].ApiStatus, err, fmt.Sprintf("Create failed for Uplinks. Err: %v", err)); err != nil {
			return err
		}
	}

	for _, uplink := range uplinks {
		dat, _ := uplink.Marshal()
		if err := i.InfraAPI.Store(uplink.Kind, uplink.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Uplink: %s | Uplink: %v", uplink.GetKey(), err))
		}
	}

	return nil
}

func (i *IrisAPI) isLocalEP(nodeuuid string) bool {
	log.Infof("Node UUID: %s | Self Node UUID: %s", nodeuuid, i.InfraAPI.GetDscName())
	epNodeUUID, _ := net.ParseMAC(nodeuuid)
	selfNodeUUID, _ := net.ParseMAC(i.InfraAPI.GetDscName())
	return epNodeUUID.String() == selfNodeUUID.String()
}
