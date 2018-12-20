// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"fmt"
	"reflect"
	"testing"

	"golang.org/x/net/context"
	. "gopkg.in/check.v1"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/orch"
	"github.com/pensando/sw/venice/orch/simapi"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// test args
var numHosts = flag.Int("hosts", numIntegTestAgents, "Number of agents")
var datapathKind = flag.String("datapath", agentDatapathKind, "Specify the datapath type. mock | hal | delphi")

// Hook up gocheck into the "go test" runner.
func TestVeniceInteg(t *testing.T) {

	// test parameters
	cfg := GetDefaultSuiteConfig()
	cfg.NumHosts = *numHosts
	cfg.DatapathKind = *datapathKind

	// integ test suite
	var sts = &veniceIntegSuite{config: cfg}

	var _ = Suite(sts)
	TestingT(t)
}

// basic test to make sure all components come up
func (it *veniceIntegSuite) TestVeniceIntegBasic(c *C) {
	// create a network using REST api
	nw, err := it.createNetwork("default", "default", "test", "10.1.1.0/24", "10.1.1.254")
	AssertOk(c, err, "Error creating network")

	// verify network gets created in agent
	AssertEventually(c, func() (bool, interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr == nil), nil
	}, "Network not found in agent", "100ms", it.pollTimeout())

	// delete the network
	_, err = it.deleteNetwork("default", "test")
	AssertOk(c, err, "Error deleting network")

	// verify network is removed from agent
	AssertEventually(c, func() (bool, interface{}) {
		_, cerr := it.agents[0].NetworkAgent.FindNetwork(nw.ObjectMeta)
		return (cerr != nil), nil
	}, "Network still found in agent", "100ms", it.pollTimeout())
}

// Verify basic vchub functionality
func (it *veniceIntegSuite) TestVeniceIntegVCH(c *C) {
	// setup vchub client
	rpcClient, err := rpckit.NewRPCClient("venice_integ_test", vchTestURL, rpckit.WithRemoteServerName(globals.VCHub))
	defer rpcClient.Close()
	c.Assert(err, IsNil)

	vcHubClient := orch.NewOrchApiClient(rpcClient.ClientConn)
	// verify number of smartnics
	filter := &orch.Filter{}
	AssertEventually(c, func() (bool, interface{}) {
		nicList, err := vcHubClient.ListSmartNICs(context.Background(), filter)
		if err == nil && len(nicList.GetItems()) == it.config.NumHosts {
			return true, nil
		}

		return false, nil
	}, "Unable to find expected snics")

	// add a nwif and verify it is seen by client.
	addReq := &simapi.NwIFSetReq{
		WLName:    "Venice-TestVM",
		IPAddr:    "22.2.2.3",
		Vlan:      "301",
		PortGroup: "userNet101",
	}

	snicMac := it.vcHub.snics[0]
	addResp, err := it.vcHub.vcSim.CreateNwIF(snicMac, addReq)
	c.Assert(err, IsNil)

	AssertEventually(c, func() (bool, interface{}) {
		nwifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false, nil
		}

		for _, nwif := range nwifList.GetItems() {
			s := nwif.GetStatus()
			if s.MacAddress != addResp.MacAddr || s.Network != "userNet101" || s.SmartNIC_ID != snicMac || s.WlName != "Venice-TestVM" {
				continue
			}

			return true, nil
		}

		return false, nil
	}, "Unable to find expected nwif")

	// delete and verify
	it.vcHub.vcSim.DeleteNwIF(snicMac, addResp.UUID)
	AssertEventually(c, func() (bool, interface{}) {
		nwifList, err := vcHubClient.ListNwIFs(context.Background(), filter)
		if err != nil {
			return false, nil
		}

		for _, nwif := range nwifList.GetItems() {
			s := nwif.GetStatus()
			if s.MacAddress == addResp.MacAddr {
				return false, nil
			}
		}

		return true, nil
	}, "Deleted nwif still exists")

}

// test tenant watch
func (it *veniceIntegSuite) TestTenantWatch(c *C) {
	c.Skip("## skip tenant test ")
	// create watch
	client := it.apisrvClient
	ctx, cancel := context.WithCancel(context.Background())
	kvWatch, err := client.ClusterV1().Tenant().Watch(ctx, &api.ListWatchOptions{})
	defer cancel()
	AssertOk(c, err, "failed to watch tenants")
	tenChan := kvWatch.EventChan()
	defer kvWatch.Stop()

	for j := 0; j < 2; j++ {
		log.Infof("########################## tpm tenant test:%d ###################", j)
		tenantName := fmt.Sprintf("vpc-%d", j)
		// create a tenant
		_, err := it.createTenant(tenantName)
		AssertOk(c, err, fmt.Sprintf("failed to create tenant %s", tenantName))
		defer it.deleteTenant(tenantName)

		AssertEventually(c, func() (bool, interface{}) {
			_, err := it.getTenant(tenantName)
			return err == nil, err
		}, fmt.Sprintf("failed to find tenant %s ", tenantName))

		AssertEventually(c, func() (bool, interface{}) {
			select {
			case _, ok := <-tenChan:
				if !ok {
					log.Infof("###### received tenant watch %v", ok)
				}
				return ok, nil
			default:
				return false, nil
			}
		}, fmt.Sprintf("failed to receive watch event for tenant %s ", tenantName))
	}

	for j := 0; j < 2; j++ {
		log.Infof("########################## tpm tenant test:%d ###################", j)
		tenantName := fmt.Sprintf("vpc-%d", j)
		// delete a tenant
		_, err := it.deleteTenant(tenantName)
		AssertOk(c, err, fmt.Sprintf("failed to delete tenant %s", tenantName))

		AssertEventually(c, func() (bool, interface{}) {
			ten, err := it.getTenant(tenantName)
			return err != nil, ten
		}, "tenant exists after delete")

		AssertEventually(c, func() (bool, interface{}) {
			select {
			case _, ok := <-tenChan:
				return ok, nil
			default:
				return false, nil
			}
		}, fmt.Sprintf("failed to receive watch event for tenant %s ", tenantName))
	}
}

// test tpm
func (it *veniceIntegSuite) TestTelemetryPolicyMgr(c *C) {
	tenantName := fmt.Sprintf("tenant-100")
	// create a tenant
	_, err := it.createTenant(tenantName)
	AssertOk(c, err, "Error creating tenant")

	defer it.deleteTenant(tenantName)

	AssertEventually(c, func() (bool, interface{}) {
		tn, err := it.getTenant(tenantName)
		return err == nil, tn
	}, fmt.Sprintf("failed to find tenant %s ", tenantName))

	AssertEventually(c, func() (bool, interface{}) {
		sp, err := it.getStatsPolicy(tenantName)
		if err == nil {
			Assert(c, reflect.DeepEqual(sp.GetSpec(), tpm.GetDefaultStatsSpec()),
				fmt.Sprintf("stats spec didn't match: got %+v, expectd %+v",
					sp.GetSpec(), tpm.GetDefaultStatsSpec()))
			return true, nil
		}
		return false, err
	}, "failed to find stats policy")

	_, err = it.deleteTenant(tenantName)
	AssertOk(c, err, "Error deleting tenant")

	AssertEventually(c, func() (bool, interface{}) {
		_, err := it.getStatsPolicy(tenantName)
		return err != nil, nil

	}, "failed to get stats policy")

	AssertEventually(c, func() (bool, interface{}) {
		_, err := it.getFwlogPolicy(tenantName)
		return err != nil, nil

	}, "failed to delete fwlog policy")
}
