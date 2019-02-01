// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cmdif

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/certs"
	debugStats "github.com/pensando/sw/venice/utils/debug/stats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Max time (in milliseconds) to complete the registration sequence,
// after which the client will cancel the the request
var (
	nicRegTimeout = 5000 * time.Millisecond
)

// CmdClient is the client of CMD server running on Venice node
type CmdClient struct {
	sync.Mutex     // Lock for CmdClient
	sync.WaitGroup // wait group to wait on all go routines to exit

	cmdRegistrationURL     string             // CMD endpoint for the NIC registration API
	cmdUpdatesURL          string             // CMD endpoint for NIC watches and updates
	resolverClient         resolver.Interface // resolver Interface
	registrationRPCClient  *rpckit.RPCClient  // RPC client for NIC registration API
	updatesRPCClient       *rpckit.RPCClient  // RPC client for NIC watches and updates
	nmd                    nmdapi.NmdAPI      // NMD instance
	watchCtx               context.Context    // ctx for object watch
	watchCancel            context.CancelFunc // cancel for object watch
	debugStats             *debugStats.Stats
	startTime              time.Time
	smartNICWatcherRunning bool
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewCmdClient creates an CMD client object
func NewCmdClient(nmd nmdapi.NmdAPI, cmdRegistrationURL, cmdUpdatesURL string, resolverClient resolver.Interface) (*CmdClient, error) {

	// create CmdClient object
	client := CmdClient{
		cmdRegistrationURL: cmdRegistrationURL,
		cmdUpdatesURL:      cmdUpdatesURL,
		resolverClient:     resolverClient,
		nmd:                nmd,
		startTime:          time.Now(),
	}

	client.debugStats = debugStats.New(fmt.Sprintf("cmdif-%s", client.getAgentName())).Tsdb().Kind("nmdStats").TsdbPeriod(5 * time.Second).Build()

	// register the NMD client as a controller plugin
	err := nmd.RegisterCMD(&client)
	if err != nil {
		log.Errorf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	return &client, nil
}

func (client *CmdClient) initRegistrationRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing NIC registration RPC client URL: %v", client.cmdRegistrationURL)
	client.registrationRPCClient, err = rpckit.NewRPCClient("nmd-nic-reg", client.cmdRegistrationURL, rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Error connecting to grpc server for NIC registration, URL: %v Err: %v", client.cmdRegistrationURL, err)
	}
	return err
}

func (client *CmdClient) initUpdatesRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing NIC updates RPC client ")
	client.updatesRPCClient, err = rpckit.NewRPCClient("nmd-nic-upd", client.cmdUpdatesURL,
		rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Cmd))
	if err != nil {
		log.Errorf("Error connecting to grpc server for NIC updates, URL: %v Err: %v", client.cmdUpdatesURL, err)
	}
	return err
}

func (client *CmdClient) closeRegistrationRPC() {
	client.Lock()
	defer client.Unlock()
	if client.registrationRPCClient != nil {
		log.Errorf("Closing nmd-nic-reg RPC client")
		client.registrationRPCClient.Close()
		client.registrationRPCClient = nil
	}
}

func (client *CmdClient) closeUpdatesRPC() {

	client.Lock()
	defer client.Unlock()
	if client.updatesRPCClient != nil {
		log.Errorf("Closing nmd-nic-upd RPC client")
		client.updatesRPCClient.Close()
		client.updatesRPCClient = nil
	}
}

func (client *CmdClient) getRegistrationRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.registrationRPCClient
}

func (client *CmdClient) getUpdatesRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.updatesRPCClient
}

// getAgentName returns a unique name for each nmd instance
func (client *CmdClient) getAgentName() string {
	return "nmd-" + client.nmd.GetAgentID()
}

// runSmartNICWatcher runs smartNIC watcher loop
func (client *CmdClient) runSmartNICWatcher() {
	// setup wait group
	client.Add(1)
	defer client.Done()

	for {

		log.Infof("Starting SmartNIC watcher")
		// Initialize RPC client if required
		if client.getUpdatesRPCClient() == nil {
			err := client.initUpdatesRPC()
			if err != nil {
				if client.watchCtx.Err() == context.Canceled {
					log.Infof("Context Cancelled, exiting smartNIC watcher %+v", client)
					return
				}
				time.Sleep(time.Second)
				continue
			}
		}

		id := client.nmd.GetPrimaryMAC()

		// Start the watch on SmartNIC object
		log.Infof("Starting watch for SmartNIC %s", id)
		smartNICRPCClient := grpc.NewSmartNICUpdatesClient(client.getUpdatesRPCClient().ClientConn)
		stream, err := smartNICRPCClient.WatchNICs(client.watchCtx, &api.ObjectMeta{Name: id})
		if err != nil {
			log.Errorf("Error watching smartNIC: Err: %v watchCtx.Err: %v",
				err, client.watchCtx.Err())

			if client.watchCtx.Err() == context.Canceled {
				log.Info("Context Cancelled, exiting smartNIC watcher")
				return
			}

			// For RPC errors, need to close current client and re-init
			client.closeUpdatesRPC()
			time.Sleep(time.Second)
			continue
		}

		// Watcher loop
		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting nic watch. Err: %v watchCtx.Err: %v",
					err, client.watchCtx.Err())

				if client.watchCtx.Err() == context.Canceled {
					log.Info("Context Cancelled, exiting smartNIC watcher")
					client.closeUpdatesRPC()
					return
				}

				// For RPC errors, need to close current client and re-init
				client.closeUpdatesRPC()
				time.Sleep(time.Second)
				break
			}

			log.Infof("CMDIF: Got nic watch event: {%+v} type: %v", evt.Nic, evt.EventType)

			// Ignore events for non-local NIC.
			// This should not happen, as CMD should only send notifications for the SmartNIC object
			// managed by this agent
			if evt.Nic.Name != id {
				log.Errorf("Notification received for non-local SmartNIC (%s): %+v:", id, evt)
				continue
			}

			t, err := evt.Nic.ModTime.Time()
			if err == nil && client.startTime.Before(t) && evt.EventType != api.EventType_DeleteEvent {
				latency := time.Since(t)
				if latency >= 0 {
					client.debugStats.AddFloat("cum_nic_latency", float64(latency))
				}
			}
			switch evt.EventType {
			case api.EventType_CreateEvent:
				client.debugStats.AddFloat("nic_create", 1.0)
				// create the nic
				err = client.nmd.CreateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error creating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_UpdateEvent:
				client.debugStats.AddFloat("nic_update", 1.0)
				// update the nic
				err = client.nmd.UpdateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error updating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_DeleteEvent:
				client.debugStats.AddFloat("nic_delete", 1.0)
				// delete the nic
				err = client.nmd.DeleteSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error deleting the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			}
		}
	}
}

// Stop stops CmdClient  and all watching go routines
func (client *CmdClient) Stop() {
	log.Infof("Stopping Client: %+v", client)
	if client.watchCancel != nil {
		client.watchCancel()
	}
	client.Wait()

	client.Lock()
	client.watchCtx, client.watchCancel = nil, nil
	client.smartNICWatcherRunning = false
	client.Unlock()

	client.closeUpdatesRPC()
	client.closeRegistrationRPC()
	log.Info("Stopped CMD Client")
}

func makeErrorResp(err error, str string, nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error) {
	name := ""
	if nic != nil {
		name = nic.Name
	}
	if err != nil {
		return grpc.RegisterNICResponse{}, errors.Wrapf(err, fmt.Sprintf("%s, nic: %s", str, name))
	}
	return grpc.RegisterNICResponse{}, fmt.Errorf("%s, nic: %s", str, name)
}

// RegisterSmartNICReq send a register request for SmartNIC to CMD
func (client *CmdClient) RegisterSmartNICReq(nic *cluster.SmartNIC) (grpc.RegisterNICResponse, error) {

	// initialize rpc client
	err := client.initRegistrationRPC()
	if err != nil {
		return makeErrorResp(err, "Error initializing registration RPC", nic)
	}
	defer client.closeRegistrationRPC()

	// Generate cluster key and CSR
	// CMD will sign if the NIC is admitted
	kp, err := client.nmd.GenClusterKeyPair()
	if err != nil {
		return makeErrorResp(err, "Error generating key pair", nic)
	}
	csr, err := certs.CreateCSR(kp, nil, []string{globals.Nmd + "-" + nic.Name}, nil)
	if err != nil {
		return makeErrorResp(err, "Error creating certificate signing request", nic)
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICRegistrationClient(client.getRegistrationRPCClient().ClientConn)
	ctx, cancel := context.WithTimeout(context.Background(), nicRegTimeout)
	defer cancel()

	stream, err := nicRPCClient.RegisterNIC(ctx)
	if err != nil {
		return makeErrorResp(err, "Error sending RegisterNICRequest", nic)
	}

	platformCert, err := client.nmd.GetPlatformCertificate(nic)
	if err != nil {
		return makeErrorResp(err, "Error retrieving factory cert", nic)
	}

	req := grpc.RegisterNICRequest{
		AdmissionRequest: &grpc.NICAdmissionRequest{
			Nic:                    *nic,
			Cert:                   platformCert,
			ClusterCertSignRequest: csr.Raw,
		},
	}
	err = stream.Send(&req)
	if err != nil {
		return makeErrorResp(err, "Error sending RegisterNICRequest", nic)
	}

	msg, err := stream.Recv()
	if err != nil {
		return makeErrorResp(err, fmt.Sprintf("Error receiving registration response, msg: %v", msg), nic)
	}

	if msg.AuthenticationRequest == nil {
		return makeErrorResp(nil, "Protocol error: no AuthenticationRequest in message", nic)
	}

	claimantRandom, challengeResp, err := client.nmd.GenChallengeResponse(nic, msg.AuthenticationRequest.Challenge)
	authResp := grpc.RegisterNICRequest{
		AuthenticationResponse: &grpc.AuthenticationResponse{
			ClaimantRandom:    claimantRandom,
			ChallengeResponse: challengeResp,
		},
	}

	err = stream.Send(&authResp)
	if err != nil {
		return makeErrorResp(err, "Error sending challenge response", nic)
	}

	msg, err = stream.Recv()
	if err != nil {
		return makeErrorResp(err, fmt.Sprintf("Error receiving admission response, msg: %v", msg), nic)
	}

	if msg.AdmissionResponse == nil {
		return makeErrorResp(nil, "Protocol error: no AdmissionResponse in message", nic)
	}

	return *msg, nil
}

// UpdateSmartNICReq send a status update of SmartNIC to CMD
func (client *CmdClient) UpdateSmartNICReq(nic *cluster.SmartNIC) (*cluster.SmartNIC, error) {

	if client.getUpdatesRPCClient() == nil {
		// initialize rpc client
		err := client.initUpdatesRPC()
		if err != nil {
			return nil, err
		}
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICUpdatesClient(client.getUpdatesRPCClient().ClientConn)
	req := grpc.UpdateNICRequest{
		Nic: *nic,
	}
	resp, err := nicRPCClient.UpdateNIC(context.Background(), &req)
	if err != nil || resp == nil {
		log.Errorf("Error resp from CMD for updateNIC, Nic: %v Err: %v Resp: %v",
			nic.Name, err, resp)
		return nil, err
	}

	return resp.GetNic(), err
}

// WatchSmartNICUpdates starts a CMD watchers to receive SmartNIC objects updates
func (client *CmdClient) WatchSmartNICUpdates() {
	client.Lock()
	defer client.Unlock()
	client.watchCtx, client.watchCancel = context.WithCancel(context.Background())
	client.smartNICWatcherRunning = true
	go client.runSmartNICWatcher()
}

// IsSmartNICWatcherRunning returns true if the client has an active watch on CMD for SmartNIC objects
func (client *CmdClient) IsSmartNICWatcherRunning() bool {
	client.Lock()
	defer client.Unlock()
	return client.smartNICWatcherRunning
}
