package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"
	"net/http"

	log "github.com/Sirupsen/logrus"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/orch/simapi"
	n "github.com/pensando/sw/utils/netutils"
)

type restAPIFunc func(r *http.Request) (interface{}, error)

// uplink interface name
var uplinkIf *string

func main() {
	// parse commandline
	uplinkIf = flag.String("uplink", "eth2", "Uplink interface")
	flag.Parse()

	log.Infof("Starting simulator...")
	InitDocker()
	InitNetworking()
	serveHTTP()
	log.Infof("Exiting simulator...")
}

func serveHTTP() {
	r := mux.NewRouter()
	r.HandleFunc("/nwifs/create", n.MakeHTTPHandler(n.RestAPIFunc(createNwIF))).Methods("POST")
	r.HandleFunc("/nwifs/{id}/delete", n.MakeHTTPHandler(n.RestAPIFunc(deleteNwIF))).Methods("POST")
	r.HandleFunc("/nwifs/cleanup", n.MakeHTTPHandler(n.RestAPIFunc(cleanup))).Methods("POST")
	log.Infof("Starting server at :5050")
	http.ListenAndServe(":5050", r)
}

func createNwIF(r *http.Request) (interface{}, error) {
	req := simapi.NwIFSetReq{}
	resp := simapi.NwIFSetResp{}
	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	if err := json.Unmarshal(content, &req); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	name, mac, err := AddNwIF(req.MacAddr, req.IPAddr, req.Vlan)
	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	resp.UUID = name
	resp.MacAddr = mac
	return resp, nil
}

func deleteNwIF(r *http.Request) (interface{}, error) {
	resp := simapi.NwIFDelResp{}
	kvs := mux.Vars(r)
	err := DeleteNwIF(kvs["id"])
	if err != nil {
		resp.ErrorMsg = err.Error()
	}

	return resp, err
}

func cleanup(r *http.Request) (interface{}, error) {
	res := CleanUp()
	resp := simapi.NwIFDelResp{ErrorMsg: res}
	return resp, nil
}
