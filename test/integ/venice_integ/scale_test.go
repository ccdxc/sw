// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"
	"time"

	"golang.org/x/net/context"
	. "gopkg.in/check.v1"

	. "github.com/pensando/sw/venice/utils/testutils"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfgen"
	"github.com/pensando/sw/nic/agent/netagent"
)

// TestNpmFirewallProfile tests firewall profile create/update/delete operations
func (it *veniceIntegSuite) TestScale(c *C) {
	if it.config.DatapathKind == "hal" {
		c.Skip("Uncomment when deletes go through to HAL.")
	}
	loginCtx, err := it.loggedInCtx()
	AssertOk(c, err, "Error creating logged in context")

	// get the list of smartNICs
	snicList, err := it.apisrvClient.ClusterV1().SmartNIC().List(loginCtx, &api.ListWatchOptions{})
	AssertOk(c, err, "Couldn't get the list of naples")
	fmt.Printf("Discovered %d SNICs\n", len(snicList))

	cfg := cfgen.DefaultCfgenParams
	cfg.NetworkParams.NumNetworks = 4
	cfg.WorkloadsPerHost = 200
	cfg.SGPolicyParams.NumPolicies = 1
	cfg.SGPolicyParams.NumRulesPerPolicy = 1000
	cfg.AppParams.NumApps = 1000
	cfg.NumDnsAlgs = 0
	cfg.Smartnics = snicList
	cfg.Do()

	it.createScaleConfig(loginCtx, c, cfg)
	it.deleteScaleConfig(loginCtx, c, cfg)

}

func (it *veniceIntegSuite) createScaleConfig(loginCtx context.Context, c *C, cfg *cfgen.Cfgen) {
	// create configuration: networks, hosts, workloads, firewal profile, apps, and sgpolifies
	for _, n := range cfg.Networks {
		_, err := it.apisrvClient.NetworkV1().Network().Create(loginCtx, n)
		AssertOk(c, err, fmt.Sprintf("Error creating network %+v", *n))
	}

	for _, h := range cfg.Hosts {
		_, err := it.apisrvClient.ClusterV1().Host().Create(loginCtx, h)
		AssertOk(c, err, fmt.Sprintf("Error creating host %+v", *h))
	}

	for _, w := range cfg.Workloads {
		_, err := it.restClient.WorkloadV1().Workload().Create(loginCtx, w)
		AssertOk(c, err, fmt.Sprintf("Error creating workload %+v", *w))
	}

	for _, app := range cfg.Apps {
		_, err := it.restClient.SecurityV1().App().Create(loginCtx, app)
		AssertOk(c, err, fmt.Sprintf("Error creating sgp %+v", *app))
	}

	for _, sgp := range cfg.SGPolicies {
		_, err := it.restClient.SecurityV1().SGPolicy().Create(loginCtx, sgp)
		AssertOk(c, err, fmt.Sprintf("Error creating sgp %+v", *sgp))
	}

	it.verifyCreateConfig(loginCtx, c, cfg)
}

// verify that all objects are created at the naples
func (it *veniceIntegSuite) verifyCreateConfig(loginCtx context.Context, c *C, cfg *cfgen.Cfgen) {
	waitCh := make(chan error, len(it.snics)*2)

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			found := CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListNetwork()) == len(cfg.Networks), nil
			}, "10ms", it.pollTimeout())
			fmt.Println(found)
			if !found {
				waitCh <- fmt.Errorf("Scale: Network count incorrect in datapath")
				return
			}
			agObjs := ag.NetworkAgent.ListNetwork()
			for _, obj := range cfg.Networks {
				foundObj := false
				for ii := 0; ii < len(agObjs); ii++ {
					if obj.ObjectMeta.Name == agObjs[ii].ObjectMeta.Name {
						foundObj = true
					}
				}
				if !foundObj {
					waitCh <- fmt.Errorf("Scale: couldn't find network %s on node %s", obj.ObjectMeta.Name, ag.NetworkAgent.NodeUUID)
					return
				}
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: Network info incorrect in datapath")
	}

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			found := CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListApp()) == len(cfg.Apps), nil
			}, "10ms", it.pollTimeout())
			fmt.Println(found)
			if !found {
				waitCh <- fmt.Errorf("Scale: App count incorrect in datapath")
				return
			}
			agObjs := ag.NetworkAgent.ListApp()
			for _, obj := range cfg.Apps {
				foundObj := false
				for ii := 0; ii < len(agObjs); ii++ {
					if obj.ObjectMeta.Name == agObjs[ii].ObjectMeta.Name {
						foundObj = true
					}
				}
				if !foundObj {
					waitCh <- fmt.Errorf("Scale: couldn't find app %s on node %s", obj.ObjectMeta.Name, ag.NetworkAgent.NodeUUID)
					return
				}
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: App info incorrect in datapath")
	}

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			found := CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListSGPolicy()) == len(cfg.SGPolicies), nil
			}, "10ms", it.pollTimeout())
			fmt.Println(found)
			if !found {
				waitCh <- fmt.Errorf("Scale: SGPolicy count incorrect found %d expected %d",
					len(ag.NetworkAgent.ListSGPolicy()), len(cfg.SGPolicies))
				return
			}
			agObjs := ag.NetworkAgent.ListSGPolicy()
			for _, obj := range cfg.SGPolicies {
				foundObj := false
				for ii := 0; ii < len(agObjs); ii++ {
					if obj.ObjectMeta.Name == agObjs[ii].ObjectMeta.Name {
						foundObj = true
					}
				}
				if !foundObj {
					waitCh <- fmt.Errorf("Scale: couldn't find sgpolicy %s on node %s", obj.ObjectMeta.Name, ag.NetworkAgent.NodeUUID)
					return
				}
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: SGPolicy info incorrect in datapath")
	}
}

// delete objects from venice
func (it *veniceIntegSuite) deleteScaleConfig(loginCtx context.Context, c *C, cfg *cfgen.Cfgen) {
	for _, sgp := range cfg.SGPolicies {
		_, err := it.restClient.SecurityV1().SGPolicy().Delete(loginCtx, &sgp.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting sgp %+v", *sgp))
	}
	for _, app := range cfg.Apps {
		_, err := it.restClient.SecurityV1().App().Delete(loginCtx, &app.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting app %+v", &app.ObjectMeta))
	}

	for _, w := range cfg.Workloads {
		_, err := it.restClient.WorkloadV1().Workload().Delete(loginCtx, &w.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting workload %+v", &w.ObjectMeta))
	}

	for _, h := range cfg.Hosts {
		_, err := it.restClient.ClusterV1().Host().Delete(loginCtx, &h.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting host %+v", &h.ObjectMeta))
	}
	listOpts := &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}}

	// make sure all eps are gone before deleting networks
	numIters := 0
	for {
		eps, err := it.restClient.WorkloadV1().Endpoint().List(loginCtx, listOpts)
		numIters++
		AssertOk(c, err, fmt.Sprintf("err getting ep list"))
		if len(eps) == 0 {
			break
		}
		Assert(c, numIters < 60, fmt.Sprintf("numIters %d eps %d ep[0] %+v", numIters, len(eps), eps[0]))
		time.Sleep(time.Second)
	}

	for _, n := range cfg.Networks {
		_, err := it.apisrvClient.NetworkV1().Network().Delete(loginCtx, &n.ObjectMeta)
		AssertOk(c, err, fmt.Sprintf("Error deleting network %+v", &n.ObjectMeta), "1s", it.pollTimeout())
	}

	it.verifyDeleteConfig(loginCtx, c, cfg)
}

// verify that various objects have been deleted at the agent
func (it *veniceIntegSuite) verifyDeleteConfig(loginCtx context.Context, c *C, cfg *cfgen.Cfgen) {
	waitCh := make(chan error, len(it.snics)*2)

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListApp()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Scale: All Apps are not deleted in datapath")
				return
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: App info incorrect in datapath")
	}

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListSGPolicy()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Scale: All SGPolicies are not deleted in datapath")
				return
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: SGPolicy info incorrect in datapath")
	}

	for _, sn := range it.snics {
		go func(ag *netagent.Agent) {
			if !CheckEventually(func() (bool, interface{}) {
				return len(ag.NetworkAgent.ListNetwork()) == 0, nil
			}, "10ms", it.pollTimeout()) {
				waitCh <- fmt.Errorf("Scale: All networks not deleted in datapath: %+v", ag.NetworkAgent.ListNetwork())
				return
			}

			waitCh <- nil
		}(sn.agent)
	}
	for ii := 0; ii < len(it.snics); ii++ {
		AssertOk(c, <-waitCh, "Scale: Network info incorrect in datapath")
	}

}
