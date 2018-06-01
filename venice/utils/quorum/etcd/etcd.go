package etcd

import (
	"context"
	"fmt"
	"os"
	"path"
	"strings"
	"time"

	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/quorum"
	"github.com/pensando/sw/venice/utils/systemd"
)

// etcdQuorum implements a Quorum using etcd.
type etcdQuorum struct {
	client *clientv3.Client
	config *quorum.Config
}

const (
	// Defaults
	defaultCfgFile  = globals.EtcdConfigFile
	defaultDataDir  = globals.EtcdDataDir
	defaultUnitFile = globals.EtcdServiceFile
	timeout         = time.Second * 5

	// Fixed parameters
	peerAuthDirName           = "peer-auth"
	peerCertFileName          = "cert.crt"
	peerPrivKeyFileName       = "peer.key"
	peerCATrustBundleFileName = "peer-ca.crt"

	// Environment variables
	nameVar               = "NAME"
	advPeerURLsVar        = "ADV_PEER_URLS"
	lisPeerURLsVar        = "LIS_PEER_URLS"
	advClientURLsVar      = "ADV_CLIENT_URLS"
	lisClientURLsVar      = "LIS_CLIENT_URLS"
	clusterTokenVar       = "INIT_CLUSTER_TOKEN"
	clusterVar            = "INIT_CLUSTER"
	clusterStateVar       = "INIT_CLUSTER_STATE"
	dataDirVar            = "DATA_DIR"
	peerCertFileVar       = "ETCD_CERT_FILE"
	peerPrivKeyFileVar    = "ETCD_KEY_FILE"
	peerClientCertReqdVar = "ETCD_CLIENT_CERT_AUTH"
	peerCATrustBundleVar  = "ETCD_TRUSTED_CA_FILE"

	// Parameters
	nameParam               = "--name"
	advPeerURLsParam        = "--initial-advertise-peer-urls"
	lisPeerURLsParam        = "--listen-peer-urls"
	advClientURLsParam      = "--advertise-client-urls"
	lisClientURLsParam      = "--listen-client-urls"
	clusterTokenParam       = "--initial-cluster-token"
	clusterParam            = "--initial-cluster"
	clusterStateParam       = "--initial-cluster-state"
	dataDirParam            = "--data-dir"
	peerCertFileParam       = "--peer-cert-file"        // File containing the client/server certificate used for TLS connections between peers
	peerPrivKeyFileParam    = "--peer-key-file"         // File containing the private key for TLS between peers
	peerClientCertReqdParam = "--peer-client-cert-auth" // Flag specifying if peer client certificates are required
	peerCATrustBundleParam  = "--peer-trusted-ca-file"  // File containing the CA bundle used to verify peer client certificates
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

func getPeerAuthDir() string {
	return path.Join(globals.EtcdConfigDir, peerAuthDirName)
}

func getPeerAuthFilePaths(c *quorum.Config) (certFilePath, keyFilePath, caBundleFilePath string) {
	authDir := getPeerAuthDir()
	certFilePath = path.Join(authDir, peerCertFileName)
	keyFilePath = path.Join(authDir, peerPrivKeyFileName)
	caBundleFilePath = path.Join(authDir, peerCATrustBundleFileName)
	return
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
		certFilePath, keyFilePath, caBundleFilePath := getPeerAuthFilePaths(c)
		cfgMap[peerCertFileVar] = fmt.Sprintf("%s %s", peerCertFileParam, certFilePath)
		cfgMap[peerPrivKeyFileVar] = fmt.Sprintf("%s %s", peerPrivKeyFileParam, keyFilePath)
		cfgMap[peerClientCertReqdVar] = fmt.Sprintf("%s", peerClientCertReqdParam)
		cfgMap[peerCATrustBundleVar] = fmt.Sprintf("%s %s", peerCATrustBundleParam, caBundleFilePath)
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
	var err error
	authDir := getPeerAuthDir()

	// wipe out old files if there and recreate
	err = os.RemoveAll(authDir)
	if err != nil {
		return err
	}
	err = os.MkdirAll(authDir, 0777)
	if err != nil {
		return err
	}
	defer func(err error) {
		if err != nil {
			os.RemoveAll(authDir)
		}
	}(err)

	certFilePath, keyFilePath, caBundleFilePath := getPeerAuthFilePaths(c)
	err = certs.SaveCertificate(certFilePath, c.PeerCert)
	if err != nil {
		return err
	}
	err = certs.SaveCertificates(caBundleFilePath, c.PeerCATrustBundle)
	if err != nil {
		return err
	}
	err = certs.SavePrivateKey(keyFilePath, c.PeerPrivateKey)
	if err != nil {
		return err
	}
	return nil
}

// StartQuorum starts a member of etcdQuorum
func StartQuorum(c *quorum.Config) (quorum.Interface, error) {
	return quorumHelper(true, c)
}

// NewQuorum creates a new etcdQuorum.
func NewQuorum(c *quorum.Config) (quorum.Interface, error) {
	return quorumHelper(false, c)
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

		if c.PeerAuthEnabled {
			err := createPeerAuthFiles(c)
			if err != nil {
				log.Errorf("Error creating peer auth files: %v. AuthDir %s", err, getPeerAuthDir())
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

	// Open a client.
	v3Config := clientv3.Config{
		Endpoints:   c.Members[idx].ClientURLs,
		DialTimeout: timeout,
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
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
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
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()
	_, err := e.client.MemberAdd(ctx, member.PeerURLs)
	if err != nil {
		return err
	}
	log.Infof("Added member: %v", member.PeerURLs)
	return nil
}

// Remove removes an existing quorum member.
func (e *etcdQuorum) Remove(id uint64) error {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*2)
	defer cancel()
	_, err := e.client.MemberRemove(ctx, id)
	if err != nil {
		return err
	}
	log.Infof("Removed member: %v", id)
	return nil
}
