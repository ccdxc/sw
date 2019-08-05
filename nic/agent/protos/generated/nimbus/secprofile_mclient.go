// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package nimbus is a auto generated package.
Input file: secprofile.proto
*/

package nimbus

import (
	"context"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"google.golang.org/grpc/connectivity"
)

type SecurityProfileReactor interface {
	CreateSecurityProfile(securityprofileObj *netproto.SecurityProfile) error   // creates an SecurityProfile
	FindSecurityProfile(meta api.ObjectMeta) (*netproto.SecurityProfile, error) // finds an SecurityProfile
	ListSecurityProfile() []*netproto.SecurityProfile                           // lists all SecurityProfiles
	UpdateSecurityProfile(securityprofileObj *netproto.SecurityProfile) error   // updates an SecurityProfile
	DeleteSecurityProfile(securityprofileObj, ns, name string) error            // deletes an SecurityProfile
	GetWatchOptions(cts context.Context, kind string) api.ObjectMeta
}

// WatchSecurityProfiles runs SecurityProfile watcher loop
func (client *NimbusClient) WatchSecurityProfiles(ctx context.Context, reactor SecurityProfileReactor) {
	// setup wait group
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()
	client.debugStats.AddInt("ActiveSecurityProfileWatch", 1)

	// make sure rpc client is good
	if client.rpcClient == nil || client.rpcClient.ClientConn == nil || client.rpcClient.ClientConn.GetState() != connectivity.Ready {
		log.Errorf("RPC client is disconnected. Exiting watch")
		return
	}

	// start the watch
	ometa := reactor.GetWatchOptions(ctx, "SecurityProfile")
	securityprofileRPCClient := netproto.NewSecurityProfileApiClient(client.rpcClient.ClientConn)
	stream, err := securityprofileRPCClient.WatchSecurityProfiles(ctx, &ometa)
	if err != nil {
		log.Errorf("Error watching SecurityProfile. Err: %v", err)
		return
	}

	// start oper update stream
	ostream, err := securityprofileRPCClient.SecurityProfileOperUpdate(ctx)
	if err != nil {
		log.Errorf("Error starting SecurityProfile oper updates. Err: %v", err)
		return
	}

	// get a list of objects
	objList, err := securityprofileRPCClient.ListSecurityProfiles(ctx, &ometa)
	if err != nil {
		log.Errorf("Error getting SecurityProfile list. Err: %v", err)
		return
	}

	// perform a diff of the states
	client.diffSecurityProfiles(objList, reactor, ostream)

	// start grpc stream recv
	recvCh := make(chan *netproto.SecurityProfileEvent, evChanLength)
	go client.watchSecurityProfileRecvLoop(stream, recvCh)

	// loop till the end
	for {
		select {
		case evt, ok := <-recvCh:
			if !ok {
				log.Warnf("SecurityProfile Watch channel closed. Exisint SecurityProfileWatch")
				return
			}
			client.debugStats.AddInt("SecurityProfileWatchEvents", 1)

			log.Infof("Ctrlerif: agent %s got SecurityProfile watch event: Type: {%+v} SecurityProfile:{%+v}", client.clientName, evt.EventType, evt.SecurityProfile.ObjectMeta)

			client.lockObject(evt.SecurityProfile.GetObjectKind(), evt.SecurityProfile.ObjectMeta)
			go client.processSecurityProfileEvent(*evt, reactor, ostream)
			//Give it some time to increment waitgrp
			time.Sleep(100 * time.Millisecond)
		// periodic resync
		case <-time.After(resyncInterval):
			// get a list of objects
			objList, err := securityprofileRPCClient.ListSecurityProfiles(ctx, &ometa)
			if err != nil {
				log.Errorf("Error getting SecurityProfile list. Err: %v", err)
				return
			}
			client.debugStats.AddInt("SecurityProfileWatchResyncs", 1)

			// perform a diff of the states
			client.diffSecurityProfiles(objList, reactor, ostream)
		}
	}
}

// watchSecurityProfileRecvLoop receives from stream and write it to a channel
func (client *NimbusClient) watchSecurityProfileRecvLoop(stream netproto.SecurityProfileApi_WatchSecurityProfilesClient, recvch chan<- *netproto.SecurityProfileEvent) {
	defer close(recvch)
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	// loop till the end
	for {
		// receive from stream
		evt, err := stream.Recv()
		if err != nil {
			log.Errorf("Error receiving from watch channel. Exiting SecurityProfile watch. Err: %v", err)
			return
		}

		recvch <- evt
	}
}

// diffSecurityProfile diffs local state with controller state
// FIXME: this is not handling deletes today
func (client *NimbusClient) diffSecurityProfiles(objList *netproto.SecurityProfileList, reactor SecurityProfileReactor, ostream netproto.SecurityProfileApi_SecurityProfileOperUpdateClient) {
	// build a map of objects
	objmap := make(map[string]*netproto.SecurityProfile)
	for _, obj := range objList.SecurityProfiles {
		key := obj.ObjectMeta.GetKey()
		objmap[key] = obj
	}

	// see if we need to delete any locally found object
	localObjs := reactor.ListSecurityProfile()
	for _, lobj := range localObjs {
		ctby, ok := lobj.ObjectMeta.Labels["CreatedBy"]
		if ok && ctby == "Venice" {
			key := lobj.ObjectMeta.GetKey()
			if _, ok := objmap[key]; !ok {
				evt := netproto.SecurityProfileEvent{
					EventType:       api.EventType_DeleteEvent,
					SecurityProfile: *lobj,
				}
				log.Infof("diffSecurityProfiles(): Deleting object %+v", lobj.ObjectMeta)
				client.lockObject(evt.SecurityProfile.GetObjectKind(), evt.SecurityProfile.ObjectMeta)
				client.processSecurityProfileEvent(evt, reactor, ostream)
			}
		} else {
			log.Infof("Not deleting non-venice object %+v", lobj.ObjectMeta)
		}
	}

	// add/update all new objects
	for _, obj := range objList.SecurityProfiles {
		evt := netproto.SecurityProfileEvent{
			EventType:       api.EventType_CreateEvent,
			SecurityProfile: *obj,
		}
		client.lockObject(evt.SecurityProfile.GetObjectKind(), evt.SecurityProfile.ObjectMeta)
		client.processSecurityProfileEvent(evt, reactor, ostream)
	}
}

// processSecurityProfileEvent handles SecurityProfile event
func (client *NimbusClient) processSecurityProfileEvent(evt netproto.SecurityProfileEvent, reactor SecurityProfileReactor, ostream netproto.SecurityProfileApi_SecurityProfileOperUpdateClient) {
	var err error
	client.waitGrp.Add(1)
	defer client.waitGrp.Done()

	// add venice label to the object
	evt.SecurityProfile.ObjectMeta.Labels = make(map[string]string)
	evt.SecurityProfile.ObjectMeta.Labels["CreatedBy"] = "Venice"

	// unlock the object once we are done
	defer client.unlockObject(evt.SecurityProfile.GetObjectKind(), evt.SecurityProfile.ObjectMeta)

	// retry till successful
	for iter := 0; iter < maxOpretry; iter++ {
		switch evt.EventType {
		case api.EventType_CreateEvent:
			fallthrough
		case api.EventType_UpdateEvent:
			_, err = reactor.FindSecurityProfile(evt.SecurityProfile.ObjectMeta)
			if err != nil {
				// create the SecurityProfile
				err = reactor.CreateSecurityProfile(&evt.SecurityProfile)
				if err != nil {
					log.Errorf("Error creating the SecurityProfile {%+v}. Err: %v", evt.SecurityProfile.ObjectMeta, err)
					client.debugStats.AddInt("CreateSecurityProfileError", 1)
				} else {
					client.debugStats.AddInt("CreateSecurityProfile", 1)
				}
			} else {
				// update the SecurityProfile
				err = reactor.UpdateSecurityProfile(&evt.SecurityProfile)
				if err != nil {
					log.Errorf("Error updating the SecurityProfile {%+v}. Err: %v", evt.SecurityProfile, err)
					client.debugStats.AddInt("UpdateSecurityProfileError", 1)
				} else {
					client.debugStats.AddInt("UpdateSecurityProfile", 1)
				}
			}

		case api.EventType_DeleteEvent:
			// delete the object
			err = reactor.DeleteSecurityProfile(evt.SecurityProfile.Tenant, evt.SecurityProfile.Namespace, evt.SecurityProfile.Name)
			if err == state.ErrObjectNotFound { // give idempotency to caller
				log.Debugf("SecurityProfile {%+v} not found", evt.SecurityProfile.ObjectMeta)
				err = nil
			}
			if err != nil {
				log.Errorf("Error deleting the SecurityProfile {%+v}. Err: %v", evt.SecurityProfile.ObjectMeta, err)
				client.debugStats.AddInt("DeleteSecurityProfileError", 1)
			} else {
				client.debugStats.AddInt("DeleteSecurityProfile", 1)
			}
		}

		// send oper status and return if there is no error
		if err == nil {
			robj := netproto.SecurityProfileEvent{
				EventType: evt.EventType,
				SecurityProfile: netproto.SecurityProfile{
					TypeMeta:   evt.SecurityProfile.TypeMeta,
					ObjectMeta: evt.SecurityProfile.ObjectMeta,
					Status:     evt.SecurityProfile.Status,
				},
			}

			// send oper status
			err := ostream.Send(&robj)
			if err != nil {
				log.Errorf("failed to send SecurityProfile oper Status, %s", err)
				client.debugStats.AddInt("SecurityProfileOperSendError", 1)
			} else {
				client.debugStats.AddInt("SecurityProfileOperSent", 1)
			}

			return
		}

		// else, retry after some time, with backoff
		time.Sleep(time.Second * time.Duration(2*iter))
	}
}
