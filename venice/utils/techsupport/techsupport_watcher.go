package techsupport

import (
	"fmt"
	"strings"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	tsconfig "github.com/pensando/sw/venice/ctrler/tsm/config"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"

	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
	export "github.com/pensando/sw/venice/utils/techsupport/exporter"
)

func (ag *TSMClient) handleTechSupportEvents(events *tsproto.TechSupportRequestEventList) {
	log.Infof("Handling Techsupport Event")

	for _, event := range events.Events {
		log.Infof("Event Type is : %v", event.EventType.String())

		switch event.EventType {
		case api.EventType_CreateEvent:
			log.Infof("Create event received")
			tsWork := event.Request
			ag.tsCh <- *tsWork
			log.Infof("Got request %v", tsWork)
			ag.sendUpdate(tsWork, tsproto.TechSupportRequestStatus_Queued)
		}
	}
}

func (ag *TSMClient) runTechSupportWatcher() {
	log.Info("Starting Techsupport Watcher")
	defer ag.waitGrp.Done()

	for {
		tsGrpcClient, err := rpckit.NewRPCClient(ag.name, globals.Tsm, rpckit.WithBalancer(balancer.New(ag.resolverClient)))
		if err != nil {
			log.Errorf("Error creating TSM client %s: %v", ag.name, err)

			if ag.isStopped() {
				return
			}

			time.Sleep(time.Second)
			continue
		}
		ag.tsGrpcClient = tsGrpcClient

		// start watch for techsupport requests
		client := tsproto.NewTechSupportApiClient(ag.tsGrpcClient.ClientConn)
		ag.tsAPIClient = client
		watchParams := &tsproto.WatchTechSupportRequestsParameters{
			NodeName: ag.name,
			NodeKind: ag.kind,
		}

		stream, err := client.WatchTechSupportRequests(ag.ctx, watchParams)
		if err != nil {
			log.Errorf("Error setting up watch for TechSupport requests. Err:%v", err)
			if ag.isStopped() {
				return
			}

			time.Sleep(time.Second)
			break
		}

		atomic.StoreInt32(&ag.watching, 1)
		log.Infof("Started watch for TechSupport requests for Node : %v Type : %v", ag.name, ag.kind)
		for {
			evtList, err := stream.Recv()
			if err != nil {
				log.Errorf("Error in receiving stream : %v", err)
				if ag.isStopped() {
					return
				}

				time.Sleep(time.Second)
				break
			}
			ag.notifications = append(ag.notifications, evtList.Events...)
			ag.handleTechSupportEvents(evtList)
		}
		log.Infof("TSMClient %s stopped watch", ag.name)
		atomic.StoreInt32(&ag.watching, 0)
	}

	ag.tsGrpcClient.Close()
	ag.tsGrpcClient = nil
}

func (ag *TSMClient) sendUpdate(work *tsproto.TechSupportRequest, status tsproto.TechSupportRequestStatus_ActionStatus) {
	log.Infof("Sending update [%v] to Venice for techsupport.", status.String())
	update := &tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: kindTechSupportRequest,
		},
		ObjectMeta: api.ObjectMeta{
			Name: work.ObjectMeta.Name,
		},
		Spec: tsproto.TechSupportRequestSpec{
			InstanceID: work.Spec.InstanceID,
		},
		Status: tsproto.TechSupportRequestStatus{
			Status: status,
		},
	}

	if status == tsproto.TechSupportRequestStatus_InProgress {
		startTime := api.Timestamp{}
		startTime.Parse("now()")
		update.Status.StartTime = &startTime
		work.Status.StartTime = &startTime
	} else if status == tsproto.TechSupportRequestStatus_Failed || status == tsproto.TechSupportRequestStatus_Completed {
		endTime := api.Timestamp{}
		err := endTime.Parse("now()")
		if err != nil {
			log.Errorf("Error parsing time.")
			return
		}
		update.Status.StartTime = work.Status.StartTime
		update.Status.EndTime = &endTime
		update.Status.URI = work.Status.URI

		if len(work.Status.Reason) > 0 {
			update.Status.Reason = work.Status.Reason
		}
	}

	updParams := &tsproto.UpdateTechSupportResultParameters{
		NodeName: ag.name,
		NodeKind: ag.kind,
		Request:  update,
	}

	if ag.tsAPIClient != nil {
		ag.tsAPIClient.UpdateTechSupportResult(ag.ctx, updParams)
	}
}

// StartWorking the work be the worker
func (ag *TSMClient) StartWorking() {
	log.Info("Worker is starting work.")
	defer ag.waitGrp.Done()

	for {
		select {
		case <-ag.ctx.Done():
			return
		case work := <-ag.tsCh:
			ag.DoWork(work)
		}
	}
}

// DoWork executes commands for collecting techsupport
func (ag *TSMClient) DoWork(work tsproto.TechSupportRequest) error {
	log.Info("Worker doing work.")
	ag.sendUpdate(&work, tsproto.TechSupportRequestStatus_InProgress)
	err := ag.pre(&work)
	if err != nil {
		log.Errorf("Failed to finish pre-work : %v", err)
		work.Status.Reason = fmt.Sprintf("Pre-Work failed. Err : %v", err)
		ag.sendUpdate(&work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	err = ag.do(&work)
	if err != nil {
		log.Errorf("Failed to finish work : %v", err)
		work.Status.Reason = fmt.Sprintf("Failed Work. Err : %v", err)
		ag.sendUpdate(&work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	err = ag.post(&work)
	if err != nil {
		log.Errorf("Failed to finish post-work : %v", err)
		work.Status.Reason = fmt.Sprintf("Post-Work failed. Err : %v", err)
		ag.sendUpdate(&work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	ag.sendUpdate(&work, tsproto.TechSupportRequestStatus_Completed)
	return nil
}

func (ag *TSMClient) deleteTechsupportFiles(work *tsproto.TechSupportRequest) error {
	log.Info("Deleting techsupport files")
	targetID := ag.generateTargetID(work.Spec.InstanceID, work.ObjectMeta.Name)
	cmd := fmt.Sprintf("rm -rf %s/%s", ag.cfg.FileSystemRoot, targetID)
	return action.RunShellCmd(cmd, ag.cfg.FileSystemRoot)
}

func (ag *TSMClient) handleTechSupportRetention(work *tsproto.TechSupportRequest) error {
	if ag.cfg.Retention == tsconfig.TechSupportConfig_Manual {
		return nil
	}

	if ag.cfg.Retention == tsconfig.TechSupportConfig_DelOnExport {
		return ag.deleteTechsupportFiles(work)
	}

	return nil
}

func (ag *TSMClient) pre(work *tsproto.TechSupportRequest) error {
	log.Info("PreWork")
	return nil
}

func (ag *TSMClient) do(work *tsproto.TechSupportRequest) error {
	log.Info("Actual Work")
	var instanceID, instanceName string
	if work.Spec.InstanceID == "" {
		instanceID = string(time.Now().Unix())
		instanceID = strings.Replace(instanceID, " ", "-", -1)
		instanceName = "techsupport"
	} else {
		instanceID = work.Spec.InstanceID
		instanceName = work.ObjectMeta.Name
	}
	targetID := ag.generateTargetID(instanceID, instanceName)
	vosTarget := fmt.Sprintf("%v.tar.gz", targetID)
	err := action.CollectTechSupport(ag.cfg, targetID, work)
	if err != nil {
		log.Errorf("Err : %v", err)
		delErr := ag.deleteTechsupportFiles(work)
		if delErr != nil {
			log.Errorf("Delete techsupport files failed. Err : %v", delErr)
		}

		return err
	}

	err = export.GenerateTechsupportZip(vosTarget, ag.cfg.FileSystemRoot+"/"+targetID)
	if err != nil {
		delErr := ag.deleteTechsupportFiles(work)
		if delErr != nil {
			log.Errorf("Delete techsupport files failed. Err : %v", delErr)
		}

		return err
	}
	tarballFile := ag.cfg.FileSystemRoot + "/" + targetID + "/" + vosTarget

	for _, destination := range work.Spec.Destinations {
		log.Infof("Destination : %v", destination)
		switch destination.Proto.String() {
		case "Local":
			log.Info("Keep file local")
		case "SCP":
			log.Info("SCP file")
			err = export.ScpFile(tarballFile, destination.Destination, "root", "docker", destination.Path)
			if err != nil {

				delErr := ag.deleteTechsupportFiles(work)
				if delErr != nil {
					log.Errorf("Delete techsupport files failed. Err : %v", delErr)
				}

				return err
			}
		case "Venice":
			// TODO : Update the tenant default field once that is used and available as part of the the techsupport request
			uri := fmt.Sprintf("/objstore/v1/downloads/tenant/default/techsupport/%v", vosTarget)
			work.Status.URI = uri
			log.Infof("Send to VENICE. WORK : %v. URL : %v", work, uri)
			err = export.SendToVenice(ag.resolverClient, tarballFile, vosTarget)
			if err != nil {
				delErr := ag.deleteTechsupportFiles(work)
				if delErr != nil {
					log.Errorf("Delete techsupport files failed. Err : %v", delErr)
				}
				return err
			}
		case "HTTPS":
			log.Info("Transfer file using HTTPs")
			err = export.SendToHTTP(tarballFile, destination.Path, "", "")
			if err != nil {
				delErr := ag.deleteTechsupportFiles(work)
				if delErr != nil {
					log.Errorf("Delete techsupport files failed. Err : %v", delErr)
				}
				return err
			}
		}
	}

	return nil
}

func (ag *TSMClient) post(work *tsproto.TechSupportRequest) error {
	log.Info("Post Work")
	return ag.handleTechSupportRetention(work)
}

// ListTechSupportRequests List of all techsupport request - complete/incomplete presented
func (ag *TSMClient) ListTechSupportRequests() []*tsproto.TechSupportRequestEvent {
	return ag.notifications
}

func (ag *TSMClient) generateTargetID(instanceID, instanceName string) string {
	// Split the instance ID received
	idSplit := strings.Split(instanceID, "-")
	return fmt.Sprintf("%s-%s-%s-%s", instanceName, idSplit[0], ag.kind, ag.name)
}
