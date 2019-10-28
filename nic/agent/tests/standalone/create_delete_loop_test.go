package standalone

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"

	. "github.com/onsi/ginkgo"

	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
)

var _ = Describe("Agent create delete loop tests", func() {
	Context("When objets are created and deleted in a loop", func() {
		agentDB, _ := ioutil.TempFile("/tmp", "")
		var (
			lis            netutils.TestListenAddr
			lisErr         = lis.GetAvailablePort()
			agentDBName    = agentDB.Name()
			agentArgs      []string
			agentBuildArgs []string
			tenantURL      = fmt.Sprintf("http://%s/api/tenants/", lis.ListenURL.String())
			namespaceURL   = fmt.Sprintf("http://%s/api/namespaces/", lis.ListenURL.String())
			networkURL     = fmt.Sprintf("http://%s/api/networks/", lis.ListenURL.String())
			epURL          = fmt.Sprintf("http://%s/api/endpoints/", lis.ListenURL.String())
			routeURL       = fmt.Sprintf("http://%s/api/routes/", lis.ListenURL.String())
			natPoolURL     = fmt.Sprintf("http://%s/api/nat/pools/", lis.ListenURL.String())
			natBindingURL  = fmt.Sprintf("http://%s/api/nat/bindings/", lis.ListenURL.String())
			natPolicyURL   = fmt.Sprintf("http://%s/api/nat/policies/", lis.ListenURL.String())
			encryptURL     = fmt.Sprintf("http://%s/api/ipsec/encryption/", lis.ListenURL.String())
			decryptURL     = fmt.Sprintf("http://%s/api/ipsec/decryption/", lis.ListenURL.String())
			ipSecPolicyURL = fmt.Sprintf("http://%s/api/ipsec/policies/", lis.ListenURL.String())
		)

		// Verify if NetAgent is up
		BeforeEach(func() {
			// Build all the type metas
			tMetas := []api.TypeMeta{
				{
					Kind: "Namespace",
				},
				{
					Kind: "Vrf",
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
				{
					Kind: "IPSecSAEncrypt",
				},
				{
					Kind: "IPSecSADecrypt",
				},
				{
					Kind: "IPSecPolicy",
				},
			}
			if lisErr != nil {
				Fail(fmt.Sprintf("could not assign an available port, %v", lisErr))
			}

			// first build netagent binary
			agentBuildArgs = []string{"install", agentPkgName}
			fmt.Println("Building Agent...")
			out, err := exec.Command("go", agentBuildArgs...).CombinedOutput()
			if err != nil {
				Fail(fmt.Sprintf("could not build agent binary, Out: %v Err: %v", out, err))
			}

			agentArgs = []string{"-logtofile", "/tmp/agent.log", "-rest-url", lis.ListenURL.String(), "-agentdb", agentDBName}

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

			// validate vrfs
			var vrfList []*netproto.Vrf
			err := netutils.HTTPGet(namespaceURL, &vrfList)
			if err != nil {
				Fail(fmt.Sprintf("could not get default namespaces, %v", err))
			}
			if len(vrfList) != 1 {
				Fail(fmt.Sprintf("expected only default vrf, after symmetric create delete. But found %v. Err: %v", vrfList, err))
			}

			// validate namespaces
			var nsList []*netproto.Namespace
			err = netutils.HTTPGet(namespaceURL, &nsList)
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
			if len(nwList) != 1 {
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

			// validate encrypt sa
			var ipSecSAEncryptList []*netproto.IPSecSAEncrypt
			err = netutils.HTTPGet(encryptURL, &ipSecSAEncryptList)
			if err != nil {
				Fail(fmt.Sprintf("could not get ipsec encrypt SA rules, %v", err))
			}
			if len(ipSecSAEncryptList) != 0 {
				Fail(fmt.Sprintf("expected no nat ipsec sa encrypt rules after symmetric create delete. But found %v. Err: %v", rtList, err))
			}

			// validate decrypt sa
			var ipSecSADecryptList []*netproto.IPSecSADecrypt
			err = netutils.HTTPGet(decryptURL, &ipSecSADecryptList)
			if err != nil {
				Fail(fmt.Sprintf("could not get ipsec decrypt SA rules, %v", err))
			}
			if len(ipSecSADecryptList) != 0 {
				Fail(fmt.Sprintf("expected no nat ipsec sa decrypt rules after symmetric create delete. But found %v. Err: %v", rtList, err))
			}

			// validate ipsec policy
			var ipSecPolicyList []*netproto.IPSecPolicy
			err = netutils.HTTPGet(ipSecPolicyURL, &ipSecPolicyList)
			if err != nil {
				Fail(fmt.Sprintf("could not get ipsec policies, %v", err))
			}
			if len(ipSecPolicyList) != 0 {
				Fail(fmt.Sprintf("expected no ipsec policies after symmetric create delete. But found %v. Err: %v", rtList, err))
			}
		})

		AfterEach(func() {
			exec.Command("pkill", "netagent").Run()
			os.Remove(agentDBName)
		})
	})
})

func testLoopedAddDelete(tMeta api.TypeMeta, baseURL string) error {
	switch tMeta.Kind {
	case "Vrf":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during network create delete loops")
			return err
		}
		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating Vrfs Iteration: %v #######\n", i)
			err := createVrf(baseURL)
			if err != nil {
				return fmt.Errorf("looped create vrfs failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting Vrfs Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteVrf(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete vrfs failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during vrf create delete loops")
			return err
		}
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
	case "IPSecSAEncrypt":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during ipsec sa encrypt create delete loops")
			return err
		}

		// Create backing infra vrf
		err = createVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf creates failed during ipsec sa encrypt create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating IPSec SA Encrypt Iteration: %v #######\n", i)
			err := createIPSecSAEncrypt(baseURL)
			if err != nil {
				return fmt.Errorf("looped create ipsec encrypt sa failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting IPSec SA Encrypt Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteIPSecSAEncrypt(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete ipsec encrypt failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing vrf
		err = deleteVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf deletes failed during ipsec sa encrypt create delete loops")
			return err
		}

		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during ipsec encrypt sa create delete loops")
			return err
		}
	case "IPSecSADecrypt":
		// Create backing namespaces
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during ipsec sa decrypt create delete loops")
			return err
		}

		// Create backing infra vrf
		err = createVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf creates failed during ipsec sa encrypt create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating IPSec SA Decrypt Iteration: %v #######\n", i)
			err := createIPSecSADecrypt(baseURL)
			if err != nil {
				return fmt.Errorf("looped create ipsec decrypt sa failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting IPSec SA Decrypt Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteIPSecSADecrypt(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete ipsec decrypt failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing vrf
		err = deleteVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf deletes failed during ipsec sa encrypt create delete loops")
			return err
		}

		// delete the backing namespaces
		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during ipsec decrypt sa create delete loops")
			return err
		}
	case "IPSecPolicy":
		// Create backing namespaces and SA rules
		err := createNS(baseURL)
		if err != nil {
			fmt.Println("Namespace creates failed during ipsec policy create delete loops")
			return err
		}

		// Create backing infra vrf
		err = createVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf creates failed during ipsec sa encrypt create delete loops")
			return err
		}

		err = createIPSecSAEncrypt(baseURL)
		if err != nil {
			fmt.Println("IPsec SA Encrypt creates failed during ipsec policy create delete loops")
			return err
		}
		err = createIPSecSADecrypt(baseURL)
		if err != nil {
			fmt.Println("IPSec SA Decrypt creates failed during ipsec policy create delete loops")
			return err
		}

		for i := 0; i < crudLoopCount; i++ {
			fmt.Printf("####### Creating IPSec Policy Iteration: %v #######\n", i)
			err := createIPSecPolicy(baseURL)
			if err != nil {
				return fmt.Errorf("looped create ipsec policu failed. %v", err)
			}
			fmt.Println("OK")
			fmt.Printf("####### Deleting IPSec Policy Iteration: %v #######\n", i)
			time.Sleep(addDelSleepDuration)
			err = deleteIPSecPolicy(baseURL)
			if err != nil {
				return fmt.Errorf("looped delete ipsec policy failed. %v", err)
			}
			fmt.Println("OK")
		}
		// delete the backing namespaces and sa rules in reverse order
		err = deleteIPSecSAEncrypt(baseURL)
		if err != nil {
			fmt.Println("IPsec SA Encrypt deletes failed during ipsec policy create delete loops")
			return err
		}
		err = deleteIPSecSADecrypt(baseURL)
		if err != nil {
			fmt.Println("IPSec SA Decrypt deletes failed during ipsec policy create delete loops")
			return err
		}

		// delete the backing vrf
		err = deleteVrf(baseURL)
		if err != nil {
			fmt.Println("Vrf deletes failed during ipsec sa encrypt create delete loops")
			return err
		}

		err = deleteNS(baseURL)
		if err != nil {
			fmt.Println("Namespace deletes failed during ipsec policy create delete loops")
			return err
		}

	}

	return nil
}

// ################## Object Create Delete Stubs ##################

// creates three namespaces
func createNS(baseURL string) error {
	var resp restapi.Response
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
	var resp restapi.Response
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
	deleteURL := fmt.Sprintf("http://%s/api/namespaces/default/kg1", baseURL)
	err := netutils.HTTPDelete(deleteURL, &ns1, &resp)
	if err != nil {
		fmt.Println("Could not delete namespace: ", deleteURL)
		return err
	}
	deleteURL = fmt.Sprintf("http://%s/api/namespaces/default/kg2", baseURL)
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
	var resp restapi.Response

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
			RouterMAC:   "0022.0A00.0201",
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
			IPv4Subnet:  "10.1.0.0/16",
			IPv4Gateway: "10.1.2.1",
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
	var resp restapi.Response

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
	var resp restapi.Response

	epURL := fmt.Sprintf("http://%s/api/endpoints/", baseURL)

	ep1 := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "kg1-router",
		},
		Spec: netproto.EndpointSpec{
			NetworkName:   "kg1",
			InterfaceType: "uplink",
			Interface:     "uplink128",
			IPv4Addresses: []string{"10.0.2.1/16"},
			MacAddress:    "0022.0A00.0201",
			NodeUUID:      "GWUUID",
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
			NetworkName:   "kg2",
			InterfaceType: "uplink",
			Interface:     "uplink129",
			IPv4Addresses: []string{"10.0.2.1/16"},
			MacAddress:    "0022.0A00.0201",
			NodeUUID:      "GWUUID",
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
			NetworkName:   "public",
			InterfaceType: "uplink",
			Interface:     "uplink129",
			IPv4Addresses: []string{"20.1.1.1/16"},
			MacAddress:    "0022.1401.0101",
			NodeUUID:      "GWUUID",
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
	var resp restapi.Response

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
	var resp restapi.Response

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
			Interface: "uplink128",
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
			Interface: "uplink129",
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
			Interface: "uplink129",
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
	var resp restapi.Response

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
	var resp restapi.Response

	natPoolURL := fmt.Sprintf("http://%s/api/nat/pools/", baseURL)

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
	var resp restapi.Response

	np1 := netproto.NatPool{
		TypeMeta: api.TypeMeta{Kind: "NatPool"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg1",
			Name:      "pool-1",
		},
	}
	// Delete the nat pool
	natPoolDeleteURL := fmt.Sprintf("http://%s/api/nat/pools/default/kg1/pool-1", baseURL)
	err := netutils.HTTPDelete(natPoolDeleteURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat pool")
		return err
	}

	return nil
}

// creates nat binding
func createNatBinding(baseURL string) error {
	var resp restapi.Response

	natBindingURL := fmt.Sprintf("http://%s/api/nat/bindings/", baseURL)

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
	var resp restapi.Response

	nb1 := netproto.NatBinding{
		TypeMeta: api.TypeMeta{Kind: "NatBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "svc1",
		},
	}
	// Delete the nat pool
	natPoolDeleteURL := fmt.Sprintf("http://%s/api/nat/bindings/default/kg2/svc1", baseURL)
	err := netutils.HTTPDelete(natPoolDeleteURL, &nb1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat binding")
		return err
	}

	return nil
}

// creates nat policy
func createNatPolicy(baseURL string) error {
	var resp restapi.Response

	natPolicyURL := fmt.Sprintf("http://%s/api/nat/policies/", baseURL)

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
	var resp restapi.Response

	np1 := netproto.NatPolicy{
		TypeMeta: api.TypeMeta{Kind: "NatPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2",
		},
	}
	// Delete the nat policy
	natPolicyDeleteURL := fmt.Sprintf("http://%s/api/nat/policies/default/kg2/kg2", baseURL)
	err := netutils.HTTPDelete(natPolicyDeleteURL, &np1, &resp)
	if err != nil {
		fmt.Println("Could not delete nat policy")
		return err
	}

	return nil
}

// creates ipsec sa encrypt
func createIPSecSAEncrypt(baseURL string) error {
	var resp restapi.Response

	ipSecSAEncryptURL := fmt.Sprintf("http://%s/api/ipsec/encryption/", baseURL)

	sa1 := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-ipsec-sa-encrypt",
		},
		Spec: netproto.IPSecSAEncryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			EncryptAlgo:   "AES_GCM_256",
			EncryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			LocalGwIP:     "20.1.1.1",
			RemoteGwIP:    "20.1.1.2",
			SPI:           1,
			TepVrf:        "infra",
		},
	}
	// Create the nat pool
	err := netutils.HTTPPost(ipSecSAEncryptURL, &sa1, &resp)
	if err != nil {
		fmt.Println("Could not create ipsec sa encrypt")
		return err
	}
	return nil
}

// deletes a ipsec sa encrypt
func deleteIPSecSAEncrypt(baseURL string) error {
	var resp restapi.Response

	sa1 := netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-ipsec-sa-encrypt",
		},
	}
	// Delete the encrypt sa
	ipSecSAEncryptDeleteURL := fmt.Sprintf("http://%s/api/ipsec/encryption/default/kg2/kg2-ipsec-sa-encrypt", baseURL)
	err := netutils.HTTPDelete(ipSecSAEncryptDeleteURL, &sa1, &resp)
	if err != nil {
		fmt.Println("Could not delete ipsec sa encrypt")
		return err
	}

	return nil
}

// creates ipsec sa decrypt
func createIPSecSADecrypt(baseURL string) error {
	var resp restapi.Response

	ipSecSADecryptURL := fmt.Sprintf("http://%s/api/ipsec/decryption/", baseURL)

	sa1 := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-ipsec-sa-decrypt",
		},
		Spec: netproto.IPSecSADecryptSpec{
			Protocol:      "ESP",
			AuthAlgo:      "AES_GCM",
			AuthKey:       "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			DecryptAlgo:   "AES_GCM_256",
			DecryptionKey: "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
			SPI:           1,
			TepVrf:        "infra",
		},
	}
	// Create the ipsec sa decrypt
	err := netutils.HTTPPost(ipSecSADecryptURL, &sa1, &resp)
	if err != nil {
		fmt.Println("Could not create ipsec sa decrypt")
		return err
	}
	return nil
}

// deletes a ipsec sa decrypt
func deleteIPSecSADecrypt(baseURL string) error {
	var resp restapi.Response

	sa1 := netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "kg2-ipsec-sa-decrypt",
		},
	}
	// Delete the decrypt sa
	ipSecSADecryptDeleteURL := fmt.Sprintf("http://%s/api/ipsec/decryption/default/kg2/kg2-ipsec-sa-decrypt", baseURL)
	err := netutils.HTTPDelete(ipSecSADecryptDeleteURL, &sa1, &resp)
	if err != nil {
		fmt.Println("Could not delete ipsec sa decrypt")
		return err
	}

	return nil
}

// creates ipsec policy
func createIPSecPolicy(baseURL string) error {
	var resp restapi.Response

	ipSecSAPolicyURL := fmt.Sprintf("http://%s/api/ipsec/policies/", baseURL)

	ipSec := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "kg2-ipsec-decrypt-policy",
		},
		Spec: netproto.IPSecPolicySpec{
			Rules: []netproto.IPSecRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.2 - 20.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"20.1.1.1 - 20.1.1.1"},
					},
					SPI:    1,
					SAName: "kg2/kg2-ipsec-sa-decrypt",
					SAType: "DECRYPT",
				},
			},
		},
	}
	// Create the ipsec policy
	err := netutils.HTTPPost(ipSecSAPolicyURL, &ipSec, &resp)
	if err != nil {
		fmt.Println("Could not create ipsec policy")
		return err
	}
	return nil
}

// deletes an ipsec policy
func deleteIPSecPolicy(baseURL string) error {
	var resp restapi.Response

	ipSec := netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "public",
			Name:      "kg2-ipsec-decrypt-policy",
		},
	}
	// Delete the decrypt sa
	ipSecPolicyDeleteURL := fmt.Sprintf("http://%s/api/ipsec/policies/default/public/kg2-ipsec-decrypt-policy", baseURL)
	err := netutils.HTTPDelete(ipSecPolicyDeleteURL, &ipSec, &resp)
	if err != nil {
		fmt.Println("Could not delete ipsec policy")
		return err
	}

	return nil
}

// creates a vrf
func createVrf(baseURL string) error {
	var resp restapi.Response

	vrfURL := fmt.Sprintf("http://%s/api/vrfs/", baseURL)
	// create backing vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	// Create the infra vrf
	err := netutils.HTTPPost(vrfURL, &vrf, &resp)
	if err != nil {
		fmt.Println("Could not create infra vrf")
		return err

	}
	return nil
}

// deletes a vrf
func deleteVrf(baseURL string) error {
	var resp restapi.Response

	// delete backing vrf
	vrf := netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "kg2",
			Name:      "infra",
		},
		Spec: netproto.VrfSpec{
			VrfType: "INFRA",
		},
	}

	vrfDeleteURL := fmt.Sprintf("http://%s/api/vrfs/default/kg2/infra", baseURL)

	err := netutils.HTTPDelete(vrfDeleteURL, &vrf, &resp)
	if err != nil {
		fmt.Println("Could not delete infra vrf")
		return err
	}

	return nil
}
