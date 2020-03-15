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
	"path/filepath"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/gorilla/mux"
	"github.com/pkg/errors"
	"github.com/vishvananda/netlink"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"

	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/httputils"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/platform"
	device "github.com/pensando/sw/nic/agent/nmd/protos/halproto"
	"github.com/pensando/sw/nic/agent/nmd/state/ipif"
	"github.com/pensando/sw/nic/agent/nmd/upg"
	"github.com/pensando/sw/nic/agent/nmd/utils"
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
func NewNMD(pipeline Pipeline,
	dbPath, listenURL, revProxyURL string,
	regInterval, updInterval time.Duration,
	opts ...NewNMDOption) (*NMD, error) {
	var emdb emstore.Emstore
	var err error

	// Create /sysconfig/config0 and /sysconfig/config1 directory if it doesn't exist
	err = os.MkdirAll(globals.NaplesConfig, 0777)
	if err != nil {
		log.Errorf("Failed to create %v. Err: %v", globals.NaplesConfig, err)
	}

	err = os.MkdirAll(globals.NaplesConfigBackup, 0777)
	if err != nil {
		log.Errorf("Failed to create %v. Err: %v", globals.NaplesConfigBackup, err)
	}

	// create a platform agent

	pa, err := platform.NewNaplesPlatformAgent()
	if err != nil {
		return nil, fmt.Errorf("error creating platform agent. Err: %v", err)
	}

	var uc *upg.NaplesUpgClient
	uc = nil
	if pipeline != nil {
		//create a upgrade client
		delClient := pipeline.GetDelphiClient()
		uc, err = upg.NewNaplesUpgradeClient(delClient)
		if err != nil {
			return nil, fmt.Errorf("error creating Upgrade client . Err: %v", err)
		}
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

	// TODO : Add more information into DSCRollout
	ro := nmd.NaplesRollout{
		TypeMeta: api.TypeMeta{
			Kind: "NaplesRollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: fru.MacStr,
		},
		InProgressOp: roprotos.DSCOpSpec{
			Op: roprotos.DSCOp_DSCNoOp,
		},
	}
	tempRO := nmd.NaplesRollout{
		TypeMeta: api.TypeMeta{
			Kind: "NaplesRollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: fru.MacStr,
		},
	}

	roObj, err := emdb.Read(&tempRO) // this function also modifies the parameter object. hence passing temp object

	if roObj != nil && err == nil { // Rollout object found in DB

		// Even if find the rollout object in DB, if upgrade manager does not think rollout is in progress
		//	we will never get a response. In such a case clear our internal state
		if _, err := os.Stat("/update/upgrade_halt_state_machine"); os.IsNotExist(err) {
			log.Infof("Found Rollout object out of sync with upgrade manager.")
		}

		// Use the persisted config moving forward
		ro = *roObj.(*nmd.NaplesRollout)
		log.Infof("Using persisted Rollout values. %v", ro)
		log.Infof("Persisting rollout object to nmd.db. ro : %v PendingOps : %v OpStatus : %v InProgressOps : %v", ro, ro.PendingOps, ro.OpStatus, ro.InProgressOp)
	} else {
		// persist the default rollout object
		log.Info("Persisting Default Rollout Object.")
		err = emdb.Write(&ro)
		if err != nil {
			log.Errorf("Error persisting the default naples config in EmDB, err: %+v", err)
		}
	}

	var config nmd.DistributedServiceCard
	// For apulu pipeline, bring up naples in network mode
	if pipeline != nil && pipeline.GetPipelineType() == globals.NaplesPipelineApollo {
		config = nmd.DistributedServiceCard{
			ObjectMeta: api.ObjectMeta{
				Name:   "DistributedServiceCardConfig",
				Tenant: "default",
			},
			TypeMeta: api.TypeMeta{
				Kind: "DistributedServiceCard",
			},
			Spec: nmd.DistributedServiceCardSpec{
				Mode:        nmd.MgmtMode_NETWORK.String(),
				NetworkMode: nmd.NetworkMode_OOB.String(),
				PrimaryMAC:  fru.MacStr,
				ID:          fru.MacStr,
				DSCProfile:  nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String(),
				IPConfig: &cluster.IPConfig{
					IPAddress:  "",
					DefaultGW:  "",
					DNSServers: nil,
				},
			},
			Status: nmd.DistributedServiceCardStatus{
				Fru:      fru,
				TimeZone: "UTC",
				DSCName:  fru.MacStr,
			},
		}
	} else {
		config = nmd.DistributedServiceCard{
			ObjectMeta: api.ObjectMeta{
				Name:   "DistributedServiceCardConfig",
				Tenant: "default",
			},
			TypeMeta: api.TypeMeta{
				Kind: "DistributedServiceCard",
			},
			Spec: nmd.DistributedServiceCardSpec{
				Mode:        nmd.MgmtMode_NETWORK.String(),
				NetworkMode: nmd.NetworkMode_INBAND.String(),
				PrimaryMAC:  fru.MacStr,
				ID:          fru.MacStr,
				DSCProfile:  nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String(),
				IPConfig: &cluster.IPConfig{
					IPAddress:  "",
					DefaultGW:  "",
					DNSServers: nil,
				},
			},
			Status: nmd.DistributedServiceCardStatus{
				Fru:      fru,
				TimeZone: "UTC",
				DSCName:  fru.MacStr,
			},
		}
	}
	// List available NaplesProfiles

	// check if naples config exists in emdb
	cfgObj, err := emdb.Read(&config)

	if cfgObj != nil && err == nil {
		log.Info("Config object found in NMD DB. Using persisted values.")
		// Use the persisted config moving forward
		config = *cfgObj.(*nmd.DistributedServiceCard)

		// For DSC being upgraded from an older version, the DSCProfile name will be "default"
		// we must update it to use "BASE"instead of "default"
		if config.Spec.DSCProfile == "default" {
			config.Spec.DSCProfile = nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String()
			log.Infof("Updating Naples config object in the DB. %v", config)
			err := emdb.Write(&config)
			if err != nil {
				log.Errorf("Failed to write Naples config into Naples DB. Err : %v", err)
			}
		}

		// Override the default mgmt mode as network in case of older version.
		if pipeline != nil && pipeline.GetPipelineType() == globals.NaplesPipelineApollo {
			config.Spec.NetworkMode = nmd.NetworkMode_OOB.String()
		} else {
			config.Spec.NetworkMode = nmd.NetworkMode_INBAND.String()
		}
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

		err = utils.BackupNMDDB()
		if err != nil {
			log.Errorf("Failed to backup nmd.db. Err : %v", err)
		}
	}

	var delClient clientAPI.Client
	if pipeline != nil {
		delClient = pipeline.GetDelphiClient()
	}

	// create NMD object
	nm := NMD{
		store:               emdb,
		nodeUUID:            fru.MacStr,
		macAddr:             fru.MacStr,
		Platform:            pa,
		Upgmgr:              uc,
		DelphiClient:        delClient,
		nic:                 nil,
		certsListenURL:      globals.Localhost + ":" + globals.CMDUnauthCertAPIPort,
		nicRegInitInterval:  regInterval,
		nicRegInterval:      regInterval,
		isRegOngoing:        false,
		nicUpdInterval:      updInterval,
		isUpdOngoing:        false,
		isRestSrvRunning:    false,
		listenURL:           listenURL,
		stopNICReg:          make(chan bool, 1),
		stopNICUpd:          make(chan bool, 1),
		config:              config,
		ro:                  ro,
		revProxy:            revProxy,
		metrics:             nil,
		RunningFirmware:     GetRunningFirmware(),
		RunningFirmwareName: GetRunningFirmwareName(),
	}

	nm.RunningFirmwareVersion = GetRunningFirmwareVersion(nm.RunningFirmwareName, nm.RunningFirmware)

	err = nm.updateLocalTimeZone()
	if err != nil {
		log.Errorf("Could not set timezone to %v. Err : %v", config.Status.TimeZone, err)
	}

	p := nmd.DSCProfile{
		TypeMeta: api.TypeMeta{Kind: "DSCProfile"},
	}
	profileObjs, err := emdb.List(&p)
	// Create Default profiles
	nm.createDefaultProfiles()
	log.Infof("Got profile objs : %v", profileObjs)

	// Overwrite the default profiles with profiles reconciled from the database
	if profileObjs != nil && err == nil {
		for _, p := range profileObjs {
			profile := p.(*nmd.DSCProfile)
			nm.UpdateNaplesProfile(*reconcileDSCProfile(profile))
		}
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

	if nm.Upgmgr != nil && uc != nil {
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

	// init whitelist for exec endpoint
	initAllowedCommands()

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

	req := nmd.DistributedServiceCard{}
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

// NaplesProfileGetHandler is the REST handler for Naples Profiles GET
func (n *NMD) NaplesProfileGetHandler(r *http.Request) (interface{}, error) {
	if n.metrics != nil && n.metrics.GetCalls != nil {
		n.metrics.GetCalls.Inc()
	}

	profiles := n.profiles
	log.Infof("Naples Profile Get Response: %+v", profiles)
	return profiles, nil
}

// NaplesRolloutHandler is the REST handler for Naples Config POST operation
func (n *NMD) NaplesRolloutHandler(r *http.Request) (interface{}, error) {
	snicRollout := roprotos.DSCRollout{}
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
	err = n.CreateUpdateDSCRollout(&snicRollout)

	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Rollout Config Response: %+v", resp)

	return resp, nil
}

// NaplesRolloutGetHandler is the REST handler for Naples Rollout Config GET operation
func (n *NMD) NaplesRolloutGetHandler(r *http.Request) (interface{}, error) {
	if n.metrics != nil && n.metrics.GetCalls != nil {
		n.metrics.GetCalls.Inc()
	}

	st := n.GetDSCRolloutStatus()
	log.Debugf("Naples Rollout Get Response: %+v", st)
	return st, nil
}

// NaplesRolloutDeleteHandler is the REST handler for Naples Rollout Config DELETE operation
func (n *NMD) NaplesRolloutDeleteHandler(r *http.Request) (interface{}, error) {
	resp := NaplesConfigResp{}
	ioutil.ReadAll(r.Body)

	log.Infof("Naples Rollout Delete Request")
	err := n.DeleteDSCRollout()

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
	if n.metrics != nil && n.metrics.GetCalls != nil {
		n.metrics.GetCalls.Inc()
	}

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

	t2 := router.Methods("GET").Subrouter()
	t2.HandleFunc(ConfigURL, httputils.MakeHTTPHandler(n.NaplesGetHandler))
	t2.HandleFunc(ProfileURL, httputils.MakeHTTPHandler(n.NaplesProfileGetHandler))
	t2.HandleFunc(NaplesInfoURL, httputils.MakeHTTPHandler(n.NaplesInfoGetHandler))
	t2.HandleFunc(CmdEXECUrl, n.NaplesCmdExecHandler)
	t2.HandleFunc(NaplesVersionURL, httputils.MakeHTTPHandler(n.NaplesVersionGetHandler))
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

	t4 := router.Methods("PUT").Subrouter()
	t4.HandleFunc(ProfileURL, httputils.MakeHTTPHandler(n.NaplesProfileUpdateHandler))

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

func (n *NMD) setClusterCredentials(cert *x509.Certificate, caTrustChain, trustRoots []*x509.Certificate) error {
	err := n.tlsProvider.SetClientCertificate(cert)
	if err != nil {
		return fmt.Errorf("Error storing cluster certificate: %v", err)
	}
	n.tlsProvider.SetCaTrustChain(caTrustChain)
	n.tlsProvider.SetTrustRoots(trustRoots)

	// Persist trust roots so that we remember what is the last Venice cluster we connected to
	// and we can authenticate offline credentials signed by Venice CA.
	err = utils.StoreTrustRoots(trustRoots)
	if err != nil {
		return fmt.Errorf("Error storing cluster trust roots: %v", err)
	}

	return nil
}

func (n *NMD) parseAdmissionResponse(resp *grpc.NICAdmissionResponse) (*x509.Certificate, []*x509.Certificate, []*x509.Certificate, []string, error) {
	cntrls := resp.GetControllers()
	if cntrls == nil {
		// TODO : Check if we should return with a failure from here.
		log.Errorf("No controllers received as part of admission response")
	}

	certMsg := resp.GetClusterCert()
	if certMsg == nil {
		return nil, nil, nil, nil, fmt.Errorf("No certificate found in registration response message")
	}
	cert, err := x509.ParseCertificate(certMsg.GetCertificate().Certificate)
	if err != nil {
		return nil, nil, nil, nil, fmt.Errorf("Error parsing cluster certificate: %v", err)
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

	// If we have trust roots obtained during previous admission, we use them to check the
	// issued certificate signature and make sure that we are connecting to the same cluster.
	// However, we always return the new trust roots, not old ones, as they may differ.
	ntr, err := utils.GetNaplesTrustRoots()
	if err != nil {
		return nil, nil, nil, nil, fmt.Errorf("Error getting trust roots: %v", err)
	}
	log.Infof("Loaded %d trust roots", len(ntr))
	if ntr != nil {
		verifyOpts := x509.VerifyOptions{
			Intermediates: certs.NewCertPool(caTrustChain),
			Roots:         certs.NewCertPool(ntr),
			KeyUsages:     []x509.ExtKeyUsage{x509.ExtKeyUsageAny},
		}
		verifiedChains, err := cert.Verify(verifyOpts)
		if err != nil || len(verifiedChains) == 0 {
			return nil, nil, nil, nil, fmt.Errorf("Error verifying certificate signature: %v, verified chain len: %d", err, len(verifiedChains))
		}
	} else {
		log.Infof("Did not find a trust roots file, skipping validation")
	}

	return cert, caTrustChain, trustRoots, cntrls, nil
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

func copyFile(src, dst string, mode os.FileMode) error {
	// First try to make the directory if it doesn't exist
	err := os.MkdirAll(filepath.Dir(dst), 0755)
	if err != nil {
		log.Errorf("failed creating directory: %s", err)
		return err
	}

	srcf, err := os.Open(src)
	if err != nil {
		log.Errorf("failed opening file: %s", err)
		return err
	}
	defer srcf.Close()

	dstf, err := os.OpenFile(dst, os.O_CREATE|os.O_RDWR, mode)
	if err != nil {
		log.Errorf("failed creating file: %s", err)
		return err
	}
	defer dstf.Close()

	_, err = io.Copy(dstf, srcf)
	if err != nil {
		log.Errorf("failed copying file: %s", err)
		return err
	}

	return nil
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

var allowedCommands map[string]int

func initAllowedCommands() {
	allowedCommands = make(map[string]int)
	allowedCommands["showRunningFirmware"] = 1
	allowedCommands["showStartupFirmware"] = 2
	allowedCommands["setStartupToMainfwa"] = 3
	allowedCommands["setStartupToMainfwb"] = 4
	allowedCommands["setStartupToAltfw"] = 5
	allowedCommands["installFirmware"] = 6
	allowedCommands["penrmfirmware"] = 7
	allowedCommands["pensettimezone"] = 8
	allowedCommands["sethwclock"] = 9
	allowedCommands["setsshauthkey"] = 10
	allowedCommands["eraseConfig"] = 11
	allowedCommands["factoryDefault"] = 12
	allowedCommands["penrmauthkeys"] = 13
	allowedCommands["penrmsshdfiles"] = 14
	allowedCommands["penrmpubkey"] = 15
	allowedCommands["halctlshowinterface"] = 16
	allowedCommands["halctlshowinterfacetunnel"] = 17
	allowedCommands["halctlshowinterfacetunnelmplsoudp"] = 18
	allowedCommands["halctldebugdeleteinterface"] = 19
	allowedCommands["halctldebugupdateinterface"] = 20
	allowedCommands["halctlshowport"] = 21
	allowedCommands["halctldebug"] = 22
	allowedCommands["mksshdir"] = 23
	allowedCommands["touchsshauthkeys"] = 24
	allowedCommands["touchsshdlock"] = 25
	allowedCommands["disablesshd"] = 26
	allowedCommands["stopsshd"] = 27
	allowedCommands["enablesshd"] = 28
	allowedCommands["startsshd"] = 29
	allowedCommands["lnlocaltime"] = 30
	allowedCommands["setdate"] = 31
	allowedCommands["getdate"] = 32
	allowedCommands["getsysmem"] = 33
	allowedCommands["getprocmeminfo"] = 34
	allowedCommands["halctlshowqosclass"] = 35
	allowedCommands["halctlshowqosclassgroup"] = 36
	allowedCommands["halctldebugdeleteqosclassqosgroup"] = 37
	allowedCommands["halctlshowsystemqueuestats"] = 38
	allowedCommands["consoledisable"] = 39
	allowedCommands["consoleenable"] = 40
	allowedCommands["rmpentechsupportdir"] = 41
	allowedCommands["penverifyfirmware"] = 42
	allowedCommands["halctlshowsystemstatisticspbdetail"] = 43
	allowedCommands["halctlshowqosclassqueues"] = 44
	allowedCommands["showinterfacemanagement"] = 45
}

func isCmdAllowed(cmd string) bool {
	_, ok := allowedCommands[cmd]
	return ok
}

func executeCmd(req *nmd.DistributedServiceCardCmdExecute, parts []string) (string, error) {
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

func naplesExecCmd(req *nmd.DistributedServiceCardCmdExecute) (string, error) {
	if !isCmdAllowed(req.Executable) {
		return "Unknown executable " + req.Executable, errors.New("Unknown executable " + req.Executable)
	}

	if req.Executable == "pensettimezone" {
		err := ioutil.WriteFile("/etc/timezone", []byte(req.Opts), 0644)
		if err != nil {
			return err.Error(), err
		}
		return "", nil
	} else if req.Executable == "setsshauthkey" {
		err := appendAuthorizedKeyFile("/update/" + req.Opts)
		if err != nil {
			return err.Error(), err
		}
		err = copyFile("/root/.ssh/authorized_keys",
			"/sysconfig/config0/.authorized_keys", 0600)
		if err != nil {
			return err.Error(), err
		}
		return "", nil
	} else if req.Executable == "penrmpubkey" {
		req.Executable = "rm"
		req.Opts = "-rf /update/*"
	} else if req.Executable == "penrmsshdfiles" {
		req.Executable = "rm"
		req.Opts = "-f /var/lock/sshd /root/.ssh/authorized_keys " +
			"/sysconfig/config0/.authorized_keys"
	} else if req.Executable == "penrmfirmware" {
		req.Executable = "rm"
		req.Opts = "-rf /update/*"
	} else if req.Executable == "penrmauthkeys" {
		req.Executable = "rm"
		req.Opts = "/root/.ssh/authorized_keys " +
			"/sysconfig/config0/.authorized_keys"
	} else if req.Executable == "showRunningFirmware" {
		req.Executable = "fwupdate"
		req.Opts = "-r"
	} else if req.Executable == "showStartupFirmware" {
		req.Executable = "fwupdate"
		req.Opts = "-S"
	} else if req.Executable == "setStartupToMainfwa" {
		req.Executable = "fwupdate"
		req.Opts = "-s mainfwa"
	} else if req.Executable == "setStartupToMainfwb" {
		req.Executable = "fwupdate"
		req.Opts = "-s mainfwb"
	} else if req.Executable == "installFirmware" {
		parts := strings.SplitN(req.Opts, " ", 2)
		req.Executable = "fwupdate"
		req.Opts = "-p /update/" + parts[0] + " -i " + parts[1]
	} else if req.Executable == "penverifyfirmware" {
		req.Executable = "fwupdate"
		req.Opts = "-p /update/" + req.Opts + " -v"
	} else if req.Executable == "setStartupToAltfw" {
		req.Executable = "fwupdate"
		req.Opts = "-s altfw"
	} else if req.Executable == "eraseConfig" {
		req.Executable = "clear_nic_config.sh"
		req.Opts = "remove-config"
	} else if req.Executable == "factoryDefault" {
		req.Executable = "clear_nic_config.sh"
		req.Opts = "factory-default"
	} else if req.Executable == "sethwclock" {
		req.Executable = "hwclock"
		req.Opts = "-wu"
	} else if req.Executable == "halctlshowinterface" {
		req.Executable = "halctl"
		req.Opts = "show interface"
	} else if req.Executable == "halctlshowinterfacetunnel" {
		req.Executable = "halctl"
		req.Opts = "show interface tunnel"
	} else if req.Executable == "halctlshowinterfacetunnelmplsoudp" {
		req.Executable = "halctl"
		req.Opts = "show interface tunnel mplsoudp"
	} else if req.Executable == "halctldebugdeleteinterface" {
		req.Executable = "halctl"
		req.Opts = "debug delete interface " + req.Opts
	} else if req.Executable == "halctldebugupdateinterface" {
		req.Executable = "halctl"
		req.Opts = "debug update interface " + req.Opts
	} else if req.Executable == "halctlshowport" {
		req.Executable = "halctl"
		req.Opts = "show port " + req.Opts
	} else if req.Executable == "halctldebug" {
		req.Executable = "halctl"
		req.Opts = "debug " + req.Opts
	} else if req.Executable == "mksshdir" {
		req.Executable = "mkdir"
		req.Opts = "/root/.ssh/"
	} else if req.Executable == "touchsshauthkeys" {
		req.Executable = "touch"
		req.Opts = "/root/.ssh/authorized_keys"
	} else if req.Executable == "killsshd" {
		req.Executable = "killall"
		req.Opts = "sshd"
	} else if req.Executable == "touchsshdlock" {
		req.Executable = "touch"
		req.Opts = "/var/lock/sshd"
	} else if req.Executable == "lnlocaltime" {
		req.Executable = "ln"
		req.Opts = "-sf " + req.Opts + " /etc/localtime"
	} else if req.Executable == "setdate" {
		req.Executable = "date"
		req.Opts = "--set " + req.Opts
		parts := strings.SplitN(req.Opts, " ", 2)
		return executeCmd(req, parts)
	} else if req.Executable == "getdate" {
		req.Executable = "date"
		req.Opts = ""
	} else if req.Executable == "getsysmem" {
		req.Executable = "free"
		req.Opts = "-m"
	} else if req.Executable == "getprocmeminfo" {
		req.Executable = "cat"
		req.Opts = "/proc/meminfo"
	} else if req.Executable == "halctlshowsystemqueuestats" {
		req.Executable = "halctl"
		req.Opts = "show system queue-statistics"
	} else if req.Executable == "halctlshowqosclass" {
		req.Executable = "halctl"
		req.Opts = "show qos-class"
	} else if req.Executable == "halctlshowqosclassgroup" {
		req.Executable = "halctl"
		req.Opts = "show qos-class --qosgroup " + req.Opts
	} else if req.Executable == "halctldebugdeleteqosclassqosgroup" {
		req.Executable = "halctl"
		req.Opts = "debug delete qos-class --qosgroup " + req.Opts
	} else if req.Executable == "halctlshowsystemstatisticspbdetail" {
		req.Executable = "halctl"
		req.Opts = "show system statistics pb detail"
	} else if req.Executable == "halctlshowqosclassqueues" {
		req.Executable = "halctl"
		req.Opts = "show qos-class queues"
	} else if req.Executable == "disablesshd" {
		req.Executable = "/etc/init.d/S50sshd"
		req.Opts = "disable"
	} else if req.Executable == "stopsshd" {
		req.Executable = "/etc/init.d/S50sshd"
		req.Opts = "stop"
	} else if req.Executable == "enablesshd" {
		req.Executable = "/etc/init.d/S50sshd"
		req.Opts = "enable"
	} else if req.Executable == "startsshd" {
		req.Executable = "/etc/init.d/S50sshd"
		req.Opts = "start"
	} else if req.Executable == "consoleenable" {
		req.Executable = "/nic/tools/serial_console.sh"
		req.Opts = "enable"
	} else if req.Executable == "consoledisable" {
		req.Executable = "/nic/tools/serial_console.sh"
		req.Opts = "disable"
	} else if req.Executable == "rmpentechsupportdir" {
		req.Executable = "rm"
		req.Opts = "-rf /data/techsupport/PenctlTechSupportRequest-penctl-techsupport/"
	} else if req.Executable == "showinterfacemanagement" {
		req.Executable = "/sbin/ifconfig"
		req.Opts = "-a"
	}
	return executeCmd(req, strings.Fields(req.Opts))
}

func naplesPkgVerify(pkgName string) (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "penverifyfirmware",
		Opts:       strings.Join([]string{pkgName}, ""),
	}
	return naplesExecCmd(v)
}

func naplesPkgInstall(pkgName string) (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "installFirmware",
		Opts:       strings.Join([]string{pkgName, " all"}, ""),
	}
	return naplesExecCmd(v)
}

func naplesSetBootImg() (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "setStartupToAltfw",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func naplesDelBootImg(pkgName string) (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "penrmfirmware",
		Opts:       strings.Join([]string{pkgName}, ""),
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
	req := nmd.DistributedServiceCardCmdExecute{}
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

	w.Write([]byte(stdErrOut))
}

// NaplesInfoGetHandler is the REST handler for Naples Profiles GET
func (n *NMD) NaplesInfoGetHandler(r *http.Request) (interface{}, error) {
	if n.metrics != nil && n.metrics.GetCalls != nil {
		n.metrics.GetCalls.Inc()
	}
	n.UpdateNaplesInfoFromConfig()
	info := n.nic
	return info, nil
}

func (n *NMD) updateLocalTimeZone() error {
	timeZone := n.GetTimeZone()

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "pensettimezone",
		Opts:       strings.Join([]string{timeZone}, ""),
	}
	_, err := naplesExecCmd(v)

	if err != nil {
		log.Error("Failed to write in /etc/timezon file")
		return err
	}

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "lnlocaltime",
		Opts:       strings.Join([]string{timeZone}, " "),
	}
	_, err = naplesExecCmd(v)
	if err != nil {
		log.Errorf("Failed to create the symlink to :%v", timeZone)
		return err
	}

	return nil
}

// NaplesVersionGetHandler is the REST handler for Naples Profiles GET
func (n *NMD) NaplesVersionGetHandler(r *http.Request) (interface{}, error) {
	if n.metrics != nil && n.metrics.GetCalls != nil {
		n.metrics.GetCalls.Inc()
	}

	return n.GetNaplesSoftwareInfo()
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
func (n *NMD) SetNaplesConfig(cfgSpec nmd.DistributedServiceCardSpec) {
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
func (n *NMD) GetNaplesConfig() nmd.DistributedServiceCard {
	n.Lock()
	defer n.Unlock()

	return n.config
}

// SetSmartNIC intializes the smartNIC object
func (n *NMD) SetSmartNIC(nic *cluster.DistributedServiceCard) error {
	n.Lock()
	defer n.Unlock()

	n.nic = nic
	return nil
}

// GetSmartNIC returns the smartNIC object
func (n *NMD) GetSmartNIC() (*cluster.DistributedServiceCard, error) {
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
func (n *NMD) CreateIPClient() {
	n.Lock()
	defer n.Unlock()
	var ipClient *ipif.IPClient
	var err error

	if n.config.Spec.Mode == nmd.MgmtMode_HOST.String() {
		n.IPClient = nil
		return
	}

	pipeline := ""
	if n.Pipeline != nil {
		pipeline = n.Pipeline.GetPipelineType()
	}

	if n.config.Spec.NetworkMode == nmd.NetworkMode_INBAND.String() {
		ipClient, err = ipif.NewIPClient(n, ipif.NaplesInbandInterface, pipeline)
		if err != nil {
			log.Errorf("Failed to instantiate ipclient on inband interface. Err: %v", err)
		}
	} else {
		ipClient, err = ipif.NewIPClient(n, ipif.NaplesOOBInterface, pipeline)
		if err != nil {
			log.Errorf("Failed to instantiate ipclient on oob interface. Err: %v", err)
		}
	}
	n.IPClient = ipClient
}

// CreateMockIPClient creates IPClient in Mock mode to run in venice integ environment
func (n *NMD) CreateMockIPClient() {
	n.Lock()
	defer n.Unlock()
	ipClient, err := ipif.NewIPClient(n, ipif.NaplesMockInterface, "")
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
		var profile *nmd.DSCProfile
		var defaultPortAdmin string
		var ok bool
		log.Info("Updating feature profile.")
		// Check if the profile exists.
		for _, p := range n.profiles {
			if p.Name == n.config.Spec.DSCProfile {
				profile = p
				ok = true
				break
			}
		}

		if !ok {
			log.Errorf("could not find profile %v in nmd state, profs: %+v", n.config.Spec.DSCProfile, n.profiles)
			err = fmt.Errorf("could not find profile %v in nmd state", n.config.Spec.DSCProfile)
			return
		}

		if profile.Spec.DefaultPortAdmin == nmd.PortAdminState_PORT_ADMIN_STATE_DISABLE.String() {
			defaultPortAdmin = device.PortAdminState_PORT_ADMIN_STATE_DISABLE.String()
		} else {
			defaultPortAdmin = device.PortAdminState_PORT_ADMIN_STATE_ENABLE.String()
		}

		err = n.PersistDeviceSpec(fwdMode, convertProfileNameToFeatureSpec(profile.Name), defaultPortAdmin, mgmtIfMAC)
		return
	}

	err = n.PersistDeviceSpec(device.ForwardingMode_FORWARDING_MODE_HOSTPIN.String(), device.FeatureProfile_FEATURE_PROFILE_NONE, device.PortAdminState_PORT_ADMIN_STATE_ENABLE.String(), mgmtIfMAC)
	return err
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
	mode, _ := device.ForwardingMode_value[fwdMode]
	deviceSpec := device.SystemSpec{
		// FwdMode:        fwdMode,
		FwdMode:        device.ForwardingMode(mode),
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
	log.Infof("Marshaled JSON is : %v ", string(data))

	if err = ioutil.WriteFile(globals.NaplesModeConfigFile, data, 0664); err != nil {
		log.Errorf("Failed to write feature profile to %s. Err: %v", globals.NaplesModeConfigFile, err)
	}

	// Update app-start.conf file. TODO Remove this workaround when all the processes are migrated to read from device.conf
	// var appStartSpec []byte
	// switch fwdMode {
	// case device.ForwardingMode_FORWARDING_MODE_HOSTPIN.String():
	// 	appStartSpec = []byte("hostpin")
	// default:
	// 	appStartSpec = []byte("classic")
	// }

	// log.Infof("App Start Spec is : %v", appStartSpec)
	// appStartConfFilePath := fmt.Sprintf("%s/app-start.conf", path.Dir(globals.NaplesModeConfigFile))
	// if err := ioutil.WriteFile(appStartConfFilePath, appStartSpec, 0755); err != nil {
	// 	log.Errorf("Failed to write app start conf. Err: %v", err)
	// }

	err = utils.BackupDeviceConfig()
	if err != nil {
		log.Errorf("Failed to backup device config. Err : %v", err)
	}
	return err
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
	n.Lock()
	defer n.Unlock()
	n.config.Status.IPConfig = cfg
}

// SetMgmtInterface sets the management interface after auto discovery
func (n *NMD) SetMgmtInterface(intf string) {
	n.Lock()
	defer n.Unlock()
	n.config.Status.ManagementInterface = intf
}

// GetVeniceIPs returns the venice co-ordinates
func (n *NMD) GetVeniceIPs() []string {
	return n.config.Status.Controllers
}

// SetVeniceIPs sets the venice co-ordinates
func (n *NMD) SetVeniceIPs(veniceIPs []string) {
	n.config.Status.Controllers = veniceIPs
}

// SetInterfaceIPs sets the DSC interface IPs
func (n *NMD) SetInterfaceIPs(interfaceIPs map[uint32]*cluster.IPConfig) {
	for intfID, intf := range interfaceIPs {
		n.DSCInterfaceIPs = append(n.DSCInterfaceIPs, agentTypes.DSCInterfaceIP{IfID: intfID, IPAddress: intf.IPAddress, GatewayIP: intf.DefaultGW})
	}
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

// NaplesProfileUpdateHandler is the REST handler for Naples Profile PUT
func (n *NMD) NaplesProfileUpdateHandler(r *http.Request) (interface{}, error) {
	req := nmd.DSCProfile{}
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
	log.Infof("Naples Update Profile Request: %+v", req)

	err = n.UpdateNaplesProfile(req)
	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}
	log.Infof("Naples Profile Response: %+v", resp)

	return resp, nil
}

func (n *NMD) createDefaultProfiles() error {

	for _, profileName := range nmd.SupportedProfiles_name {
		if profileName == "default" || profileName == nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String() {
			profileName = nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String()
		}

		profile := &nmd.DSCProfile{
			ObjectMeta: api.ObjectMeta{
				Name: profileName,
			},
			TypeMeta: api.TypeMeta{
				Kind: "DSCProfile",
			},
			Spec: nmd.DSCProfileSpec{
				DefaultPortAdmin: nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String(),
			},
		}

		err := n.CreateNaplesProfile(*profile)
		if err != nil {
			log.Errorf("Failed to create profile %v", profileName)
		}
	}

	return nil
}

func convertProfileNameToFeatureSpec(profileName string) device.FeatureProfile {
	switch profileName {
	case nmd.SupportedProfiles_FEATURE_PROFILE_STORAGE.String():
		return device.FeatureProfile_FEATURE_PROFILE_STORAGE
	case nmd.SupportedProfiles_FEATURE_PROFILE_SRIOV.String():
		return device.FeatureProfile_FEATURE_PROFILE_SRIOV
	case nmd.SupportedProfiles_FEATURE_PROFILE_VIRTUALIZED.String():
		return device.FeatureProfile_FEATURE_PROFILE_VIRTUALIZED
	case nmd.SupportedProfiles_FEATURE_PROFILE_PROXY.String():
		return device.FeatureProfile_FEATURE_PROFILE_PROXY
	case nmd.SupportedProfiles_FEATURE_PROFILE_DEVELOPER.String():
		return device.FeatureProfile_FEATURE_PROFILE_DEVELOPER
	case "default": // required for older software
		fallthrough
	case nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String():
		fallthrough
	default:
		return device.FeatureProfile_FEATURE_PROFILE_BASE
	}
}

func reconcileDSCProfile(p *nmd.DSCProfile) *nmd.DSCProfile {
	log.Infof("Reconciling DSC profile : %v", p)
	if p.Name == "default" || p.Name == "DEFAULT" || p.Spec.NumLifs == 16 {
		log.Infof("Updating the feature profile to use base config with the Base config")

		// The default profile is the base config
		return &nmd.DSCProfile{
			ObjectMeta: api.ObjectMeta{
				Name: nmd.SupportedProfiles_FEATURE_PROFILE_BASE.String(),
			},
			TypeMeta: api.TypeMeta{
				Kind: "DSCProfile",
			},
			Spec: nmd.DSCProfileSpec{
				NumLifs:          1,
				DefaultPortAdmin: p.Spec.DefaultPortAdmin,
			},
		}
	}

	return p
}
