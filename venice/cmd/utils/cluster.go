package utils

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path"
	"sort"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/utils/log"
)

// Cluster defines the information stored to disk on a node that belongs to a
// cluster. This information is used by CMD to determine whether a node is
// already part of a cluster (on reboot, process restart etc).
type Cluster struct {
	Name        string
	UUID        string
	VirtualIP   string
	QuorumNodes []string
	NodeID      string
}

// GetCluster retrieves information about the current cluster, returns nil if
// not part of a cluster.
func GetCluster() (*Cluster, error) {
	confFile := path.Join(env.Options.ConfigDir, env.Options.ClusterConfigFile)
	if _, err := os.Stat(confFile); err != nil {
		// Stat error is treated as not part of cluster.
		return nil, nil
	}
	in, err := ioutil.ReadFile(confFile)
	if err != nil {
		return nil, err
	}
	var cluster Cluster
	if err := json.Unmarshal(in, &cluster); err != nil {
		return nil, err
	}
	return &cluster, nil
}

// SaveCluster saves the cluster information to disk.
func SaveCluster(cluster *Cluster) error {
	os.Mkdir(env.Options.ConfigDir, 0644)

	out, err := json.Marshal(cluster)
	if err != nil {
		log.Errorf("Failed to marshal cluster object with error: %v", err)
		return err
	}

	confFile := path.Join(env.Options.ConfigDir, env.Options.ClusterConfigFile)
	if err = ioutil.WriteFile(confFile, out, 0644); err != nil {
		log.Errorf("Failed to write cluster file with error: %v", err)
		return err
	}

	file, err := os.Open(confFile)
	if err != nil {
		log.Errorf("Failed to open cluster file with error: %v", err)
		os.Remove(confFile)
		return err
	}
	defer file.Close()
	if err := file.Sync(); err != nil {
		log.Errorf("Failed to sync cluster file with error: %v", err)
		os.Remove(confFile)
		return err
	}
	log.Infof("Wrote cluster config file: %+v", cluster)
	return nil
}

// DeleteCluster removes the cluster information from disk.
func DeleteCluster() error {
	log.Infof("Removing cluster config file")
	return os.Remove(path.Join(env.Options.ConfigDir, env.Options.ClusterConfigFile))
}

// ContainerInfo has info about containers to run on venice.
//	To start with it has only image name.
type ContainerInfo struct {
	ImageName string
}

// ImageConfig is created by Installer containing info about various images and other dynamic info about image
type ImageConfig struct {
	ImageMap                map[string]string   `json:"imageMap,omitempty"`
	UpgradeOrder            []string            `json:"upgradeOrder,omitempty"`
	SupportedNaplesVersions map[string][]string `json:"supportedNaplesVersions,omitempty"`
	GitVersion              map[string]string   `json:"cmdVersionMap,omitempty"`
}

func readImageConfigFile(imageConfig *ImageConfig) error {
	confFile := path.Join(env.Options.CommonConfigDir, env.Options.ContainerConfigFile)
	if _, err := os.Stat(confFile); err != nil {
		// Stat error is treated as not part of cluster.
		log.Errorf("unable to find confFile %s error: %v", confFile, err)
		return err
	}
	var in []byte
	var err error
	if in, err = ioutil.ReadFile(confFile); err != nil {
		log.Errorf("unable to read confFile %s error: %v", confFile, err)
		return err
	}
	if err := json.Unmarshal(in, imageConfig); err != nil {
		log.Errorf("unable to understand confFile %s error: %v", confFile, err)
		return err
	}
	return nil
}

// GetGitVersion reads config file and returns a map of ContainerInfo indexed by name
func GetGitVersion() map[string]string {
	var imageConfig ImageConfig
	readImageConfigFile(&imageConfig)
	return imageConfig.GitVersion
}

// GetContainerInfo reads config file and returns a map of ContainerInfo indexed by name
func GetContainerInfo() map[string]ContainerInfo {
	info := make(map[string]ContainerInfo)
	var imageConfig ImageConfig
	readImageConfigFile(&imageConfig)
	for k, v := range imageConfig.ImageMap {
		info[k] = ContainerInfo{ImageName: v}
	}
	return info
}

// GetUpgradeOrder reads config file and returns the order of services to be upgraded
func GetUpgradeOrder() []string {
	var imageConfig ImageConfig
	readImageConfigFile(&imageConfig)
	return imageConfig.UpgradeOrder
}

// GetSupportedNaplesVersions reads config file and returns the compatible naples versions
func GetSupportedNaplesVersions() map[string][]string {
	var imageConfig ImageConfig
	readImageConfigFile(&imageConfig)
	return imageConfig.SupportedNaplesVersions
}

// GetOtherQuorumNodes returns quorum list excluding quorum node whose node id is 'nodeID'
func GetOtherQuorumNodes(envQuorumNodes []string, nodeID string) []string {
	var (
		i           int
		quorumNodes []string
		found       bool
	)
	found = false
	quorumNodes = append(quorumNodes, envQuorumNodes...)
	sort.Strings(quorumNodes)
	for i = range quorumNodes {
		if quorumNodes[i] == nodeID {
			found = true
			break
		}
	}
	if found {
		quorumNodes = append(quorumNodes[:i], quorumNodes[i+1:]...)
	}
	return quorumNodes
}
