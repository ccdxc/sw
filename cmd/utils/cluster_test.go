package utils

import (
	"reflect"
	"testing"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/cmd/server/options"
)

func TestClusterConfigFile(t *testing.T) {
	env.Options = options.NewServerRunOptions()
	env.Options.ConfigDir = "/tmp"

	cluster := Cluster{
		Name:      "testCluster",
		UUID:      "AAAA-BBBB-CCCC-DDDD",
		VirtualIP: "192.168.30.10",
	}
	if err := SaveCluster(&cluster); err != nil {
		t.Fatalf("Saving cluster failed with error: %v", err)
	}
	if newCluster, err := GetCluster(); err != nil {
		t.Fatalf("Getting cluster failed with error: %v", err)
	} else if !reflect.DeepEqual(cluster, *newCluster) {
		t.Fatalf("Got %v, expected %v", *newCluster, cluster)
	}
	if err := DeleteCluster(); err != nil {
		t.Fatalf("Deleting cluster failed with error: %v", err)
	}
	if newCluster, err := GetCluster(); err != nil {
		t.Fatalf("Getting cluster failed with error: %v", err)
	} else if newCluster != nil {
		t.Fatalf("Got %v, expected nil", *newCluster)
	}
}
