package standalone

import (
	"fmt"
	"os"
	"os/exec"

	. "github.com/onsi/ginkgo"

	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

var _ = Describe("Agent create delete loop tests", func() {
	Context("When objets are created and deleted in a loop", func() {
		var (
			lis            netutils.TestListenAddr
			lisErr         = lis.GetAvailablePort()
			agentArgs      []string
			agentBuildArgs []string
			tenantURL      = fmt.Sprintf("http://%s/api/tenants/", lis.ListenURL.String())
			namespaceURL   = fmt.Sprintf("http://%s/api/namespaces/", lis.ListenURL.String())
			networkURL     = fmt.Sprintf("http://%s/api/networks/", lis.ListenURL.String())
			epURL          = fmt.Sprintf("http://%s/api/endpoints/", lis.ListenURL.String())
			routeURL       = fmt.Sprintf("http://%s/api/routes/", lis.ListenURL.String())
			natPoolURL     = fmt.Sprintf("http://%s/api/natpools/", lis.ListenURL.String())
			natBindingURL  = fmt.Sprintf("http://%s/api/natbindings/", lis.ListenURL.String())
			natPolicyURL   = fmt.Sprintf("http://%s/api/natpolicies/", lis.ListenURL.String())
		)

		// Verify if NetAgent is up
		BeforeEach(func() {
			// Build all the type metas
			tMetas := []api.TypeMeta{
				{
					Kind: "Namespace",
				},
				{
					Kind: "Network",
				},
				{
					Kind: "Endpoint",
				},
				{
					Kind: "Route",
				},
				{
					Kind: "NatPool",
				},
				{
					Kind: "NatBinding",
				},
				{
					Kind: "NatPolicy",
				},
			}
			if lisErr != nil {
				Fail(fmt.Sprintf("could not assign an available port, %v", lisErr))
			}
			agentBuildArgs = []string{"install", agentPkgName}
			fmt.Println("Building Agent...")
			out, err := exec.Command("go", agentBuildArgs...).CombinedOutput()
			if err != nil {
				Fail(fmt.Sprintf("could not build agent binary, Out: %v Err: %v", out, err))
			}

			agentArgs = []string{"-hostif", "lo", "-logtofile", "/tmp/agent.log", "-rest-url", lis.ListenURL.String()}

			fmt.Println("Agent CLI: ", agentArgs)
			// start as the agent binary needs to run in the background
			err = exec.Command("netagent", agentArgs...).Start()
			if err != nil {
				Fail(fmt.Sprintf("could not start netagent, Err: %v", err))
			}

			ok := checkAgentIsUp(tenantURL)
			if !ok {
				Fail(fmt.Sprintf("netagent bringup took too long, %v", err))
			}

			for _, tm := range tMetas {
				err := testLoopedAddDelete(tm, lis.ListenURL.String())
				if err != nil {
					Fail(fmt.Sprintf("Looped add delete failed for %s. Err: %v", tm.Kind, err))
				}
			}
		})

		It("agent gets for namespaces should work and have only defaults", func() {

			// validate namespaces
			var nsList []*netproto.Namespace
			err := netutils.HTTPGet(namespaceURL, &nsList)
			if err != nil {
				Fail(fmt.Sprintf("could not get default namespaces, %v", err))
			}
			if len(nsList) != 1 {
				Fail(fmt.Sprintf("expected only default namespace, after symmetric create delete. But found %v. Err: %v", nsList, err))
			}

			// validate networks
			var nwList []*netproto.Network
			err = netutils.HTTPGet(networkURL, &nwList)
			if err != nil {
				Fail(fmt.Sprintf("could not get networks, %v", err))
			}
			if len(nwList) != 0 {
				Fail(fmt.Sprintf("expected no networks after symmetric create delete. But found %v. Err: %v", nwList, err))
			}

			// validate endpoints
			var epList []*netproto.Endpoint
			err = netutils.HTTPGet(epURL, &epList)
			if err != nil {
				Fail(fmt.Sprintf("could not get endpoints, %v", err))
			}
			if len(epList) != 0 {
				Fail(fmt.Sprintf("expected no endpoints after symmetric create delete. But found %v. Err: %v", epList, err))
			}

			// validate route
			var rtList []*netproto.Route
			err = netutils.HTTPGet(routeURL, &rtList)
			if err != nil {
				Fail(fmt.Sprintf("could not get routes, %v", err))
			}
			if len(rtList) != 0 {
				Fail(fmt.Sprintf("expected no routes after symmetric create delete. But found %v. Err: %v", rtList, err))
			}

			// validate nat pool
			var npList []*netproto.NatPool
			err = netutils.HTTPGet(natPoolURL, &npList)
			if err != nil {
				Fail(fmt.Sprintf("could not get nat pools, %v", err))
			}
			if len(npList) != 0 {
				Fail(fmt.Sprintf("expected no nat pools after symmetric create delete. But found %v. Err: %v", rtList, err))
			}

			// validate nat binding
			var nbList []*netproto.NatBinding
			err = netutils.HTTPGet(natBindingURL, &nbList)
			if err != nil {
				Fail(fmt.Sprintf("could not get nat bindings, %v", err))
			}
			if len(nbList) != 0 {
				Fail(fmt.Sprintf("expected no nat bindings after symmetric create delete. But found %v. Err: %v", rtList, err))
			}

			// validate nat policy
			var natPolicyList []*netproto.NatPolicy
			err = netutils.HTTPGet(natPolicyURL, &natPolicyList)
			if err != nil {
				Fail(fmt.Sprintf("could not get nat policies, %v", err))
			}
			if len(natPolicyList) != 0 {
				Fail(fmt.Sprintf("expected no nat policies after symmetric create delete. But found %v. Err: %v", rtList, err))
			}
		})

		AfterEach(func() {
			exec.Command("pkill", "netagent").Run()
			os.Remove("/tmp/naples-netagent.db")
		})
	})
})

func testLoopedAddDelete(tMeta api.TypeMeta, baseURL string) error {
	switch tMeta.Kind {
	case "Namespace":
		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Namespaces Iteration: %v #######\n", i)
			err := createNS(baseURL)
			if err != nil {
				return fmt.Errorf("looped create namespaces failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Namespaces Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteNS(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete namespaces failed. %v", err)
			}
			fmt.Println("OK")
		}
	case "Network":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during network create delete loops")
			return err
		}
		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Networks Iteration: %v #######\n", i)
			err := createNetwork(baseURL)
			if err != nil {
				return fmt.Errorf("looped create networks failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Networks Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteNetwork(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete networks failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during network create delete loops")
			return err
		}
	case "Endpoint":
		// Create backing namespaces and networks
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during endpoint create delete loops")
			return err
		}
		err = createNetwork(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during endpoint create delete loops")
			return err
		}
		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Endpoints Iteration: %v #######\n", i)
			err := createEndpoint(baseURL)
			if err != nil {
				return fmt.Errorf("looped create endpoints failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Endpoints Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteEndpoint(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete endpoints failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing objects in reverse order.
		err = deleteNetwork(baseURL)
		if err != nil {
			fmt.Println("Network deletes failed during endpoint create delete loops")
			return err
		}

		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during endpoint create delete loops")
			return err
		}
	case "Route":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during route create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Routes Iteration: %v #######\n", i)
			err := createRoute(baseURL)
			if err != nil {
				return fmt.Errorf("looped create routes failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Routes Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteRoute(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete endpoints failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during endpoint create delete loops")
			return err
		}
	case "NatPool":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during nat pool create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating NatPool Iteration: %v #######\n", i)
			err := createNatPool(baseURL)
			if err != nil {
				return fmt.Errorf("looped create routes failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting NatPool Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteNatPool(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete natpools failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during nat pool create delete loops")
			return err
		}
	case "NatBinding":
		// Create backing Namespaces and NatPool
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during nat binding create delete loops")
			return err
		}

		err = createNatPool(baseURL)
		if err != nil {
			fmt.Println("Nat Pool creates failed during nat binding create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Nat Binding Iteration: %v #######\n", i)
			err := createNatBinding(baseURL)
			if err != nil {
				return fmt.Errorf("looped create nat bindings failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Nat Binding Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteNatBinding(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete nat bindings failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces and nat pools in reverse order
		err = deleteNatPool(baseURL)
		if err != nil {
			fmt.Println("Nat Pool deletes failed during nat binding create delete loops")
			return err
		}
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during nat binding create delete loops")
			return err
		}
	case "NatPolicy":
		// Create backing Namespaces and NatPool
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during nat binding create delete loops")
			return err
		}

		err = createNatPool(baseURL)
		if err != nil {
			fmt.Println("Nat Pool creates failed during nat binding create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Nat Policy Iteration: %v #######\n", i)
			err := createNatPolicy(baseURL)
			if err != nil {
				return fmt.Errorf("looped create nat policy failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Nat Policy Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteNatPolicy(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete nat policy failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces and nat pools in reverse order
		err = deleteNatPool(baseURL)
		if err != nil {
			fmt.Println("Nat Pool deletes failed during nat policy create delete loops")
			return err
		}
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during nat policy create delete loops")
			return err
		}
	}

	return nil
}

// ################## Object Create Delete Stubs ##################

// creates three namespaces
func createNS(baseURL string) error {
	var resp error
	nsURL := fmt.Sprintf("http://%s/api/namespaces/", baseURL)

	ns1 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "kg1",
			Tenant: "default",
		},
	}
	ns2 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "kg2",
			Tenant: "default",
		},
	}
	publicNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "public",
			Tenant: "default",
		},
	}
	// Create all the namespaces
	err := netutils.HTTPPost(nsURL, &ns1, &resp)
	if err != nil {
		fmt.Println("Could not create namespace")
		return err
	}
	err = netutils.HTTPPost(nsURL, &ns2, &resp)
	if err != nil {
		fmt.Println("Could not create namespace")
		return err
	}
	err = netutils.HTTPPost(nsURL, &publicNS, &resp)
	if err != nil {
		fmt.Println("Could not create namespace")
		return err
	}
	return nil
}

// deletes three namespaces
func deleteNS(baseURL string) error {
	var resp error
	ns1 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "kg1",
			Tenant: "default",
		},
	}
	ns2 := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "kg2",
			Tenant: "default",
		},
	}
	publicNS := netproto.Namespace{
		TypeMeta: api.TypeMeta{Kind: "Namespace"},
		ObjectMeta: api.ObjectMeta{
			Name:   "public",
			Tenant: "default",
		},
	}
	// Delete all the namespaces
	deleteURL := fmt.Sprintf("http://%s/api/namespaces/default/ns1", baseURL)
	err := netutils.HTTPDelete(deleteURL, &ns1, &resp)
	if err != nil {
		fmt.Println("Could not delete namespace: ", deleteURL)
		return err
	}
	deleteURL = fmt.Sprintf("http://%s/api/namespaces/default/ns2", baseURL)
	err = netutils.HTTPDelete(deleteURL, &ns2, &resp)
	if err != nil {
		fmt.Println("Could not delete namespace")
		return err
	}
	deleteURL = fmt.Sprintf("http://%s/api/namespaces/default/public", baseURL)
	err = netutils.HTTPDelete(deleteURL, &publicNS, &resp)
	if err != nil {
		fmt.Println("Could not delete namespace")
		return err
	}
	return nil
}

// creates three networks
func createNetwork(baseURL string) error {
	var resp error

	nwURL := fmt.Sprintf("http://%s/api/networks/", baseURL)

	nw1 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.0.0.0/16",
			IPv4Gateway: "10.0.2.1",
			VlanID:      100,
		},
	}

	nw2 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "10.0.0.0/16",
			IPv4Gateway: "10.0.2.1",
			VlanID:      300,
		},
	}

	publicNw := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public",
		},
		Spec: netproto.NetworkSpec{
			IPv4Subnet:  "20.0.0.0/16",
			IPv4Gateway: "20.1.1.1",
			VlanID:      200,
		},
	}

	// Create all the networks
	err := netutils.HTTPPost(nwURL, &nw1, &resp)
	if err != nil {
		fmt.Println("Could not create network")
		return err
	}
	err = netutils.HTTPPost(nwURL, &nw2, &resp)
	if err != nil {
		fmt.Println("Could not create network")
		return err
	}
	err = netutils.HTTPPost(nwURL, &publicNw, &resp)
	if err != nil {
		fmt.Println("Could not create network")
		return err
	}
	return nil
}

// deletes three networks
func deleteNetwork(baseURL string) error {
	var resp error

	nw1 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1",
		},
	}

	nw2 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
	}

	publicNw := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public",
		},
	}

	// Delete all the networks
	networkDeleteURL := fmt.Sprintf("http://%s/api/networks/default/kg1/kg1", baseURL)
	err := netutils.HTTPDelete(networkDeleteURL, &nw1, &resp)
	if err != nil {
		fmt.Println("Could not delete network")
		return err
	}

	networkDeleteURL = fmt.Sprintf("http://%s/api/networks/default/kg2/kg2", baseURL)
	err = netutils.HTTPDelete(networkDeleteURL, &nw2, &resp)
	if err != nil {
		fmt.Println("Could not delete network")
		return err
	}

	networkDeleteURL = fmt.Sprintf("http://%s/api/networks/default/public/public", baseURL)
	err = netutils.HTTPDelete(networkDeleteURL, &publicNw, &resp)
	if err != nil {
		fmt.Println("Could not delete network")
		return err
	}
	return nil
}

// creates three endpoints
func createEndpoint(baseURL string) error {
	var resp error

	epURL := fmt.Sprintf("http://%s/api/endpoints/", baseURL)

	ep1 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1-router",
		},
		Spec: netproto.EndpointSpec{
			NetworkName: "kg1",
			Interface:   "default-uplink-0",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.2.1/16",
			MacAddress:  "00:22:0A:00:02:01",
			NodeUUID:    "GWUUID",
		},
	}

	ep2 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-router",
		},
		Spec: netproto.EndpointSpec{
			NetworkName: "kg2",
			Interface:   "default-uplink-1",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "10.0.2.1/16",
			MacAddress:  "00:22:0A:00:02:01",
			NodeUUID:    "GWUUID",
		},
	}

	publicEP := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public-router",
		},
		Spec: netproto.EndpointSpec{
			NetworkName: "public",
			Interface:   "default-uplink-1",
		},
		Status: netproto.EndpointStatus{
			IPv4Address: "20.1.1.1/16",
			MacAddress:  "00:22:14:01:01:01",
			NodeUUID:    "GWUUID",
		},
	}

	// Create all the endpoints
	err := netutils.HTTPPost(epURL, &ep1, &resp)
	if err != nil {
		fmt.Println("Could not create endpoint")
		return err
	}
	err = netutils.HTTPPost(epURL, &ep2, &resp)
	if err != nil {
		fmt.Println("Could not create endpoint")
		return err
	}
	err = netutils.HTTPPost(epURL, &publicEP, &resp)
	if err != nil {
		fmt.Println("Could not create endpoint")
		return err
	}
	return nil
}

// deletes three endpoints
func deleteEndpoint(baseURL string) error {
	var resp error

	ep1 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1-router",
		},
	}

	ep2 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-router",
		},
	}

	publicEP := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public-router",
		},
	}
	// Delete all the endpoints
	epDeleteURL := fmt.Sprintf("http://%s/api/endpoints/default/kg1/kg1-router", baseURL)
	err := netutils.HTTPDelete(epDeleteURL, &ep1, &resp)
	if err != nil {
		fmt.Println("Could not delete endpoint")
		return err
	}

	epDeleteURL = fmt.Sprintf("http://%s/api/endpoints/default/kg2/kg2-router", baseURL)
	err = netutils.HTTPDelete(epDeleteURL, &ep2, &resp)
	if err != nil {
		fmt.Println("Could not delete endpoint")
		return err
	}

	epDeleteURL = fmt.Sprintf("http://%s/api/endpoints/default/public/public-router", baseURL)
	err = netutils.HTTPDelete(epDeleteURL, &publicEP, &resp)
	if err != nil {
		fmt.Println("Could not delete endpoint")
		return err
	}
	return nil
}

// creates three routes
func createRoute(baseURL string) error {
	var resp error

	rtURL := fmt.Sprintf("http://%s/api/routes/", baseURL)

	rt1 := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.0.0.0/16",
			Interface: "default-uplink-0",
			GatewayIP: "10.0.2.1",
		},
	}

	rt2 := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "10.0.0.0/16",
			Interface: "default-uplink-1",
			GatewayIP: "10.0.2.1",
		},
	}

	publicRt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public",
		},
		Spec: netproto.RouteSpec{
			IPPrefix:  "20.0.0.0/16",
			Interface: "default-uplink-1",
			GatewayIP: "20.1.1.1",
		},
	}

	// Create all the routes
	err := netutils.HTTPPost(rtURL, &rt1, &resp)
	if err != nil {
		fmt.Println("Could not create route")
		return err
	}
	err = netutils.HTTPPost(rtURL, &rt2, &resp)
	if err != nil {
		fmt.Println("Could not create route")
		return err
	}
	err = netutils.HTTPPost(rtURL, &publicRt, &resp)
	if err != nil {
		fmt.Println("Could not create route")
		return err
	}
	return nil
}

// deletes three routes
func deleteRoute(baseURL string) error {
	var resp error

	rt1 := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1",
		},
	}

	rt2 := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
	}

	publicRt := netproto.Route{
		TypeMeta: api.TypeMeta{Kind: "Route"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "public",
		},
	}

	// Delete all the routes
	routeDeleteURL := fmt.Sprintf("http://%s/api/routes/default/kg1/kg1", baseURL)
	err := netutils.HTTPDelete(routeDeleteURL, &rt1, &resp)
	if err != nil {
		fmt.Println("Could not delete route")
		return err
	}

	routeDeleteURL = fmt.Sprintf("http://%s/api/routes/default/kg2/kg2", baseURL)
	err = netutils.HTTPDelete(routeDeleteURL, &rt2, &resp)
	if err != nil {
		fmt.Println("Could not delete route")
		return err
	}

	routeDeleteURL = fmt.Sprintf("http://%s/api/routes/default/public/public", baseURL)
	err = netutils.HTTPDelete(routeDeleteURL, &publicRt, &resp)
	if err != nil {
		fmt.Println("Could not delete route")
		return err
	}
	return nil
}

// creates nat pool
func createNatPool(baseURL string) error {
	var resp error

	natPoolURL := fmt.Sprintf("http://%s/api/natpools/", baseURL)

	np1 := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "pool-1",
		},
		Spec: netproto.NatPoolSpec{
			IPRange: "10.100.0.0-10.100.255.255",
		},
	}
	// Create the nat pool
	err := netutils.HTTPPost(natPoolURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not create nat pool")
		return err
	}
	return nil
}

// deletes a nat pool
func deleteNatPool(baseURL string) error {
	var resp error

	np1 := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "pool-1",
		},
	}
	// Delete the nat pool
	natPoolDeleteURL := fmt.Sprintf("http://%s/api/natpools/default/kg1/pool-1", baseURL)
	err := netutils.HTTPDelete(natPoolDeleteURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat pool")
		return err
	}

	return nil
}

// creates nat binding
func createNatBinding(baseURL string) error {
	var resp error

	natBindingURL := fmt.Sprintf("http://%s/api/natbindings/", baseURL)

	nb1 := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "svc1",
		},
		Spec: netproto.NatBindingSpec{
			NatPoolName: "kg1/pool-1",
			IPAddress:   "10.0.2.15",
		},
	}
	// Create the nat pool
	err := netutils.HTTPPost(natBindingURL, &nb1, &resp)
	if err != nil {
		fmt.Println("Could not create nat pool")
		return err
	}
	return nil
}

// deletes a nat binding
func deleteNatBinding(baseURL string) error {
	var resp error

	nb1 := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "svc1",
		},
	}
	// Delete the nat pool
	natPoolDeleteURL := fmt.Sprintf("http://%s/api/natbindings/default/kg2/svc1", baseURL)
	err := netutils.HTTPDelete(natPoolDeleteURL, &nb1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat binding")
		return err
	}

	return nil
}

// creates nat policy
func createNatPolicy(baseURL string) error {
	var resp error

	natPolicyURL := fmt.Sprintf("http://%s/api/natpolicies/", baseURL)

	np1 := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
		Spec: netproto.NatPolicySpec{
			Rules: []netproto.NatRule{
				{
					NatPool: "kg1/pool-1",
					Action:  "SNAT",
				},
			},
		},
	}
	// Create the nat pool
	err := netutils.HTTPPost(natPolicyURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not create nat policy")
		return err
	}
	return nil
}

// deletes a nat policy
func deleteNatPolicy(baseURL string) error {
	var resp error

	np1 := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
	}
	// Delete the nat policy
	natPolicyDeleteURL := fmt.Sprintf("http://%s/api/natpolicies/default/kg2/kg2", baseURL)
	err := netutils.HTTPDelete(natPolicyDeleteURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat policy")
		return err
	}

	return nil
}
