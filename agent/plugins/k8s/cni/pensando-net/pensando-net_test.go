// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package main

import (
	"log"
	"net"
	"net/http"
	"os"
	"testing"

	"github.com/appc/cni/pkg/types"
	cni "github.com/containernetworking/cni/pkg/skel"
	"github.com/gorilla/mux"
	"github.com/pensando/sw/agent/netagent/netutils"
	cniServer "github.com/pensando/sw/agent/plugins/k8s/cni"
	. "github.com/pensando/sw/utils/testutils"
)

const fakeCniServerURL = "/tmp/fake-cni.sock"

type fakeCniServer struct {
	addPodCount int
	delPodCount int
}

func (c *fakeCniServer) AddPod(r *http.Request) (interface{}, error) {
	c.addPodCount++
	return &types.Result{}, nil
}

func (c *fakeCniServer) DelPod(r *http.Request) (interface{}, error) {
	c.delPodCount++
	return &types.Result{}, nil
}

func startFakeCniServer() (*fakeCniServer, error) {
	fakeServer := &fakeCniServer{}
	// register handlers for cni
	router := mux.NewRouter()
	t := router.Headers("Content-Type", "application/json").Methods("POST").Subrouter()
	t.HandleFunc(cniServer.AddPodURL, netutils.MakeHTTPHandler(fakeServer.AddPod))
	t.HandleFunc(cniServer.DelPodURL, netutils.MakeHTTPHandler(fakeServer.DelPod))

	// create a listener
	l, err := net.ListenUnix("unix", &net.UnixAddr{Name: fakeCniServerURL, Net: "unix"})
	if err != nil {
		log.Fatalf("Error listening to %s. Err: %v", fakeCniServerURL, err)
	}

	// start serving HTTP requests
	// http.Serve is a blocking call. so, do this in a seperate go routine..
	go func() {
		http.Serve(l, router)
		defer l.Close()
	}()

	return fakeServer, nil
}

// TestCNIPlugin basic test tests CNI plugin
func TestCNIPlugin(t *testing.T) {
	// remove ol .sock files
	os.Remove(fakeCniServerURL)

	// start fake CNI server
	fakeServer, err := startFakeCniServer()
	AssertOk(t, err, "creating fake cni server")

	// create a CNI plugin
	n := NewCNIPlugin(fakeCniServerURL)

	// fake add command args
	addArgs := cni.CmdArgs{
		ContainerID: "fake-container-id",
		StdinData:   []byte("fake-stdin-data"),
	}

	// make an add pod call
	n.CmdAdd(&addArgs)

	// verify fake CNI server got the data
	AssertEquals(t, 1, fakeServer.addPodCount, "checking add pod call count")

	// make an del pod call
	n.CmdDel(&addArgs)

	// verify fake CNI server got the data
	AssertEquals(t, 1, fakeServer.delPodCount, "checking del pod call count")
}

func TestCNIPluginError(t *testing.T) {
	// Create a cni plugin with wrong url
	n := NewCNIPlugin("/tmp/invalid.sock")

	// verify both add and delete calls return error
	Assert(t, (n.CmdAdd(&cni.CmdArgs{}) != nil), "add pod succeded")
	Assert(t, (n.CmdDel(&cni.CmdArgs{}) != nil), "del pod succeded")
}
