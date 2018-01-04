// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package writer

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/api/generated/network"
	_ "github.com/pensando/sw/api/hooks"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	apisrvURL = "localhost:9194"
)

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "writer_test")

	// api server config
	sch := runtime.NewScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: url,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}
	// create api server
	apiSrv := apisrvpkg.MustGetAPIServer()
	go apiSrv.Run(apisrvConfig)
	time.Sleep(time.Millisecond * 100)

	return apiSrv
}

func TestNetworkWriter(t *testing.T) {
	// create network state manager
	wr, err := NewAPISrvWriter(apisrvURL, nil)
	AssertOk(t, err, "Error creating apisrv writer")

	// api server
	apiSrv := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server")

	// api server client
	logger := log.WithContext("Pkg", "writer_test")
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, logger)
	AssertOk(t, err, "Error creating api client")

	// network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "test",
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// create the network object in api server
	_, err = apicl.NetworkV1().Network().Create(context.Background(), &nw)
	AssertOk(t, err, "Error creating network")

	// write the update
	nw.Status = network.NetworkStatus{
		Workloads: []string{"test1", "test2"},
	}
	err = wr.WriteNetwork(&nw)
	AssertOk(t, err, "Error writing to apisrv")

	// get the values back from api server
	ns, err := apicl.NetworkV1().Network().Get(context.Background(), &nw.ObjectMeta)
	AssertOk(t, err, "Error getting network")
	Assert(t, (len(ns.Status.Workloads) == len(nw.Status.Workloads)), "Network params did not match", ns)

	// stop api server
	apiSrv.Stop()
}

func TestEndpointWriter(t *testing.T) {
	// create network state manager
	wr, err := NewAPISrvWriter(apisrvURL, nil)
	AssertOk(t, err, "Error creating apisrv writer")

	// api server
	apiSrv := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server")

	// app server client
	logger := log.WithContext("Pkg", "writer_test")
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, logger)
	AssertOk(t, err, "Error creating api client")

	// network object
	ep := network.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testep",
		},
		Status: network.EndpointStatus{
			IPv4Address: "10.1.1.2",
			MacAddress:  "00:01:02:03:04:05",
		},
	}

	// create the endpoint object in api server
	err = wr.WriteEndpoint(&ep, false)
	AssertOk(t, err, "Error writing to apisrv")

	// get the values back from api server
	eps, err := apicl.EndpointV1().Endpoint().Get(context.Background(), &ep.ObjectMeta)
	AssertOk(t, err, "Error getting endpoint")
	Assert(t, (eps.Status.IPv4Address == ep.Status.IPv4Address), "Endpoint params did not match", eps)

	// update the endpoint
	ep.Status.EndpointState = "newstate"
	err = wr.WriteEndpoint(&ep, true)
	AssertOk(t, err, "Error writing to apisrv")

	// get the endpoint again and verify the new values
	eps, err = apicl.EndpointV1().Endpoint().Get(context.Background(), &ep.ObjectMeta)
	AssertOk(t, err, "Error getting endpoint")
	Assert(t, (eps.Status.EndpointState == ep.Status.EndpointState), "Endpoint state wasnt updated", eps)

	// stop api server
	apiSrv.Stop()
}

func TestSgWriter(t *testing.T) {
	// create network state manager
	wr, err := NewAPISrvWriter(apisrvURL, nil)
	AssertOk(t, err, "Error creating apisrv writer")

	// api server
	apiSrv := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server")

	// api server client
	logger := log.WithContext("Pkg", "writer_test")
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, logger)
	AssertOk(t, err, "Error creating api client")

	// network object
	sg := network.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsg",
		},
		Spec: network.SecurityGroupSpec{
			WorkloadSelector: []string{"env:prod", "tier:front"},
		},
	}

	// create the network object in api server
	_, err = apicl.SecurityGroupV1().SecurityGroup().Create(context.Background(), &sg)
	AssertOk(t, err, "Error creating sg")

	// write the update
	sg.Status = network.SecurityGroupStatus{
		Workloads: []string{"test1", "test2"},
	}
	err = wr.WriteSecurityGroup(&sg)
	AssertOk(t, err, "Error writing to apisrv")

	// get the values back from api server
	sgs, err := apicl.SecurityGroupV1().SecurityGroup().Get(context.Background(), &sg.ObjectMeta)
	AssertOk(t, err, "Error getting sg")
	Assert(t, (len(sgs.Status.Workloads) == len(sg.Status.Workloads)), "Sg params did not match", sgs)

	// stop api server
	apiSrv.Stop()
}

func TestSgPolicyWriter(t *testing.T) {
	// create network state manager
	wr, err := NewAPISrvWriter(apisrvURL, nil)
	AssertOk(t, err, "Error creating apisrv writer")

	// api server
	apiSrv := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server")

	// api server client
	logger := log.WithContext("Pkg", "writer_test")
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, logger)
	AssertOk(t, err, "Error creating api client")

	// network object
	sgp := network.Sgpolicy{
		TypeMeta: api.TypeMeta{Kind: "Sgpolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   "testsgpolicy",
		},
		Spec: network.SgpolicySpec{
			AttachGroups: []string{"testsg"},
			InRules: []network.SGRule{
				{
					PeerGroup: "testsg2",
					Action:    "allow",
				},
			},
		},
	}

	// create the network object in api server
	_, err = apicl.SgpolicyV1().Sgpolicy().Create(context.Background(), &sgp)
	AssertOk(t, err, "Error creating sgpolicy")

	// write the update
	sgp.Status = network.SgpolicyStatus{
		Workloads: []string{"test1", "test2"},
	}
	err = wr.WriteSgPolicy(&sgp)
	AssertOk(t, err, "Error writing to apisrv")

	// get the values back from api server
	sgps, err := apicl.SgpolicyV1().Sgpolicy().Get(context.Background(), &sgp.ObjectMeta)
	AssertOk(t, err, "Error getting sg")
	Assert(t, (len(sgps.Status.Workloads) == len(sgp.Status.Workloads)), "Sgpolicy params did not match", sgps)

	// stop api server
	apiSrv.Stop()
}

func TestTenantWriter(t *testing.T) {
	// create network state manager
	wr, err := NewAPISrvWriter(apisrvURL, nil)
	AssertOk(t, err, "Error creating apisrv writer")

	// api server
	apiSrv := createAPIServer(apisrvURL)
	Assert(t, (apiSrv != nil), "Error creating api server")

	// api server client
	logger := log.WithContext("Pkg", "writer_test")
	apicl, err := apiclient.NewGrpcAPIClient(apisrvURL, logger)
	AssertOk(t, err, "Error creating api client")

	tn := network.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "testPostTenant",
			Name:   "testPostTenant",
		},
	}

	// create the tenant object in api server
	_, err = apicl.TenantV1().Tenant().Create(context.Background(), &tn)
	AssertOk(t, err, "Error creating network")

	// write the update
	tn.Spec.AdminUser = "testUser"
	err = wr.WriteTenant(&tn)
	AssertOk(t, err, "Error writing to apisrv")

	// get the values back from api server
	ts, err := apicl.TenantV1().Tenant().Get(context.Background(), &tn.ObjectMeta)
	AssertOk(t, err, "Error getting network")
	AssertEquals(t, "testUser", ts.Spec.AdminUser, "Tenant admin user did not match")

	// stop api server
	apiSrv.Stop()
}
