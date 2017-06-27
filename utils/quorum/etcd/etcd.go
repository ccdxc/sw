package etcd

import (
	"context"
	"fmt"
	"os"
	"strings"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/coreos/etcd/clientv3"

	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/systemd"
)

// etcdQuorum implements a Quorum using etcd.
type etcdQuorum struct {
	client *clientv3.Client
	config *quorum.Config
}

const (
	// Defaults
	defaultCfgFile  = "/etc/etcd.conf"
	defaultDataDir  = "/var/lib/etcd"
	defaultUnitFile = "etcd.service"
	timeout         = time.Second * 5

	// Environment variables
	nameVar          = "NAME"
	advPeerURLsVar   = "ADV_PEER_URLS"
	lisPeerURLsVar   = "LIS_PEER_URLS"
	advClientURLsVar = "ADV_CLIENT_URLS"
	lisClientURLsVar = "LIS_CLIENT_URLS"
	clusterTokenVar  = "INIT_CLUSTER_TOKEN"
	clusterVar       = "INIT_CLUSTER"
	clusterStateVar  = "INIT_CLUSTER_STATE"
	dataDirVar       = "DATA_DIR"

	// Parameters
	nameParam          = "--name"
	advPeerURLsParam   = "--initial-advertise-peer-urls"
	lisPeerURLsParam   = "--listen-peer-urls"
	advClientURLsParam = "--advertise-client-urls"
	lisClientURLsParam = "--listen-client-urls"
	clusterTokenParam  = "--initial-cluster-token"
	clusterParam       = "--initial-cluster"
	clusterStateParam  = "--initial-cluster-state"
	dataDirParam       = "--data-dir"
)

// NewQuorum creates a new etcdQuorum.
func NewQuorum(c *quorum.Config) (quorum.Interface, error) {
	// Validate provided configuration.
	if c.MemberName == "" || len(c.Members) == 0 {
		return nil, fmt.Errorf("Missing member name or members")
	}

	idx := -1
	for ii, member := range c.Members {
		if c.MemberName == member.Name {
			idx = ii
			break
		}
	}
	if idx == -1 {
		return nil, fmt.Errorf("Missing member %v in member list", c.MemberName)
	}

	// Configure defaults.
	if c.CfgFile == "" {
		c.CfgFile = defaultCfgFile
	}
	if c.UnitFile == "" {
		c.UnitFile = defaultUnitFile
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

	// Generate the config file.
	cfgFile, err := os.Create(c.CfgFile)
	if err != nil {
		return nil, err
	}
	defer cfgFile.Close()

	for k, v := range cfgMap {
		_, err := cfgFile.WriteString(fmt.Sprintf("%s='%s'\n", k, v))
		if err != nil {
			return nil, err
		}
	}

	cfgFile.Sync()

	// Start etcd using systemd.
	err = systemd.StartTarget("etcd.service")
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
