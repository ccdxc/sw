// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package writer

import (
	"context"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Writer is the api provided by writer object
type Writer interface {
	WriteNetwork(nw *network.Network) error
	WriteEndpoint(ep *network.Endpoint, update bool) error
	WriteTenant(tn *network.Tenant) error
	WriteSecurityGroup(sg *network.SecurityGroup) error
	WriteSgPolicy(sgp *network.Sgpolicy) error
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
	l := log.WithContext("Pkg", "NpmApiWriter")
	apicl, err := apiclient.NewGrpcAPIClient(wr.apisrvURL, l, rpckit.WithBalancer(balancer.New(wr.resolver)))
	if err != nil {
		log.Errorf("Failed to connect to gRPC server [%s]\n", wr.apisrvURL)
		return nil, err
	}

	wr.apicl = apicl
	return apicl, err
}

// WriteNetwork writes network object
func (wr *APISrvWriter) WriteNetwork(nw *network.Network) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	nw.ObjectMeta.ResourceVersion = ""

	// write it
	_, err = apicl.NetworkV1().Network().Update(context.Background(), nw)
	return err
}

// WriteEndpoint writes endpoint object
func (wr *APISrvWriter) WriteEndpoint(ep *network.Endpoint, update bool) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	ep.ObjectMeta.ResourceVersion = ""

	// write it
	if update {
		_, err = apicl.EndpointV1().Endpoint().Update(context.Background(), ep)
	} else {
		_, err = apicl.EndpointV1().Endpoint().Create(context.Background(), ep)
		// if create fails, try update instead
		if err != nil {
			_, err = apicl.EndpointV1().Endpoint().Update(context.Background(), ep)
		}
	}
	return err
}

// WriteTenant writes tenant object
func (wr *APISrvWriter) WriteTenant(tn *network.Tenant) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	tn.ObjectMeta.ResourceVersion = ""

	// write it
	_, err = apicl.TenantV1().Tenant().Update(context.Background(), tn)
	return err
}

// WriteSecurityGroup writes security group object
func (wr *APISrvWriter) WriteSecurityGroup(sg *network.SecurityGroup) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	sg.ObjectMeta.ResourceVersion = ""

	// write it
	_, err = apicl.SecurityGroupV1().SecurityGroup().Update(context.Background(), sg)
	return err
}

// WriteSgPolicy write sg policy object
func (wr *APISrvWriter) WriteSgPolicy(sgp *network.Sgpolicy) error {
	// if we have no URL, we are done
	if wr.apisrvURL == "" {
		return nil
	}

	// get the api client
	apicl, err := wr.getAPIClient()
	if err != nil {
		return err
	}

	// FIXME: clear the resource version till we figure out CAS semantics
	sgp.ObjectMeta.ResourceVersion = ""

	// write it
	_, err = apicl.SgpolicyV1().Sgpolicy().Update(context.Background(), sgp)
	return err
}
