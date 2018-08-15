package api_test

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"sync"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

var _ = Describe("API Crud tests", func() {
	var (
		grpcClient apiclient.Services
		restClient apiclient.Services
		err        error
	)
	addToWatchList := func(list []kvstore.WatchEvent, obj runtime.Object, tpe kvstore.WatchEventType) []kvstore.WatchEvent {
		return append(list, kvstore.WatchEvent{Type: tpe, Object: obj})
	}

	getAPiServerNodeNRestartCount := func() (int64, string, string, error) {
		out := strings.Split(ts.tu.LocalCommandOutput("kubectl get pods -o wide --no-headers | grep pen-apiserver "), "\n")
		for _, line := range out {
			fields := strings.Fields(line)
			if len(fields) == 7 {
				restarts, err := strconv.ParseInt(fields[3], 10, 64)
				if err != nil {
					return 0, "", "", err
				}
				return restarts, fields[5], fields[0], nil
			}
		}
		return 0, "", "", errors.New("api server not found")
	}

	getDockerContainerId := func(node, name string) string {
		return ts.tu.CommandOutput(node, fmt.Sprintf("docker ps -q -f Name=%s", name))
	}

	restartDockerContainer := func(node, id string) {
		cmd := fmt.Sprintf("docker kill %s > /dev/null", id)
		_ = ts.tu.CommandOutputIgnoreError(node, cmd)
	}

	TestAPICRUDOps := func() func() {
		return func() {
			lctx, cancel := context.WithCancel(ts.tu.NewLoggedInContext(context.Background()))
			waitWatch := make(chan bool)

			var rcvNEventsMutex sync.Mutex
			var rcvNEvents, expNEvents []kvstore.WatchEvent
			var rcvTEventsMutex sync.Mutex
			var rcvTEvents, expTEvents []kvstore.WatchEvent
			wctx, wcancel := context.WithCancel(lctx)
			go func() {
				opts := api.ListWatchOptions{}
				tWatcher, err := grpcClient.ClusterV1().Tenant().Watch(wctx, &opts)
				Expect(err).To(BeNil())
				nWatcher, err := grpcClient.NetworkV1().Network().Watch(wctx, &opts)
				Expect(err).To(BeNil())
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
							active = false
						}
					case ev, ok := <-nWatcher.EventChan():
						if ok {
							rcvNEventsMutex.Lock()
							rcvNEvents = append(rcvNEvents, *ev)
							rcvNEventsMutex.Unlock()
						} else {
							active = false
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
						Tenant: "e2eTenant",
						Name:   "e2eTenant",
					},
					Spec: cluster.TenantSpec{
						AdminUser: "admin",
					},
				}
				_, err := grpcClient.ClusterV1().Tenant().Get(lctx, &tenant.ObjectMeta)
				if err == nil {
					ret, err := grpcClient.ClusterV1().Tenant().Delete(lctx, &tenant.ObjectMeta)
					Expect(err).To(BeNil())
					// We would have also seen the existing tenant object as a Create
					expTEvents = addToWatchList(expTEvents, ret, kvstore.Created)
					expTEvents = addToWatchList(expTEvents, ret, kvstore.Deleted)
				}
				ret, err := grpcClient.ClusterV1().Tenant().Create(lctx, &tenant)
				Expect(err).To(BeNil())
				Expect(reflect.DeepEqual(ret.Spec, tenant.Spec)).To(Equal(true))
				expTEvents = addToWatchList(expTEvents, ret, kvstore.Created)
			}

			{ // gRPC Crud operations
				netw := network.Network{
					TypeMeta: api.TypeMeta{
						Kind:       "Network",
						APIVersion: "v1",
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: "e2eTenant",
						Name:   "e2eNetwork1",
					},
					Spec: network.NetworkSpec{
						Type:        "vlan",
						IPv4Subnet:  "255.255.255.0",
						IPv4Gateway: "10.1.1.1",
					},
				}
				{ // Cleanup if objects already exists
					var del1, del2 bool
					objMeta1, objMeta2 := netw.ObjectMeta, netw.ObjectMeta
					objMeta2.Name = "e2eNetwork2"
					if ret, err := grpcClient.NetworkV1().Network().Get(lctx, &objMeta1); err == nil {
						del1 = true
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
					}
					if ret, err := grpcClient.NetworkV1().Network().Get(lctx, &objMeta2); err == nil {
						del2 = true
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

					if del1 {
						ret, err := grpcClient.NetworkV1().Network().Delete(lctx, &objMeta1)
						Expect(err).To(BeNil())
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
					}
					if del2 {
						ret, err := grpcClient.NetworkV1().Network().Delete(lctx, &objMeta2)
						Expect(err).To(BeNil())
						expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
					}
				}
				{ // Create network via GRPC
					ret, err := grpcClient.NetworkV1().Network().Create(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
				}
				{ // Update operation
					netw.Spec.IPv4Gateway = "10.1.1.254"
					ret, err := grpcClient.NetworkV1().Network().Update(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Updated)
				}
				{ // Get operations
					ret, err := grpcClient.NetworkV1().Network().Get(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
				}
				{ // Delete operation
					ret, err := grpcClient.NetworkV1().Network().Delete(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
				}
			}

			{ // REST Crud operations
				netw := network.Network{
					TypeMeta: api.TypeMeta{
						Kind:       "Network",
						APIVersion: "v1",
					},
					ObjectMeta: api.ObjectMeta{
						Tenant: "e2eTenant",
						Name:   "e2eNetwork2",
					},
					Spec: network.NetworkSpec{
						Type:        "vlan",
						IPv4Subnet:  "255.255.255.0",
						IPv4Gateway: "11.1.1.1",
					},
				}
				{ // Create network via REST
					ret, err := restClient.NetworkV1().Network().Create(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Created)
				}
				{ // Update network operation
					netw.Spec.IPv4Gateway = "11.1.1.254"
					ret, err := restClient.NetworkV1().Network().Update(lctx, &netw)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Updated)
				}
				{ // Get operation
					ret, err := restClient.NetworkV1().Network().Get(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
				}
				{ // Delete operation
					ret, err := restClient.NetworkV1().Network().Delete(lctx, &netw.ObjectMeta)
					Expect(err).To(BeNil())
					Expect(reflect.DeepEqual(ret.Spec, netw.Spec)).To(Equal(true))
					expNEvents = addToWatchList(expNEvents, ret, kvstore.Deleted)
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
					retStr = retStr + fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
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
					retStr = retStr + fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
				}
				return retStr
			}, 10, 1).Should(Equal("success"), "Number of Network watch events did not match")
			wcancel()
			for k := range expTEvents {
				eSpec := expTEvents[k].Object.(*cluster.Tenant).Spec
				rSpec := rcvTEvents[k].Object.(*cluster.Tenant).Spec
				retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
				for i, ev := range rcvNEvents {
					retStr = retStr + fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
				}
				Expect(expTEvents[k].Type).To(Equal(rcvTEvents[k].Type), fmt.Sprintf("[%d] [%v]want[%+v], got [%v][%+v]\n %s", k, expTEvents[k].Key, eSpec, rcvTEvents[k].Key, rSpec, retStr))
				Expect(reflect.DeepEqual(eSpec, rSpec)).To(Equal(true), fmt.Sprintf("[%d] [%v]want[%+v], got [[%v]%+v]\n %s", k, expTEvents[k].Key, eSpec, rcvTEvents[k].Key, rSpec, retStr))
			}
			for k := range expNEvents {
				eSpec := expNEvents[k].Object.(*network.Network).Spec
				rSpec := rcvNEvents[k].Object.(*network.Network).Spec
				retStr := fmt.Sprintf("Got %v Network events expecing %v events\n", len(rcvNEvents), len(expNEvents))
				for i, ev := range rcvNEvents {
					retStr = retStr + fmt.Sprintf("[%d]: %v, %v\n", i, ev.Type, ev.Key)
				}
				Expect(expNEvents[k].Type).To(Equal(rcvNEvents[k].Type), fmt.Sprintf("[%d] [%v]want[%+v], [%v]got [%+v]\n %s", k, expNEvents[k].Key, eSpec, rcvNEvents[k].Key, rSpec, retStr))
				Expect(reflect.DeepEqual(eSpec, rSpec)).To(Equal(true), fmt.Sprintf("[%d] [%v]want[%+v], [%v]got [%+v]\n %s", k, expNEvents[k].Key, eSpec, rcvNEvents[k].Key, rSpec, retStr))
			}
			cancel()
		}
	}
	Context("API server and API Gateway should be running", func() {
		It("API Gateway should be running on all nodes", func() {
			for _, ip := range ts.tu.QuorumNodes {
				Eventually(func() string {
					return ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get pods -o wide --no-headers | grep pen-apigw | grep %v", ip))
				}, 10, 1).ShouldNot(BeEmpty(), "pen-apigateway container should be running on %s", ip)
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
			}, 10, 1).ShouldNot(BeEmpty(), "API server should be in running state")
		})
	})

	Context("API CRUD operations via gRPC and API Gateway should work", func() {
		var restarts int64
		BeforeEach(func() {
			grpcClient = ts.tu.APIClient
			apigwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restClient, err = apiclient.NewRestAPIClient(apigwAddr)
			Expect(err).To(BeNil())
			starts, _, _, err := getAPiServerNodeNRestartCount()
			Expect(err).To(BeNil())
			restarts = starts
		})

		It("Validate CRUD ops", TestAPICRUDOps())

		AfterEach(func() {
			Consistently(func() int {
				newRestarts, _, _, err := getAPiServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted")
			restClient.Close()
		})
	})

	Context("API CRUD operations via gRPC and API Gateway should work after a restart", func() {
		var (
			restClient apiclient.Services
			err        error
			restarts   int64
		)

		BeforeEach(func() {
			grpcClient = ts.tu.APIClient
			apigwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			restClient, err = apiclient.NewRestAPIClient(apigwAddr)
			Expect(err).To(BeNil())
			starts, apiServerNode, name, err := getAPiServerNodeNRestartCount()
			Expect(err).To(BeNil())
			container := getDockerContainerId(apiServerNode, name)
			restartDockerContainer(apiServerNode, container)
			Eventually(func() string {
				newRestarts, _, _, err := getAPiServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				if newRestarts != starts+1 {
					return fmt.Sprintf("waiting for restart got want: %d/got :%d", starts+1, newRestarts)
				}
				restarts = newRestarts
				return ""
			}, 30, 1).Should(BeEmpty(), "API server should have restarted")
			Consistently(func() int {
				newRestarts, _, _, err := getAPiServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 90, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
		})

		It("Restart API gateway backends and validate CRUD ops", TestAPICRUDOps())

		AfterEach(func() {
			Consistently(func() int {
				newRestarts, _, _, err := getAPiServerNodeNRestartCount()
				Expect(err).To(BeNil(), "Could not get API server restart count")
				return int(newRestarts)
			}, 30, 1).Should(BeNumerically("==", restarts), "API server should not have restarted again")
			restClient.Close()
		})
	})
})
