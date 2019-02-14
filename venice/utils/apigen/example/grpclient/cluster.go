package main

import (
	"context"
	"flag"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
)

var (
	createCl = flag.Bool("clusterCreate", false, "create a new cluster")
)

func clusterCreate(apicl apiclient.Services) {
	cl := &cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "TestCluster",
		},
		Status: cluster.ClusterStatus{
			AuthBootstrapped: true,
		},
	}
	ret, err := apicl.ClusterV1().Cluster().Create(context.Background(), cl)
	if err != nil {
		fmt.Printf("**** Cluster create failed\n")
		printError(apierrors.FromError(err))
	}
	fmt.Printf("Created cluster [%+v]\n", ret)
}
