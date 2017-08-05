// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"

	log "github.com/Sirupsen/logrus"
	cni "github.com/containernetworking/cni/pkg/skel"
	"github.com/containernetworking/cni/pkg/types"
	cniServer "github.com/pensando/sw/agent/plugins/k8s/cni"
)

// CNIPlugin is the cni plugin instance
type CNIPlugin struct {
	agentURL string       // URL for making call to pensando agent
	client   *http.Client // HTTP client
}

// NewCNIPlugin creates a new instance of CNI plugin
func NewCNIPlugin(agentURL string) *CNIPlugin {
	// create cni plugin instance
	cniPlugin := &CNIPlugin{
		agentURL: agentURL,
	}

	// create a HTTP transport
	transport := &http.Transport{Dial: cniPlugin.unixDial}
	cniPlugin.client = &http.Client{Transport: transport}

	return cniPlugin
}

// dial a unix domain socket
func (c *CNIPlugin) unixDial(proto, addr string) (conn net.Conn, err error) {
	return net.Dial("unix", c.agentURL)
}

// make a HTTP call and return the result
func (c *CNIPlugin) httpCall(callURL string, args, result interface{}) error {
	// json encode the request
	buf, err := json.Marshal(args)
	if err != nil {
		return err
	}

	// make a call to cni server
	body := bytes.NewBuffer(buf)
	url := "http://localhost" + callURL
	r, err := c.client.Post(url, "application/json", body)
	if err != nil {
		log.Errorf("Error making call to %s/%s. Err: %v", c.agentURL, callURL, err)
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

// CmdAdd handles an Add call from kubelet
func (c *CNIPlugin) CmdAdd(args *cni.CmdArgs) error {
	var result types.Result

	log.Infof("Got CmdAdd: {%+v}. Stdin: %s", args, string(args.StdinData))

	// make a call to cni server
	err := c.httpCall(cniServer.AddPodURL, args, &result)
	if err != nil {
		log.Errorf("Error making call to %s/%s. Err: %v", c.agentURL, cniServer.AddPodURL, err)
		return err
	}

	log.Infof("Sending result: %s", result.String())

	// print the json to stdout
	return result.Print()
}

// CmdDel handles Delete call from kubelet
func (c *CNIPlugin) CmdDel(args *cni.CmdArgs) error {
	var result types.Result

	log.Infof("Got CmdDel: {%+v}. Stdin: %s", args, string(args.StdinData))

	// make a call to cni server
	err := c.httpCall(cniServer.DelPodURL, args, &result)
	if err != nil {
		log.Errorf("Error making call to %s/%s. Err: %v", c.agentURL, cniServer.DelPodURL, err)
		return err
	}

	log.Infof("Sending delete pod success. result: %s", result.String())

	return nil
}
