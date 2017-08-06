package utils

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/utils/log"
)

// Cluster defines the information stored to disk on a node that belongs to a
// cluster. This information is used by CMD to determine whether a node is
// already part of a cluster (on reboot, process restart etc).
type Cluster struct {
	Name      string
	UUID      string
	VirtualIP string
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
	if err := ioutil.WriteFile(confFile, out, 0644); err != nil {
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
