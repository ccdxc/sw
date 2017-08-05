// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cni

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"testing"

	"github.com/containernetworking/cni/pkg/skel"
	"github.com/containernetworking/cni/pkg/types"
	"github.com/pensando/sw/agent/netagent"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/ctrler/npm/rpcserver/netproto"
	. "github.com/pensando/sw/utils/testutils"
	"github.com/prometheus/common/log"
)

const fakeCniServerURL = "/tmp/fake-cni.sock"

// dial a unix domain socket
func unixDial(proto, addr string) (conn net.Conn, err error) {
	return net.Dial("unix", fakeCniServerURL)
}

// make a HTTP call and return the result
func httpCall(callURL string, args, result interface{}) error {
	// create a HTTP client
	transport := &http.Transport{Dial: unixDial}
	client := &http.Client{Transport: transport}

	// json encode the request
	buf, err := json.Marshal(args)
	if err != nil {
		return err
	}

	// make a call to cni server
	body := bytes.NewBuffer(buf)
	url := "http://localhost" + callURL
	r, err := client.Post(url, "application/json", body)
	if err != nil {
		log.Errorf("Error making call to %s/%s. Err: %v", fakeCniServerURL, callURL, err)
		return err
	}
	defer r.Body.Close()

	// check the HTTP status code
	if r.StatusCode != 200 {
		log.Errorf("Got error response from cni server. Code: %v. Err: %v", r.StatusCode, r.Status)
		return fmt.Errorf("Error response from cni server")
	}

	// read the response
	response, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Error reading response from cni server: Err: %v", err)
		return err
	}

	// decode the response
	err = json.Unmarshal(response, &result)
	if err != nil {
		log.Errorf("Error decoding response from cni server. Err: %v. Resp; %v", err, response)
		return err
	}

	return nil
}

type DummyAgent struct {
}

func (d *DummyAgent) EndpointCreateReq(epinfo *netproto.Endpoint) (*netproto.Endpoint, *netagent.IntfInfo, error) {
	// fail the operation if we are supposed to
	if epinfo.Spec.EndpointUUID == "fail" {
		return nil, nil, fmt.Errorf("Endpoint create failure")
	}

	// create dummy endpoint
	ep := netproto.Endpoint{
		TypeMeta:   api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: epinfo.ObjectMeta,
		Spec:       epinfo.Spec,
		Status: netproto.EndpointStatus{
			IPv4Address: "10.1.1.1/24",
			IPv4Gateway: "10.1.1.254",
		},
	}

	// container and switch interface names
	cintf := fmt.Sprintf("cvport-%s", epinfo.Spec.EndpointUUID[:8])
	sintf := fmt.Sprintf("svport-%s", epinfo.Spec.EndpointUUID[:8])

	// create an interface info
	intfInfo := netagent.IntfInfo{
		ContainerIntfName: cintf,
		SwitchIntfName:    sintf,
	}

	return &ep, &intfInfo, nil
}

func (d *DummyAgent) EndpointDeleteReq(epinfo *netproto.Endpoint) error {
	// fail the operation if we are supposed to
	if epinfo.Spec.EndpointUUID == "fail" {
		return fmt.Errorf("Endpoint delete failure")
	}

	return nil
}

// TestCNIServer tests basic cni server functionality
func TestCNIServer(t *testing.T) {
	var result types.Result

	// create a dummy network agent
	dummyAgent := DummyAgent{}

	// remove old .sock files
	os.Remove(fakeCniServerURL)

	// create a new CNI server
	cniServer, err := NewCniServer(fakeCniServerURL, &dummyAgent)
	defer os.Remove(fakeCniServerURL)
	AssertOk(t, err, "creating cni server")
	Assert(t, (cniServer != nil), "error creating cni server")

	// call add pod API
	// create addpod args
	stdinBytes, _ := json.Marshal(types.NetConf{Name: "pensando", Type: "pensando-net"})
	addPodArgs := skel.CmdArgs{
		ContainerID: "testContainerID",
		Netns:       "/proc/self/ns/net",
		IfName:      "eth10",
		Args:        "IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=nginx3-3072235686-may09;K8S_POD_INFRA_CONTAINER_ID=6b4b163b08bca761dda5bff027ca407e8c30f54f451ab5412475718b288a9566",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   stdinBytes,
	}

	// make a call to cni server
	err = httpCall(AddPodURL, &addPodArgs, &result)
	AssertOk(t, err, "Making AddPod call")

	// call delete pod API
	// create delete pod args
	delPodArgs := skel.CmdArgs{
		ContainerID: "test-container-id",
		Netns:       "/proc/self/ns/net",
		IfName:      "eth10",
		Args:        "IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=nginx3-3072235686-may09;K8S_POD_INFRA_CONTAINER_ID=6b4b163b08bca761dda5bff027ca407e8c30f54f451ab5412475718b288a9566",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   stdinBytes,
	}
	// make a call to cni server
	err = httpCall(DelPodURL, &delPodArgs, &result)
	AssertOk(t, err, "Making DelPod call")

}

func TestCNIServerErrors(t *testing.T) {
	var result types.Result

	// create a dummy network agent
	dummyAgent := DummyAgent{}

	// create the .sock file to induce an error
	os.Create(fakeCniServerURL)

	// create a new CNI server
	_, err := NewCniServer(fakeCniServerURL, &dummyAgent)
	Assert(t, (err != nil), "creating cni server succesful")

	// remove old .sock files
	os.Remove(fakeCniServerURL)

	// verify we can create now.
	cniServer, err := NewCniServer(fakeCniServerURL, &dummyAgent)
	AssertOk(t, err, "creating cni server")
	Assert(t, (cniServer != nil), "error creating cni server")
	defer os.Remove(fakeCniServerURL)

	// build bad pod args
	stdinBytes, _ := json.Marshal(types.NetConf{Name: "pensando", Type: "pensando-net"})
	addPodArgs := skel.CmdArgs{
		ContainerID: "testContainerID",
		Netns:       "/proc/self/ns/net",
		IfName:      "eth10",
		Args:        "Invalid",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   stdinBytes,
	}

	// make a call to cni server
	err = httpCall(AddPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "Invalid pod argument accepted")

	// build bad pod args
	addPodArgs = skel.CmdArgs{
		ContainerID: "testContainerID",
		Netns:       "/proc/self/ns/net",
		IfName:      "eth10",
		Args:        "IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=nginx3-3072235686-may09;K8S_POD_INFRA_CONTAINER_ID=6b4b163b08bca761dda5bff027ca407e8c30f54f451ab5412475718b288a9566",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   []byte{1, 2, 3, 4},
	}

	// make a call to cni server
	err = httpCall(AddPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "Invalid pod argument accepted")

	// make a call to cni server
	err = httpCall(DelPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "Invalid pod argument accepted")

	// verify call to bad URL fails
	err = httpCall("/Pensando/invalid", &addPodArgs, &result)
	Assert(t, (err != nil), "Invalid URL accepted")

	// build bad pod args
	stdinBytes, _ = json.Marshal(types.NetConf{Name: "pensando", Type: "pensando-net"})
	addPodArgs = skel.CmdArgs{
		ContainerID: "testContainerID",
		Netns:       "/proc/inv1234/ns/net",
		IfName:      "eth10",
		Args:        "IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=nginx3-3072235686-may09;K8S_POD_INFRA_CONTAINER_ID=6b4b163b08bca761dda5bff027ca407e8c30f54f451ab5412475718b288a9566",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   stdinBytes,
	}
	// make a call to cni server
	err = httpCall(AddPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "Invalid pod argument accepted")

	// make a call to cni server
	transport := &http.Transport{Dial: unixDial}
	client := &http.Client{Transport: transport}
	r, err := client.Post("http://localhost"+AddPodURL, "application/json", new(bytes.Buffer))
	AssertOk(t, err, "Failed to make http call")
	defer r.Body.Close()
	Assert(t, (r.StatusCode != 200), "add pod call with bad args suceeded")

	// build bad pod args
	stdinBytes, _ = json.Marshal(types.NetConf{Name: "pensando", Type: "pensando-net"})
	addPodArgs = skel.CmdArgs{
		ContainerID: "fail",
		Netns:       "/proc/self/ns/net",
		IfName:      "eth10",
		Args:        "IgnoreUnknown=1;K8S_POD_NAMESPACE=default;K8S_POD_NAME=nginx3-3072235686-may09;K8S_POD_INFRA_CONTAINER_ID=6b4b163b08bca761dda5bff027ca407e8c30f54f451ab5412475718b288a9566",
		Path:        "/opt/cni/bin:/opt/pensando-net/bin",
		StdinData:   stdinBytes,
	}

	// verify we get a failure
	err = httpCall(AddPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "expecting pod create failure")

	// verify we get a failure
	err = httpCall(DelPodURL, &addPodArgs, &result)
	Assert(t, (err != nil), "expecting pod create failure")
}
