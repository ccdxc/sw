package etcd

import (
	"context"
	"crypto/tls"
	"fmt"
	"net/http"
	"os"
	"path"
	"strings"
	"sync"
	"time"

	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/systemd"
)

// etcdQuorum implements a Quorum using etcd.
type etcdQuorum struct {
	client *clientv3.Client
	config *quorum.Config

	httpclient *netutils.HTTPClient // for Health checks of members - just keep one and reuse
	sync.Mutex                      // one call to httpclient at a time since httpclient.ctx is shared across calls
}

const (
	// Defaults
	defaultCfgFile  = globals.EtcdConfigFile
	defaultDataDir  = globals.EtcdDataDir
	defaultUnitFile = globals.EtcdServiceFile
	timeout         = time.Second * 15
	defragTimeout   = time.Minute * 2

	// Fixed parameters
	instanceAuthDirName = "auth"

	// Environment variables
	nameVar                 = "NAME"
	advPeerURLsVar          = "ADV_PEER_URLS"
	lisPeerURLsVar          = "LIS_PEER_URLS"
	advClientURLsVar        = "ADV_CLIENT_URLS"
	lisClientURLsVar        = "LIS_CLIENT_URLS"
	clusterTokenVar         = "INIT_CLUSTER_TOKEN"
	clusterVar              = "INIT_CLUSTER"
	clusterStateVar         = "INIT_CLUSTER_STATE"
	dataDirVar              = "DATA_DIR"
	clientCertFileVar       = "ETCD_CLIENT_CERT_FILE"
	clientPrivKeyFileVar    = "ETCD_CLIENT_KEY_FILE"
	clientClientCertReqdVar = "ETCD_CLIENT_CERT_AUTH"
	clientCATrustBundleVar  = "ETCD_CLIENT_TRUSTED_CA_FILE"
	peerCertFileVar         = "ETCD_PEER_CERT_FILE"
	peerPrivKeyFileVar      = "ETCD_PEER_KEY_FILE"
	peerClientCertReqdVar   = "ETCD_PEER_CLIENT_CERT_AUTH"
	peerCATrustBundleVar    = "ETCD_PEER_TRUSTED_CA_FILE"
	peerCertAllowedCNVar    = "ETCD_PEER_CERT_ALLOWED_CN"

	// Parameters
	nameParam                 = "--name"
	advPeerURLsParam          = "--initial-advertise-peer-urls"
	lisPeerURLsParam          = "--listen-peer-urls"
	advClientURLsParam        = "--advertise-client-urls"
	lisClientURLsParam        = "--listen-client-urls"
	clusterTokenParam         = "--initial-cluster-token"
	clusterParam              = "--initial-cluster"
	clusterStateParam         = "--initial-cluster-state"
	dataDirParam              = "--data-dir"
	clientCertFileParam       = "--cert-file"             // File containing the certificate used by an etcd instance to authenticate itself to a client
	clientPrivKeyFileParam    = "--key-file"              // File containing the private key used by an etcd instance to authenticate itself to a client
	clientClientCertReqdParam = "--client-cert-auth"      // Flag specifying if client certificate is required
	clientCATrustBundleParam  = "--trusted-ca-file"       // File containing the CA bundle used to verify client certificates
	peerCertFileParam         = "--peer-cert-file"        // File containing the client/server certificate used for TLS connections between peers
	peerPrivKeyFileParam      = "--peer-key-file"         // File containing the private key for TLS between peers
	peerClientCertReqdParam   = "--peer-client-cert-auth" // Flag specifying if peer client certificates are required
	peerCATrustBundleParam    = "--peer-trusted-ca-file"  // File containing the CA bundle used to verify peer client certificates
	peerCertAllowedCNParam    = "--peer-cert-allowed-cn"  // The CN expected in a peer certificate
)

// memberIndex returns -1 if member is cant be found
func memberIndex(c *quorum.Config) int {
	idx := -1
	for ii, member := range c.Members {
		if c.MemberName == member.Name {
			idx = ii
			break
		}
	}
	return idx
}

func getInstanceAuthDir() string {
	return path.Join(globals.EtcdPKIDir, instanceAuthDirName)
}

// getPeerAuthFilePaths returns the paths to the files containing the credentials that the etcd instance uses to authenticate itself to its peers
func getPeerAuthFilePaths() (certFilePath, keyFilePath, caBundleFilePath string) {
	certFilePath, keyFilePath, caBundleFilePath = certs.GetTLSCredentialsPaths(getInstanceAuthDir())
	return
}

// getClientAuthFilePaths returns the paths to the files containing the credentials that the etcd instance uses to authenticate itself to its clients.
// Currently we use the same credentials for peers and clients.
func getClientAuthFilePaths() (certFilePath, keyFilePath, caBundleFilePath string) {
	return getPeerAuthFilePaths()
}

func createConfigFile(c *quorum.Config) error {
	idx := memberIndex(c)

	// Configure defaults.
	if c.CfgFile == "" {
		c.CfgFile = defaultCfgFile
	}
	if c.DataDir == "" {
		c.DataDir = defaultDataDir
	}

	// Populate the config map.
	cfgMap := make(map[string]string)
	cfgMap[nameVar] = fmt.Sprintf("%s %s", nameParam, c.MemberName)
	cfgMap[clusterTokenVar] = fmt.Sprintf("%s %s", clusterTokenParam, c.ID)
	cfgMap[dataDirVar] = fmt.Sprintf("%s %s", dataDirParam, c.DataDir)

	peerURLs := strings.Join(c.Members[idx].PeerURLs, ",")
	cfgMap[advPeerURLsVar] = fmt.Sprintf("%s %s", advPeerURLsParam, peerURLs)
	cfgMap[lisPeerURLsVar] = fmt.Sprintf("%s %s", lisPeerURLsParam, peerURLs)

	clientURLs := strings.Join(c.Members[idx].ClientURLs, ",")
	cfgMap[advClientURLsVar] = fmt.Sprintf("%s %s", advClientURLsParam, clientURLs)
	cfgMap[lisClientURLsVar] = fmt.Sprintf("%s %s", lisClientURLsParam, clientURLs)

	clusterMembers := []string{}
	for _, member := range c.Members {
		clusterMembers = append(clusterMembers, fmt.Sprintf("%s=%s", member.Name, strings.Join(member.PeerURLs, ",")))
	}
	cfgMap[clusterVar] = fmt.Sprintf("%s %s", clusterParam, strings.Join(clusterMembers, ","))
	state := "new"
	if c.Existing {
		state = "existing"
	}
	cfgMap[clusterStateVar] = fmt.Sprintf("%s %s", clusterStateParam, state)

	// Parameters for mTLS between peers
	if c.PeerAuthEnabled {
		certFilePath, keyFilePath, caBundleFilePath := getPeerAuthFilePaths()
		cfgMap[peerCertFileVar] = fmt.Sprintf("%s %s", peerCertFileParam, certFilePath)
		cfgMap[peerPrivKeyFileVar] = fmt.Sprintf("%s %s", peerPrivKeyFileParam, keyFilePath)
		cfgMap[peerClientCertReqdVar] = fmt.Sprintf("%s", peerClientCertReqdParam)
		cfgMap[peerCATrustBundleVar] = fmt.Sprintf("%s %s", peerCATrustBundleParam, caBundleFilePath)
		cfgMap[peerCertAllowedCNVar] = fmt.Sprintf("%s %s", peerCertAllowedCNParam, globals.Etcd)
	}

	// Parameters for mTLS with clients
	if c.ClientAuthEnabled {
		certFilePath, keyFilePath, caBundleFilePath := getClientAuthFilePaths()
		cfgMap[clientCertFileVar] = fmt.Sprintf("%s %s", clientCertFileParam, certFilePath)
		cfgMap[clientPrivKeyFileVar] = fmt.Sprintf("%s %s", clientPrivKeyFileParam, keyFilePath)
		cfgMap[clientClientCertReqdVar] = fmt.Sprintf("%s", clientClientCertReqdParam)
		cfgMap[clientCATrustBundleVar] = fmt.Sprintf("%s %s", clientCATrustBundleParam, caBundleFilePath)
	}

	// Generate the config file.
	cfgFile, err := os.Create(c.CfgFile)
	if err != nil {
		return err
	}
	defer cfgFile.Close()

	for k, v := range cfgMap {
		_, err = cfgFile.WriteString(fmt.Sprintf("%s='%s'\n", k, v))
		if err != nil {
			return err
		}
	}
	cfgFile.Sync()
	return nil
}

func createPeerAuthFiles(c *quorum.Config) error {
	return certs.StoreTLSCredentials(c.PeerCert, c.PeerPrivateKey, c.PeerCATrustBundle, getInstanceAuthDir(), 0700)
}

// StartQuorum starts a member of etcdQuorum
func StartQuorum(c *quorum.Config) (quorum.Interface, error) {
	return quorumHelper(true, c)
}

// NewQuorum creates a new etcdQuorum.
func NewQuorum(c *quorum.Config) (quorum.Interface, error) {
	return quorumHelper(false, c)
}

// GetQuorumClientCredentials returns credentials needed to connect to the quorum
// in the form of a TLS config
func GetQuorumClientCredentials() (*tls.Config, error) {
	tlsConfig, err := certs.LoadTLSCredentials(globals.EtcdClientAuthDir)
	if err != nil {
		return nil, err
	}
	tlsConfig.ServerName = globals.Etcd
	return tlsConfig, nil
}

func quorumHelper(existing bool, c *quorum.Config) (quorum.Interface, error) {
	// Validate provided configuration.
	if c.MemberName == "" || len(c.Members) == 0 {
		return nil, fmt.Errorf("Missing member name or members")
	}
	if c.UnitFile == "" {
		c.UnitFile = defaultUnitFile
	}

	idx := memberIndex(c)
	if idx == -1 {
		return nil, fmt.Errorf("Missing member %v in member list", c.MemberName)
	}

	if existing == false {
		err := createConfigFile(c)
		if err != nil {
			return nil, err
		}

		if c.PeerAuthEnabled || c.ClientAuthEnabled {
			err := createPeerAuthFiles(c)
			if err != nil {
				log.Errorf("Error creating auth files: %v. AuthDir %s", err, getInstanceAuthDir())
				return nil, err
			}
		}
	}

	s := systemd.New()
	// Start etcd using systemd.
	err := s.StartTarget(c.UnitFile)
	if err != nil {
		return nil, err
	}

	tlsConfig, err := GetQuorumClientCredentials()
	if err != nil {
		log.Errorf("Error getting quorum client credentials: %v", err)
		return nil, err
	}

	// Open a client.
	v3Config := clientv3.Config{
		Endpoints:   c.Members[idx].ClientURLs,
		DialTimeout: timeout,
		TLS:         tlsConfig,
	}
	client, err := clientv3.New(v3Config)
	if err != nil {
		return nil, err
	}

	return &etcdQuorum{
		client: client,
		config: c,
	}, nil
}

// List returns the current quorum members.
func (e *etcdQuorum) List() ([]quorum.Member, error) {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	resp, err := e.client.MemberList(ctx)
	if err != nil {
		return nil, err
	}

	result := make([]quorum.Member, 0)
	for _, member := range resp.Members {
		result = append(result, quorum.Member{
			ID:         member.ID,
			Name:       member.Name,
			PeerURLs:   member.PeerURLs,
			ClientURLs: member.ClientURLs,
		})
	}
	return result, nil
}

// Add adds new member to the quorum.
func (e *etcdQuorum) Add(member *quorum.Member) error {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	_, err := e.client.MemberAdd(ctx, member.PeerURLs)
	if err != nil {
		return err
	}
	log.Infof("Added member: %v", member.PeerURLs)
	return nil
}

// Defrag performs defrgmentation on a quorum member
func (e *etcdQuorum) Defrag(member *quorum.Member) error {
	if len(member.ClientURLs) == 0 {
		return fmt.Errorf("Error defragmenting quorum member %s: no client URL", member.Name)
	}
	ctx, cancel := context.WithTimeout(context.Background(), defragTimeout)
	defer cancel()

	_, err := e.client.Defragment(ctx, member.ClientURLs[0])
	if err != nil {
		return err
	}
	log.Infof("Defragmented member: %v", member.ClientURLs)
	return nil
}

// Remove removes an existing quorum member.
func (e *etcdQuorum) Remove(id uint64) error {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	_, err := e.client.MemberRemove(ctx, id)
	if err != nil {
		return err
	}
	log.Infof("Removed member: %v", id)
	return nil
}

// GetStatus returns the status of the quorum member.
func (e *etcdQuorum) GetStatus(member *quorum.Member) (interface{}, error) {
	if len(member.ClientURLs) == 0 {
		return nil, fmt.Errorf("Error getting status for quorum member %s: no client URL", member.Name)
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	resp, err := e.client.Status(ctx, member.ClientURLs[0])
	if err != nil {
		return nil, fmt.Errorf("Error getting endpoint %s status: %v", member.Name, err)
	}
	return resp, nil
}

func (e *etcdQuorum) getHTTPClient() (*netutils.HTTPClient, error) {
	if e.httpclient != nil {
		return e.httpclient, nil
	}
	tlsConfig, err := GetQuorumClientCredentials()
	if err != nil {
		log.Errorf("Error getting quorum client credentials: %v", err)
		return nil, err
	}
	e.httpclient = netutils.NewHTTPClient()
	e.httpclient.WithTLSConfig(tlsConfig)
	return e.httpclient, nil
}

// GetHealth returns the health of the quorum member by queryinh the /health REST endpoint
// It queries the member REST endpoint /health
func (e *etcdQuorum) GetHealth(member *quorum.Member) (bool, error) {
	log.Infof("Entered GetHealth for member %s", member.Name)
	if len(member.ClientURLs) == 0 {
		return false, fmt.Errorf("Error getting health for quorum member %s: no client URL", member.Name)
	}
	client, err := e.getHTTPClient()
	if err != nil {
		return false, fmt.Errorf("error %v getting httpClient", err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()

	e.Lock()
	client.WithContext(ctx)
	defer e.Unlock()

	type HealthResp struct {
		Health string `json:"health,omitempty"`
	}
	var resp HealthResp
	status, err := client.Req("GET", fmt.Sprintf("%s/health", member.ClientURLs[0]), nil, &resp)
	if err != nil || status != http.StatusOK {
		return false, fmt.Errorf("Error getting endpoint %s health: %v, status code: %d", member.Name, err, status)
	}
	return resp.Health == "true", nil
}
