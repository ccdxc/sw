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

	"github.com/pensando/sw/nic/agent/protos/tsproto"

	"github.com/mdlayher/arp"

	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"

	"github.com/gogo/protobuf/proto"
	protoTypes "github.com/gogo/protobuf/types"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/common"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris"
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
	SysmgrClient    *sysmgr.Client
	InfraAPI        types.InfraAPI
	ControllerAPI   types.ControllerAPI
	VrfClient       halapi.VrfClient
	L2SegmentClient halapi.L2SegmentClient
	EpClient        halapi.EndpointClient
	IntfClient      halapi.InterfaceClient
	NspClient       halapi.NwSecurityClient
	EventClient     halapi.EventClient
	PortClient      halapi.PortClient
	TelemetryClient halapi.TelemetryClient
	SystemClient    halapi.SystemClient
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
		SystemClient:    halapi.NewSystemClient(conn),
	}

	delphiClient, err := delphi.NewClient(&i)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	i.SysmgrClient = sysmgr.NewClient(delphiClient, types.Netagent)
	go delphiClient.Run()

	if err := i.PipelineInit(); err != nil {
		log.Error(errors.Wrapf(types.ErrPipelineInit, "Iris Init: %v", err))
	}

	return &i, nil
}

// ############################################### SysMgr Methods  ###############################################

// OnMountComplete informs sysmgr that mount is done
func (i *IrisAPI) OnMountComplete() {
	i.SysmgrClient.InitDone()
}

// Name returns netagent svc name
func (i *IrisAPI) Name() string {
	return types.Netagent
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

	// Clean up stale objects from store. This will be recomputed during PipelineInit
	if err := i.InfraAPI.Delete(defaultVrf.Kind, defaultVrf.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Vrf: %s | Err: %v", defaultVrf.GetKey(), err))
	}

	if err := i.InfraAPI.Delete(defaultNetwork.Kind, defaultNetwork.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Network: %s | Err: %v", defaultNetwork.GetKey(), err))
	}

	if _, err := i.HandleVrf(types.Create, defaultVrf); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Iris API: %s | %s", types.InfoPipelineInit, types.InfoDefaultVrfCreate)

	if _, err := i.HandleNetwork(types.Create, defaultNetwork); err != nil {
		log.Error(err)
		return err
	}
	log.Infof("Iris API: %s | %s", types.InfoPipelineInit, types.InfoDefaultUntaggedNetworkCreate)

	// parse the uuid
	resp, err := i.SystemClient.SystemUUIDGet(context.Background(), &halapi.Empty{})
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Failed to get system uuid, err %v", err))
		return err
	}
	if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
		return fmt.Errorf("HAL returned non OK status. %v", resp.ApiStatus.String())
	}

	// Remove the colon from UUID
	uid := strings.Join(strings.Split(resp.Uuid, ":"), "")

	i.initLifStream(uid)

	if err := i.createPortsAndUplinks(uid); err != nil {
		log.Error(err)
		return err
	}

	return nil
}

// HandleVeniceCoordinates initializes the pipeline when VeniceCoordinates are discovered
func (i *IrisAPI) HandleVeniceCoordinates(dsc types.DistributedServiceCardStatus) {
	log.Infof("Iris API: received venice co-ordinates [%v]", dsc)
	ip, _, _ := net.ParseCIDR(i.InfraAPI.GetConfig().MgmtIP)
	mgmtIP := ip.String()
	mgmtLink := utils.GetMgmtLink(mgmtIP)
	if mgmtLink == nil {
		log.Errorf("Failed to get the mgmt interface. MgmtIP: %s", mgmtIP)
		return
	}
	log.Infof("Management Link: %v", mgmtLink.Attrs().Name)
	mgmtIntf, _ := net.InterfaceByName(mgmtLink.Attrs().Name)
	log.Infof("Management Inft: %v", mgmtIntf.Name)

	// Init Agent's ARP Client
	i.Lock()
	defer i.Unlock()
	// Check for idempotency and close older ARP clients
	if iris.ArpClient != nil {
		iris.ArpClient.Close()
	}
	client, err := arp.Dial(mgmtIntf)
	if err != nil {
		log.Errorf("Failed to initiate an ARP client. Err: %v", err)
		return
	}
	iris.ArpClient = client
	iris.MgmtLink = mgmtLink
	log.Infof("Starting the ARP watch loop")
	go iris.ResolveWatch()
}

// RegisterControllerAPI ensures the handles for controller API is appropriately set up
func (i *IrisAPI) RegisterControllerAPI(controllerAPI types.ControllerAPI) {
	log.Info("Setting the controller api")
	i.ControllerAPI = controllerAPI
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
	uplinkIDs, vrf, err := validator.ValidateNetwork(i.InfraAPI, oper, network)
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
		fallthrough
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
	collectorToIDMap := make(map[string]uint64)
	err = validator.ValidateInterface(i.InfraAPI, intf, collectorToIDMap)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjBegin)
	defer log.Infof("Interface: %v | Op: %s | %s", intf, oper, types.InfoHandleObjEnd)

	// Take a lock to ensure a single HAL API is active at any given point
	err = iris.HandleInterface(i.InfraAPI, i.IntfClient, oper, intf, collectorToIDMap)
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

	apps, err = common.HandleApp(i.InfraAPI, oper, app)
	if err != nil {
		return
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

// ValidateSecurityProfile validates the contents of SecurityProfile objects
func (i *IrisAPI) ValidateSecurityProfile(profile netproto.SecurityProfile) (vrf netproto.Vrf, err error) {
	vrf, err = validator.ValidateVrf(i.InfraAPI, profile.Tenant, profile.Namespace, types.DefaultVrf)
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
	vrf, err := i.ValidateSecurityProfile(profile)
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
	mirrorDestToKeys := map[string]int{}
	for dest, keys := range iris.MirrorDestToIDMapping {
		mirrorDestToKeys[dest] = len(keys.MirrorKeys)
	}
	vrf, err := validator.ValidateMirrorSession(i.InfraAPI, mirror, oper, mirrorDestToKeys)
	if err != nil {
		log.Error(err)
		return nil, err
	}
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
	collectorToKeys := map[string]int{}
	for dest, keys := range iris.CollectorToNetflow {
		collectorToKeys[dest] = len(keys.NetflowKeys)
	}
	vrf, err := validator.ValidateFlowExportPolicy(i.InfraAPI, netflow, oper, collectorToKeys)
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

// HandleRoutingConfig handles CRUDs for NetworkSecurityPolicy object
func (i *IrisAPI) HandleRoutingConfig(oper types.Operation, obj netproto.RoutingConfig) ([]netproto.RoutingConfig, error) {
	return nil, errors.Wrapf(types.ErrNotImplemented, "Routing Config not implemented by Iris Pipeline")
}

// HandleProfile handles CRUD Methods for Profile Object
func (i *IrisAPI) HandleProfile(oper types.Operation, profile netproto.Profile) (profiles []netproto.Profile, err error) {
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
			log.Infof("Profile: %s | Info: %s ", profile.GetKey(), types.Update)
			return nil, nil
		}

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

		// Take a lock to ensure a single HAL API is active at any given point
		if err := iris.HandleProfile(i.InfraAPI, i.SystemClient, oper, profile); err != nil {
			log.Error(err)
			return nil, err
		}

		return nil, nil

	}
	log.Infof("Profile: %v | Op: %s | %s", profile, oper, types.InfoHandleObjBegin)
	defer log.Infof("Profile: %v | Op: %s | %s", profile, oper, types.InfoHandleObjEnd)

	// Perform object validations
	if err := validator.ValidateProfile(profile); err != nil {
		log.Error(err)
		return nil, err
	}
	// Take a lock to ensure a single HAL API is active at any given point
	if err := iris.HandleProfile(i.InfraAPI, i.SystemClient, oper, profile); err != nil {
		log.Error(err)
		return nil, err
	}

	//ToDo: have to check for idemptonet calls??
	if strings.ToLower(profile.Spec.FwdMode) == strings.ToLower(netproto.ProfileSpec_INSERTION.String()) {
		// Once we are in this mode, we should not stop this routine.
		// On ctrler restart : WatchObjects will exit
		//                     newNimbusClient will be recreated
		//                      newWatchCtx witll be create in Start()
		// repeated venice coordinates : GRPC will be closed in HandleVeniceCoordinates, and we will call start again
		// Upgrade : Before watchCtx is set, ReplayConfigs calls HandleProfile and that setsup the watches. As npm sync will not result in change of object
		// and we wil not setup new watchers.
		insertionKinds := []string{"App", "NetworkSecurityPolicy", "Vrf", "Network", "Endpoint", "SecurityProfile"}

		log.Infof("Start InsertionMode Watcher")
		startInsertionWatcher := func() {
			for {
				if i.ControllerAPI == nil {
					log.Info("Wait for controller registration")
					time.Sleep(time.Second)
					continue
				}
				log.Infof("AggWatchers Start for kinds %s", insertionKinds)
				i.ControllerAPI.WatchObjects(insertionKinds)
				log.Infof("Watchers stopped for kinds %s", insertionKinds)
				time.Sleep(time.Minute)
			}
		}
		go startInsertionWatcher()
	}
	// ToDO remove this commented out code
	//if strings.ToLower(profile.Spec.FwdMode) == strings.ToLower(netproto.ProfileSpec_TRANSPARENT.String()) {
	//	go func() {
	//		i.ControllerAPI.WatchObjects([]string{"Interface", "Collector"})
	//	}()
	//} else { // We support only insertion enforced
	//	go func() {
	//		i.ControllerAPI.WatchObjects([]string{"App", "NetworkSecurityPolicy"})
	//	}()
	//	go func() {
	//		i.ControllerAPI.WatchObjects([]string{"Vrf", "Network", "Endpoint", "SecurityProfile"})
	//	}()
	//}

	return
}

// HandleCollector handles CRUD Methods for Collector Object
func (i *IrisAPI) HandleCollector(oper types.Operation, col netproto.Collector) (cols []netproto.Collector, err error) {
	i.Lock()
	defer i.Unlock()

	err = utils.ValidateMeta(oper, col.Kind, col.ObjectMeta)
	if err != nil {
		log.Error(err)
		return
	}

	// Handle Get and LIST. This doesn't need any pipeline specific APIs
	switch oper {
	case types.Get:
		var (
			dat []byte
			obj netproto.Collector
		)
		dat, err = i.InfraAPI.Read(col.Kind, col.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound)
		}
		err = obj.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err)
		}
		cols = append(cols, obj)

		return
	case types.List:
		var (
			dat [][]byte
		)
		dat, err = i.InfraAPI.List(col.Kind)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound)
		}

		for _, o := range dat {
			var collector netproto.Collector
			err := proto.Unmarshal(o, &collector)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", collector.GetKey(), err))
				continue
			}
			cols = append(cols, collector)
		}

		return
	case types.Create:

	case types.Update:
		// Get to ensure that the object exists
		var existingCollector netproto.Collector
		dat, err := i.InfraAPI.Read(col.Kind, col.GetKey())
		if err != nil {
			log.Error(errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound))
			return nil, errors.Wrapf(types.ErrBadRequest, "Collector: %s | Err: %v", col.GetKey(), types.ErrObjNotFound)
		}
		err = existingCollector.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err)
		}

		// Check for idempotency
		if proto.Equal(&col.Spec, &existingCollector.Spec) {
			//log.Infof("Collector: %s | Info: %s ", col.GetKey(), types.InfoIgnoreUpdate)
			return nil, nil
		}

		// Reuse ID from store
		col.Status.Collector = existingCollector.Status.Collector
	case types.Delete:
		var existingCollector netproto.Collector
		dat, err := i.InfraAPI.Read(col.Kind, col.GetKey())
		if err != nil {
			log.Infof("Controller API: %s | Err: %s", types.InfoIgnoreDelete, err)
			return nil, nil
		}
		err = existingCollector.Unmarshal(dat)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err))
			return nil, errors.Wrapf(types.ErrUnmarshal, "Collector: %s | Err: %v", col.GetKey(), err)
		}
		col = existingCollector
	}
	log.Infof("Collector: %v | Op: %s | %s", col, oper, types.InfoHandleObjBegin)
	defer log.Infof("Collector: %v | Op: %s | %s", col, oper, types.InfoHandleObjEnd)

	// Perform object validations
	uniqueCollectors := map[string]bool{}
	for dest := range iris.MirrorDestToIDMapping {
		uniqueCollectors[dest] = true
	}
	vrf, err := validator.ValidateCollector(i.InfraAPI, col, oper, uniqueCollectors)
	if err != nil {
		log.Error(err)
		return nil, err
	}

	// Take a lock to ensure a single HAL API is active at any given point
	if err := iris.HandleCollector(i.InfraAPI, i.TelemetryClient, i.IntfClient, i.EpClient, oper, col, vrf.Status.VrfID); err != nil {
		log.Error(err)
		return nil, err
	}

	return
}

// HandleIPAMPolicy handles CRUD methods for IPAMPolicy
func (i *IrisAPI) HandleIPAMPolicy(oper types.Operation, policy netproto.IPAMPolicy) (policies []netproto.IPAMPolicy, err error) {
	return nil, err
}

// HandleRouteTable handles CRUDs for RouteTable object
func (i *IrisAPI) HandleRouteTable(oper types.Operation, routetableObj netproto.RouteTable) ([]netproto.RouteTable, error) {
	return nil, types.ErrNotImplemented
}

// ReplayConfigs replays last known configs from boltDB
func (i *IrisAPI) ReplayConfigs() error {

	// Replay Profile Object
	profiles, err := i.InfraAPI.List("Profile")
	if err == nil {
		for _, o := range profiles {
			var profile netproto.Profile
			err := profile.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Profile. Err: %v", err)
				continue
			}
			creator, ok := profile.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Profile object")
				if _, err := i.HandleProfile(types.Create, profile); err != nil {
					log.Errorf("Failed to recreate Profile: %v. Err: %v", profile.GetKey(), err)
				}
			}
		}
	}
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

	// Replay Collector Object
	cols, err := i.InfraAPI.List("Collector")
	if err == nil {
		for _, o := range cols {
			var collector netproto.Collector
			err := collector.Unmarshal(o)
			if err != nil {
				log.Errorf("Failed to unmarshal object to Collector. Err: %v", err)
				continue
			}
			creator, ok := collector.ObjectMeta.Labels["CreatedBy"]
			if ok && creator == "Venice" {
				log.Info("Replaying persisted Collector object")
				if _, err := i.HandleCollector(types.Create, collector); err != nil {
					log.Errorf("Failed to recreate Collector: %v. Err: %v", collector.GetKey(), err)
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

	c := netproto.Collector{TypeMeta: api.TypeMeta{Kind: "Collector"}}
	cols, _ := i.HandleCollector(types.List, c)
	for _, col := range cols {
		if _, err := i.HandleCollector(types.Delete, col); err != nil {
			log.Errorf("Failed to purge the Collector. Err: %v", err)
		}
	}

	return nil
}

// GetWatchOptions returns the options to be used while establishing a watch from this agent.
func (i *IrisAPI) GetWatchOptions(ctx context.Context, kind string) (ret api.ListWatchOptions) {
	switch kind {
	case "Endpoint":
		str := fmt.Sprintf("spec.node-uuid=%s", i.InfraAPI.GetDscName())
		log.Info("WatchOptions for: ", kind, " ", str)
		ret.FieldSelector = str
	}
	return ret
}

// ############################################### Helper Methods  ###############################################
func (i *IrisAPI) createHostInterface(uid string, spec *halapi.LifSpec, status *halapi.LifStatus) error {
	// skip any internal lifs
	if spec.GetType() != halapi.LifType_LIF_TYPE_HOST {
		log.Infof("Skipping LIF_CREATE event for lif %v, type %v", uid, spec.GetType().String())
		return nil
	}
	lifIndex := utils.GetLifIndex(status.HwLifId)
	// form the interface name
	ifName, err := utils.GetIfName(uid, lifIndex, spec.GetType().String())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest,
			"Failed to form interface name, uuid %v, ifindex %x, err %v",
			uid, lifIndex, err))
		return err
	}
	// create host interface instance
	l := netproto.Interface{
		TypeMeta: api.TypeMeta{
			Kind: "Interface",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      ifName,
			UUID:      uid,
		},
		Spec: netproto.InterfaceSpec{
			Type:        netproto.InterfaceSpec_HOST_PF.String(),
			AdminStatus: strings.ToLower(netproto.IFStatus_UP.String()),
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: uint64(lifIndex),
			IFHostStatus: netproto.InterfaceHostStatus{
				HostIfName: spec.GetName(),
			},
			OperStatus: status.GetLifStatus().String(),
		},
	}
	b, _ := json.MarshalIndent(l, "", "   ")
	fmt.Println(string(b))
	ifEvnt := types.UpdateIfEvent{
		Oper: types.Create,
		Intf: l,
	}
	i.InfraAPI.UpdateIfChannel() <- ifEvnt
	dat, _ := l.Marshal()
	if err := i.InfraAPI.Store(l.Kind, l.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Lif: %s | Lif: %v", l.GetKey(), err))
		return err
	}
	return nil
}

func (i *IrisAPI) createUplinkInterface(uid string, spec *halapi.PortSpec, status *halapi.PortStatus) error {
	var ifType string
	// form the interface name
	ifName, err := utils.GetIfName(uid, status.GetIfIndex(), spec.GetPortType().String())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest,
			"Failed to form interface name, uuid %v, ifindex %x, err %v",
			uid, status.GetIfIndex(), err))
		return err
	}
	if spec.GetPortType().String() == "PORT_TYPE_ETH" {
		ifType = netproto.InterfaceSpec_UPLINK_ETH.String()
	} else if spec.GetPortType().String() == "PORT_TYPE_MGMT" {
		ifType = netproto.InterfaceSpec_UPLINK_MGMT.String()
	}

	uplink := netproto.Interface{
		TypeMeta: api.TypeMeta{
			Kind: "Interface",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      ifName,
			UUID:      uid,
		},
		Spec: netproto.InterfaceSpec{
			Type:        ifType,
			AdminStatus: strings.ToLower(netproto.IFStatus_UP.String()),
		},
		Status: netproto.InterfaceStatus{
			InterfaceID: utils.EthIfIndexToUplinkIfIndex(uint64(status.GetIfIndex())),
			OperStatus:  status.GetLinkStatus().GetOperState().String(),
			IFUplinkStatus: netproto.InterfaceUplinkStatus{
				PortID: spec.KeyOrHandle.GetPortId(),
			},
		},
	}
	ifEvnt := types.UpdateIfEvent{
		Oper: types.Create,
		Intf: uplink,
	}
	i.InfraAPI.UpdateIfChannel() <- ifEvnt
	dat, _ := uplink.Marshal()
	if err := i.InfraAPI.Store(uplink.Kind, uplink.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Uplink: %s | Uplink: %v", uplink.GetKey(), err))
		return err
	}
	return nil
}

func (i *IrisAPI) initLifStream(uid string) {

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
			if err == io.EOF {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "HAL Event stream closed"))
				break
			}
			if err != nil {
				log.Error(errors.Wrapf(types.ErrPipelineEventStreamClosed, "Init: %v", err))
				break
			}
			if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
				log.Error(errors.Wrapf(types.ErrDatapathHandling, "Iris Init: %v", err))
			}

			lif := resp.GetLifEvent()

			err = i.createHostInterface(uid, lif.Spec, lif.Status)

		}

	}(lifStream)

	// Store initial Lifs
	for _, lif := range lifs.Response {
		i.createHostInterface(uid, lif.Spec, lif.Status)
	}

}

// HandleCPRoutingConfig unimplemented
func (i *IrisAPI) HandleCPRoutingConfig(obj types.DSCStaticRoute) error {
	return errors.Wrapf(types.ErrNotImplemented, "Handle CP Routing Config not implemented by Iris Pipeline")
}

// HandleTechSupport unimplemented
func (i *IrisAPI) HandleTechSupport(obj tsproto.TechSupportRequest) (string, error) {
	return "", errors.Wrapf(types.ErrNotImplemented, "Tech Support Capture not implemented by Iris Pipeline")
}

// TODO Remove PortCreates once the linkmgr changes are stable
func (i *IrisAPI) createPortsAndUplinks(uid string) error {
	portReqMsg := &halapi.PortGetRequestMsg{
		Request: []*halapi.PortGetRequest{{}},
	}

	ports, err := i.PortClient.PortGet(context.Background(), portReqMsg)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrPipelinePortGet, "Iris Init: %v", err))
		return errors.Wrapf(types.ErrPipelinePortGet, "Iris Init: %v", err)
	}

	for _, port := range ports.Response {
		i.createUplinkInterface(uid, port.Spec, port.Status)
	}

	return nil
}

func (i *IrisAPI) isLocalEP(nodeuuid string) bool {
	log.Infof("Node UUID: %s | Self Node UUID: %s", nodeuuid, i.InfraAPI.GetDscName())
	epNodeUUID, _ := net.ParseMAC(nodeuuid)
	selfNodeUUID, _ := net.ParseMAC(i.InfraAPI.GetDscName())
	return epNodeUUID.String() == selfNodeUUID.String()
}

// HandleDSCL3Interface handles configuring L3 interfaces on DSC interfaces
func (i *IrisAPI) HandleDSCL3Interface(obj types.DSCInterfaceIP) error {
	return errors.Wrapf(types.ErrNotImplemented, "Handle CP Routing Config not implemented by Iris Pipeline")
}
