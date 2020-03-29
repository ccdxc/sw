// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package writer

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Writer is the api provided by writer object
type Writer interface {
	WriteRollout(ro *rollout.Rollout) error
	WriteRolloutAction(ro *rollout.Rollout) error
	Close() error
	GetAPIClient() (apiclient.Services, error)
	GetClusterVersion() string
	SetRolloutBuildVersion(string) error
	CheckRolloutInProgress() bool
}

// APISrvWriter is the writer instance
type APISrvWriter struct {
	apisrvURL string
	resolver  resolver.Interface
	apicl     apiclient.Services
}

// NewAPISrvWriter returns an API server writer
func NewAPISrvWriter(apiSrvURL string, resolver resolver.Interface) (Writer, error) {
	// create apisrv writer instance
	wr := APISrvWriter{
		apisrvURL: apiSrvURL,
		resolver:  resolver,
	}

	return &wr, nil
}

// GetAPIClient gets an rpc client
func (wr *APISrvWriter) GetAPIClient() (apiclient.Services, error) {
	// if we already have a client, just return it
	if wr.apicl != nil {
		return wr.apicl, nil
	}

	// create the api client
	l := log.WithContext("Pkg", "RolloutApiWriter")
	apicl, err := apiclient.NewGrpcAPIClient(globals.Rollout, wr.apisrvURL, l, rpckit.WithBalancer(balancer.New(wr.resolver)))
	if err != nil {
		log.Errorf("Failed to connect to api gRPC server [%s]\n", wr.apisrvURL)
		return nil, err
	}

	wr.apicl = apicl
	return apicl, err
}

// CheckRolloutInProgress returns true if rollout is in progress or errored
func (wr *APISrvWriter) CheckRolloutInProgress() bool {

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		log.Infof("Updating Rollout Failed to connect get APIClient %v", err)
		return true
	}

	obj := api.ObjectMeta{
		Name: "version",
	}

	vobj, err := apicl.ClusterV1().Version().Get(context.Background(), &obj)
	if err != nil {
		log.Errorf("Failed to get cluster object (%+v)", err)
		return true
	}
	log.Debugf("Version Object %+v", vobj)
	if vobj.Status.RolloutBuildVersion == "" {
		return false
	}
	return true
}

// GetClusterVersion returns running venice version
func (wr *APISrvWriter) GetClusterVersion() string {

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		log.Infof("Updating Rollout Failed to connect get APIClient %v", err)
		return ""
	}

	obj := api.ObjectMeta{
		Name: "version",
	}

	vobj, err := apicl.ClusterV1().Version().Get(context.Background(), &obj)
	if err != nil {
		log.Errorf("Failed to get cluster object (%+v)", err)
		return ""
	}
	log.Debugf("Version Object %+v", vobj)
	return vobj.Status.BuildVersion
}

// SetRolloutBuildVersion sets the in progress rollout build version
func (wr *APISrvWriter) SetRolloutBuildVersion(version string) error {

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		log.Infof("Updating Rollout Failed to connect get APIClient %v", err)
		return err
	}
	obj := api.ObjectMeta{
		Name: "version",
	}

	vobj, err := apicl.ClusterV1().Version().Get(context.Background(), &obj)
	if err != nil {
		log.Errorf("Failed to get cluster object (%+v)", err)
		return err
	}
	log.Debugf("Version Object %+v", vobj)
	if version == "" {
		vobj.Status.RolloutBuildVersion = version
		_, err := apicl.ClusterV1().Version().UpdateStatus(context.Background(), vobj)
		if err != nil {
			log.Errorf("Failed to update Rollout Version to cluster.Version %+v", err)
			return err
		}
	} else {
		vobj.Status.RolloutBuildVersion = fmt.Sprintf("Rollout in progress to version %s", version)
		_, err := apicl.ClusterV1().Version().UpdateStatus(context.Background(), vobj)
		if err != nil {
			log.Errorf("Failed to update Rollout Version to cluster.Version %+v", err)
			return err
		}
	}
	return nil
}

// WriteRollout updates Rollout object
func (wr *APISrvWriter) WriteRollout(ro *rollout.Rollout) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		log.Infof("apisrvURL is null")
		return nil
	}

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		log.Infof("Updating Rollout Failed to connect get APIClient %v", err)
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	ro.ObjectMeta.ResourceVersion = ""

	// write it

	log.Infof("Updating Rollout %v Status %v Percent %v", ro.Name, ro.Status.OperationalState, ro.Status.CompletionPercentage)
	for ii := 0; ii < 30; ii++ {
		k, err := apicl.RolloutV1().Rollout().UpdateStatus(context.Background(), ro)
		if err != nil {
			log.Debugf("Rollout Update errored %v", err)
			time.Sleep(time.Second)
			continue
		}
		log.Infof("Rollout Update successful %+v", k.Name)
		break
	}
	return err
}

// WriteRolloutAction updates Rollout object
func (wr *APISrvWriter) WriteRolloutAction(ro *rollout.Rollout) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		return err
	}

	// write it
	log.Infof("Updating RolloutAction %v Status %v OperationalState %v", ro.Name, ro.Status, ro.Status.OperationalState)
	obj := api.ObjectMeta{}
	roa, err := apicl.RolloutV1().RolloutAction().Get(context.Background(), &obj)
	roa.Status.OperationalState = ro.Status.OperationalState
	_, err = apicl.RolloutV1().RolloutAction().Update(context.Background(), roa)
	return err
}

// Close stops the client and releases resources
func (wr *APISrvWriter) Close() error {
	if wr.resolver != nil {
		wr.resolver.Stop()
		wr.resolver = nil
	}
	if wr.apicl != nil {
		wr.apicl.Close()
		wr.apicl = nil
	}
	return nil
}
