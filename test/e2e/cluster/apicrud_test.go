package cluster

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"sync"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

type savedConfig struct {
	networks []*network.Network
	tenants  []*cluster.Tenant
}

const (
	etcdCertFile = "/var/lib/pensando/pki/pen-etcd/auth/cert.pem"
	etcdKeyFile  = "/var/lib/pensando/pki/pen-etcd/auth/key.pem"
	caBundle     = "/var/lib/pensando/pki/pen-etcd/auth/ca-bundle.pem"
	etcdCtlPath  = "/usr/local/bin/etcdctl"
)

func testAPICRUDOps() func() {
	return func() {
		lctx, cancel := context.WithCancel(ts.tu.MustGetLoggedInContext(context.Background()))
		waitWatch := make(chan bool)
		grpcClient := ts.tu.APIClient
		Expect(grpcClient).ShouldNot(BeNil())
		apigwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
		restClient, err := apiclient.NewRestAPIClient(apigwAddr)
		Expect(err).To(BeNil())
		defer restClient.Close()
		addToWatchList := func(list []kvstore.WatchEvent, obj runtime.Object, tpe kvstore.WatchEventType) []kvstore.WatchEvent {
			return append(list, kvstore.WatchEvent{Type: tpe, Object: obj})
		}
		var rcvNEventsMutex sync.Mutex
		var rcvNEvents, expNEvents []kvstore.WatchEvent
		var rcvTEventsMutex sync.Mutex
		var rcvTEvents, expTEvents []kvstore.WatchEvent
		wctx, wcancel := context.WithCancel(lctx)
		stopped := false
		Expect(grpcClient).ShouldNot(BeNil())
		go func() {
			success := false
			defer func() {
				if !success {
					close(waitWatch)
				}
				GinkgoRecover()
			}()
			opts := api.ListWatchOptions{}
			opts.FieldChangeSelector = []string{"."}
			opts.Name = globals.DefaultTenant
			var tWatcher, nWatcher kvstore.Watcher
			var err error
			setupWatchers := func() {
				Eventually(func() error {
					tWatcher, err = grpcClient.ClusterV1().Tenant().Watch(wctx, &opts)
					return err
				}, 30, 1).Should(BeNil(), "Watch should be successful")
				opts = api.ListWatchOptions{}
				opts.Tenant = globals.DefaultTenant
				Eventually(func() error {
					nWatcher, err = grpcClient.NetworkV1().Network().Watch(wctx, &opts)
					return err
				}, 30, 1).Should(BeNil(), "Watch should be successful")
			}
			setupWatchers()

			Expect(err).To(BeNil())
			success = true
			close(waitWatch)
			active := true
			for active {
				select {
				case ev, ok := <-tWatcher.EventChan():
					if ok {
						rcvTEventsMutex.Lock()
						rcvTEvents = append(rcvTEvents, *ev)
						rcvTEventsMutex.Unlock()
					} else {
						if !stopped {
							setupWatchers()
						} else {
							active = false
						}
					}
				case ev, ok := <-nWatcher.EventChan():
					if ok {
						rcvNEventsMutex.Lock()
						rcvNEvents = append(rcvNEvents, *ev)
						rcvNEventsMutex.Unlock()
					} else {
						if !stopped {
							setupWatchers()
						} else {
							active = false
						}
					}
				case <-wctx.Done():
					active = false
				}
			}
		}()
		<-waitWatch

		{ // Create tenant
			tenant := cluster.Tenant{
				TypeMeta: api.TypeMeta{
					Kind:       "Tenant",
					APIVersion: "v1",
				},
				ObjectMeta: api.ObjectMeta{
					Name: globals.DefaultTenant,
				},
				Spec: cluster.TenantSpec{
					AdminUser: "admin",
				},
			}
			retten, err := grpcClient.ClusterV1().Tenant().Get(lctx, &tenant.ObjectMeta)
			if err == nil {
				// Delete all networks
				netws, err := grpcClient.NetworkV1().Network().List(lctx, &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
				By(fmt.Sprintf("got networks [%+v]", netws))
				Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
				for _, n := range netws {
					expNEvents = addToWatchList(expNEvents, n, kvstore.Created)
					_, err = grpcClient.NetworkV1().Network().Delete(lctx, &api.ObjectMeta{Tenant: n.Tenant, Name: n.Name})
					Expect(err).Should(BeNil(), fmt.Sprintf("got error deleting networks[%v] (%v)", n.Name, apierrors.FromError(err)))
				}
				// Add deleted for the network
				for _, n := range netws {
					expNEvents = addToWatchList(expNEvents, n, kvstore.Deleted)
				}
				expTEvents = addToWatchList(expTEvents, retten, kvstore.Created)
			} else {
				ret, err := grpcClient.ClusterV1().Tenant().Create(lctx, &tenant)
				Expect(err).To(BeNil())
				Expect(reflect.DeepEqual(ret.Spec, tenant.Spec)).To(Equal(true))
				expTEvents = addToWatchList(expTEvents, ret, kvstore.Created)
			}
		}

		{ // gRPC Crud operations
			numNetw := 20 // Number of networks
			netwPrefix := "e2eNetwork"
			netw := network.Network{
				TypeMeta: api.TypeMeta{
					Kind:       "Network",
					APIVersion: "v1",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Name:      netwPrefix,
					Namespace: globals.DefaultNamespace,
				},
				Spec: network.NetworkSpec{
					Type:        network.NetworkType_Bridged.String(),
					IPv4Subnet:  "10.0.0.0/8",
					IPv4Gateway: "10.1.1.1",
				},
			}
			{ // Cleanup if object already exists
				for i := 0; i < numNetw; i++ {
					var del bool
					objMeta := netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					if ret, err := grpcClient.NetworkV1().Network().Get(lctx, &objMeta); err == nil {
						del = true
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
					}
					// Wait to receive event before proceeding
					if len(expNEvents) > 0 {
						Eventually(func() string {
							rcvNEventsMutex.Lock()
							defer rcvNEventsMutex.Unlock()
							if len(rcvNEvents) == len(expNEvents) {
								return "success"
							}
							return fmt.Sprintf("got %v Tenant events expecing %v events", len(rcvTEvents), len(expTEvents))
						}, 10, 1).Should(Equal("success"), "Number of bulk Tenant watch events did not match")
					}
					if del {
						ret, err := grpcClient.NetworkV1().Network().Delete(lctx, &objMeta)
						Expect(err).To(BeNil())
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
					}
				}
			}
			{ // Create networks via GRPC
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := grpcClient.NetworkV1().Network().Create(lctx, &netw)
					Expect(err).Should(BeNil(), fmt.Sprintf("got error Creating network %s (%s)", objMeta.Name, err))
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
				}
			}
			{ // List operation
				// List all the networks, from:0
				retList, err := grpcClient.NetworkV1().Network().List(lctx, &api.ListWatchOptions{From: 0})
				Expect(err).To(BeNil())
				Expect(len(retList)).To(Equal(numNetw))
				for _, retNetw := range retList {
					netwFound := false
					for i := 0; i < numNetw; i++ {
						objMeta := &netw.ObjectMeta
						objMeta.Name = netwPrefix + strconv.Itoa(i)
						if retNetw.GetObjectMeta().GetName() == objMeta.Name {
							Expect(reflect.DeepEqual(retNetw.Spec, netw.Spec)).To(Equal(true))
							netwFound = true
							break
						}
					}
					Expect(netwFound).To(Equal(true), "Network not found "+retNetw.Name)
				}
			}
			{ // Update operation
				netw.Spec.IPv4Gateway = "10.1.1.254"
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := grpcClient.NetworkV1().Network().Update(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Updated)
				}
			}
			{ // Get operations
				ret, err := grpcClient.NetworkV1().Network().Get(lctx, &netw.ObjectMeta)
				Expect(err).To(BeNil())
				Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
			}
			{ // Delete operation
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := grpcClient.NetworkV1().Network().Delete(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
				}
			}
		}

		{ // REST Crud operations
			numNetw := 30 // Number of networks
			netwPrefix := "e2eNetwork300"
			netw := network.Network{
				TypeMeta: api.TypeMeta{
					Kind:       "Network",
					APIVersion: "v1",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    globals.DefaultTenant,
					Namespace: globals.DefaultNamespace,
					Name:      netwPrefix,
				},
				Spec: network.NetworkSpec{
					Type:        network.NetworkType_Bridged.String(),
					IPv4Subnet:  "10.0.0.0/8",
					IPv4Gateway: "11.1.1.1",
				},
			}
			{ // Cleanup if object already exists
				for i := 0; i < numNetw; i++ {
					var del bool
					objMeta := netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					if ret, err := restClient.NetworkV1().Network().Get(lctx, &objMeta); err == nil {
						del = true
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
					}
					// Wait to receive event before proceeding
					if len(expNEvents) > 0 {
						Eventually(func() string {
							rcvNEventsMutex.Lock()
							defer rcvNEventsMutex.Unlock()
							if len(rcvNEvents) == len(expNEvents) {
								return "success"
							}
							return fmt.Sprintf("got %v Tenant events expecing %v events", len(rcvTEvents), len(expTEvents))
						}, 10, 1).Should(Equal("success"), "Number of bulk Tenant watch events did not match")
					}
					if del {
						ret, err := restClient.NetworkV1().Network().Delete(lctx, &objMeta)
						Expect(err).To(BeNil())
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
					}
				}
			}
			{ // Create network via GRPC
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := restClient.NetworkV1().Network().Create(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
				}
			}
			{ // List operation
				// List networks from index 10 to 19, both inclusive
				retList, err := restClient.NetworkV1().Network().List(lctx, &api.ListWatchOptions{SortOrder: api.ListWatchOptions_ByCreationTime.String(), From: 11, MaxResults: 10})
				Expect(err).To(BeNil())
				Expect(len(retList)).To(Equal(10))
				for _, retNetw := range retList {
					netwFound := false
					for i := 10; i < 20; i++ {
						objMeta := &netw.ObjectMeta
						objMeta.Name = netwPrefix + strconv.Itoa(i)
						if retNetw.GetObjectMeta().GetName() == objMeta.Name {
							Expect(reflect.DeepEqual(retNetw.Spec, netw.Spec)).To(Equal(true))
							netwFound = true
							break
						}
					}
					Expect(netwFound).To(Equal(true), "Network not found "+retNetw.Name)
				}
			}
			{ // Update network operation
				netw.Spec.IPv4Gateway = "11.1.1.254"
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := restClient.NetworkV1().Network().Update(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Updated)
				}
			}
			{ // Get operation
				var ret *network.Network
				var err error
				Eventually(func() error {
					ret, err = restClient.NetworkV1().Network().Get(lctx, &netw.ObjectMeta)
					return err
				}, 30, 1).Should(BeNil(), "should be able to get Network")
				Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
			}
			{ // Delete operation
				for i := 0; i < numNetw; i++ {
					objMeta := &netw.ObjectMeta
					objMeta.Name = netwPrefix + strconv.Itoa(i)
					ret, err := restClient.NetworkV1().Network().Delete(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
				}
			}
		}
		// Validate Watch Events
		Eventually(func() string {
			rcvTEventsMutex.Lock()
			defer rcvTEventsMutex.Unlock()
			if len(rcvTEvents) == len(expTEvents) {
				return "success"
			}
			return fmt.Sprintf("got %v Tenant events expecing %v events", len(rcvTEvents), len(expTEvents))
		}, 10, 1).Should(Equal("success"), "Number of Tenant watch events did not match")
		Eventually(func() string {
			rcvNEventsMutex.Lock()
			defer rcvNEventsMutex.Unlock()
			if len(rcvNEvents) == len(expNEvents) {
				return "success"
			}
			retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
			for i, ev := range rcvNEvents {
				retStr += fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
			}
			return retStr
		}, 10, 1).Should(Equal("success"), "Number of Network watch events did not match")
		Consistently(func() string {
			rcvNEventsMutex.Lock()
			defer rcvNEventsMutex.Unlock()
			if len(rcvNEvents) == len(expNEvents) {
				return "success"
			}
			retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
			for i, ev := range rcvNEvents {
				retStr += fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
			}
			return retStr
		}, 10, 1).Should(Equal("success"), "Number of Network watch events did not match")
		stopped = true
		wcancel()
		for k := range expTEvents {
			eSpec := expTEvents[k].Object.(*cluster.Tenant).Spec
			rSpec := rcvTEvents[k].Object.(*cluster.Tenant).Spec
			retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
			for i, ev := range rcvNEvents {
				retStr += fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
			}
			Expect(expTEvents[k].Type).To(Equal(rcvTEvents[k].Type), fmt.Sprintf("[%d] [%v]want[%+v], got [%v][%+v]\n %s", k, expTEvents[k].Key, eSpec, rcvTEvents[k].Key, rSpec, retStr))
			Expect(reflect.DeepEqual(eSpec, rSpec)).To(Equal(true), fmt.Sprintf("[%d] [%v]want[%+v], got [[%v]%+v]\n %s", k, expTEvents[k].Key, eSpec, rcvTEvents[k].Key, rSpec, retStr))
		}
		for k := range expNEvents {
			eSpec := expNEvents[k].Object.(*network.Network).Spec
			rSpec := rcvNEvents[k].Object.(*network.Network).Spec
			retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
			for i, ev := range rcvNEvents {
				retStr += fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
			}
			Expect(expNEvents[k].Type).To(Equal(rcvNEvents[k].Type), fmt.Sprintf("[%d] [%v]want[%+v], [%v]got [%+v]\n %s", k, expNEvents[k].Key, eSpec, rcvNEvents[k].Key, rSpec, retStr))
			Expect(reflect.DeepEqual(eSpec, rSpec)).To(Equal(true), fmt.Sprintf("[%d] [%v]want[%+v], [%v]got [%+v]\n %s", k, expNEvents[k].Key, eSpec, rcvNEvents[k].Key, rSpec, retStr))
		}
		cancel()
	}
}

// Basic CRUD Tests with reload
var _ = Describe("api crud tests", func() {
	getAPIServerNodeNRestartCount := func() (int64, string, string, error) {
		out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods -o wide --no-headers | grep pen-apiserver "), "\n")
		By(fmt.Sprintf("kubectl get pods -o wide --no-headers | grep pen-apiserver: %s", out))
		for _, line := range out {
			fields := strings.Fields(line)
			if len(fields) == 9 {
				restarts, err := strconv.ParseInt(fields[3], 10, 64)
				if err != nil {
					return 0, "", "", err
				}
				return restarts, fields[6], fields[0], nil
			}
		}
		return 0, "", "", errors.New("api server not found")
	}

	waitETCDHealthy := func(node string) bool {
		ip := ts.tu.NameToIPMap[node]
		healthCmd := fmt.Sprintf("%s --cert-file=%s  --key-file=%s --ca-file=%s --endpoints https://%s:5002 cluster-health", etcdCtlPath, etcdCertFile, etcdKeyFile, caBundle, node)
		id := ts.tu.GetContainerOnNode(ip, "etcd")
		Eventually(func() bool {
			out := ts.tu.CommandOutputIgnoreError(ip, fmt.Sprintf("docker exec %v %s", id, healthCmd))
			lines := strings.Split(out, "\n")
			for _, l := range lines {
				if l != "" {
					if !strings.Contains(l, "is healthy") {
						return false
					}
				}
			}
			return true
		}, 30, 1).Should(BeTrue(), "ETCD cluster was not healty in time")
		return true
	}

	restartETCD := func(node string) {
		ip := ts.tu.NameToIPMap[node]
		waitETCDHealthy(node)
		ts.tu.KillContainerOnNodeByName(ip, "etcd")
	}

	drainNode := func(selNode string) {
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl drain %v --ignore-daemonsets", selNode))
		By(fmt.Sprintf("restart got the following output \n %v\n", out))
		time.Sleep(time.Second)
		out = ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get nodes | grep \"%v\"", selNode))
		By(fmt.Sprintf("get nodes got the following output [%v]", out))
		Eventually(strings.Contains(out, "SchedulingDisabled"), 10, 1).Should(BeTrue())
	}

	undrainNode := func(selNode string) {
		ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl uncordon %v", selNode))
		time.Sleep(time.Second)
		out := ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get nodes | grep \"%v\"", selNode))
		Eventually(strings.Contains(out, "SchedulingDisabled"), 10, 1).Should(BeFalse())
	}

	saveConfig := func(cfg *savedConfig) {
		var err error
		grpcClient := ts.tu.APIClient
		Expect(grpcClient).ShouldNot(BeNil())
		numNetw := 40 // Create 40 networks
		netwPrefix := "e2eSaveNetwork"

		netw := network.Network{
			TypeMeta: api.TypeMeta{
				Kind:       "Network",
				APIVersion: "v1",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Name:      netwPrefix,
			},
			Spec: network.NetworkSpec{
				Type:        network.NetworkType_Bridged.String(),
				IPv4Subnet:  "10.1.1.1/24",
				IPv4Gateway: "10.1.1.1",
			},
		}

		for i := 0; i < numNetw; i++ {
			netw.ObjectMeta.Name = netwPrefix + strconv.Itoa(i)
			_, err := grpcClient.NetworkV1().Network().Create(context.Background(), &netw)
			Expect(err).Should(BeNil(), fmt.Sprintf("got error Creatring network %s (%s)", netw.ObjectMeta.Name, err))
		}
		cfg.networks, err = grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{SortOrder: api.ListWatchOptions_ByCreationTime.String(), ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
		Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
		cfg.tenants, err = grpcClient.ClusterV1().Tenant().List(context.Background(), &api.ListWatchOptions{})
		Expect(err).Should(BeNil(), fmt.Sprintf("got error listing tenants (%s)", err))
	}

	clearSavedConfig := func(cfg *savedConfig) {
		grpcClient := ts.tu.APIClient
		Expect(grpcClient).ShouldNot(BeNil())
		for _, n := range cfg.networks {
			grpcClient.NetworkV1().Network().Delete(context.Background(), &n.ObjectMeta)
		}
	}

	checkConfig := func(cfg *savedConfig) func() {
		return func() {
			grpcClient := ts.tu.APIClient
			Expect(grpcClient).ShouldNot(BeNil())

			// List last 10 networks
			netws, err := grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{From: 31, MaxResults: 20, SortOrder: api.ListWatchOptions_ByCreationTime.String(), ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
			Expect(len(netws)).Should(Equal(10))
			// By(fmt.Sprintf("Got Networks [%+v]", netws))
			// By(fmt.Sprintf("Saved Networks [%+v]", cfg.networks))
			for _, n := range netws {
				found := false
				for _, n1 := range cfg.networks[30:] {
					if n1.Name == n.Name {
						found = true
						Expect(reflect.DeepEqual(n1.Spec, n.Spec)).Should(BeTrue(), fmt.Sprintf("Specs dont match [%+v][%+v]", n1, n))
					}
				}
				Expect(found).Should(BeTrue(), fmt.Sprintf("not found object [%v]", n.Name))
			}

			// List last 10-17 networks
			netws, err = grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{From: 11, MaxResults: 7, SortOrder: api.ListWatchOptions_ByCreationTime.String(), ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
			Expect(len(netws)).Should(Equal(7))
			// By(fmt.Sprintf("Got Networks [%+v]", netws))
			// By(fmt.Sprintf("Saved Networks [%+v]", cfg.networks))
			for _, n := range netws {
				found := false
				for _, n1 := range cfg.networks[10:17] {
					if n1.Name == n.Name {
						found = true
						Expect(reflect.DeepEqual(n1.Spec, n.Spec)).Should(BeTrue(), fmt.Sprintf("Specs dont match [%+v][%+v]", n1, n))
					}
				}
				Expect(found).Should(BeTrue(), fmt.Sprintf("not found object [%v]", n.Name))
			}

			// List first 25 networks
			netws, err = grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{From: 1, MaxResults: 25, SortOrder: api.ListWatchOptions_ByCreationTime.String(), ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
			Expect(len(netws)).Should(Equal(25))
			// By(fmt.Sprintf("Got Networks [%+v]", netws))
			// By(fmt.Sprintf("Saved Networks [%+v]", cfg.networks))
			for _, n := range netws {
				found := false
				for _, n1 := range cfg.networks[0:25] {
					if n1.Name == n.Name {
						found = true
						Expect(reflect.DeepEqual(n1.Spec, n.Spec)).Should(BeTrue(), fmt.Sprintf("Specs dont match [%+v][%+v]", n1, n))
					}
				}
				Expect(found).Should(BeTrue(), fmt.Sprintf("not found object [%v]", n.Name))
			}

			// List all networks, From = 0
			netws, err = grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			Expect(err).Should(BeNil(), fmt.Sprintf("got error listing networks (%s)", err))
			Expect(len(netws)).Should(Equal(len(cfg.networks)))

			// By(fmt.Sprintf("Got Networks [%+v]", netws))
			// By(fmt.Sprintf("Saved Networks [%+v]", cfg.networks))
			for _, n := range netws {
				found := false
				for _, n1 := range cfg.networks {
					if n1.Name == n.Name {
						found = true
						Expect(reflect.DeepEqual(n1.Spec, n.Spec)).Should(BeTrue(), fmt.Sprintf("Specs dont match [%+v][%+v]", n1, n))
					}
				}
				Expect(found).Should(BeTrue(), fmt.Sprintf("not found object [%v]", n.Name))
			}

			tens, err := grpcClient.ClusterV1().Tenant().List(context.Background(), &api.ListWatchOptions{})
			Expect(err).Should(BeNil(), fmt.Sprintf("got error listing tenants (%s)", err))
			Expect(len(tens)).Should(Equal(len(cfg.tenants)))
			By(fmt.Sprintf("Got Tenants [%+v]", tens))
			By(fmt.Sprintf("Saved Tenants [%+v]", cfg.tenants))
			for _, n := range tens {
				found := false
				for _, n1 := range cfg.tenants {
					By(fmt.Sprintf("Looking for tenant [%v] [%v]", n.Name, n1.Name))
					if n1.Name == n.Name {
						found = true
						Expect(reflect.DeepEqual(n1.Spec, n.Spec)).Should(BeTrue(), fmt.Sprintf("Specs dont match [%+v][%+v]", n1, n))
					}
				}
				Expect(found).Should(BeTrue(), fmt.Sprintf("not found object [%v]", n.Name))
			}
		}
	}

	BeforeEach(func() {
		validateCluster()
	})

	Context("API server and API Gateway should be running", func() {
		It("API Gateway should be running on all nodes", func() {
			for _, ip := range ts.tu.QuorumNodes {
				Eventually(func() string {
					return ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get pods -o wide --no-headers | grep pen-apigw | grep %v", ip))
				}, 30, 1).ShouldNot(BeEmpty(), "pen-apigateway container should be running on %s", ip)
			}
		})

		It("kubernetes indicates API server to be running", func() {
			Eventually(func() string {
				out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods -o wide --no-headers | grep pen-apiserver "), "\n")
				for _, line := range out {
					if strings.Contains(line, "Running") {
						return line
					}
				}
				return ""
			}, 30, 1).ShouldNot(BeEmpty(), "API server should be in running state")
		})
	})

	Context("API CRUD operations via gRPC and API Gateway should work", func() {
		var restarts int64
		BeforeEach(func() {
			starts, _, _, err := getAPIServerNodeNRestartCount()
			Expect(err).To(BeNil())
			restarts = starts
		})

		It("Validate CRUD ops", testAPICRUDOps())

		AfterEach(func() {
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted")
		})
	})

	Context("API CRUD operations via gRPC and API Gateway should work after a restart", func() {
		var (
			restarts int64
		)

		BeforeEach(func() {
			if ts.tu.NumQuorumNodes < 3 {
				Skip(fmt.Sprintf("Skipping API Infra Tests test: %d quorum nodes found, need >= 3", ts.tu.NumQuorumNodes))
			}
			starts, apiServerNode, name, err := getAPIServerNodeNRestartCount()
			Expect(err).To(BeNil())
			By(fmt.Sprintf("APIServer is running at %v as %v with restart count [%d]", apiServerNode, name, starts))
			drainNode(apiServerNode)
			Eventually(func() string {
				newRestarts, newNode, newName, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				if newNode == apiServerNode {
					return fmt.Sprintf("waiting for restart got want: !(%v)/got :(%v)", apiServerNode, newNode)
				}
				restarts = newRestarts
				By(fmt.Sprintf("After restart APIServer is running at %v as %v with restart count [%d]", newNode, newName, newRestarts))
				return ""
			}, 30, 1).Should(BeEmpty(), "API server should have restarted")
			undrainNode(apiServerNode)

			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
		})

		It("Restart API Server and validate CRUD ops", testAPICRUDOps())

		AfterEach(func() {
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
		})
	})

	Context("API CRUD operations via gRPC and API Gateway should work after a restart", func() {
		var (
			restarts int64
		)

		BeforeEach(func() {
			starts, apiServerNode, name, err := getAPIServerNodeNRestartCount()
			Expect(err).To(BeNil())
			ip := ts.tu.NameToIPMap[apiServerNode]

			err = ts.tu.KillContainerOnNodeByName(ip, name)
			Expect(err).To(BeNil())

			Eventually(func() string {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				if newRestarts != starts+1 {
					return fmt.Sprintf("waiting for restart got want: %d/got :%d", starts+1, newRestarts)
				}
				restarts = newRestarts
				return ""
			}, 30, 1).Should(BeEmpty(), "API server should have restarted")
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 90, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")

		})

		It("Restart API Server container and validate CRUD ops", testAPICRUDOps())

		AfterEach(func() {
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
		})
	})

	Context("Configuration should be saved across API server reloads", func() {
		var (
			restarts int64
			scfg     savedConfig
		)

		BeforeEach(func() {
			if ts.tu.NumQuorumNodes < 3 {
				Skip(fmt.Sprintf("Skipping API Infra Tests test: %d quorum nodes found, need >= 3", ts.tu.NumQuorumNodes))
			}
			saveConfig(&scfg)
			starts, apiServerNode, name, err := getAPIServerNodeNRestartCount()
			Expect(err).To(BeNil())
			By(fmt.Sprintf("APIServer is running at %v as %v with restart count [%d]", apiServerNode, name, starts))
			drainNode(apiServerNode)
			Eventually(func() string {
				newRestarts, newNode, newName, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				if newNode == apiServerNode {
					return fmt.Sprintf("waiting for restart got want: !(%v)/got :(%v)", apiServerNode, newNode)
				}
				restarts = newRestarts
				By(fmt.Sprintf("After restart APIServer is running at %v as %v with restart count [%d]", newNode, newName, newRestarts))
				return ""
			}, 30, 1).Should(BeEmpty(), "API server should have restarted")
			undrainNode(apiServerNode)

			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
		})

		It("Check config after API server restart", checkConfig(&scfg))

		AfterEach(func() {
			grpcClient := ts.tu.APIClient
			Expect(grpcClient).ShouldNot(BeNil())
			netws, err := grpcClient.NetworkV1().Network().List(context.Background(), &api.ListWatchOptions{})
			for _, n := range netws {
				_, err = grpcClient.NetworkV1().Network().Delete(context.Background(), &n.ObjectMeta)
				Expect(err).Should(BeNil(), fmt.Sprintf("got error deleting network(%s)", err))
			}
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
			clearSavedConfig(&scfg)
		})
	})

	Context("Cycle through ETCD restarts on all Quorum nodes", func() {
		var (
			restarts int64
			scfg     savedConfig
			qnode    int
		)

		BeforeEach(func() {
			if ts.tu.NumQuorumNodes < 3 {
				Skip(fmt.Sprintf("Skipping API Infra Tests test: %d quorum nodes found, need >= 3", ts.tu.NumQuorumNodes))
			}
			saveConfig(&scfg)
			starts, _, _, err := getAPIServerNodeNRestartCount()
			restarts = starts
			Expect(err).To(BeNil())
			rnode := ts.tu.QuorumNodes[qnode]
			qnode = (qnode + 1) % len(ts.tu.QuorumNodes)
			snode := ts.tu.QuorumNodes[qnode]
			By(fmt.Sprintf("restarting Etcd on node [%v]/[%v]", rnode, snode))
			restartETCD(rnode)
			waitETCDHealthy(snode)
		})

		// Cycle through 3 nodes
		It(fmt.Sprintf("Check API after [1] ETCD restart"), func() {
			checkConfig(&scfg)()
			testAPICRUDOps()()
		})
		It(fmt.Sprintf("Check API after [2] ETCD restart"), func() {
			checkConfig(&scfg)()
			testAPICRUDOps()()
		})
		It(fmt.Sprintf("Check API after [3] ETCD restart"), func() {
			checkConfig(&scfg)()
			testAPICRUDOps()()
		})

		AfterEach(func() {
			grpcClient := ts.tu.APIClient
			Expect(grpcClient).ShouldNot(BeNil())
			Consistently(func() int {
				newRestarts, _, _, err := getAPIServerNodeNRestartCount()
				if err != nil {
					return 0
				}
				return int(newRestarts)
			}, 60, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
			clearSavedConfig(&scfg)
		})
	})
	AfterEach(func() {
		validateCluster()
	})

})
