// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	rolloutName        = "e2e_rollout"
)

func processMetadataFile(metadata string) map[string]map[string]string {
	versionMap := make(map[string]map[string]string)

	if _, err := os.Stat(metadata); err != nil {
		// Stat error is treated as not part of cluster.
		log.Errorf("unable to find confFile %s error: %v", metadata, err)
		return nil
	}
	var in []byte
	var err error
	if in, err = ioutil.ReadFile(metadata); err != nil {
		log.Errorf("unable to read confFile %s error: %v", metadata, err)
		return nil
	}
	if err := json.Unmarshal(in, &versionMap); err != nil {
		log.Errorf("unable to understand confFile %s error: %v", metadata, err)
		return nil
	}

	return versionMap
}

// GetCmdGitVersion reads config file and returns a map of ContainerInfo indexed by name
func GetCmdGitVersion() string {
	metadataFile := "/tmp/metadata.json"
	versionMap := processMetadataFile(metadataFile)
	if versionMap != nil {
		return versionMap["Venice"]["Version"]
	}
	return ""
}

// CreateRollout creates a new rollout instance
func (sm *SysModel) GetRolloutObject() (*rollout.Rollout, error) {

	seconds := time.Now().Unix()
	scheduledStartTime := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
		},
	}
	jsonUrl := []string{"http://pxe.pensando.io/kickstart/veniceImageForRollout/metadata.json", "--output", "/tmp/metadata.json"}

	out, err := exec.Command("curl", jsonUrl...).CombinedOutput()
	outStr := strings.TrimSpace(string(out))
	fmt.Println(fmt.Sprintf("curl output: %s, err: %v\n", outStr, err))
	if err != nil {
		fmt.Println(fmt.Sprintf("curl Error: %s, err: %v\n", outStr, err))
		fmt.Println()
	}

	version := GetCmdGitVersion()
	log.Errorf("Calling GetGitVersion %s", version)
	if version == "" {
		log.Errorf("ts:%s Build Failure. Couldnt get version.json", time.Now().String())
		err := errors.New("Build Failure. Couldnt get version information")
		return nil, err

	}

	return &rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutName,
		},
		Spec: rollout.RolloutSpec{
			Version:                     version,
			ScheduledStartTime:          scheduledStartTime,
			Duration:                    "",
			Strategy:                    "LINEAR",
			MaxParallel:                 0,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               false,
			//SmartNICMustMatchConstraint: true, // hence venice upgrade only
			UpgradeType:                 "Disruptive",
		},
	}, nil
}
