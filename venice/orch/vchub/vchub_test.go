package main

import (
	"os"
	"testing"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/vchub/sim"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestVCHub(t *testing.T) {
	var opts cliOpts
	err := parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}
	os.Args = []string{"vchub", "-store-url", "blah:", "-vcenter-list", "user:pass@127.0.0.1:8990/sdk"}
	err = parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}

	os.Args = []string{"vchub", "-store-url", "blah:", "-vcenter-list", "http://foo", "-foo", "bar"}
	err = parseOpts(&opts)
	if err == nil {
		t.Errorf("parseOpts returned success while expecting error")
	}
	sim.Setup()
	defer sim.TearDown()
	vc1, err := sim.Simulate("127.0.0.1:8990", 2, 2)
	if err != nil {
		t.Errorf("Error %v simulating vCenter", err)
		return
	}

	time.Sleep(50 * time.Millisecond)

	os.Args = []string{"vchub", "-listen-url", ":9898", "-store-url", "memkv:", "-vcenter-list", vc1 + ",user:pass@127.0.0.1:8880/sdk"}
	err = parseOpts(&opts)
	if err != nil {
		t.Errorf("parseOpts returned %v", err)
		return
	}

	go launchVCHub(&opts)
	time.Sleep(1100 * time.Millisecond)

	// verify nwifs are created
	// setup client
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	conn, err := grpc.Dial("127.0.0.1:9898", grpcOpts...)
	if err != nil {
		t.Errorf("fail to dial: %v", err)
		return
	}
	vcHubClient := orch.NewOrchApiClient(conn)
	filter := &orch.Filter{}

	AssertEventually(t, func() bool {
		nicList, err := vcHubClient.ListSmartNICs(context.Background(), filter)
		if err != nil {
			return false
		}
		nics := nicList.GetItems()
		if len(nics) == 0 {
			return false
		}

		return true
	}, "Default SmartNICs", "100ms", "20s")

	os.Args = []string{"vchub", "-vcenter-list", vc1 + ",user:pass@127.0.0.1:8880/sdk"}
	err = parseOpts(&opts)
	go main()
	time.Sleep(100 * time.Millisecond)
}
