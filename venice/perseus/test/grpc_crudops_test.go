package test

import (
	"context"
	"encoding/json"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func printError(in api.Status) {
	j, err := json.MarshalIndent(&in, "", "  ")
	if err != nil {
		fmt.Printf("failed to marshall")
	}
	fmt.Println(string(j))
}

func TestCrudOps(t *testing.T) {
	apiserverAddr := "localhost" + ":" + tinfo.apiserverport

	ctx := context.Background()
	// gRPC client
	apicl, err := client.NewGrpcUpstream("test", apiserverAddr, tinfo.l)
	if err != nil {
		t.Fatalf("cannot create grpc client")
	}
	defer apicl.Close()

	// REST Client
	restcl, err := apiclient.NewRestAPIClient("https://localhost:" + tinfo.apigwport)
	if err != nil {
		t.Fatalf("cannot create REST client")
	}
	defer restcl.Close()
	// create logged in context
	ctx, err = NewLoggedInContext(ctx, "https://localhost:"+tinfo.apigwport, tinfo.userCred)
	AssertOk(t, err, "cannot create logged in context")

	// ========= TEST gRPC CRUD Operations ========= //
	t.Logf("test GRPC crud operations")
	{ // Create a resource with Tenant and namespace where it is not allowed and expect failure
		macAddr := "0002.0000.0100"
		name := "testHost-1"
		snic := cluster.DistributedServiceCard{
			TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
			ObjectMeta: api.ObjectMeta{
				Name: name,
			},
			Spec: cluster.DistributedServiceCardSpec{
				MgmtMode:    "NETWORK",
				NetworkMode: "OOB",
			},
			Status: cluster.DistributedServiceCardStatus{
				AdmissionPhase: "ADMITTED",
				PrimaryMAC:     macAddr,
				Conditions: []cluster.DSCCondition{
					{
						Type:   "HEALTHY",
						Status: cluster.ConditionStatus_TRUE.String(),
					},
				},
			},
		}
		_, err := apicl.ClusterV1().DistributedServiceCard().Create(context.Background(), &snic)
		if err != nil {
			t.Fatalf("could not create smartnic (%s)", err)
			return
		}

		intType := "loopback-tep"
		nic := network.NetworkInterface{
			TypeMeta: api.TypeMeta{Kind: "NetworkInterface"},
			ObjectMeta: api.ObjectMeta{
				Name: name,
			},
			Spec: network.NetworkInterfaceSpec{
				Type: intType,
				IPConfig: &cluster.IPConfig{
					IPAddress: "9.9.9.9/32",
				},
				AdminStatus: "up",
				IPAllocType: "static",
			},
			Status: network.NetworkInterfaceStatus{
				Name:       name,
				DSC:        name,
				Type:       intType,
				OperStatus: "up",
			},
		}
		_, err = apicl.NetworkV1().NetworkInterface().Create(context.Background(), &nic)
		if err != nil {
			printError(apierrors.FromError(err))
			t.Fatalf("could not create tep int (%s)", err)
			return
		}
	}
	time.Sleep(10 * time.Second)
}
