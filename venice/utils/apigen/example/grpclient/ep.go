package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/workload"
)

var (
	epCreate = flag.Bool("epcreate", false, "create EP operation")
	wlName   = flag.String("wl", "", "workload name")
	netName  = flag.String("nw", "", "network name")
	sgroups  = flag.String("sgs", "", "comma seperated list of security groups")
	tenant   = flag.String("tenant", "", "tenant name")
	epName   = flag.String("name", "TestEp1", "name of the EP")
)

func printError(in api.Status) {
	j, err := json.MarshalIndent(&in, "", "  ")
	if err != nil {
		fmt.Printf("failed to marshall")
	}
	fmt.Println(string(j))
}

func configureEps(apicl apiclient.Services) {
	sgs := strings.Split(*sgroups, ",")
	ep := &workload.Endpoint{
		ObjectMeta: api.ObjectMeta{
			Name:   *epName,
			Tenant: *tenant,
		},
		Status: workload.EndpointStatus{
			WorkloadName:   *wlName,
			SecurityGroups: sgs,
			Network:        *netName,
		},
	}
	ret, err := apicl.WorkloadV1().Endpoint().Create(context.Background(), ep)
	if err != nil {
		fmt.Printf("****Failed to create endpoint\n")
		printError(apierrors.FromError(err))
		return
	}
	fmt.Printf("Created Endpoint [%+v]\n", ret)
}
