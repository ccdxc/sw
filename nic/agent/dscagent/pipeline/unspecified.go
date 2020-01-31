// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build !iris
// +build !apulu

package pipeline

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"sync"
	"time"

	"github.com/gogo/protobuf/proto"
	protoTypes "github.com/gogo/protobuf/types"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// FakeAgentAPI implements PipelineAPI for FakeAgent Pipeline
type FakeAgentAPI struct {
	sync.Mutex
	InfraAPI    types.InfraAPI
	IntfClient  halapi.InterfaceClient
	EventClient halapi.EventClient
	PortClient  halapi.PortClient
}

// NewPipelineAPI returns the implementer or PipelineAPI for FakeAgent Pipeline
func NewPipelineAPI(infraAPI types.InfraAPI) (*FakeAgentAPI, error) {

	conn, err := utils.CreateNewGRPCClient("HAL_GRPC_PORT", types.HalGRPCDefaultPort)
	if err != nil {
		log.Errorf("Failed to create GRPC Connection to HAL. Err: %v", err)
		return nil, err
	}

	i := FakeAgentAPI{
		InfraAPI:    infraAPI,
		IntfClient:  halapi.NewInterfaceClient(conn),
		EventClient: halapi.NewEventClient(conn),
		PortClient:  halapi.NewPortClient(conn),
	}

	if err := i.PipelineInit(); err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineInit, "FakeAgent Init: %v", err))
		return nil, errors.Wrapf(types.ErrPipelineInit, "FakeAgent Init: %v", err)
	}

	return &i, nil
}

// ############################################### PipelineAPI Methods  ###############################################

// PipelineInit does FakeAgent Pipeline init. Creating default Vrfs, Untagged collector network and uplinks
func (i *FakeAgentAPI) PipelineInit() error {
	log.Infof("FakeAgent API: %s", types.InfoPipelineInit)
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
	log.Infof("FakeAgent API: %s | %s", types.InfoPipelineInit, types.InfoDefaultVrfCreate)

	defaultNetwork := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "_internal_untagged_nw",
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
	}

	if _, err := i.HandleNetwork(types.Create, defaultNetwork); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("FakeAgent API: %s | %s", types.InfoPipelineInit, types.InfoDefaultUntaggedNetworkCreate)

	i.initLifStream()

	if err := i.createPortsAndUplinks(); err != nil {
		log.Error(err)
		return err
	}

	return nil
}

// HandleVrf handles CRUD Methods for Vrf Object
func (i *FakeAgentAPI) HandleVrf(oper types.Operation, vrf netproto.Vrf) (vrfs []netproto.Vrf, err error) {
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

		vrfBytes, _ := vrf.Marshal()

		if err := i.InfraAPI.Store(vrf.Kind, vrf.GetKey(), vrfBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}
		return

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
		vrfBytes, _ := vrf.Marshal()

		if err := i.InfraAPI.Store(vrf.Kind, vrf.GetKey(), vrfBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}
		return nil, nil
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
		if err := i.InfraAPI.Delete(vrf.Kind, vrf.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Vrf: %s | Err: %v", vrf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Vrf: %s | Err: %v", vrf.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleNetwork handles CRUD Methods for Network Object
func (i *FakeAgentAPI) HandleNetwork(oper types.Operation, network netproto.Network) (networks []netproto.Network, err error) {
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
		networkBytes, _ := network.Marshal()

		if err := i.InfraAPI.Store(network.Kind, network.GetKey(), networkBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Err: %v", network.GetKey(), err)
		}
		return

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

		networkBytes, _ := network.Marshal()

		if err := i.InfraAPI.Store(network.Kind, network.GetKey(), networkBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Network: %s | Err: %v", network.GetKey(), err)
		}
		return nil, nil
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
		if err := i.InfraAPI.Delete(network.Kind, network.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", network.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", network.GetKey(), err)
		}
		return nil, nil
	}
	return
}

// HandleEndpoint handles CRUD Methods for Endpoint Object
func (i *FakeAgentAPI) HandleEndpoint(oper types.Operation, endpoint netproto.Endpoint) (endpoints []netproto.Endpoint, err error) {
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
		endpoint.Status.EnicID = i.InfraAPI.AllocateID(types.EnicID, types.EnicOffset)
		endpointBytes, _ := endpoint.Marshal()

		if err := i.InfraAPI.Store(endpoint.Kind, endpoint.GetKey(), endpointBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}
		return

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
		endpoint.Status.EnicID = existingEndpoint.Status.EnicID
		endpointBytes, _ := endpoint.Marshal()

		if err := i.InfraAPI.Store(endpoint.Kind, endpoint.GetKey(), endpointBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}
		return nil, nil
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
		if err := i.InfraAPI.Delete(endpoint.Kind, endpoint.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleInterface handles CRUD Methods for Interface Object
func (i *FakeAgentAPI) HandleInterface(oper types.Operation, intf netproto.Interface) (intfs []netproto.Interface, err error) {
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

		interfaceBytes, _ := intf.Marshal()

		if err := i.InfraAPI.Store(intf.Kind, intf.GetKey(), interfaceBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		return

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

		interfaceBytes, _ := intf.Marshal()

		if err := i.InfraAPI.Store(intf.Kind, intf.GetKey(), interfaceBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		return nil, nil
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
		if err := i.InfraAPI.Delete(intf.Kind, intf.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Interface: %s | Err: %v", intf.GetKey(), err)
		}
		return nil, nil
	}
	return
}

// HandleTunnel handles CRUD Methods for Tunnel Object
func (i *FakeAgentAPI) HandleTunnel(oper types.Operation, tunnel netproto.Tunnel) (tunnels []netproto.Tunnel, err error) {
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

		tunnelBytes, _ := tunnel.Marshal()

		if err := i.InfraAPI.Store(tunnel.Kind, tunnel.GetKey(), tunnelBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}
		return

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

		tunnelBytes, _ := tunnel.Marshal()

		if err := i.InfraAPI.Store(tunnel.Kind, tunnel.GetKey(), tunnelBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}
		return nil, nil

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

		if err := i.InfraAPI.Delete(tunnel.Kind, tunnel.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Tunnel: %s | Err: %v", tunnel.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Tunnel: %s | Err: %v", tunnel.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleApp handles CRUD Methods for App Object
func (i *FakeAgentAPI) HandleApp(oper types.Operation, app netproto.App) (apps []netproto.App, err error) {
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

		appBytes, _ := app.Marshal()

		if err := i.InfraAPI.Store(app.Kind, app.GetKey(), appBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | Err: %v", app.GetKey(), err)
		}
		return

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

		appBytes, _ := app.Marshal()

		if err := i.InfraAPI.Store(app.Kind, app.GetKey(), appBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "App: %s | Err: %v", app.GetKey(), err)
		}
		return nil, nil
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

		if err := i.InfraAPI.Delete(app.Kind, app.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "App: %s | Err: %v", app.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "App: %s | Err: %v", app.GetKey(), err)
		}
		return nil, nil
	}
	return
}

// HandleNetworkSecurityPolicy handles CRUD Methods for NetworkSecurityPolicy Object
func (i *FakeAgentAPI) HandleNetworkSecurityPolicy(oper types.Operation, nsp netproto.NetworkSecurityPolicy) (netSecPolicies []netproto.NetworkSecurityPolicy, err error) {
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

		nspBytes, _ := nsp.Marshal()

		if err := i.InfraAPI.Store(nsp.Kind, nsp.GetKey(), nspBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}
		return

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
		nspBytes, _ := nsp.Marshal()

		if err := i.InfraAPI.Store(nsp.Kind, nsp.GetKey(), nspBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}
		return nil, nil

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

		if err := i.InfraAPI.Delete(nsp.Kind, nsp.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
		}
		return nil, nil

	}
	return
}

// ValidateSecurityProfile validates the contents of SecurityProfile objects
func (i *FakeAgentAPI) ValidateSecurityProfile(profile netproto.SecurityProfile) (vrf netproto.Vrf, err error) {
	return vrf, nil
}

// HandleSecurityProfile handles CRUD Methods for SecurityProfile Object
func (i *FakeAgentAPI) HandleSecurityProfile(oper types.Operation, profile netproto.SecurityProfile) (profiles []netproto.SecurityProfile, err error) {
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
		profileBytes, _ := profile.Marshal()

		if err := i.InfraAPI.Store(profile.Kind, profile.GetKey(), profileBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil

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

		profileBytes, _ := profile.Marshal()
		if err := i.InfraAPI.Store(profile.Kind, profile.GetKey(), profileBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil

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
		if err := i.InfraAPI.Delete(profile.Kind, profile.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "SecurityProfile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "SecurityProfile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleMirrorSession handles CRUD methods for MirrorSession Object
func (i *FakeAgentAPI) HandleMirrorSession(oper types.Operation, mirror netproto.MirrorSession) (mirrors []netproto.MirrorSession, err error) {
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
		mirrorBytes, _ := mirror.Marshal()

		if err := i.InfraAPI.Store(mirror.Kind, mirror.GetKey(), mirrorBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		return nil, nil

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

		mirrorBytes, _ := mirror.Marshal()
		if err := i.InfraAPI.Store(mirror.Kind, mirror.GetKey(), mirrorBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		return nil, nil

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
		if err := i.InfraAPI.Delete(mirror.Kind, mirror.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "MirrorSession: %s | Err: %v", mirror.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleFlowExportPolicy handles CRUD methods for FlowExportPolicy Object
func (i *FakeAgentAPI) HandleFlowExportPolicy(oper types.Operation, netflow netproto.FlowExportPolicy) (netflows []netproto.FlowExportPolicy, err error) {
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
		netflowBytes, _ := netflow.Marshal()

		if err := i.InfraAPI.Store(netflow.Kind, netflow.GetKey(), netflowBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		return nil, nil

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

		netflowBytes, _ := netflow.Marshal()
		if err := i.InfraAPI.Store(netflow.Kind, netflow.GetKey(), netflowBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		return nil, nil

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
		if err := i.InfraAPI.Delete(netflow.Kind, netflow.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "FlowExportPolicy: %s | Err: %v", netflow.GetKey(), err)
		}
		return nil, nil
	}

	return
}

// HandleProfile handles CRUD Methods for Profile Object
func (i *FakeAgentAPI) HandleProfile(oper types.Operation, profile netproto.Profile) (profiles []netproto.Profile, err error) {
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
			obj netproto.Profile
		)
		dat, err = i.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err)
		}
		profiles = append(profiles, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(profile.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var profile netproto.Profile
			err := proto.Unmarshal(o, &profile)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err))
				continue
			}
			profiles = append(profiles, profile)
		}

		return
	case types.Create:
		profileBytes, _ := profile.Marshal()

		if err := i.InfraAPI.Store(profile.Kind, profile.GetKey(), profileBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Profile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil

	case types.Update:
		// Get to ensure that the object exists
		var existingProfile netproto.Profile
		dat, err := i.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Profile: %s | Err: %v", profile.GetKey(), types.ErrObjNotFound)
		}
		err = existingProfile.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&profile.Spec, &existingProfile.Spec) {
			//log.Infof("Profile: %s | Info: %s ", profile.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		profileBytes, _ := profile.Marshal()
		if err := i.InfraAPI.Store(profile.Kind, profile.GetKey(), profileBytes); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreCreate, "Profile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil

	case types.Delete:
		var existingProfile netproto.Profile
		dat, err := i.InfraAPI.Read(profile.Kind, profile.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingProfile.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Profile: %s | Err: %v", profile.GetKey(), err)
		}
		profile = existingProfile
		if err := i.InfraAPI.Delete(profile.Kind, profile.GetKey()); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Profile: %s | Err: %v", profile.GetKey(), err))
			return nil, errors.Wrapf(types.ErrBoltDBStoreDelete, "Profile: %s | Err: %v", profile.GetKey(), err)
		}
		return nil, nil
	}

	return
}

//func (i *FakeAgentAPI) HandleTelemetry(oper int, tm *netproto.Telemetry) ([]*netproto.Telemetry, error) {
//	return nil, nil
//}

// HandleDevice handles CRUD methods for Device objects
func (i *FakeAgentAPI) HandleDevice(oper types.Operation) error {
	return nil
}

// HandleIPAMPolicy handles CRUD methods for IPAMPolicy objects
func (i *FakeAgentAPI) HandleIPAMPolicy(oper types.Operation, policy netproto.IPAMPolicy) (policies []netproto.IPAMPolicy, err error) {
	return nil, err
}

// HandleCPRoutingConfig handles creation of control plane route objects
func (i *FakeAgentAPI) HandleCPRoutingConfig(obj types.DSCStaticRoute) error {
	return nil
}

// HandleRoutingConfig handles CRUDs for NetworkSecurityPolicy object
func (i *FakeAgentAPI) HandleRoutingConfig(oper types.Operation, obj netproto.RoutingConfig) (rtCfg []netproto.RoutingConfig, err error) {
	return nil, err
}

// ReplayConfigs replays last known configs from boltDB
func (i *FakeAgentAPI) ReplayConfigs() error {
	return nil
}

// PurgeConfigs deletes all configs on Naples Decommission
func (i *FakeAgentAPI) PurgeConfigs() error {
	return nil
}

// GetWatchOptions returns the options to be used while establishing a watch from this agent.
func (i *FakeAgentAPI) GetWatchOptions(ctx context.Context, kind string) (ret api.ListWatchOptions) {
	switch kind {
	case "Endpoint":
		ret.Name = i.InfraAPI.GetDscName()
	}
	return ret
}

// ############################################### Helper Methods  ###############################################
func (i *FakeAgentAPI) initLifStream() {

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
		log.Error(errors.Wrapf(types.ErrPipelineLifGet, "FakeAgent Init: %v", err))
	}
	log.Infof("FakeAgent API: %s | %s", types.InfoPipelineInit, types.InfoSingletonLifGet)

	lifStream, err := i.EventClient.EventListen(context.Background(), evtReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineEventListen, "FakeAgent Init: %v", err))
	}

	go func(stream halapi.Event_EventListenClient) {
		for {
			resp, err := stream.Recv()
			if err != io.EOF {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "FakeAgent init: %v", err))
				break
			}

			if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Error(errors.Wrapf(types.ErrDatapathHandling, "FakeAgent Init: %v", err))
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
func (i *FakeAgentAPI) createPortsAndUplinks() error {
	portReqMsg := &halapi.PortInfoGetRequestMsg{
		Request: []*halapi.PortInfoGetRequest{{}},
	}

	ports, err := i.PortClient.PortInfoGet(context.Background(), portReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelinePortGet, "FakeAgent Init: %v", err))
		return errors.Wrapf(types.ErrPipelinePortGet, "FakeAgent Init: %v", err)
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
		log.Errorf("Port Create Response: %v | Err: %v", pResp, err)
	}

	uResp, err := i.IntfClient.InterfaceCreate(context.Background(), uplinkCreateReqMsg)
	if uResp != nil {
		log.Errorf("Interface Create Response: %v | Err: %v", uResp, err)
	}

	for _, uplink := range uplinks {
		dat, _ := uplink.Marshal()
		if err := i.InfraAPI.Store(uplink.Kind, uplink.GetKey(), dat); err != nil {
			log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Uplink: %s | Uplink: %v", uplink.GetKey(), err))
		}
	}

	return nil
}

func (i *FakeAgentAPI) isLocalEP(nodeuuid string) bool {
	log.Infof("Node UUID: %s | Self Node UUID: %s", nodeuuid, i.InfraAPI.GetDscName())
	epNodeUUID, _ := net.ParseMAC(nodeuuid)
	selfNodeUUID, _ := net.ParseMAC(i.InfraAPI.GetDscName())
	return epNodeUUID.String() == selfNodeUUID.String()
}
