// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package writer

import (
	"context"
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

// getAPIClient gets an rpc client
func (wr *APISrvWriter) getAPIClient() (apiclient.Services, error) {
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

// WriteRollout updates Rollout object
func (wr *APISrvWriter) WriteRollout(ro *rollout.Rollout) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		log.Infof("apisrvURL is null")
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		log.Infof("Updating Rollout Failed to connect get APIClient %v", err)
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	ro.ObjectMeta.ResourceVersion = ""

	// write it

	log.Infof("Updating Rollout %v Status %v", ro.Name, ro.Status)
	for ii := 0; ii < 30; ii++ {
		k, err := apicl.RolloutV1().Rollout().Update(context.Background(), ro)
		if err != nil {
			log.Infof("Rollout Update errored %v", err)
			time.Sleep(time.Second)
			continue
		}
		log.Infof("Rollout Update successful %+v", k)
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
	apicl, err := wr.getAPIClient()
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
