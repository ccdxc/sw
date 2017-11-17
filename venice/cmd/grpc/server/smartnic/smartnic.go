// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package smartnic

import (
	"errors"
	"fmt"
	"strings"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/venice/cmd/grpc"
	perror "github.com/pensando/sw/venice/utils/errors"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (

	// HealthWatchInterval is default health watch interval
	HealthWatchInterval = 60 * time.Second

	// DeadInterval is default dead time interval, after
	// which NIC health status is declared UNKNOWN by CMD
	DeadInterval = 120 * time.Second
)

var (
	errAPIServerDown = fmt.Errorf("API Server not reachable or down")
)

// RPCServer implements SmartNIC gRPC service.
type RPCServer struct {
	// ClientGetter is an interface to get the API client.
	ClientGetter APIClientGetter

	// HealthWatchIntvl is the health watch interval
	HealthWatchIntvl time.Duration

	// DeadIntvl is the dead time interval
	DeadIntvl time.Duration
}

// APIClientGetter is an interface that returns an API Client.
type APIClientGetter interface {
	APIClient() cmd.CmdV1Interface
}

// NewRPCServer returns a SmartNIC RPC server object
func NewRPCServer(clientGetter APIClientGetter, healthInvl, deadInvl time.Duration) (*RPCServer, error) {
	if clientGetter == nil {
		return nil, fmt.Errorf("Client getter is nil")
	}
	return &RPCServer{
		ClientGetter:     clientGetter,
		HealthWatchIntvl: healthInvl,
		DeadIntvl:        deadInvl,
	}, nil
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
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}
	opts := api.ListWatchOptions{}
	clusterObjs, err := cl.Cluster().List(context.Background(), &opts)
	if err != nil || len(clusterObjs) == 0 {
		return nil, perror.NewNotFound("Cluster", "")
	}

	// There should be only one Cluster object
	return clusterObjs[0], nil
}

// CreateNode creates the Node object based on object meta
func (s *RPCServer) CreateNode(ometa api.ObjectMeta) (*cmd.Node, error) {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

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

	nodeObj, err := cl.Node().Create(context.Background(), node)
	if err != nil || nodeObj == nil {
		return nil, perror.NewNotFound("Node", ometa.Name)
	}

	return nodeObj, nil
}

// GetNode fetches the Node object based on object meta
func (s *RPCServer) GetNode(om api.ObjectMeta) (*cmd.Node, error) {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

	nodeObj, err := cl.Node().Get(context.Background(), &om)
	if err != nil || nodeObj == nil {
		return nil, perror.NewNotFound("Node", om.Name)
	}

	return nodeObj, nil
}

// UpdateNode updates the list of Nics in status of Node object
// Node object is  created if it does not exist.
func (s *RPCServer) UpdateNode(nic *cmd.SmartNIC) (*cmd.Node, error) {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

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

		nodeObj, err = cl.Node().Create(context.Background(), node)
	} else {

		// Update the Nics list in Node status
		refObj.Status.Nics = append(refObj.Status.Nics, nic.ObjectMeta.Name)
		nodeObj, err = cl.Node().Update(context.Background(), refObj)
	}

	return nodeObj, err
}

// DeleteNode deletes the Node object based on object meta name
func (s *RPCServer) DeleteNode(om api.ObjectMeta) error {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return errAPIServerDown
	}

	_, err := cl.Node().Delete(context.Background(), &om)
	if err != nil {
		log.Errorf("Error deleting Node object name:%s err: %v", om.Name, err)
		return err
	}

	return nil
}

// GetSmartNIC fetches the SmartNIC object based on object meta
func (s *RPCServer) GetSmartNIC(om api.ObjectMeta) (*cmd.SmartNIC, error) {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

	nicObj, err := cl.SmartNIC().Get(context.Background(), &om)
	if err != nil || nicObj == nil {
		return nil, perror.NewNotFound("SmartNIC", om.Name)
	}

	return nicObj, nil
}

// UpdateSmartNIC creates or updates the smartNIC object
func (s *RPCServer) UpdateSmartNIC(obj *cmd.SmartNIC) (*cmd.SmartNIC, error) {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

	// Check if object exists
	var nicObj *cmd.SmartNIC
	refObj, err := s.GetSmartNIC(obj.ObjectMeta)
	if err != nil || refObj == nil {

		// Create smartNIC object
		nicObj, err = cl.SmartNIC().Create(context.Background(), obj)
	} else {

		// Update the object with CAS semantics
		obj.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
		nicObj, err = cl.SmartNIC().Update(context.Background(), obj)
	}

	return nicObj, err
}

// DeleteSmartNIC deletes the SmartNIC object based on object meta name
func (s *RPCServer) DeleteSmartNIC(om api.ObjectMeta) error {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return errAPIServerDown
	}

	_, err := cl.SmartNIC().Delete(context.Background(), &om)
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
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

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

	log.Infof("Validated NIC: %s, phase: %s", mac, phase)

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
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return nil, errAPIServerDown
	}

	obj := req.GetNic()

	// Get existing Object from Object store
	ometa := api.ObjectMeta{Name: obj.ObjectMeta.Name, Tenant: obj.ObjectMeta.Tenant}
	refObj, err := cl.SmartNIC().Get(context.Background(), &ometa)
	if err != nil {
		log.Errorf("Error getting SmartNIC object: %+v err: %v", obj, err)
		return &grpc.UpdateNICResponse{Nic: nil}, err
	}

	// Update smartNIC object with CAS semantics
	obj.ObjectMeta.ResourceVersion = refObj.ObjectMeta.ResourceVersion
	nicObj, err := cl.SmartNIC().Update(context.Background(), &obj)

	if err != nil || nicObj == nil {
		log.Errorf("Error updating SmartNIC object: %+v err: %v", obj, err)
		return &grpc.UpdateNICResponse{Nic: nil}, err
	}

	return &grpc.UpdateNICResponse{Nic: nicObj}, nil
}

// WatchNICs watches smartNICs for changes and sends them as streaming rpc
func (s *RPCServer) WatchNICs(sel *api.ObjectMeta, stream grpc.SmartNIC_WatchNICsServer) error {
	cl := s.ClientGetter.APIClient()
	if cl == nil {
		return errAPIServerDown
	}

	// watch for changes
	opts := api.ListWatchOptions{}
	var watcher kvstore.Watcher
	watcher, err := cl.SmartNIC().Watch(stream.Context(), &opts)
	if err != nil {
		log.Errorf("Failed to start watch, err: %v", err)
		return err
	}
	defer watcher.Stop()

	// first get a list of all existing smartNICs
	opts = api.ListWatchOptions{}
	nics, err := cl.SmartNIC().List(stream.Context(), &opts)
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

// MonitorHealth periodically inspects that health status of
// smartNIC objects every 30sec. For NICs that haven't received
// health updates in over 120secs, CMD would mark the status as unknown.
func (s *RPCServer) MonitorHealth() {
	for {
		select {

		// NIC health watch timer callback
		case <-time.After(s.HealthWatchIntvl):

			cl := s.ClientGetter.APIClient()
			if cl == nil {
				log.Errorf("Failed to get API client")
				continue
			}
			// Get a list of all existing smartNICs
			opts := api.ListWatchOptions{}
			nics, err := cl.SmartNIC().List(context.Background(), &opts)
			if err != nil {
				log.Errorf("Failed to getting a list of nics, err: %v", err)
				continue
			}

			log.Debugf("Health watch timer callback, #nics: %d", len(nics))

			// Iterate on smartNIC objects
			for _, nic := range nics {

				for i := 0; i < len(nic.Status.Conditions); i++ {

					condition := nic.Status.Conditions[i]

					// Inspect Health condition with status that is marked healthy or unhealthy (i.e not unknown)
					if condition.Type == cmd.SmartNICCondition_HEALTHY.String() && condition.Status != cmd.ConditionStatus_UNKNOWN.String() {

						// parse the last reported time
						t, err := time.Parse(time.RFC3339, condition.LastTransitionTime)
						if err != nil {
							log.Errorf("Failed parsing last transition time for NIC health, nic: %+v, err: %v", nic, err)
							break
						}

						// if the time elapsed since last health update is over
						// the deadInterval, update the Health status to unknown
						if err == nil && time.Since(t) > s.DeadIntvl {

							// update the nic health status to unknown
							log.Infof("Updating NIC health to unknown, nic: %s", nic.Name)
							lastUpdateTime := nic.Status.Conditions[i].LastTransitionTime
							nic.Status.Conditions[i].Status = cmd.ConditionStatus_UNKNOWN.String()
							nic.Status.Conditions[i].LastTransitionTime = time.Now().Format(time.RFC3339)
							nic.Status.Conditions[i].Reason = fmt.Sprintf("NIC health update not received since %s", lastUpdateTime)
							_, err := cl.SmartNIC().Update(context.Background(), nic)
							if err != nil {
								log.Errorf("Failed updating the NIC health status to unknown, nic: %s err: %s", nic.Name, err)
							}
						}

						break
					}

				}
			}
		}
	}
}
