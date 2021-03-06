package base

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/labels"
	loginctx "github.com/pensando/sw/api/login/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	rolloutName = "e2e_rollout"
)

type BuildMeta struct {
	Repo            string
	Branch          string
	Prefix          string
	Label           string
	NextBuildNumber int
}

func getLatestBuildVersionInfo(targetBranch string) (string, int, error) {
	jsonURL := []string{"-s", "http://jobd.pensando.io:3456/tags"}
	out, err := exec.Command("curl", jsonURL...).CombinedOutput()
	outStr := strings.TrimSpace(string(out))
	fmt.Println(fmt.Sprintf("curl output: %s, err: %v\n", outStr, err))
	if err != nil {
		fmt.Println(fmt.Sprintf("curl Error: %s, err: %v\n", outStr, err))
		fmt.Println()
	}
	jsonResponseBytes := []byte(outStr)
	var buildMetas = []*BuildMeta{}
	err = json.Unmarshal(jsonResponseBytes, &buildMetas)
	if err != nil {
		return "", 0, fmt.Errorf("Unable to unmarshal response from jobd [%v]", err)
	}
	for _, bm := range buildMetas {
		if bm.Branch == targetBranch {
			return bm.Prefix, bm.NextBuildNumber, nil
		}
	}
	return "", 0, fmt.Errorf("No build metadata found for targetBranch: %s", targetBranch)
}

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

func getImageVersion(bundleType string) string {
	metadataFile := fmt.Sprintf("%s/src/github.com/pensando/sw/%s/metadata.json", os.Getenv("GOPATH"), bundleType)
	versionMap := processMetadataFile(metadataFile)
	if versionMap != nil {
		return versionMap["Bundle"]["Version"]
	}
	return ""
}

// CreateRolloutObject gets rollout instance
func (sm *SysModel) CreateRolloutObject(bundleType, name, upgradeType string) (*rollout.Rollout, error) {

	seconds := time.Now().Unix()
	scheduledStartTime := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
		},
	}

	version := getImageVersion(bundleType)
	log.Errorf("getImageVersion %s", version)
	if version == "" {
		log.Errorf("ts:%s Build Failure. Couldnt get version.json", time.Now().String())
		return nil, errors.New("Build Failure. Couldnt get version information")
	}

	return &rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: rollout.RolloutSpec{
			Version:                   version,
			ScheduledStartTime:        scheduledStartTime,
			ScheduledEndTime:          nil,
			Strategy:                  "LINEAR",
			MaxParallel:               1,
			MaxNICFailuresBeforeAbort: 2,
			OrderConstraints:          nil,
			Suspend:                   false,
			DSCsOnly:                  false,
			UpgradeType:               upgradeType,
		},
	}, nil
}

//GetClusterVersion returns the running version
func (sm *SysModel) GetClusterVersion() string {

	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)
	if err != nil {
		log.Errorf("Failed to get login context (%+v)", err)
		return ""
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		log.Errorf("Failed to get rest clients (%+v)", err)
		return ""
	}
	obj := api.ObjectMeta{
		Name: "version",
	}
	vobj, err := restcls[0].ClusterV1().Version().Get(ctx, &obj)
	if err != nil {
		log.Errorf("Failed to get cluster object (%+v)", err)
		return ""
	}
	log.Infof("Version Object %+v", vobj)
	return vobj.Status.BuildVersion
}

// GetRolloutObject gets rollout instance
func (sm *SysModel) GetRolloutObject(scaleData bool) (*rollout.Rollout, error) {

	version := ""
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
	}*/

	clusterVersion := sm.GetClusterVersion()

	bundleLocalFilePath := fmt.Sprintf("%s/src/github.com/pensando/sw/upgrade-bundle/bundle.tar", os.Getenv("GOPATH"))

	versionPrefix, buildNumber, err := getLatestBuildVersionInfo("master")
	if err != nil {
		log.Errorf("ts:%s Unable to fetch latest build version information: %s", time.Now().String(), err)
		return nil, errors.New("Unable to fetch latest build version information")
	}
	for b := buildNumber; b > 0; b-- {
		version = versionPrefix + "-" + strconv.Itoa(b)
		if version == clusterVersion {
			log.Errorf("Cluster running the same version %s", version)
			continue
		}
		log.Errorf("ts:%s Trying to download bundle.tar, version: %s clusterVersion %s", time.Now().String(), version, clusterVersion)
		url := fmt.Sprintf("http://pxe.pensando.io/builds/hourly/%s/bundle/bundle.tar", version)
		jsonUrl := []string{url, "--output", bundleLocalFilePath, "-f"}

		fmt.Println(fmt.Sprintf("curl string: %v\n", jsonUrl))
		out, err := exec.Command("curl", jsonUrl...).CombinedOutput()
		outStr := strings.TrimSpace(string(out))
		fmt.Println(fmt.Sprintf("curl output: %s, err: %v\n", outStr, err))
		if strings.Contains(outStr, "404 Not Found") || strings.Contains(outStr, " error: ") {
			log.Errorf("ts:%s Error when trying to download Bundle with version: %s, response: %s", time.Now().String(), version, outStr)
			fmt.Println(fmt.Sprintf("curl Error: %s, err: %v\n", outStr, err))
			fmt.Println()
		} else {
			fmt.Println(fmt.Sprintf("curl success: %s\n", outStr))
			fmt.Println()
			break
		}
	}

	log.Infof("ts:%s Successfully downloaded bundle.tar for version: %s", time.Now().String(), version)

	var req labels.Requirement
	req.Key = "type"
	req.Operator = "in"
	req.Values = append(req.Values, "bm")

	var orderelem labels.Selector
	orderelem.Requirements = append(orderelem.Requirements, &req)

	var order []*labels.Selector
	order = append(order, &orderelem)

	log.Errorf("rollout version %s", version)
	if version == "" {
		log.Errorf("ts:%s Build Failure. Couldnt get version.json", time.Now().String())
		return nil, errors.New("Build Failure. Couldnt get version information")
	}

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
				ScheduledEndTime:          nil,
				Strategy:                  "LINEAR",
				MaxParallel:               10,
				MaxNICFailuresBeforeAbort: 0,
				OrderConstraints:          order,
				Suspend:                   false,
				DSCsOnly:                  true,
				UpgradeType:               "Graceful",
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
				ScheduledEndTime:          nil,
				Strategy:                  "LINEAR",
				MaxParallel:               1,
				MaxNICFailuresBeforeAbort: 2,
				OrderConstraints:          nil,
				Suspend:                   false,
				DSCsOnly:                  false,
				UpgradeType:               "Graceful",
			},
		}, nil
	}

}

// PerformImageUpload triggers image upgrade
func (sm *SysModel) PerformImageUpload(bundleType string) error {

	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 10*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)
	if err != nil {
		return err
	}
	_, err = sm.UploadBundle(ctx, bundleType)
	if err != nil {
		log.Infof("Error (%+v) uploading bundle.", err)
		return err
	}
	return nil
}

// UploadBundle performs a rollout in the cluster
func (sm *SysModel) UploadBundle(ctx context.Context, bundleType string) (int, error) {

	rolloutFile := fmt.Sprintf("%s/src/github.com/pensando/sw/%s/bundle.tar", os.Getenv("GOPATH"), bundleType)
	log.Infof("ts:%s Rollout Upload file name [%s]", time.Now().String(), rolloutFile)
	r, w := io.Pipe()
	m := multipart.NewWriter(w)
	go func() {
		defer w.Close()
		defer m.Close()
		part, err := m.CreateFormFile("file", "bundle.tar")
		if err != nil {
			log.Infof("CreateFromFile failed %+v", err)
			return
		}
		file, err := os.Open(rolloutFile)
		if err != nil {
			log.Infof("Open of file %s failed %+v", rolloutFile, err)
			return
		}
		defer file.Close()
		if sz, err := io.Copy(part, file); err != nil {
			log.Infof("io copy failed %+v  %+v", sz, err)
			return
		}
	}()

	uri := fmt.Sprintf("https://%s/objstore/v1/uploads/images/", sm.GetVeniceURL()[0])
	req, err := http.NewRequest("POST", uri, r)
	if err != nil {
		return 0, fmt.Errorf("http.newRequest failed %v", err)
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return 0, fmt.Errorf("no authorization header in context")
	}
	req.Header.Set("Authorization", authzHeader)
	req.Header.Set("Content-Type", m.FormDataContentType())
	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}
	resp, err := client.Do(req)
	if err != nil {
		return 0, fmt.Errorf("Sending req %v", err)
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		body, _ := ioutil.ReadAll(resp.Body)
		log.Errorf("Did not get a success on upload [%v]", string(body))
		return 0, fmt.Errorf("failed to get upload [%v][%v]", resp.Status, string(body))
	}
	return 0, nil

}

var names []string

// VerifyRolloutStatus verifies status of rollout in the iota cluster
func (sm *SysModel) VerifyRolloutStatus(rolloutName string) error {
	var numRetries int
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 60*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)

	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}

	for numRetries = 0; numRetries < 24; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil || r1.Name != rolloutName {
			log.Errorf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1)
			time.Sleep(time.Second)
			continue
		}
		log.Infof("ts:%s Rollout GET validated for [%s]", time.Now().String(), rolloutName)
		numRetries = 0
		break
	}

	if numRetries != 0 {
		return fmt.Errorf("Failed to create rollout object")
	}

	// Verify pre-install rollout Node status
	for numRetries = 0; numRetries < 60; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Errorf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		if r1.Spec.DSCsOnly {
			break
		}
		status := r1.Status.GetControllerNodesStatus()
		if len(status) == 0 {
			log.Errorf("ts:%s Pre-install in progress", time.Now().String())
			time.Sleep(time.Second * 5)
			continue
		}
		log.Infof("ts:%s Pre-install completed for : %d nodes", time.Now().String(), len(status))
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout pre-check failed")
	}

	// Verify pre-install Node status
	for numRetries = 0; numRetries < 60; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Errorf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		var numNodes int
		if r1.Spec.DSCsOnly {
			break
		}
		status := r1.Status.GetControllerNodesStatus()
		if len(status) == 0 {
			log.Errorf("ts:%s Pre-install in progress", time.Now().String())
			time.Sleep(time.Second * 5)
			continue
		}

		for i := 0; i < len(status); i++ {
			log.Errorf("ts:%s Controller node status %+v", time.Now().String(), status[i])
			if status[i].Phase == rollout.RolloutPhase_PROGRESSING.String() {
				log.Errorf("ts:%s Controller node Pre-install Complete", time.Now().String())
				numNodes = len(sm.VeniceNodes().Nodes)
				break
			}
			if status[i].Phase == rollout.RolloutPhase_WAITING_FOR_TURN.String() {
				numNodes++
				continue
			}
		}

		if numNodes != len(sm.VeniceNodes().Nodes) {
			log.Errorf("ts:%s Pre-install completed for : %d nodes", time.Now().String(), numNodes)
			time.Sleep(time.Second * 5)
			continue
		}
		log.Infof("ts:%s Pre-install Status: Complete", time.Now().String())
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout pre-check not completed for all nodes")
	}

	// Verify pre-install of Naples
outerLoop:
	for numRetries = 0; numRetries < 60; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Errorf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		status := r1.Status.GetDSCsStatus()
		log.Infof("ts:%s Precheck smartNIC status len %d: status:  %+v", time.Now().String(), len(status), status)
		var numNodes int
		if len(status) == 0 || r1.Status.OperationalState == rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String() {
			log.Infof("ts:%s Precheck smartNIC in progress", time.Now().String())
			time.Sleep(time.Second * 5)
			continue
		}
		if r1.Status.OperationalState == rollout.RolloutStatus_PROGRESSING.String() ||
			r1.Status.OperationalState == rollout.RolloutStatus_FAILURE.String() ||
			r1.Status.OperationalState == rollout.RolloutStatus_SUCCESS.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			numRetries = 0
			break
		}

		for i := 0; i < len(status); i++ {
			log.Errorf("ts:%s. Naples Pre Install %+v", time.Now().String(), status[i])
			if status[i].Phase == rollout.RolloutPhase_WAITING_FOR_TURN.String() || status[i].Phase == rollout.RolloutPhase_COMPLETE.String() {
				numNodes++
			}
		}
		if r1.Status.OperationalState == rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String() {
			log.Errorf("ts:%s Pre-install in progress:completed for %d naples", time.Now().String(), numNodes)
			time.Sleep(time.Second * 5)
			continue
		}
		if numNodes < len(sm.Naples().Nodes) {
			log.Errorf("ts:%s Pre-install completed for : %d naples", time.Now().String(), numNodes)
			time.Sleep(time.Second * 5)
			continue
		}
		log.Infof("ts:%s Naples Pre-install Complete. Status ", time.Now().String(), r1.Status.OperationalState)
		numRetries = 0
		break
	}

	if numRetries != 0 {
		return fmt.Errorf("rollout pre-check failed")
	}

	// Verify Start Time of rollout object
	for numRetries = 0; numRetries < 20; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil || r1.Name != rolloutName {
			log.Infof("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		if r1.Status.StartTime == nil || r1.Spec.ScheduledStartTime.Seconds > r1.Status.StartTime.Seconds {
			log.Infof("ts:%s Waiting for pre-install to complete and to schedule rollout : %s status %+v", time.Now().String(), rolloutName, r1.Status)
			time.Sleep(time.Second * 5)
			continue
		}
		log.Infof("ts:%s Rollout successfully started at [%+v] for [%s]", time.Now().String(), time.Unix(r1.Status.StartTime.Seconds, 0), rolloutName)
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout failed to trigger")
	}

	// Verify rollout Node status
	for numRetries = 0; numRetries < 120; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 10)
			continue
		}
		if r1.Spec.DSCsOnly {
			break
		}

		status := r1.Status.GetControllerNodesStatus()
		var numNodes int
		if len(status) == 0 {
			log.Infof("ts:%s Rollout controller node status: not found", time.Now().String())
			time.Sleep(time.Second * 10)
			continue
		}

		for i := 0; i < len(status); i++ {
			if status[i].Phase == rollout.RolloutPhase_COMPLETE.String() {
				numNodes++
			}
		}
		if numNodes != len(status) {
			log.Infof("ts:%s Rollout completed for : %d nodes", time.Now().String(), numNodes)
			time.Sleep(time.Second * 10)
			continue
		}
		log.Infof("ts:%s Rollout Node Status: Complete", time.Now().String())
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout controller node failed")
	}

	// Verify rollout service status
	for numRetries = 0; numRetries < 180; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 10)
			continue
		}
		if r1.Spec.DSCsOnly {
			break
		}
		status := r1.Status.GetControllerServicesStatus()
		if len(status) == 0 {
			log.Infof("ts:%s Rollout controller services status: Not Found", time.Now().String())
			time.Sleep(time.Second * 10)
			continue
		}
		if status[0].Phase != rollout.RolloutPhase_COMPLETE.String() {
			log.Infof("ts:%s Rollout controller services status: : %+v", time.Now().String(), status[0].Phase)
			time.Sleep(time.Second * 10)
			continue
		}
		numRetries = 0
		log.Infof("ts:%s Rollout Controller Services Status: Complete", time.Now().String())
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout services failed on some nodes")
	}

	// Verify rollout smartNIC status
	for numRetries = 0; numRetries < 180; numRetries++ {
		restcls, err := sm.VeniceRestClient()
		if err != nil {
			log.Infof("ts:%s Failed to get restclient err %+v", time.Now().String(), err)
			time.Sleep(time.Second * 5)
			continue
		}
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		status := r1.Status.GetDSCsStatus()
		log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
		log.Infof("ts:%s Rollout smartNIC status len %d: status:  %+v", time.Now().String(), len(status), status)

		var numNodes int
		if len(status) == 0 {
			log.Infof("ts:%s Rollout smartNIC status: not found", time.Now().String())
			time.Sleep(time.Second * 5)
			continue
		}

		for i := 0; i < len(status); i++ {
			log.Infof("ts:%s SmartNIC Rollout status %s", time.Now().String(), status[i].Phase)
			if status[i].Phase == rollout.RolloutPhase_COMPLETE.String() || status[i].Reason != "success" {
				numNodes++
			}
		}

		if r1.Status.OperationalState == rollout.RolloutStatus_PRECHECK_IN_PROGRESS.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			time.Sleep(time.Second * 5)
			continue
		}

		if r1.Status.OperationalState == rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String() ||
			r1.Status.OperationalState == rollout.RolloutStatus_SUCCESS.String() ||
			r1.Status.OperationalState == rollout.RolloutStatus_FAILURE.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			numRetries = 0
			break
		}

		if numNodes != len(status) || r1.Status.OperationalState == rollout.RolloutStatus_PROGRESSING.String() {
			log.Infof("ts:%s SmartNIC Rollout completed for : %d nodes", time.Now().String(), numNodes)
			time.Sleep(time.Second * 5)
			continue
		}
		log.Infof("ts:%s Rollout Smart NIC Status: %s", time.Now().String(), r1.Status.OperationalState)
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout smartNIC node failed")
	}
	var opState string
	// Verify rollout in retry states status
	for numRetries = 0; numRetries < 60; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		opState = r1.Status.OperationalState

		if opState == rollout.RolloutStatus_PROGRESSING.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			time.Sleep(time.Second * 5)
			continue
		}
		if opState == rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			break
		}
		if opState == rollout.RolloutStatus_FAILURE.String() || opState == rollout.RolloutStatus_SUSPENDED.String() ||
			opState == rollout.RolloutStatus_SUCCESS.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			break
		}
	}

	if opState == rollout.RolloutStatus_SCHEDULED_FOR_RETRY.String() {

		go func() {
			if err := sm.AddNaplesNodes(names); err != nil {
				log.Errorf("Failed to add naples nodes %v error %v", names, err)
			}
		}()

		log.Infof("Rollout is scheduled for retry.. Going to precheck SmartNIC")
		time.Sleep(5 * time.Minute) //wait for rollout to trigger retry
		goto outerLoop
	}

	// Verify rollout overall status
	for numRetries = 0; numRetries < 60; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		opState := r1.Status.OperationalState

		if opState == rollout.RolloutStatus_PROGRESSING.String() || opState == rollout.RolloutStatus_SUSPEND_IN_PROGRESS.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			time.Sleep(time.Second * 5)
			continue
		}

		if opState == rollout.RolloutStatus_FAILURE.String() || opState == rollout.RolloutStatus_DEADLINE_EXCEEDED.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			return fmt.Errorf("rollout smartNIC node failed")
		}

		if opState == rollout.RolloutStatus_SUCCESS.String() || opState == rollout.RolloutStatus_SUSPENDED.String() {
			log.Infof("ts:%s Overall Rollout status: %s", time.Now().String(), r1.Status.OperationalState)
			break
		}

		log.Infof("ts:%s Overall Rollout Status: %s", time.Now().String(), opState)
		numRetries = 0
		break
	}

	log.Infof("ts:%s Verify DSC Versions", time.Now().String())
	snics, err := sm.ListSmartNIC()
	if err != nil {
		log.Errorf("Unable to list dsc")
		return err
	}
	for _, snic := range snics {
		log.Infof("DSC Version : %s", snic.Status.DSCVersion)
	}

	// Verify delete on rollout object
	for numRetries = 0; numRetries < 25; numRetries++ {
		obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
		r1, err := restcls[0].RolloutV1().Rollout().Get(ctx, &obj)
		if err != nil {
			log.Infof("ts:%s Rollout LIST failed during delete, err: %+v rollouts: %+v", time.Now().String(), err, r1)
			time.Sleep(time.Second * 5)
			continue
		}
		r1, err = restcls[0].RolloutV1().Rollout().RemoveRollout(ctx, r1)
		if err != nil || r1.Name != rolloutName {
			time.Sleep(time.Second)
			continue
		}
		log.Infof("ts:%s Rollout DELETE validated for [%s]", time.Now().String(), rolloutName)
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("rollout delete object failed")
	}

	// Verify GET for all rollout objects (LIST) returns empty
	for numRetries = 0; numRetries < 25; numRetries++ {
		ometa := api.ObjectMeta{Tenant: "default"}
		rollouts, err := restcls[0].RolloutV1().Rollout().List(ctx, &api.ListWatchOptions{ObjectMeta: ometa})
		if err != nil || len(rollouts) != 0 {
			log.Infof("ts:%s Rollout LIST has unexpected objects, err: %+v rollouts: %+v", time.Now().String(), err, rollouts)
			time.Sleep(time.Second)
			continue
		}
		numRetries = 0
		break
	}
	if numRetries != 0 {
		return fmt.Errorf("Error: rollout list returned objects")
	}
	return nil
}

// PerformRollout performs a rollout in the cluster
func (sm *SysModel) PerformRollout(rollout *rollout.Rollout, scaleData bool, bundleType string) error {
	bkCtx, cancelFunc := context.WithTimeout(context.Background(), 15*time.Minute)
	defer cancelFunc()
	ctx, err := sm.VeniceLoggedInCtx(bkCtx)
	if err != nil {
		return err
	}

	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return err
	}
	//cleanup the existing rollout object with the same name
	//fetch the image and upload
	err = sm.PerformImageUpload(bundleType)
	if err != nil {
		log.Infof("Errored PerformImageUpload")
		return err
	}
	names = make([]string, 0)
	for _, obj := range sm.Naples().Nodes {
		names = append(names, obj.NodeName())
		//just add one naples to simulate intent based rollout
		break
	}

	/*if err := sm.DeleteNaplesNodes(names); err != nil {
		log.Errorf("Failed to delete naples nodes %v", names)
		return err
	}*/
	//create the rollout object
	r1, err := restcls[0].RolloutV1().Rollout().CreateRollout(ctx, rollout)
	if err != nil || r1.Name != rollout.Name {
		return err
	}

	return nil
}
