// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cni

import (
	"encoding"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"reflect"
	"strings"

	cniapi "github.com/containernetworking/cni/pkg/skel"
	"github.com/containernetworking/cni/pkg/types"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/httputils"
	agentTypes "github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// CniServerListenURL URL to listen on
const CniServerListenURL = "/run/pensando/pensando-cni.sock"

// URL definitions
const (
	AddPodURL = "/Pensando/AddPod" // to add a pod
	DelPodURL = "/Pensando/DelPod" // to delete a pod
)

// CommonArgs is common to all arg types
type CommonArgs struct {
	IgnoreUnknown types.UnmarshallableBool `json:"IgnoreUnknown,omitempty"`
}

// PodArgs is a struct to parse pod add/delete call arguments
type PodArgs struct {
	CommonArgs
	K8sPodNamespace      types.UnmarshallableString `json:"K8S_POD_NAMESPACE,omitempty"`
	K8sPodName           types.UnmarshallableString `json:"K8S_POD_NAME,omitempty"`
	K8sPodInfraContainer types.UnmarshallableString `json:"K8S_POD_INFRA_CONTAINER_ID,omitempty"`
}

// Server is an instance of CNI http server
type Server struct {
	listenURL  string                // URL where this server is listening
	listener   net.Listener          // listener socket
	kubeclient *KubeClient           // k8s api server client
	agent      agentTypes.PluginIntf // network agent
}

// Catchall for additional driver functions.
func unknownAction(w http.ResponseWriter, r *http.Request) {
	log.Infof("Unknown CniServer action at %q", r.URL.Path)
	content, _ := ioutil.ReadAll(r.Body)
	log.Infof("Body content: %s", string(content))
	w.WriteHeader(503)
}

// getKeyField is a helper function to receive Values
// Values that represent a pointer to a struct
func getKeyField(keyString string, t reflect.Type, v reflect.Value) reflect.Value {
	for _, fieldName := range []string{"IgnoreUnknown", "K8sPodNamespace", "K8sPodName"} {
		field, found := t.Elem().FieldByName(fieldName)
		if found {
			// get json annotation for the field
			jsonTag := strings.Split(field.Tag.Get("json"), ",")[0]

			// see if this is the field we are looking for
			if jsonTag == keyString {
				return v.Elem().FieldByName(fieldName)
			}
		}

	}

	return reflect.Value{}
}

// loadArgs parses args from a string in the form "K=V;K2=V2;..."
func loadArgs(args string, container interface{}) error {
	if args == "" {
		return nil
	}

	containerValue := reflect.ValueOf(container)

	pairs := strings.Split(args, ";")
	unknownArgs := []string{}
	for _, pair := range pairs {
		kv := strings.Split(pair, "=")
		if len(kv) != 2 {
			return fmt.Errorf("ARGS: invalid pair %q", pair)
		}
		keyString := kv[0]
		valueString := kv[1]
		keyField := getKeyField(keyString, reflect.TypeOf(container), containerValue)
		if !keyField.IsValid() {
			unknownArgs = append(unknownArgs, pair)
			continue
		}

		u := keyField.Addr().Interface().(encoding.TextUnmarshaler)
		err := u.UnmarshalText([]byte(valueString))
		if err != nil {
			return fmt.Errorf("ARGS: error parsing value of pair %q: %v)", pair, err)
		}
	}

	isIgnoreUnknown := getKeyField("IgnoreUnknown", reflect.TypeOf(container), containerValue).Bool()
	if len(unknownArgs) > 0 && !isIgnoreUnknown {
		return fmt.Errorf("ARGS: unknown args %q", unknownArgs)
	}

	return nil
}

// parsePodArgs parses pod args from http req body
func parsePodArgs(r *http.Request, args *cniapi.CmdArgs, netconf *types.NetConf, podArgs *PodArgs) error {
	// parse request args
	if err := httputils.ReadJSON(r, args); err != nil {
		return fmt.Errorf("failed to parse http req: %v", err)
	}

	// parse net conf
	if err := json.Unmarshal(args.StdinData, netconf); err != nil {
		return fmt.Errorf("failed to load netconf: %v", err)
	}

	log.Infof("parsing: %s", args.Args)

	// parse pod args from args
	if err := loadArgs(args.Args, podArgs); err != nil {
		return fmt.Errorf("Failed to parse pod args: %v", err)
	}

	log.Infof("Parsed pod args: %+v", podArgs)

	return nil
}

// NewCniServer starts the CNI http server and returns the instance
func NewCniServer(listenURL string, nagent agentTypes.PluginIntf) (*Server, error) {
	var err error

	// Create an instance of CNI server
	cniServer := Server{
		listenURL: listenURL,
		agent:     nagent,
	}

	// register handlers for cni
	router := mux.NewRouter()
	t := router.Headers("Content-Type", "application/json").Methods("POST").Subrouter()
	t.HandleFunc(AddPodURL, httputils.MakeHTTPHandler(cniServer.AddPod))
	t.HandleFunc(DelPodURL, httputils.MakeHTTPHandler(cniServer.DelPod))
	t.HandleFunc("/Pensando/{*}", unknownAction)

	// create a k8s api server client.
	cniServer.kubeclient, err = NewKubeclient()
	if err != nil {
		log.Errorf("Error creating kubeclient. Err: %v", err)
		return nil, err
	}

	// create a listener
	l, err := net.ListenUnix("unix", &net.UnixAddr{Name: cniServer.listenURL, Net: "unix"})
	if err != nil {
		log.Errorf("Error listening to %s. Err: %v", cniServer.listenURL, err)
		return nil, err
	}

	// save the listener so that we can Close it later
	cniServer.listener = l

	// start serving HTTP requests
	// http.Serve is a blocking call. so, do this in a separate go routine..
	go func() {
		http.Serve(l, router)
		defer l.Close()
	}()

	// return the server instance
	return &cniServer, nil
}

// AddPod handles an add Pod call from cni plugin
func (c *Server) AddPod(r *http.Request) (interface{}, error) {
	// parse request args and net conf
	args := cniapi.CmdArgs{}
	netconf := types.NetConf{}
	podArgs := PodArgs{}
	if err := parsePodArgs(r, &args, &netconf, &podArgs); err != nil {
		return nil, fmt.Errorf("Failed to parse pod args: %v", err)
	}

	log.Infof("Got AddPod call. Args: {%+v}, NetConf: {%+v}, PodArgs: {%+v}", args, netconf, podArgs)

	// read the Pod spec from api server
	pod, err := c.kubeclient.GetPod(string(podArgs.K8sPodNamespace), string(podArgs.K8sPodName))
	if err != nil {
		log.Errorf("Failed to read pod info from pod name %s/%s. Err: %v", podArgs.K8sPodNamespace, podArgs.K8sPodName, err)
		return nil, err
	}

	log.Infof("Got Pod info: %+v", pod)

	// build endpoint info
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      args.ContainerID,
			Namespace: string(podArgs.K8sPodNamespace),
			Tenant:    "default", // FIXME: where should we get the tenant from?
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   args.ContainerID,
			WorkloadUUID:   args.ContainerID,
			WorkloadName:   string(podArgs.K8sPodName),
			NetworkName:    "default", // FIXME: get the network name from somewhere
			HomingHostAddr: "",        // FIXME: get my host name/addr
			HomingHostName: "",
			IPv4Addresses:  []string{"10.1.1.1/24"},
			IPv4Gateway:    "10.1.1.254",
		},
	}

	// Ask network agent to create an endpoint
	ep, intfInfo, err := c.agent.EndpointCreateReq(&epinfo)
	if err != nil {
		log.Errorf("Error creating the endpoint: {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	log.Infof("Got endpoint info: {%+v}, IntfInfo: {%+v}", ep, intfInfo)

	// move interface to pod's network namespace
	err = netutils.MoveIntfToNamespace(intfInfo.ContainerIntfName, args.Netns)
	if err != nil {
		log.Errorf("Error moving intf %s to namespace %s. Err: %v", intfInfo.ContainerIntfName, args.Netns, err)
		return nil, err
	}

	// create ipv4 address and netmask
	ipAddr, ipNet, _ := net.ParseCIDR(ep.Spec.IPv4Addresses[0])
	ipv4AddrMask := net.IPNet{
		IP:   ipAddr,
		Mask: ipNet.Mask,
	}

	// configure the interface in new namespace
	err = netutils.SetupIntfInNamespace(intfInfo.ContainerIntfName, args.IfName, args.Netns, &ipv4AddrMask, nil)
	if err != nil {
		log.Errorf("Error configuring intf %s in namespace %s. Err: %v", intfInfo.ContainerIntfName, args.Netns, err)
		return nil, err
	}

	// build response
	result := types.Result{
		IP4: &types.IPConfig{
			IP: net.IPNet{
				IP:   ipAddr,
				Mask: ipNet.Mask,
			},
			Gateway: net.ParseIP(ep.Spec.IPv4Gateway),
		},
	}

	return &result, nil
}

// DelPod handles the delete pod call from cni plugin
func (c *Server) DelPod(r *http.Request) (interface{}, error) {
	// parse request args and net conf
	args := cniapi.CmdArgs{}
	netconf := types.NetConf{}
	podArgs := PodArgs{}
	if err := parsePodArgs(r, &args, &netconf, &podArgs); err != nil {
		return nil, fmt.Errorf("Failed to parse pod args: %v", err)
	}

	log.Infof("Got DelPod call. Args: {%+v}, NetConf: {%+v}", args, netconf)

	// build endpoint info
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      args.ContainerID,
			Namespace: string(podArgs.K8sPodNamespace),
			Tenant:    "default", // FIXME: get tenant name?
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   args.ContainerID,
			WorkloadUUID:   args.ContainerID,
			WorkloadName:   string(podArgs.K8sPodName),
			NetworkName:    "default", // FIXME: get the network name from somewhere
			HomingHostAddr: "",        // FIXME: get my host name/addr
			HomingHostName: "",
		},
	}

	// Ask network agent to create and endpoint
	err := c.agent.EndpointDeleteReq(&epinfo)
	if err != nil && !strings.Contains(err.Error(), statemgr.ErrEndpointNotFound.Error()) {
		log.Errorf("Error deleting the endpoint: {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	return &types.Result{}, nil
}
