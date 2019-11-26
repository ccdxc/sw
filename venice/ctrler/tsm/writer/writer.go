// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package writer

import (
	"context"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Writer is the api provided by writer object
type Writer interface {
	WriteMirrorSession(ms *monitoring.MirrorSession) error
	WriteTechSupportRequest(ms *monitoring.TechSupportRequest) error
	GetAPIClient() (apiclient.Services, error)
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

// GetAPIClient gets an rpc client
func (wr *APISrvWriter) GetAPIClient() (apiclient.Services, error) {
	// if we already have a client, just return it
	if wr.apicl != nil {
		return wr.apicl, nil
	}

	// create the api client
	l := log.WithContext("Pkg", "TsmApiWriter")
	apicl, err := apiclient.NewGrpcAPIClient(globals.Tsm, wr.apisrvURL, l, rpckit.WithBalancer(balancer.New(wr.resolver)))
	if err != nil {
		log.Errorf("Failed to connect to api gRPC server [%s]\n", wr.apisrvURL)
		return nil, err
	}

	wr.apicl = apicl
	return apicl, err
}

// WriteMirrorSession updates MirrorSession object
func (wr *APISrvWriter) WriteMirrorSession(ms *monitoring.MirrorSession) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		return err
	}
	log.Infof("Updating MirrorSession %v Status %v Version %v", ms.Name, ms.Status.State, ms.ResourceVersion)

	// Don't clear the revision - it can endup overwriting new(updated) object, or even a newer object if delete-create
	// is done
	// ms.ObjectMeta.ResourceVersion = ""

	for i := 0; i < 5; i++ {
		_, err = apicl.MonitoringV1().MirrorSession().Update(context.Background(), ms)
		if err == nil {
			break
		}
		time.Sleep(time.Millisecond * 100)
	}

	return err
}

// WriteTechSupportRequest updates TechSupportRequest object
func (wr *APISrvWriter) WriteTechSupportRequest(tsr *monitoring.TechSupportRequest) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.GetAPIClient()
	if err != nil {
		return err
	}
	log.Infof("Updating TechSupportRequest %s Status %v Version %v", tsr.Name, tsr.Status, tsr.ResourceVersion)

	tsr.ObjectMeta.ResourceVersion = "" // no need to worry about CAS because object Spec is immutable

	_, err = apicl.MonitoringV1().TechSupportRequest().Update(context.Background(), tsr)
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
