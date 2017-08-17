package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"strings"

	log "github.com/Sirupsen/logrus"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/orch/simapi"
	"github.com/pensando/sw/orch/vchub/sim"
	n "github.com/pensando/sw/utils/netutils"
)

const (
	vmsPerHost    = 2
	vcSimDefPort  = "18086"
	vcSimSoapPort = "8989"
)

var vcSim simapi.OrchSim
var hostSims map[string]string
var nwIFs map[string]string

type cliOpts struct {
	listenURL string
	snicList  string
	hostUrls  string
	soapPort  string
}

func main() {
	var opts cliOpts

	flagSet := flag.NewFlagSet("vcsim", flag.ExitOnError)
	flagSet.StringVar(&opts.listenURL,
		"listen-url",
		":"+vcSimDefPort,
		"IP:Port vcsim should listen for OrchSim requests")
	flagSet.StringVar(&opts.snicList,
		"snic-list",
		"",
		"Comma separated list of snics that should be part of inventory")
	flagSet.StringVar(&opts.hostUrls,
		"hostsim-urls",
		"",
		"Comma separated list of hostsim urls, *in the same order* as the snic macs")
	flagSet.StringVar(&opts.soapPort,
		"soap-port",
		vcSimSoapPort,
		"Port where sim should listen for SOAP requests")

	flagSet.Parse(os.Args[1:])

	if opts.snicList == "" {
		log.Errorf("snic-list is required")
		os.Exit(1)
	}
	if opts.hostUrls == "" {
		log.Errorf("hostsim-urls is required")
		os.Exit(1)
	}

	snics := strings.Split(opts.snicList, ",")
	hosts := strings.Split(opts.hostUrls, ",")
	if len(snics) != len(hosts) {
		log.Errorf("Expected %d hosts, got %d", len(snics), len(hosts))
		os.Exit(1)
	}

	for _, m := range snics {
		_, err := net.ParseMAC(m)
		if err != nil {
			log.Errorf("Mac %s is not valid", m)
			os.Exit(1)
		}
	}

	// start a vc simulator
	vcSim = sim.New()
	vcURL, err := vcSim.Run("127.0.0.1:"+opts.soapPort, snics, vmsPerHost)
	if err != nil {
		log.Fatalf("vcSim.Run returned %v", err)
	}

	log.Infof("== SOAP url is %s ==", vcURL)

	hostSims = make(map[string]string)
	for ix, s := range snics {
		err = vcSim.SetHostURL(s, hosts[ix])
		if err != nil {
			log.Errorf("vcSim.SetHostURL returned %v", err)
			os.Exit(1)
		}

		hostSims[s] = hosts[ix]
	}

	nwIFs = make(map[string]string)
	// start an http server
	serveHTTP(opts.listenURL)
}

func serveHTTP(url string) {
	r := mux.NewRouter()
	r.HandleFunc("/nwifs/create", n.MakeHTTPHandler(n.RestAPIFunc(createNwIF))).Methods("POST")
	r.HandleFunc("/nwifs/{id}/delete", n.MakeHTTPHandler(n.RestAPIFunc(deleteNwIF))).Methods("POST")
	log.Infof("Starting server at %s", url)
	http.ListenAndServe(url, r)
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

	u1, ok := hostSims[req.SmartNIC]
	if !ok {
		resp.ErrorMsg = "req.SmartNIC is unrecognized"
		return resp, fmt.Errorf("snic not recognized")
	}

	rr, err := vcSim.CreateNwIF(u1, &req)
	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	nwIFs[rr.UUID] = u1

	return rr, nil
}

func deleteNwIF(r *http.Request) (interface{}, error) {
	resp := simapi.NwIFDelResp{}
	kvs := mux.Vars(r)
	nwif := kvs["id"]
	u1, ok := nwIFs[nwif]
	if !ok {
		resp.ErrorMsg = "nwif-id is unrecognized"
		return resp, fmt.Errorf("nwif-id not recognized")
	}
	del := vcSim.DeleteNwIF(u1, nwif)
	if del == nil {
		resp.ErrorMsg = "nwif-id is unrecognized"
		return resp, fmt.Errorf("nwif-id not recognized")
	}

	return del, nil
}
