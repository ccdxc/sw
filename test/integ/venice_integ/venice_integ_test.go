// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"flag"
	"fmt"
	"testing"
	"time"

	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"

	"golang.org/x/net/context"
	. "gopkg.in/check.v1"

	_ "github.com/influxdata/influxdb/tsdb/engine"
	_ "github.com/influxdata/influxdb/tsdb/index"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
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

	// set timeout values
	SetDefaultIntervals(time.Millisecond*500, time.Second*60)

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
		ntMeta := netproto.Network{
			TypeMeta:   api.TypeMeta{Kind: "Network"},
			ObjectMeta: nw.ObjectMeta,
		}
		_, cerr := it.snics[0].agent.PipelineAPI.HandleNetwork(agentTypes.Get, ntMeta)
		return (cerr == nil), nil
	}, "Network not found in agent", "100ms", it.pollTimeout())

	// delete the network
	_, err = it.deleteNetwork("default", "test")
	AssertOk(c, err, "Error deleting network")

	// verify network is removed from agent
	AssertEventually(c, func() (bool, interface{}) {
		ntMeta := netproto.Network{
			TypeMeta:   api.TypeMeta{Kind: "Network"},
			ObjectMeta: nw.ObjectMeta,
		}
		_, cerr := it.snics[0].agent.PipelineAPI.HandleNetwork(agentTypes.Get, ntMeta)
		return (cerr != nil), nil
	}, "Network still found in agent", "100ms", it.pollTimeout())
}

// TODO: Implement test once Orch API is refactored
func (it *veniceIntegSuite) TestVeniceIntegOCH(c *C) {
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
