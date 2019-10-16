// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/labels"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	rolloutName = "e2e_rollout"
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

func getCmdGitVersion(metadataFile string) string {
	versionMap := processMetadataFile(metadataFile)
	if versionMap != nil {
		return versionMap["Venice"]["Version"]
	}
	return ""
}

// GetRolloutObject gets rollout instance
func (sm *SysModel) GetRolloutObject(scaleData bool) (*rollout.Rollout, error) {

	seconds := time.Now().Unix()
	scheduledStartTime := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
		},
	}
	/*jsonUrl := []string{"http://pxe.pensando.io/kickstart/veniceImageForRollout/metadata.json", "--output", "/tmp/metadata.json"}

	out, err := exec.Command("curl", jsonURL...).CombinedOutput()
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

	}*/
	var req labels.Requirement
	req.Key = "type"
	req.Operator = "in"
	req.Values = append(req.Values, "bm")

	var orderelem labels.Selector
	orderelem.Requirements = append(orderelem.Requirements, &req)

	var order []*labels.Selector
	order = append(order, &orderelem)

	version := "1.2.0-140"

	if scaleData {
		var req labels.Requirement
		req.Key = "type"
		req.Operator = "in"
		req.Values = append(req.Values, "bm")

		var orderelem labels.Selector
		orderelem.Requirements = append(orderelem.Requirements, &req)

		var order []*labels.Selector
		order = append(order, &orderelem)

		return &rollout.Rollout{
			TypeMeta: api.TypeMeta{
				Kind: "Rollout",
			},
			ObjectMeta: api.ObjectMeta{
				Name: rolloutName,
			},
			Spec: rollout.RolloutSpec{
				Version:                   version,
				ScheduledStartTime:        scheduledStartTime,
				Duration:                  "",
				Strategy:                  "LINEAR",
				MaxParallel:               10,
				MaxNICFailuresBeforeAbort: 0,
				OrderConstraints:          order,
				Suspend:                   false,
				DSCsOnly:                  true,
				UpgradeType:               "Disruptive",
			},
		}, nil
	} else {
		return &rollout.Rollout{
			TypeMeta: api.TypeMeta{
				Kind: "Rollout",
			},
			ObjectMeta: api.ObjectMeta{
				Name: rolloutName,
			},
			Spec: rollout.RolloutSpec{
				Version:                   version,
				ScheduledStartTime:        scheduledStartTime,
				Duration:                  "",
				Strategy:                  "LINEAR",
				MaxParallel:               1,
				MaxNICFailuresBeforeAbort: 0,
				OrderConstraints:          nil,
				Suspend:                   false,
				DSCsOnly:                  false,
				UpgradeType:               "Disruptive",
			},
		}, nil
	}

}
