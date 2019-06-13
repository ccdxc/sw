// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"crypto/x509"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"net/http/pprof"
	"os"
	"os/exec"
	"path"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/gorilla/mux"
	"github.com/pkg/errors"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/httputils"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	device "github.com/pensando/sw/nic/agent/nmd/protos/halproto"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	"github.com/pensando/sw/nic/agent/protos/nmd"
	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/cmd/grpc"
	roprotos "github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/revproxy"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// NewNMDOption is a functional option type that allows dependency injection in NMD constructor
// This is mostly intended for testing, as in the normal code flow dependencies are instantiated
// based on mode
type NewNMDOption func(*NMD)

// WithCMDAPI returns a functional option used to pass a CMD API implementation to NewNMD
func WithCMDAPI(cmd nmdapi.CmdAPI) NewNMDOption {
	return func(nmd *NMD) {
		nmd.cmd = cmd
	}
}

// WithRolloutAPI returns a functional option used to pass a Rollout API implementation to NewNMD
func WithRolloutAPI(ro nmdapi.RolloutCtrlAPI) NewNMDOption {
	return func(nmd *NMD) {
		nmd.rollout = ro
	}
}

// NewNMD returns a new NMD instance
func NewNMD(delphiClient clientAPI.Client,
	dbPath, listenURL, revProxyURL string,
	regInterval, updInterval time.Duration,
	opts ...NewNMDOption) (*NMD, error) {
	var emdb emstore.Emstore
	var err error

	// create a platform agent

	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		return nil, fmt.Errorf("error creating platform agent. Err: %v", err)
	}

	//create a upgrade client
	uc, err := upg.NewNaplesUpgradeClient(delphiClient)
	if err != nil {
		return nil, fmt.Errorf("error creating Upgrade client . Err: %v", err)
	}

	// create reverse proxy for all NAPLES REST APIs
	revProxy, err := revproxy.NewReverseProxyRouter(revProxyURL, revProxyConfig)
	if err != nil {
		return nil, fmt.Errorf("Could not start Reverse Proxy Router. Err: %v", err)
	}

	// open the embedded database
	if dbPath == "" {
		emdb, err = emstore.NewEmstore(emstore.MemStoreType, "")
	} else {
		emdb, err = emstore.NewEmstore(emstore.BoltDBType, dbPath)
	}

	if err != nil {
		log.Errorf("Error opening the embedded db. Err: %v", err)
		return nil, err
	}
	c, _ := types.TimestampProto(time.Now())
	ts := api.Timestamp{
		Timestamp: *c,
	}

	fru := ReadFruFromJSON()

	// TODO : Add more information into SmartNICRollout
	ro := nmd.NaplesRollout{
		TypeMeta: api.TypeMeta{
			Kind: "NaplesRollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: fru.MacStr,
		},
		Status: nmd.RolloutStatus{
			InProgressOp: &roprotos.SmartNICOpSpec{
				Op: roprotos.SmartNICOp_SmartNICNoOp,
			},
		},
	}

	roObj, err := emdb.Read(&ro)

	if roObj != nil && err == nil {
		// Use the persisted config moving forward
		ro = *roObj.(*nmd.NaplesRollout)
		log.Infof("Rollout Object found in NMD DB. Using persisted values. %v", ro)
	} else {
		// persist the default rollout object
		log.Info("Rollout Object not found in NMD DB. Persisting it in the DB.")
		err = emdb.Write(&ro)
		if err != nil {
			log.Errorf("Error persisting the default naples config in EmDB, err: %+v", err)
		}
	}

	// construct default config and a default profile
	defaultProfile := &nmd.NaplesProfile{
		ObjectMeta: api.ObjectMeta{
			Name: "default",
		},
		TypeMeta: api.TypeMeta{
			Kind: "NaplesProfile",
		},
		Spec: nmd.NaplesProfileSpec{
			NumLifs:          1,
			DefaultPortAdmin: nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String(),
		},
	}

	config := nmd.Naples{
		ObjectMeta: api.ObjectMeta{
			Name: "NaplesConfig",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Naples",
		},
		Spec: nmd.NaplesSpec{
			Mode:          nmd.MgmtMode_HOST.String(),
			PrimaryMAC:    fru.MacStr,
			ID:            fru.MacStr,
			NaplesProfile: "default",
			IPConfig: &cluster.IPConfig{
				IPAddress:  "",
				DefaultGW:  "",
				DNSServers: nil,
			},
		},
		Status: nmd.NaplesStatus{
			Fru:          fru,
			TimeZone:     "UTC",
			SmartNicName: fru.MacStr,
		},
	}
	// List available NaplesProfiles

	// check if naples config exists in emdb
	cfgObj, err := emdb.Read(&config)

	if cfgObj != nil && err == nil {
		log.Info("Config object found in NMD DB. Using persisted values.")
		// Use the persisted config moving forward
		config = *cfgObj.(*nmd.Naples)

		// Always re-read the contents of fru.json upon startup
		config.Status.Fru = ReadFruFromJSON()
	} else {
		// persist the default naples config
		log.Info("Config object not found in NMD DB. Persisting it in the DB.")
		config.CreationTime = ts
		config.ModTime = ts
		err = emdb.Write(&config)
		if err != nil {
			log.Errorf("Error persisting the default naples config in EmDB, err: %+v", err)
		}

		BackupNMDDB()
	}

	// create NMD object
	nm := NMD{
		store:              emdb,
		nodeUUID:           fru.MacStr,
		macAddr:            fru.MacStr,
		Platform:           pa,
		Upgmgr:             uc,
		DelphiClient:       delphiClient,
		nic:                nil,
		certsListenURL:     globals.Localhost + ":" + globals.CMDUnauthCertAPIPort,
		nicRegInitInterval: regInterval,
		nicRegInterval:     regInterval,
		isRegOngoing:       false,
		nicUpdInterval:     updInterval,
		isUpdOngoing:       false,
		isRestSrvRunning:   false,
		listenURL:          listenURL,
		stopNICReg:         make(chan bool, 1),
		stopNICUpd:         make(chan bool, 1),
		config:             config,
		completedOps:       make(map[roprotos.SmartNICOpSpec]bool),
		ro:                 ro,
		revProxy:           revProxy,
	}

	err = nm.updateLocatTimeZone()
	if err != nil {
		log.Errorf("Could not set timezone to %v. Err : %v", config.Status.TimeZone, err)
	}

	// check if naples NaplesProfiles exist in emdb
	p := nmd.NaplesProfile{
		TypeMeta: api.TypeMeta{Kind: "NaplesProfile"},
	}
	profileObjs, err := emdb.List(&p)

	if profileObjs != nil && err == nil {
		// Use Persisted profiles moving forward
		for _, p := range profileObjs {
			profile := p.(*nmd.NaplesProfile)
			nm.CreateNaplesProfile(*profile)
		}
	} else {
		nm.CreateNaplesProfile(*defaultProfile)
	}

	for _, o := range opts {
		o(&nm)
	}

	// register NMD with the platform agent
	err = nm.Platform.RegisterNMD(&nm)
	if err != nil {
		// cleanup emstore and return
		emdb.Close()
		return nil, err
	}

	if nm.Upgmgr != nil {
		err = nm.Upgmgr.RegisterNMD(&nm)
		if err != nil {
			log.Fatalf("Error Registering NMD with upgmgr, err: %+v", err)
			// cleanup emstore and return
			emdb.Close()
			return nil, err
		}
	}

	if err := nm.StartNMDRestServer(); err != nil {
		log.Infof("Failed to start NMD RestServer. Err: %v", err)
		return nil, err
	}

	// start reverse proxy for all NAPLES REST APIs
	nm.StartReverseProxy()

	return &nm, nil
}

// RegisterCMD registers a CMD object
func (n *NMD) RegisterCMD(cmd nmdapi.CmdAPI) error {
	n.Lock()
	defer n.Unlock()

	// ensure two controller plugins dont register
	if n.cmd != nil {
		log.Errorf("Attempt to register multiple controllers with NMD.")
	}

	// initialize cmd
	n.cmd = cmd

	return nil
}

// UnRegisterCMD ensures that nmd cleans up its old cmd information.
func (n *NMD) UnRegisterCMD() error {
	n.Lock()
	defer n.Unlock()
	log.Infof("Received UnRegisterCMD message.")
	n.cmd = nil
	return nil
}

// UpdateCMDClient updates the cmd client with the resolver information obtained by DHCP
func (n *NMD) UpdateCMDClient(resolverURLs []string) error {
	var cmdResolverURL []string
	for _, res := range resolverURLs {
		cmdResolverURL = append(cmdResolverURL, fmt.Sprintf("%s:%s", res, globals.CMDGRPCAuthPort))
	}
	// stop resolver client if active
	if n.resolverClient != nil {
		n.resolverClient.UpdateServers(cmdResolverURL)
		log.Infof("Updated cmd client with newer cmd resolver URLs: %v", resolverURLs)
		return nil
	}

	n.resolverClient = resolver.New(&resolver.Config{Name: "NMD", Servers: cmdResolverURL})
	tsdb.Start(n.resolverClient)

	// Use the first resolverURL as registration URL
	cmdRegistrationURL := fmt.Sprintf("%s:%s", resolverURLs[0], globals.CMDSmartNICRegistrationPort)

	newCMDClient, err := cmdif.NewCmdClient(n, cmdRegistrationURL, n.resolverClient)
	if err != nil {
		log.Errorf("Failed to update CMD Client. Err: %v", err)
		return err
	}
	n.cmd = newCMDClient
	log.Infof("Updated cmd client with newer cmd resolver URLs: %v", resolverURLs)
	return nil
}

// GenClusterKeyPair generates a (public key, private key) for this NMD instance
// to authenticate itself to other entities in the Venice cluster.
// When the instance is admitted to a cluster, it receives a corresponding
// certificate signed by CMD
func (n *NMD) GenClusterKeyPair() (*keymgr.KeyPair, error) {
	return n.tlsProvider.CreateClientKeyPair(keymgr.ECDSA384)
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// GetAgentID returns UUID of the NMD
func (n *NMD) GetAgentID() string {
	return n.nodeUUID
}

// GetPrimaryMAC returns primaryMac of NMD
func (n *NMD) GetPrimaryMAC() string {
	return n.config.Status.Fru.MacStr
}

// GetControllerIps returns Controllers IP
func (n *NMD) GetControllerIps() []string {
	return n.config.Spec.Controllers
}

// NaplesConfigHandler is the REST handler for Naples Config POST operation
func (n *NMD) NaplesConfigHandler(r *http.Request) (interface{}, error) {

	req := nmd.Naples{}
	resp := NaplesConfigResp{}
	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	if err = json.Unmarshal(content, &req); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	log.Infof("Naples Config Request: %+v", req)
	err = n.UpdateNaplesConfig(req)

	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Config Response: %+v", resp)

	return resp, nil
}

// NaplesProfileHandler is the REST handler for Naples Profile POST
func (n *NMD) NaplesProfileHandler(r *http.Request) (interface{}, error) {
	req := nmd.NaplesProfile{}
	resp := NaplesConfigResp{}

	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	if err = json.Unmarshal(content, &req); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Profile Request: %+v", req)

	err = n.CreateNaplesProfile(req)

	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Profile Response: %+v", resp)

	return resp, nil
}

// NaplesProfileGetHandler is the REST handler for Naples Profiles GET
func (n *NMD) NaplesProfileGetHandler(r *http.Request) (interface{}, error) {
	profiles := n.profiles
	log.Infof("Naples Profile Get Response: %+v", profiles)
	return profiles, nil
}

// NaplesProfileDeleteHandler deletes a napels pr
func (n *NMD) NaplesProfileDeleteHandler(r *http.Request) (interface{}, error) {
	profileName := mux.Vars(r)["ProfileName"]

	for i, p := range n.profiles {
		if profileName == p.Name {
			copy(n.profiles[i:], n.profiles[i+1:])
			n.profiles[len(n.profiles)-1] = nil
			n.profiles = n.profiles[:len(n.profiles)-1]
			break
		}
	}
	return nil, nil
}

// NaplesRolloutHandler is the REST handler for Naples Config POST operation
func (n *NMD) NaplesRolloutHandler(r *http.Request) (interface{}, error) {
	snicRollout := roprotos.SmartNICRollout{}
	resp := NaplesConfigResp{}
	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	if err = json.Unmarshal(content, &snicRollout); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	log.Infof("Naples Rollout Config Request: %+v", snicRollout)
	err = n.CreateUpdateSmartNICRollout(&snicRollout)

	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Rollout Config Response: %+v", resp)

	return resp, nil
}

// NaplesRolloutGetHandler is the REST handler for Naples Rollout Config GET operation
func (n *NMD) NaplesRolloutGetHandler(r *http.Request) (interface{}, error) {
	st := n.GetSmartNICRolloutStatus()
	log.Debugf("Naples Rollout Get Response: %+v", st)
	return st, nil
}

// NaplesRolloutDeleteHandler is the REST handler for Naples Rollout Config DELETE operation
func (n *NMD) NaplesRolloutDeleteHandler(r *http.Request) (interface{}, error) {
	resp := NaplesConfigResp{}
	ioutil.ReadAll(r.Body)

	log.Infof("Naples Rollout Delete Request")
	err := n.DeleteSmartNICRollout(&roprotos.SmartNICRollout{})

	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Rollout Delete Response: %+v", resp)
	return resp, nil
}

// NaplesFileUploadHandler is the REST handler for Naples File Upload POST operation
func NaplesFileUploadHandler(w http.ResponseWriter, r *http.Request) {
	// parse and validate file and post parameters
	file, fileHeader, err := r.FormFile("uploadFile")
	if err != nil {
		renderError(w, err.Error(), http.StatusBadRequest)
		return
	}
	defer file.Close()
	uploadPath := r.FormValue("uploadPath")
	if uploadPath == "" {
		renderError(w, "Upload Path Not Specified\n", http.StatusBadRequest)
		return
	}

	fileNameSlice := strings.Split(fileHeader.Filename, "/")

	newPath := uploadPath + fileNameSlice[len(fileNameSlice)-1]

	// write file
	newFile, err := os.Create(newPath)
	if err != nil {
		renderError(w, err.Error(), http.StatusInternalServerError)
		return
	}
	defer newFile.Close() // idempotent, okay to call twice
	_, err = io.Copy(newFile, file)
	if err != nil {
		renderError(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Write([]byte("File Copied Successfully\n"))
	if r.MultipartForm != nil {
		r.MultipartForm.RemoveAll()
	}
}

func renderError(w http.ResponseWriter, message string, statusCode int) {
	w.WriteHeader(statusCode)
	w.Write([]byte(message))
}

// NaplesGetHandler is the REST handler for Naples Config GET operation
func (n *NMD) NaplesGetHandler(r *http.Request) (interface{}, error) {

	cfg := &n.config
	log.Infof("Naples Get Response: %+v", cfg)
	return cfg, nil
}

// Stop stops the NMD
func (n *NMD) Stop() error {

	log.Errorf("Stopping NMD")

	n.StopClassicMode(true)

	if n.GetConfigMode() == nmd.MgmtMode_NETWORK.String() {

		// Cleanup Managed mode tasks, if any
		n.StopManagedMode()
	}

	// Close the embedded object store
	err := n.store.Close()

	return err
}

// Catchall http handler
func unknownAction(w http.ResponseWriter, r *http.Request) {
	log.Infof("Unknown REST URL %q", r.URL.Path)
	w.WriteHeader(503)
}

// StartRestServer creates a new HTTP server serving REST api
func (n *NMD) StartRestServer() error {

	// if no URL was specified, just return (used during unit/integ tests)
	if n.listenURL == "" {
		log.Errorf("Empty listenURL for REST server")
		return errors.New("ListenURL cannot be empty")
	}

	// setup the routes
	router := mux.NewRouter()
	t1 := router.Methods("POST").Subrouter()
	t1.HandleFunc(ConfigURL, httputils.MakeHTTPHandler(n.NaplesConfigHandler))
	t1.HandleFunc(RolloutURL, httputils.MakeHTTPHandler(n.NaplesRolloutHandler))
	t1.HandleFunc(UpdateURL, NaplesFileUploadHandler)
	t1.HandleFunc(ProfileURL, httputils.MakeHTTPHandler(n.NaplesProfileHandler))

	t2 := router.Methods("GET").Subrouter()
	t2.HandleFunc(ConfigURL, httputils.MakeHTTPHandler(n.NaplesGetHandler))
	t2.HandleFunc(ProfileURL, httputils.MakeHTTPHandler(n.NaplesProfileGetHandler))
	t2.HandleFunc(NaplesInfoURL, httputils.MakeHTTPHandler(n.NaplesInfoGetHandler))
	t2.HandleFunc(CmdEXECUrl, n.NaplesCmdExecHandler)
	t2.HandleFunc(NaplesVersionURL, httputils.MakeHTTPHandler(NaplesVersionGetHandler))
	t2.HandleFunc(RolloutURL, httputils.MakeHTTPHandler(n.NaplesRolloutGetHandler))

	t2.HandleFunc("/api/{*}", unknownAction)
	t2.HandleFunc("/debug/pprof/", pprof.Index)
	t2.HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	t2.HandleFunc("/debug/pprof/profile", pprof.Profile)
	t2.HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	t2.HandleFunc("/debug/pprof/trace", pprof.Trace)
	t2.HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	t2.HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	t2.HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	t2.HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	t2.HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	t2.HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)

	t3 := router.Methods("DELETE").Subrouter()
	t3.HandleFunc(CoresURL+"{*}", httputils.MakeHTTPHandler(NaplesCoreDeleteHandler))
	t3.HandleFunc(RolloutURL, httputils.MakeHTTPHandler(n.NaplesRolloutDeleteHandler))

	router.PathPrefix(MonitoringURL + "logs/").Handler(http.StripPrefix(MonitoringURL+"logs/", http.FileServer(http.Dir(globals.PenCtlLogDir))))
	router.PathPrefix(MonitoringURL + "obfl/").Handler(http.StripPrefix(MonitoringURL+"obfl/", http.FileServer(http.Dir(globals.ObflLogDir))))
	router.PathPrefix(MonitoringURL + "events/").Handler(http.StripPrefix(MonitoringURL+"events/", http.FileServer(http.Dir(globals.EventsDir))))
	router.PathPrefix(CoresURL).Handler(http.StripPrefix(CoresURL, http.FileServer(http.Dir(globals.CoresDir))))
	router.PathPrefix(UpdateURL).Handler(http.StripPrefix(UpdateURL, http.FileServer(http.Dir(globals.UpdateDir))))
	router.PathPrefix(DataURL).Handler(http.StripPrefix(DataURL, http.FileServer(http.Dir(globals.DataDir))))

	router.HandleFunc("/api/v1/naples/profiles/{ProfileName}", httputils.MakeHTTPHandler(n.NaplesProfileDeleteHandler))

	// create listener
	listener, err := net.Listen("tcp", n.listenURL)
	if err != nil {
		log.Errorf("Error starting listener. Err: %v", err)
		return err
	}

	// Init & create a http server
	n.listener = listener
	n.httpServer = &http.Server{Addr: n.listenURL, Handler: router}
	n.isRestSrvRunning = true

	log.Infof("Started NMD Rest server at %s", n.GetListenURL())
	os.Setenv("PATH", os.Getenv("PATH")+":/platform/bin:/nic/bin:/platform/tools:/nic/tools")

	// Launch the server
	go n.httpServer.Serve(listener)

	return nil
}

// StopRestServer stops the http server
func (n *NMD) StopRestServer(shutdown bool) error {
	n.Lock()
	defer n.Unlock()

	// TODO: This code has to go once we have Delphi integrated and we should use Delphi instead
	var postData interface{}
	var resp interface{}
	netutils.HTTPPost("http://localhost:8888/revproxy/stop", &postData, &resp)

	if shutdown && n.httpServer != nil {
		err := n.httpServer.Close()
		if err != nil {
			log.Errorf("Failed to stop NMD Rest server at %s", n.listenURL)
			return err
		}
		n.httpServer = nil
		log.Infof("Stopped REST server at %s", n.listenURL)

	}
	n.isRestSrvRunning = false

	return nil
}

// GetNMDUrl returns the REST URL
func (n *NMD) GetNMDUrl() string {
	return "http://" + n.GetListenURL() + ConfigURL
}

// GetNMDRolloutURL returns the REST URL
func (n *NMD) GetNMDRolloutURL() string {
	return "http://" + n.GetListenURL() + RolloutURL
}

// GetNMDCmdExecURL returns the REST URL
func (n *NMD) GetNMDCmdExecURL() string {
	return "http://" + n.GetListenURL() + CmdEXECUrl
}

// GetGetNMDUploadURL returns the REST URL
func (n *NMD) GetGetNMDUploadURL() string {
	return "http://" + n.GetListenURL() + UpdateURL
}

// GetReverseProxyListenURL returns the URL of the reverse proxy
func (n *NMD) GetReverseProxyListenURL() string {
	return n.revProxy.GetListenURL()
}

func (n *NMD) initTLSProvider() error {
	// Instantiate a KeyMgr to store the cluster certificate and a TLS provider
	// to use it to connect to other cluster components.
	// Keys are not persisted. They will be refreshed next time we access the cluster
	// or when they expire (TBD).
	tlsProvider, err := tlsproviders.NewDefaultKeyMgrBasedProvider("")
	if err != nil {
		return errors.Wrapf(err, "Error instantiating tls provider")
	}
	n.tlsProvider = tlsProvider
	return nil
}

func (n *NMD) setClusterCredentials(resp *grpc.NICAdmissionResponse) error {
	certMsg := resp.GetClusterCert()
	if certMsg == nil {
		return fmt.Errorf("No certificate found in registration response message")
	}
	cert, err := x509.ParseCertificate(certMsg.GetCertificate().Certificate)
	if err != nil {
		return fmt.Errorf("Error parsing cluster certificate: %v", err)
	}
	err = n.tlsProvider.SetClientCertificate(cert)
	if err != nil {
		return fmt.Errorf("Error storing cluster certificate: %v", err)
	}
	var caTrustChain []*x509.Certificate
	if resp.GetCaTrustChain() != nil {
		for i, c := range resp.GetCaTrustChain().GetCertificates() {
			cert, err := x509.ParseCertificate(c.Certificate)
			if err != nil {
				log.Errorf("Error parsing CA trust chain certificate index %d: %v", i, err)
				// continue anyway
			}
			caTrustChain = append(caTrustChain, cert)
		}
	}
	n.tlsProvider.SetCaTrustChain(caTrustChain)

	var trustRoots []*x509.Certificate
	if resp.GetTrustRoots() != nil {
		for i, c := range resp.GetTrustRoots().GetCertificates() {
			cert, err := x509.ParseCertificate(c.Certificate)
			if err != nil {
				log.Errorf("Error parsing trust roots certificate index %d: %v", i, err)
				// continue anyway
			}
			trustRoots = append(trustRoots, cert)
		}
	}
	n.tlsProvider.SetTrustRoots(trustRoots)

	// Persist trust roots so that we remember what is the last Venice cluster we connected to
	// and we can authenticate offline credentials signed by Venice CA.
	err = certs.SaveCertificates(globals.NaplesTrustRootsFile, trustRoots)
	if err != nil {
		return fmt.Errorf("Error storing cluster trust roots in %s: %v", globals.NaplesTrustRootsFile, err)
	}

	return nil
}

// RegisterROCtrlClient registers client of RolloutController to NMD
func (n *NMD) RegisterROCtrlClient(rollout nmdapi.RolloutCtrlAPI) error {

	n.Lock()
	defer n.Unlock()

	// ensure two clients dont register
	if n.rollout != nil {
		log.Errorf("Attempt to register multiple rollout clients with NMD.")
		return errors.New("attempt to register multiple rollout clients with NMD")
	}

	// initialize rollout
	n.rollout = rollout

	return nil
}

// NaplesCoreDeleteHandler is the REST handler for Naples delete core file operation
func NaplesCoreDeleteHandler(r *http.Request) (interface{}, error) {
	resp := NaplesConfigResp{}
	file := globals.CoresDir + "/" + strings.TrimPrefix(r.URL.Path, CoresURL)
	err := os.Remove(file)
	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	return resp, nil
}

func appendAuthorizedKeyFile(sshPubKeyFile string) error {
	file, err := os.OpenFile("/root/.ssh/authorized_keys", os.O_WRONLY|os.O_APPEND, os.ModeAppend)
	if err != nil {
		log.Errorf("failed opening file: %s", err)
		return err
	}
	defer file.Close()

	data, err := ioutil.ReadFile(sshPubKeyFile)
	if err != nil {
		log.Errorf("failed reading data from file: %s", err)
		return err
	}

	_, err = file.WriteString(string(data))
	if err != nil {
		log.Errorf("failed writing to file: %s", err)
		return err
	}
	return nil
}

func executeCmd(req *nmd.NaplesCmdExecute, parts []string) (string, error) {
	cmd := exec.Command(req.Executable, parts...)
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, req.Env)

	log.Infof("Naples Cmd Execute Request: %+v env: [%s]", req, os.Environ())
	stdoutStderr, err := cmd.CombinedOutput()
	if err != nil {
		return string(fmt.Sprintf(err.Error()) + ":" + string(stdoutStderr)), err
	}
	return string(stdoutStderr), nil
}

func naplesExecCmd(req *nmd.NaplesCmdExecute) (string, error) {
	parts := strings.Fields(req.Opts)
	if req.Executable == "/bin/date" && req.Opts != "" {
		parts = strings.SplitN(req.Opts, " ", 2)
	} else if req.Executable == "pensettimezone" {
		err := ioutil.WriteFile("/etc/timezone", []byte(req.Opts), 0644)
		if err != nil {
			return err.Error(), err
		}
		return "", nil
	} else if req.Executable == "setsshauthkey" {
		err := appendAuthorizedKeyFile(req.Opts)
		if err != nil {
			return err.Error(), err
		}
		return "", nil
	}
	return executeCmd(req, parts)
}

func naplesPkgVerify(pkgName string) (string, error) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + pkgName, " -v"}, ""),
	}
	return naplesExecCmd(v)
}

func naplesPkgInstall(pkgName string) (string, error) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + pkgName, " -i all"}, ""),
	}
	return naplesExecCmd(v)
}

func naplesSetBootImg() (string, error) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-s ", "altfw"}, ""),
	}
	return naplesExecCmd(v)
}

func naplesDelBootImg(pkgName string) (string, error) {
	v := &nmd.NaplesCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/update/" + pkgName}, ""),
	}
	return naplesExecCmd(v)
}

func naplesHostDisruptiveUpgrade(pkgName string) (string, error) {
	if resp, err := naplesPkgInstall(pkgName); err != nil {
		return resp, err
	}
	if resp, err := naplesSetBootImg(); err != nil {
		return resp, err
	}
	if resp, err := naplesDelBootImg(pkgName); err != nil {
		return resp, err
	}
	return "", nil
}

//NaplesCmdExecHandler is the REST handler to execute any binary on naples and return the output
func (n *NMD) NaplesCmdExecHandler(w http.ResponseWriter, r *http.Request) {
	req := nmd.NaplesCmdExecute{}
	resp := NaplesConfigResp{}
	defer r.Body.Close()

	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		fmt.Fprintln(w, resp)
		return
	}

	if err = json.Unmarshal(content, &req); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		fmt.Fprintln(w, resp)
		return
	}

	stdErrOut, err := naplesExecCmd(&req)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
	}

	// TODO : Remove this once we get rid of all the Exec commands
	if req.Executable == "pensettimezone" {
		timeZone := req.Opts[:len(req.Opts)-1]
		n.SetTimeZone(timeZone)

		err := n.updateLocatTimeZone()
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
		} else {
			n.store.Write(&n.config)
		}
	}

	w.Write([]byte(stdErrOut))
}

// NaplesInfoGetHandler is the REST handler for Naples Profiles GET
func (n *NMD) NaplesInfoGetHandler(r *http.Request) (interface{}, error) {
	n.UpdateNaplesInfoFromConfig()
	info := n.nic
	return info, nil
}

func (n *NMD) updateLocatTimeZone() error {
	timeZone := n.GetTimeZone()

	v := &nmd.NaplesCmdExecute{
		Executable: "pensettimezone",
		Opts:       strings.Join([]string{timeZone}, ""),
	}
	_, err := naplesExecCmd(v)

	if err != nil {
		log.Error("Failed to write in /etc/timezon file")
		return err
	}

	symLink := "/usr/share/zoneinfo/" + timeZone

	v = &nmd.NaplesCmdExecute{
		Executable: "ln",
		Opts:       strings.Join([]string{"-sf", symLink, "/etc/localtime"}, " "),
	}
	_, err = naplesExecCmd(v)
	if err != nil {
		log.Errorf("Failed to create the symlink to :%v", symLink)
		return err
	}

	return nil
}

// NaplesVersionGetHandler is the REST handler for Naples Profiles GET
func NaplesVersionGetHandler(r *http.Request) (interface{}, error) {
	return GetNaplesSoftwareInfo()
}

// GetRegStatus returns the current status of NIC registration task
func (n *NMD) GetRegStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRegOngoing
}

func (n *NMD) setRegStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRegOngoing = value
}

// GetUpdStatus returns the current running status of NIC update task
func (n *NMD) GetUpdStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isUpdOngoing
}

func (n *NMD) setUpdStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isUpdOngoing = value
}

// GetRestServerStatus returns the current running status of REST server
func (n *NMD) GetRestServerStatus() bool {
	n.Lock()
	defer n.Unlock()
	return n.isRestSrvRunning
}

func (n *NMD) setRestServerStatus(value bool) {
	n.Lock()
	defer n.Unlock()
	n.isRestSrvRunning = value
}

// GetConfigMode returns the configured Naples Mode
func (n *NMD) GetConfigMode() string {
	n.Lock()
	defer n.Unlock()
	return n.config.Spec.Mode
}

// SetNaplesConfig sets naples config and status
func (n *NMD) SetNaplesConfig(cfgSpec nmd.NaplesSpec) {
	n.Lock()
	defer n.Unlock()
	c, _ := types.TimestampProto(time.Now())

	ts := api.Timestamp{
		Timestamp: *c,
	}
	n.config.ModTime = ts
	n.config.Spec = cfgSpec
}

// GetNaplesConfig returns the current naples config received via REST
func (n *NMD) GetNaplesConfig() nmd.Naples {
	n.Lock()
	defer n.Unlock()

	return n.config
}

// SetSmartNIC intializes the smartNIC object
func (n *NMD) SetSmartNIC(nic *cluster.SmartNIC) error {
	n.Lock()
	defer n.Unlock()

	n.nic = nic
	return nil
}

// GetSmartNIC returns the smartNIC object
func (n *NMD) GetSmartNIC() (*cluster.SmartNIC, error) {
	n.Lock()
	defer n.Unlock()

	return n.nic, nil
}

// GetListenURL returns the listen URL of the http server
func (n *NMD) GetListenURL() string {
	n.Lock()
	defer n.Unlock()

	if n.listener != nil {
		return n.listener.Addr().String()
	}
	return ""
}

// CreateIPClient creates IPClient to run DHCP
func (n *NMD) CreateIPClient(delphiClient clientAPI.Client) {
	n.Lock()
	defer n.Unlock()
	var ipClient *ipif.IPClient
	var err error

	if n.config.Spec.Mode == nmd.MgmtMode_HOST.String() {
		n.IPClient = nil
		return
	}

	if n.config.Spec.NetworkMode == nmd.NetworkMode_INBAND.String() {
		ipClient, err = ipif.NewIPClient(n, ipif.NaplesInbandInterface)
		if err != nil {
			log.Errorf("Failed to instantiate ipclient on inband interface. Err: %v", err)
		}
	} else {
		ipClient, err = ipif.NewIPClient(n, ipif.NaplesOOBInterface)
		if err != nil {
			log.Errorf("Failed to instantiate ipclient on oob interface. Err: %v", err)
		}
	}
	n.IPClient = ipClient
}

// CreateMockIPClient creates IPClient in Mock mode to run in venice integ environment
func (n *NMD) CreateMockIPClient(delphiClient clientAPI.Client) {
	n.Lock()
	defer n.Unlock()
	ipClient, err := ipif.NewIPClient(n, ipif.NaplesMockInterface)
	if err != nil {
		log.Errorf("Failed to instantiate ipclient on mock interface. Err: %v", err)
	}

	n.IPClient = ipClient
}

// GetIPClient returns the handle to the ip client
func (n *NMD) GetIPClient() *ipif.IPClient {
	n.Lock()
	defer n.Unlock()

	return n.IPClient
}

//PersistHALConfiguration updates feature profile
func (n *NMD) writeDeviceFiles() (err error) {
	mgmtIfMAC := parseMgmtIfMAC(n.config.Spec.NetworkMode)
	if n.config.Spec.Mode == nmd.MgmtMode_HOST.String() {
		fwdMode := device.ForwardingMode_FORWARDING_MODE_CLASSIC.String()
		var featureProfile device.FeatureProfile
		var profile *nmd.NaplesProfile
		var defaultPortAdmin string
		var ok bool
		log.Info("Updating feature profile.")
		// Check if the profile exists.
		for _, p := range n.profiles {
			if p.Name == n.config.Spec.NaplesProfile {
				profile = p
				ok = true
				break
			}
		}

		if !ok {
			log.Errorf("could not find profile %v in nmd state, profs: %+v", n.config.Spec.NaplesProfile, n.profiles)
			err = fmt.Errorf("could not find profile %v in nmd state", n.config.Spec.NaplesProfile)
			return
		}

		// Interpret 16 numLifs as scale profile. TODO Remove this when nicmgr can directly read numLifs from device.conf
		if profile.Spec.NumLifs == 16 {
			featureProfile = device.FeatureProfile_FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE
		} else {
			featureProfile = device.FeatureProfile_FEATURE_PROFILE_CLASSIC_DEFAULT
		}

		if profile.Spec.DefaultPortAdmin == nmd.PortAdminState_PORT_ADMIN_STATE_DISABLE.String() {
			defaultPortAdmin = device.PortAdminState_PORT_ADMIN_STATE_DISABLE.String()
		} else {
			defaultPortAdmin = device.PortAdminState_PORT_ADMIN_STATE_ENABLE.String()
		}

		err = n.PersistDeviceSpec(fwdMode, featureProfile, defaultPortAdmin, mgmtIfMAC)
		return
	}

	err = n.PersistDeviceSpec(device.ForwardingMode_FORWARDING_MODE_HOSTPIN.String(), device.FeatureProfile_FEATURE_PROFILE_NONE, device.PortAdminState_PORT_ADMIN_STATE_ENABLE.String(), mgmtIfMAC)
	return
}

//// UpdateMgmtIP updates the management IP
//func (n *NMD) UpdateMgmtIP() error {
//	log.Info("Update Mgmt IP Called.")
//	//n.Lock()
//	//defer n.Unlock()
//
//
//	return nil
//	//log.Infof("NaplesConfig: %v", n.config)
//	//return n.IPClient.Update()
//}

// GetCMDSmartNICWatcherStatus returns true if the NMD CMD interface has an active watch
func (n *NMD) GetCMDSmartNICWatcherStatus() bool {
	return n.cmd != nil && n.cmd.IsSmartNICWatcherRunning()
}

// GetRoSmartNICWatcherStatus returns true if the NMD rollout interface has an active watch
func (n *NMD) GetRoSmartNICWatcherStatus() bool {
	return n.rollout != nil && n.rollout.IsSmartNICWatcherRunning()
}

// UpdateCurrentManagementMode fixes the currently active Management mode
func (n *NMD) UpdateCurrentManagementMode() {
	appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	if appStartSpec, err := ioutil.ReadFile(appStartConfFilePath); err != nil {
		n.config.Status.Mode = nmd.MgmtMode_HOST.String()
	} else {
		if string(appStartSpec) == "classic" {
			n.config.Status.Mode = nmd.MgmtMode_HOST.String()
		} else {
			n.config.Status.Mode = nmd.MgmtMode_NETWORK.String()
		}
	}
}

// PersistDeviceSpec accepts forwarding mode and feature profile and persists this in device.conf
func (n *NMD) PersistDeviceSpec(fwdMode string, featureProfile device.FeatureProfile, defaultPortAdmin string, mgmtIfMAC uint64) (err error) {
	log.Infof("Setting forwarding mode to : %v", fwdMode)
	log.Infof("Setting default port admin to : %v", defaultPortAdmin)
	deviceSpec := device.SystemSpec{
		FwdMode:        fwdMode,
		FeatureProfile: featureProfile,
		PortAdminState: defaultPortAdmin,
		MgmtIfMac:      mgmtIfMAC,
	}

	// Create the /sysconfig/config0 if it doesn't exist. Needed for non naples nmd test environments
	if _, err := os.Stat(globals.NaplesModeConfigFile); os.IsNotExist(err) {
		os.MkdirAll(path.Dir(globals.NaplesModeConfigFile), 0664)
	}

	data, err := json.MarshalIndent(deviceSpec, "", "  ")
	if err != nil {
		log.Errorf("Failed to marshal device spec. Err: %v", err)
		return err
	}
	log.Infof("Marshalled JSON is : %v ", string(data))

	if err = ioutil.WriteFile(globals.NaplesModeConfigFile, data, 0664); err != nil {
		log.Errorf("Failed to write feature profile to %s. Err: %v", globals.NaplesModeConfigFile, err)
	}

	// Update app-start.conf file. TODO Remove this workaround when all the processes are migrated to read from device.conf
	var appStartSpec []byte
	switch fwdMode {
	case device.ForwardingMode_FORWARDING_MODE_HOSTPIN.String():
		appStartSpec = []byte("hostpin")
	default:
		appStartSpec = []byte("classic")
	}

	log.Infof("App Start Spec is : %v", appStartSpec)
	appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	if err := ioutil.WriteFile(appStartConfFilePath, appStartSpec, 0755); err != nil {
		log.Errorf("Failed to write app start conf. Err: %v", err)
	}

	err = BackupDeviceConfig()
	if err != nil {
		log.Errorf("Failed to backup device config")
	}
	return
}

// BackupDeviceConfig backs up the device.conf from config0 to config1
func BackupDeviceConfig() error {
	return copyFiles(globals.NaplesModeConfigFile, globals.NaplesModeBackupConfigFile)
}

// BackupNMDDB backs up nmd.db from config0 to config1
func BackupNMDDB() error {
	return copyFiles(globals.NmdDBPath, globals.NmdBackupDBPath)
}

func copyFiles(src, dst string) error {
	_, err := os.Stat(src)
	if err != nil {
		return err
	}

	input, err := ioutil.ReadFile(src)
	if err != nil {
		log.Errorf("Failed to read from %v. Err : %v", src, err)
		return err
	}

	err = ioutil.WriteFile(dst, input, 0644)
	if err != nil {
		log.Errorf("Failed to write to %v. Err : %v", dst, err)
		return err
	}

	return nil
}

// SetTimeZone sets the timezone of Naples
func (n *NMD) SetTimeZone(timeZone string) {
	n.Lock()
	defer n.Unlock()

	n.config.Status.TimeZone = timeZone
}

// GetTimeZone gets the timezone of Naples
func (n *NMD) GetTimeZone() string {
	n.Lock()
	defer n.Unlock()

	return n.config.Status.TimeZone
}

// GetIPConfig gets the current IPConfig
func (n *NMD) GetIPConfig() *cluster.IPConfig {
	return n.config.Spec.IPConfig
}

// SetIPConfig sets the IPConfig
func (n *NMD) SetIPConfig(cfg *cluster.IPConfig) {
	n.config.Status.IPConfig = cfg
}

// GetVeniceIPs returns the venice co-ordinates
func (n *NMD) GetVeniceIPs() []string {
	return n.config.Status.Controllers
}

// SetVeniceIPs sets the venice co-ordinates
func (n *NMD) SetVeniceIPs(veniceIPs []string) {
	n.config.Status.Controllers = veniceIPs
}

func runCmd(cmdStr string) error {
	log.Infof("Running : " + cmdStr)
	cmd := exec.Command("bash", "-c", cmdStr)
	_, err := cmd.Output()

	if err != nil {
		log.Errorf("Failed Running : " + cmdStr)
	}

	return err
}

func parseMgmtIfMAC(mgmtNetwork string) (mgmtIfMAC uint64) {
	switch mgmtNetwork {
	case nmd.NetworkMode_INBAND.String():
		mgmtLink, err := netlink.LinkByName("bond0")
		if err != nil {
			log.Errorf("Could not find system mac on interface bond0. Err: %v", err)
			return
		}
		mgmtIfMAC = macToUint64(mgmtLink.Attrs().HardwareAddr)
		return
	case nmd.NetworkMode_OOB.String():
		mgmtLink, err := netlink.LinkByName("oob_mnic0")
		if err != nil {
			log.Errorf("Could not find system mac on interface oob_mnic0. Err: %v", err)
			return
		}
		mgmtIfMAC = macToUint64(mgmtLink.Attrs().HardwareAddr)
		return
	default:
		return
	}
}

func macToUint64(macAddr net.HardwareAddr) (mac uint64) {
	b := make([]byte, 8)
	// oui-48 format
	if len(macAddr) == 6 {
		// fill 0 lsb
		copy(b[2:], macAddr)
	}
	mac = binary.BigEndian.Uint64(b)
	return
}
