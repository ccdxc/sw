// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package smartnic

import (
	"errors"
	"strings"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/grpc"
	perror "github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// RPCServer implements SmartNIC gRPC service.
type RPCServer struct {

	// ClusterAPI is CRUD interface for Cluster object
	ClusterAPI cmd.ClusterInterface

	// NodeAPI is CRUD interface for Node object
	NodeAPI cmd.NodeInterface

	// SmartNICAPI is CRUD interface for SmartNIC object
	SmartNICAPI cmd.SmartNICInterface
}

// NewRPCServer returns a SmartNIC RPC server object
func NewRPCServer(cIf cmd.ClusterInterface, nIf cmd.NodeInterface, sIf cmd.SmartNICInterface) *RPCServer {
	return &RPCServer{
		ClusterAPI:  cIf,
		NodeAPI:     nIf,
		SmartNICAPI: sIf,
	}
}

// Temporary definition, until CKM provides a API to validate Cert
const (
	ValidCertSignature = "O=Pensando Systems, Inc., OU=Pensando Manufacturing, CN=Pensando Manufacturing CA/emailAddress=mfgca@pensando.io"
)

// IsValidFactoryCert inspects the certificate for validity and returns boolean
func (s *RPCServer) IsValidFactoryCert(cert []byte) bool {

	// TODO: For now, it just looks for a pensando string signature
	// Will use CKM APIs once it is available for checking validity
	str := string(cert)
	if strings.Contains(str, ValidCertSignature) {
		return true
	}

	return false
}

// GetCluster fetches the Cluster object based on object meta
func (s *RPCServer) GetCluster() (*cmd.Cluster, error) {
	opts := api.ListWatchOptions{}
	clusterObjs, err := s.ClusterAPI.List(context.Background(), &opts)
	if err != nil || len(clusterObjs) == 0 {
		return nil, perror.NewNotFound("Cluster", "")
	}

	// There should be only one Cluster object
	return clusterObjs[0], nil
}

// CreateNode creates the Node object based on object meta
func (s *RPCServer) CreateNode(ometa api.ObjectMeta) (*cmd.Node, error) {

	node := &cmd.Node{
		TypeMeta:   api.TypeMeta{Kind: "Node"},
		ObjectMeta: ometa,
		Spec: cmd.NodeSpec{
			Roles: []string{cmd.NodeSpec_WORKLOAD.String()},
		},
		Status: cmd.NodeStatus{
			Phase: cmd.NodeStatus_JOINED.String(),
		},
	}

	nodeObj, err := s.NodeAPI.Create(context.Background(), node)
	if err != nil || nodeObj == nil {
		return nil, perror.NewNotFound("Node", ometa.Name)
	}

	return nodeObj, nil
}

// GetNode fetches the Node object based on object meta
func (s *RPCServer) GetNode(om api.ObjectMeta) (*cmd.Node, error) {

	nodeObj, err := s.NodeAPI.Get(context.Background(), &om)
	if err != nil || nodeObj == nil {
		return nil, perror.NewNotFound("Node", om.Name)
	}

	return nodeObj, nil
}

// UpdateNode updates the list of Nics in status of Node object
// Node object is  created if it does not exist.
func (s *RPCServer) UpdateNode(nic *cmd.SmartNIC) (*cmd.Node, error) {

	// Check if object exists
	var nodeObj *cmd.Node
	ometa := api.ObjectMeta{
		Name: nic.Status.NodeName,
	}
	refObj, err := s.GetNode(ometa)
	if err != nil || refObj == nil {

		// Create Node object
		node := &cmd.Node{
			TypeMeta: api.TypeMeta{Kind: "Node"},
			ObjectMeta: api.ObjectMeta{
				Name: nic.Status.NodeName,
			},
			Spec: cmd.NodeSpec{
				Roles: []string{cmd.NodeSpec_WORKLOAD.String()},
			},
			Status: cmd.NodeStatus{
				Nics:  []string{nic.ObjectMeta.Name},
				Phase: cmd.NodeStatus_JOINED.String(),
			},
		}

		nodeObj, err = s.NodeAPI.Create(context.Background(), node)
	} else {

		// Update the Nics list in Node status
		refObj.Status.Nics = append(refObj.Status.Nics, nic.ObjectMeta.Name)
		nodeObj, err = s.NodeAPI.Update(context.Background(), refObj)
	}

	return nodeObj, err
}

// DeleteNode deletes the Node object based on object meta name
func (s *RPCServer) DeleteNode(om api.ObjectMeta) error {

	_, err := s.NodeAPI.Delete(context.Background(), &om)
	if err != nil {
		log.Errorf("Error deleting Node object name:%s err: %v", om.Name, err)
		return err
	}

	return nil
}

// GetSmartNIC fetches the SmartNIC object based on object meta
func (s *RPCServer) GetSmartNIC(om api.ObjectMeta) (*cmd.SmartNIC, error) {

	nicObj, err := s.SmartNICAPI.Get(context.Background(), &om)
	if err != nil || nicObj == nil {
		return nil, perror.NewNotFound("SmartNIC", om.Name)
	}

	return nicObj, nil
}

// UpdateSmartNIC creates or updates the smartNIC object
func (s *RPCServer) UpdateSmartNIC(obj *cmd.SmartNIC) (*cmd.SmartNIC, error) {

	// Check if object exists
	var nicObj *cmd.SmartNIC
	refObj, err := s.GetSmartNIC(obj.ObjectMeta)
	if err != nil || refObj == nil {

		// Create smartNIC object
		nicObj, err = s.SmartNICAPI.Create(context.Background(), obj)
	} else {

		// Update the object with CAS semantics
		obj.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
		nicObj, err = s.SmartNICAPI.Update(context.Background(), obj)
	}

	return nicObj, err
}

// DeleteSmartNIC deletes the SmartNIC object based on object meta name
func (s *RPCServer) DeleteSmartNIC(om api.ObjectMeta) error {

	_, err := s.SmartNICAPI.Delete(context.Background(), &om)
	if err != nil {
		log.Errorf("Error deleting smartNIC object name:%s err: %v", om.Name, err)
		return err
	}

	return nil
}

// RegisterNIC handles the register NIC request and upon validation creates SmartNIC object
// NMD starts the NIC registration process when the NIC is placed in managed mode.
// NMD is expected to retry with backoff if there are API errors or if the NIC is rejected.
func (s *RPCServer) RegisterNIC(ctx context.Context, req *grpc.RegisterNICRequest) (*grpc.RegisterNICResponse, error) {

	mac := req.GetNic().Name
	cert := req.GetCert()

	log.Infof("Received NIC registration request for MAC:%s", mac)

	// Validate the factory cert obtained in the request
	if s.IsValidFactoryCert(cert) == false {

		// Reject NIC if the certificate is not valid
		// TODO: Add exact reason to the response once CKM api is used
		log.Errorf("Invalid certificate, NIC rejected mac: %v", mac)
		return &grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_REJECTED.String(), Reason: string("Invalid Cert")}, nil
	}

	// Get the Cluster object
	clusterObj, err := s.GetCluster()
	if err != nil {
		log.Errorf("Error getting Cluster object, err: %v", err)
		return &grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_UNKNOWN.String()}, err
	}

	// Process the request based on the configured admission mode
	var phase string
	if clusterObj.Spec.AutoAdmitNICs == true {

		// Admit NIC if autoAdmission is enabled
		phase = cmd.SmartNICSpec_ADMITTED.String()
	} else {

		// Set the NIC to pendingState for Manual Approval
		phase = cmd.SmartNICSpec_PENDING.String()
	}

	// Create SmartNIC object, if the status is either admitted or pending.
	if phase == cmd.SmartNICSpec_ADMITTED.String() || phase == cmd.SmartNICSpec_PENDING.String() {
		nic := req.GetNic()
		nic.Spec.Phase = phase
		_, err = s.UpdateSmartNIC(&nic)
		if err != nil {
			log.Errorf("Error updating smartNIC object, mac: %s err: %v", nic.ObjectMeta.Name, err)
			return &grpc.RegisterNICResponse{Phase: phase}, err
		}

		// Create or Update the Node object for Workload nodes
		// to link the NICs to Nodes.
		_, err = s.UpdateNode(&nic)
		if err != nil {
			log.Errorf("Error creating or updating Node object, node: %s mac: %s err: %v",
				nic.Status.NodeName, nic.ObjectMeta.Name, err)
		}
	}

	return &grpc.RegisterNICResponse{Phase: phase}, err
}

// UpdateNIC handles the update to smartNIC object
func (s *RPCServer) UpdateNIC(ctx context.Context, req *grpc.UpdateNICRequest) (*grpc.UpdateNICResponse, error) {

	obj := req.GetNic()

	// Get existing Object from Object store
	ometa := api.ObjectMeta{Name: obj.ObjectMeta.Name, Tenant: obj.ObjectMeta.Tenant}
	refObj, err := s.SmartNICAPI.Get(context.Background(), &ometa)
	if err != nil {
		log.Errorf("Error getting SmartNIC object: %+v err: %v", obj, err)
		return &grpc.UpdateNICResponse{Nic: nil}, err
	}

	// Update smartNIC object with CAS semantics
	obj.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
	nicObj, err := s.SmartNICAPI.Update(context.Background(), &obj)

	if err != nil || nicObj == nil {
		log.Errorf("Error updating SmartNIC object: %+v err: %v", obj, err)
		return &grpc.UpdateNICResponse{Nic: nil}, err
	}

	return &grpc.UpdateNICResponse{Nic: nicObj}, nil
}

// WatchNICs watches smartNICs for changes and sends them as streaming rpc
func (s *RPCServer) WatchNICs(sel *api.ObjectMeta, stream grpc.SmartNIC_WatchNICsServer) error {

	// watch for changes
	opts := api.ListWatchOptions{}
	watcher, err := s.SmartNICAPI.Watch(context.Background(), &opts)
	if err != nil {
		log.Errorf("Failed to start watch, err: %v", err)
		return err
	}

	// first get a list of all existing smartNICs
	opts = api.ListWatchOptions{}
	nics, err := s.SmartNICAPI.List(context.Background(), &opts)
	if err != nil {
		log.Errorf("Error getting a list of nics, err: %v", err)
		return err
	}

	// send the objects out as a stream
	for _, nic := range nics {
		watchEvt := grpc.SmartNICEvent{
			EventType: api.EventType_CreateEvent,
			Nic:       *nic,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream, err: %v", err)
			return err
		}
	}

	for {
		select {
		case ev, ok := <-watcher.EventChan():
			log.Infof("received event [%v]", ok)
			if !ok {
				log.Errorf("publisher watcher closed")
				return errors.New("Error reading smartNIC watch channel")
			}
			log.Infof(" SmartNIC watch event [%+v]", *ev)

			// get event type
			var etype api.EventType
			switch ev.Type {
			case kvstore.Created:
				etype = api.EventType_CreateEvent
			case kvstore.Updated:
				etype = api.EventType_UpdateEvent
			case kvstore.Deleted:
				etype = api.EventType_DeleteEvent
			}

			var nicObj *cmd.SmartNIC
			nicObj, ok = ev.Object.(*cmd.SmartNIC)
			if !ok {
				log.Errorf("Invalid object type, expected SmartNIC")
				return errors.New("Invalid object type")
			}

			// construct the smartNIC object
			watchEvt := grpc.SmartNICEvent{
				EventType: etype,
				Nic: cmd.SmartNIC{
					TypeMeta:   nicObj.TypeMeta,
					ObjectMeta: nicObj.ObjectMeta,
					Spec: cmd.SmartNICSpec{
						Phase: nicObj.Spec.Phase,
					},
					Status: cmd.SmartNICStatus{
						PrimaryMacAddress: nicObj.Status.PrimaryMacAddress,
					},
				},
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending stream. Err: %v", err)
				return err
			}
		case <-stream.Context().Done():
			log.Errorf("Context cancelled for SmartNIC Watcher")
			return stream.Context().Err()
		}
	}
}
