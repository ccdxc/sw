// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cni

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"

	log "github.com/Sirupsen/logrus"
	cniapi "github.com/containernetworking/cni/pkg/skel"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/agent/netagent/netutils"
	"github.com/pensando/sw/agent/netagent/netutils/httputils"

	"github.com/containernetworking/cni/pkg/types"
	"github.com/gorilla/mux"
)

// Type for HTTP handler functions
type HttpApiFunc func(r *http.Request) (interface{}, error)

// URL to listen on
const CniServerListenURL = "/run/pensando/pensando-cni.sock"

// URL definitions
const AddPodURL = "/Pensando/AddPod"
const DelPodURL = "/Pensando/DelPod"

// PodArgs is a struct to parse pod add/delete call arguments
type PodArgs struct {
	types.CommonArgs
	K8S_POD_NAMESPACE          types.UnmarshallableString `json:"K8S_POD_NAMESPACE,omitempty"`
	K8S_POD_NAME               types.UnmarshallableString `json:"K8S_POD_NAME,omitempty"`
	K8S_POD_INFRA_CONTAINER_ID types.UnmarshallableString `json:"K8S_POD_INFRA_CONTAINER_ID,omitempty"`
}

// CniServer is an instance of CNI http server
type CniServer struct {
	listenURL  string               // URL where this server is listening
	listener   net.Listener         // listener socket
	kubeclient *KubeClient          // k8s api server client
	agent      netagent.NetAgentAPI // network agent
}

// Catchall for additional driver functions.
func unknownAction(w http.ResponseWriter, r *http.Request) {
	log.Infof("Unknown CniServer action at %q", r.URL.Path)
	content, _ := ioutil.ReadAll(r.Body)
	log.Infof("Body content: %s", string(content))
	w.WriteHeader(503)
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

	// parse pod args from args
	if err := types.LoadArgs(args.Args, podArgs); err != nil {
		return fmt.Errorf("Failed to parse pod args: %v", err)
	}

	return nil
}

// NewCniServer starts the CNI http server and returns the instance
func NewCniServer(listenURL string, nagent netagent.NetAgentAPI) (*CniServer, error) {
	var err error

	// Create an instance of CNI server
	cniServer := CniServer{
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
	// http.Serve is a blocking call. so, do this in a seperate go routine..
	go func() {
		http.Serve(l, router)
		defer l.Close()
	}()

	// return the server instance
	return &cniServer, nil
}

// AddPod handles an add Pod call from cni plugin
func (c *CniServer) AddPod(r *http.Request) (interface{}, error) {
	// parse request args and net conf
	args := cniapi.CmdArgs{}
	netconf := types.NetConf{}
	podArgs := PodArgs{}
	if err := parsePodArgs(r, &args, &netconf, &podArgs); err != nil {
		return nil, fmt.Errorf("Failed to parse pod args: %v", err)
	}

	log.Infof("Got AddPod call. Args: {%+v}, NetConf: {%+v}, PodArgs: {%+v}", args, netconf, podArgs)

	// read the Pod spec from api server
	pod, err := c.kubeclient.GetPod(string(podArgs.K8S_POD_NAMESPACE), string(podArgs.K8S_POD_NAME))
	if err != nil {
		log.Errorf("Failed to read pod info from pod name %s/%s. Err: %v", podArgs.K8S_POD_NAMESPACE, podArgs.K8S_POD_NAME, err)
		return nil, err
	}

	log.Infof("Got Pod info: %+v", pod)

	// build endpoint info
	epinfo := netagent.EndpointInfo{
		EndpointUUID:  args.ContainerID,
		ContainerUUID: args.ContainerID,
		ContainerName: string(podArgs.K8S_POD_NAME),
		NetworkName:   "default",
		TenantName:    string(podArgs.K8S_POD_NAMESPACE),
	}

	// Ask network agent to create an endpoint
	ep, intfInfo, err := c.agent.CreateEndpoint(&epinfo)
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
	ipv4AddrMask := net.IPNet{
		IP:   ep.IPv4Address,
		Mask: ep.IPv4Netmask,
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
				IP:   ep.IPv4Address,
				Mask: ep.IPv4Netmask,
			},
			Gateway: ep.IPv4Gateway,
		},
	}

	return &result, nil
}

// DelPod handles the delete pod call from cni plugin
func (c *CniServer) DelPod(r *http.Request) (interface{}, error) {
	// parse request args and net conf
	args := cniapi.CmdArgs{}
	netconf := types.NetConf{}
	podArgs := PodArgs{}
	if err := parsePodArgs(r, &args, &netconf, &podArgs); err != nil {
		return nil, fmt.Errorf("Failed to parse pod args: %v", err)
	}

	log.Infof("Got DelPod call. Args: {%+v}, NetConf: {%+v}", args, netconf)

	// build endpoint info
	epinfo := netagent.EndpointInfo{
		EndpointUUID:  args.ContainerID,
		ContainerUUID: args.ContainerID,
		ContainerName: string(podArgs.K8S_POD_NAME),
		NetworkName:   "default",
		TenantName:    string(podArgs.K8S_POD_NAMESPACE),
	}

	// Ask network agent to create and endpoint
	err := c.agent.DeleteEndpoint(&epinfo)
	if err != nil {
		log.Errorf("Error creating the endpoint: {%+v}. Err: %v", epinfo, err)
		return nil, err
	}

	return &types.Result{}, nil
}
