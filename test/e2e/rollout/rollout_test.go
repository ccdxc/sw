// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rollout

import (
	"bytes"
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"math/rand"
	"mime/multipart"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"

	loginctx "github.com/pensando/sw/api/login/context"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/gogo/protobuf/types"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/rollout"

	"github.com/pkg/errors"
)

const (
	rolloutName        = "e2e_rollout"
	rolloutSuspendName = "e2e_rollout_suspend"
)

var version string

type BuildMeta struct {
	Repo            string
	Branch          string
	Prefix          string
	Label           string
	NextBuildNumber int
}

func uploadBundle(ctx context.Context, filename string, metadata map[string]string, content []byte) (int, error) {

	body := &bytes.Buffer{}
	writer := multipart.NewWriter(body)
	part, err := writer.CreateFormFile("file", filename)
	if err != nil {
		return 0, errors.Wrap(err, "CreateFormFile failed")
	}
	written, err := part.Write(content)
	if err != nil {
		return 0, errors.Wrap(err, "writing form")
	}

	for key, val := range metadata {
		_ = writer.WriteField(key, val)
	}
	err = writer.Close()
	if err != nil {
		return 0, errors.Wrap(err, "closing writer")
	}
	uri := fmt.Sprintf("https://%s/objstore/v1/uploads/images/", ts.tu.APIGwAddr)
	By(fmt.Sprintf("upload URI [%v]", uri))
	req, err := http.NewRequest("POST", uri, body)
	if err != nil {
		return 0, errors.Wrap(err, "http.newRequest failed")
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return 0, fmt.Errorf("no authorization header in context")
	}
	req.Header.Set("Authorization", authzHeader)
	req.Header.Set("Content-Type", writer.FormDataContentType())
	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}
	resp, err := client.Do(req)
	if err != nil {
		return 0, errors.Wrap(err, "Sending req")
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		body, _ := ioutil.ReadAll(resp.Body)
		By(fmt.Sprintf("Did not get a success on upload [%v]", string(body)))
		return 0, fmt.Errorf("failed to get upload [%v][%v]", resp.Status, string(body))
	}
	return written, nil
}

func checkNodeRolloutStatus() error {
	obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil {
		By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1))
		return errors.New("Unable to GET rollout status")
	}

	By(fmt.Sprintf("Able to GET rollout status, proceeding with checking node rollout status"))
	statuses := r1.Status.GetControllerNodesStatus()
	var (
		preCheckNodeCount,
		dependenciesCheckNodeCount,
		completeNodeCount,
		progressingNodeCount,
		failedNodeCount,
		waitingForTurnNodeCount int
	)
	if len(statuses) == 0 {
		By(fmt.Sprintf("ts:%s Rollout controller node status: not found", time.Now().String()))
		return errors.New("No Rollout status found")
	}

	for _, status := range statuses {
		switch strings.Trim(strings.ToLower(status.Phase), " \t") {
		case "pre-check":
			preCheckNodeCount++
		case "dependencies-check":
			dependenciesCheckNodeCount++
		case "waiting-for-turn":
			waitingForTurnNodeCount++
		case "progressing":
			progressingNodeCount++
		case "complete":
			completeNodeCount++
		case "fail":
			failedNodeCount++
		default:
			By(fmt.Sprintf("Found unexpected node rollout phase:(%s)", status.Phase))
			return fmt.Errorf("Invalid node rollout status:(%s)", status.Phase)
		}
	}

	nodeStatusCounts := map[string]int{
		"PRE_CHECK":          preCheckNodeCount,
		"DEPENDENCIES_CHECK": dependenciesCheckNodeCount,
		"WAITING_FOR_TURN":   waitingForTurnNodeCount,
		"PROGRESSING":        progressingNodeCount,
		"COMPLETE":           completeNodeCount,
		"FAIL":               failedNodeCount,
	}

	for k, v := range nodeStatusCounts {
		By(fmt.Sprintf("Node status: %s, count: %d", k, v))
	}

	if failedNodeCount > 0 {
		return fmt.Errorf("Found %d nodes with rollout failures", failedNodeCount)
	}

	if completeNodeCount < len(statuses) {
		waitingorProgressingNodes := preCheckNodeCount + dependenciesCheckNodeCount + waitingForTurnNodeCount + progressingNodeCount
		return fmt.Errorf("Waiting for %d nodes to finish rollout", waitingorProgressingNodes)
	}

	By(fmt.Sprintf("ts:%s Rollout completed on all nodes", time.Now().String()))
	return nil
}

func downloadVeniceBundle(bundleLocalFilePath string) (string, error) {
	var version string
	err := deleteBundleIfExists(bundleLocalFilePath)
	if err != nil {
		By(fmt.Sprintf("ts:%s Error trying to deleting bundle file, error: ", err.Error()))
		return "", err
	}
	node := ts.tu.QuorumNodes[rand.Intn(len(ts.tu.QuorumNodes))]
	nodeIP := ts.tu.NameToIPMap[node]
	versionPrefix, buildNumber, err := getLatestBuildVersionInfo(nodeIP)
	if err != nil {
		By(fmt.Sprintf("ts:%s Unable to fetch latest build version information: %s", time.Now().String(), err.Error()))
		return "", errors.Wrap(err, "Unable to fetch latest build version information")
	}
	for b := buildNumber; b > 0; b-- {
		version = versionPrefix + "-" + strconv.Itoa(b)
		By(fmt.Sprintf("ts:%s Trying to download bundle.tar, version: %s", time.Now().String(), version))
		url := fmt.Sprintf("http://pxe.pensando.io/builds/hourly/%s/bundle/bundle.tar --output %s -f", version, bundleLocalFilePath)
		res := ts.tu.CommandOutputIgnoreError(nodeIP, fmt.Sprintf(`curl %s`, url))

		if strings.Contains(res, "404 Not Found") || strings.Contains(res, " error: ") {
			By(fmt.Sprintf("ts:%s Error when trying to download Bundle with version: %s, response: %s", time.Now().String(), version, res))
		} else {
			By(fmt.Sprintf("ts:%s Successfully downloaded bundle.tar for version: %s", time.Now().String(), version))
			By(res)
			return version, nil
		}
	}
	return "", errors.Errorf("Unable to download image with version prefix: %s", versionPrefix)
}

func deleteBundleIfExists(filename string) error {
	_, err := os.Stat(filename)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return errors.Wrap(err, fmt.Sprintf("Unexpected error when trying to get stats for file: %s", filename))
	}
	By(fmt.Sprintf("Found file %s, deleting it.", filename))
	err = os.Remove(filename)
	if err != nil {
		return errors.Wrap(err, fmt.Sprintf("Unable to delete the file: %s", filename))
	}
	return nil
}

func getLatestBuildVersionInfo(nodeIP string) (string, int, error) {
	targetBranch := "master"
	jsonResponse := ts.tu.LocalCommandOutput(`curl -s http://jobd.pensando.io:3456/tags`)
	By(fmt.Sprintf("ts:%s BuildMeta response from jobd: %s", time.Now().String(), jsonResponse))
	jsonResponseBytes := []byte(jsonResponse)
	var buildMetas = []*BuildMeta{}
	err := json.Unmarshal(jsonResponseBytes, &buildMetas)
	if err != nil {
		return "", 0, errors.Wrap(err, "Unable to unmarshal response from jobd")
	}
	for _, bm := range buildMetas {
		if bm.Branch == targetBranch {
			return bm.Prefix, bm.NextBuildNumber, nil
		}
	}
	return "", 0, errors.Errorf("No build metadata found for targetBranch: %s", targetBranch)
}

func deleteRolloutObject() error {
	obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil {
		By(fmt.Sprintf("ts:%s Rollout GET failed during delete, err: %+v rollouts: %+v", time.Now().String(), err, r1))
		return errors.Wrap(err, "Unable to GET rollout object")
	}

	r1, err = ts.restSvc.RolloutV1().Rollout().RemoveRollout(ts.loggedInCtx, r1)
	if err != nil || r1.Name != rolloutName {
		By(fmt.Sprintf("ts:%s Rollout DELETE failed, err: %+v rollouts: %+v", time.Now().String(), err, r1))
		return errors.Wrap(err, "Unable to delete rollout object")
	}
	By(fmt.Sprintf("ts:%s Rollout DELETE validated for [%s]", time.Now().String(), rolloutName))
	return nil
}

func isServiceRolloutComplete() bool {
	obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil {
		By(fmt.Sprintf("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1))
		return false
	}
	status := r1.Status.GetControllerServicesStatus()
	if len(status) == 0 {
		By(fmt.Sprintf("ts:%s Rollout controller services status: Not Found", time.Now().String()))
		return false
	}
	if strings.Trim(strings.ToLower(status[0].Phase), " \t") != "complete" {
		By(fmt.Sprintf("ts:%s Rollout controller services status: : %+v", time.Now().String(), status[0].Phase))
		return false
	}

	By(fmt.Sprintf("ts:%s Rollout Controller Services Status: Complete", time.Now().String()))

	return true
}

func isRolloutStartTimeCorrect() bool {
	obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil || r1.Name != rolloutName {
		By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
		return false
	}
	if r1.Status.StartTime == nil || r1.Spec.ScheduledStartTime.Seconds > r1.Status.StartTime.Seconds {
		By(fmt.Sprintf("ts:%s Waiting for pre-install to complete and to schedule rollout : %s", time.Now().String(), rolloutName))
		return false
	}
	By(fmt.Sprintf("ts:%s Rollout successfully started at [%+v] for [%s]", time.Now().String(), time.Unix(r1.Status.StartTime.Seconds, 0), rolloutName))
	return true
}

func isPreinstallDone() bool {
	obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil {
		By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, r1))
		return false
	}
	if len(r1.Status.GetControllerNodesStatus()) == 0 || r1.Status.OperationalState == "precheck-in-progress" {
		By(fmt.Sprintf("ts:%s Pre-install in progress", time.Now().String()))
		return false
	}
	By(fmt.Sprintf("ts:%s Pre-install completed, current rollout state: %s", time.Now().String(), r1.Status.OperationalState))
	return true
}

func isRolloutObjectPresent(rName string) bool {
	obj := api.ObjectMeta{Name: rName, Tenant: "default"}
	r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil || r1.Name != rName {
		By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rName, err, r1))
		return false
	}
	By(fmt.Sprintf("ts:%s Rollout GET validated for [%s]", time.Now().String(), rName))
	return true
}

func createRolloutObject(rolloutObj *rollout.Rollout) error {
	r1, err := ts.restSvc.RolloutV1().Rollout().CreateRollout(ts.loggedInCtx, rolloutObj)
	if err != nil || r1.Name != rolloutName {
		By(fmt.Sprintf("ts:%s Rollout CREATE failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
		return errors.Wrap(err, "Unable to CREATE rollout")
	}
	By(fmt.Sprintf("ts:%s Rollout created at [%s] and scheduled at [%+v] for [%s]", time.Now().String(), time.Unix(r1.CreationTime.Seconds, 0), time.Unix(r1.Spec.ScheduledStartTime.Seconds, 0), rolloutName))
	return nil
}

func cleanupRolloutObjects(objMeta api.ObjectMeta) error {
	rollouts, err := ts.restSvc.RolloutV1().Rollout().List(ts.loggedInCtx, &api.ListWatchOptions{ObjectMeta: objMeta})
	if err != nil {
		By(fmt.Sprintf("ts:%s Unable to LIST rollout objects for cleanup, err: %+v meta: %+v", time.Now().String(), err, objMeta))
		return errors.Wrap(err, "Unable to LIST rollout objects for cleanup")
	}
	By(fmt.Sprintf("Found %d rollout objects for cleanup, rolloutObjs: %+v", len(rollouts), rollouts))
	for _, r := range rollouts {
		_, deleteErr := ts.restSvc.RolloutV1().Rollout().RemoveRollout(ts.loggedInCtx, r)
		if deleteErr != nil {
			By(fmt.Sprintf("ts:%s Rollout DELETE failed, err: %+v rollouts: %+v", time.Now().String(), deleteErr, r))
			return errors.Wrap(deleteErr, "Unable to DELETE rollout")
		}
	}
	return nil
}

func suspendRollout(rolloutObj rollout.Rollout) bool {
	rolloutObj.Spec.Suspend = true
	r1, err := ts.restSvc.RolloutV1().Rollout().StopRollout(ts.loggedInCtx, &rolloutObj)
	if err != nil || r1.Name != rolloutSuspendName {
		By(fmt.Sprintf("ts:%s Rollout Update failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
		return false
	}
	obj := api.ObjectMeta{Name: rolloutSuspendName, Tenant: "default"}
	r1, err = ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
	if err != nil || r1.Name != rolloutSuspendName {
		By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
		return false
	}

	if r1.Spec.GetSuspend() {
		By(fmt.Sprintf("ts:%s Rollout suspended for [%s]", time.Now().String(), rolloutSuspendName))
		return true
	}
	return false
}

// run the tests
var _ = Describe("Rollout object tests", func() {

	Context("Rollout tests", func() {

		// setup
		It("Rollout setup: upload bundle should succeed", func() {

			fileName := "bundle.tar"
			bundleLocalFilePath := "/import/src/github.com/pensando/sw/bin/" + fileName
			var err error
			version, err = downloadVeniceBundle(bundleLocalFilePath)
			Expect(err).Should(BeNil(), fmt.Sprintf("Failed to find and download a venice bundle"))

			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			metadata := map[string]string{
				"Version":     version,
				"Environment": "production",
				"Description": "E2E rollout test Image upload",
				"Releasedate": "Feb2020",
			}
			fileContent, err := ioutil.ReadFile(bundleLocalFilePath)
			if err != nil {
				log.Infof("Error (%+v) reading file %s", err, bundleLocalFilePath)
				Fail(fmt.Sprintf("Failed to read %s", bundleLocalFilePath))
			}

			Eventually(func() error {
				_, err = uploadBundle(ctx, fileName, metadata, fileContent)
				return err
			}, 300, 5).Should(BeNil(), fmt.Sprintf("Failed to upload file %s due to (%s)", bundleLocalFilePath, err))
			By(fmt.Sprintf("ts:%s Successfully uploaded bundle to VOS", time.Now().String()))
		})

		// run tests
		It("Rollout operations should succeed", func() {
			seconds := time.Now().Unix()
			scheduledStartTime := &api.Timestamp{
				Timestamp: types.Timestamp{
					Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
				},
			}

			rolloutObj := rollout.Rollout{
				TypeMeta: api.TypeMeta{
					Kind: "Rollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name: rolloutName,
				},
				Spec: rollout.RolloutSpec{
					Version:                   version,
					ScheduledStartTime:        scheduledStartTime,
					Strategy:                  "LINEAR",
					MaxParallel:               0,
					MaxNICFailuresBeforeAbort: 0,
					OrderConstraints:          nil,
					Suspend:                   false,
					DSCsOnly:                  false,
					DSCMustMatchConstraint:    true, // hence venice upgrade only
					UpgradeType:               "Disruptive",
				},
			}

			Eventually(func() error {
				oMeta := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				return cleanupRolloutObjects(oMeta)
			}, 30, 1).Should(BeNil(), fmt.Sprintf("Failed to cleanup %s object", rolloutName))

			Eventually(func() error {
				return createRolloutObject(&rolloutObj)
			}, 30, 1).Should(BeNil(), fmt.Sprintf("Failed to create %s object", rolloutName))

			Eventually(func() bool {
				return isRolloutObjectPresent(rolloutName)
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to verify presence of %s object", rolloutName))

			Eventually(isPreinstallDone, 600, 5).Should(BeTrue(), "Failed to verify pre-install step status for controller node")
			Eventually(checkNodeRolloutStatus, 1500, 5).Should(BeNil(), "Failed to verify rollout status on all nodes")
			Eventually(isRolloutStartTimeCorrect, 100, 5).Should(BeTrue(), fmt.Sprintf("Failed to validate scheduled rollout for object %s", rolloutName))
			Eventually(isServiceRolloutComplete, 600, 10).Should(BeTrue(), "Failed to verify rollout status for controller services")
			Eventually(deleteRolloutObject, 60, 5).Should(BeNil(), fmt.Sprintf("Failed to delete %s object", rolloutName))
			Eventually(func() bool {
				return isRolloutObjectPresent(rolloutName)
			}, 30, 1).Should(BeFalse(), fmt.Sprintf("Failed to verify the absence of %s object", rolloutName))
		})

		It("Rollout suspend operations should succeed", func() {
			Skip(fmt.Sprintf("Skipping suspend rollout tests"))
			if ts.tu.NumQuorumNodes < 2 {
				Skip(fmt.Sprintf("Skipping suspend rollout tests :%d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
			}

			rolloutObj := rollout.Rollout{
				TypeMeta: api.TypeMeta{
					Kind: "Rollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name: rolloutSuspendName,
				},
				Spec: rollout.RolloutSpec{
					Version:                   version,
					ScheduledStartTime:        nil,
					Strategy:                  "LINEAR",
					MaxParallel:               0,
					MaxNICFailuresBeforeAbort: 0,
					OrderConstraints:          nil,
					Suspend:                   false,
					DSCsOnly:                  false,
					DSCMustMatchConstraint:    true, // hence venice upgrade only
					UpgradeType:               "Disruptive",
				},
			}

			Eventually(func() error {
				return createRolloutObject(&rolloutObj)
			}).Should(BeNil(), fmt.Sprintf("Failed to create %s object", rolloutSuspendName))

			Eventually(func() bool {
				return isRolloutObjectPresent(rolloutSuspendName)
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", rolloutSuspendName))

			// Verify suspend for rollout object
			Eventually(func() bool {
				//Sleep 20 seconds
				time.Sleep(20 * time.Second)
				//Now Suspend rollout
				return suspendRollout(rolloutObj)
			}).Should(BeTrue(), fmt.Sprintf("Failed to suspend rollout %s object", rolloutSuspendName))

			// Verify Status of rollout
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutSuspendName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}

				time.Sleep(20 * time.Second)
				count := 0

				r2, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r2.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r2: %+v", time.Now().String(), rolloutSuspendName, err, r2))
					return false
				}

				for _, r1Status := range r1.Status.ControllerNodesStatus {
					for _, r2Status := range r2.Status.ControllerNodesStatus {
						if r1Status.Name == r2Status.Name && r1Status.Phase != r2Status.Phase {
							count++
						}
					}
				}

				//LINEAR Rollout Scheme

				if count > 1 {
					By(fmt.Sprintf("ts:%s More than one rollout object changed status : %v", time.Now().String(), count))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout successfully suspended for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to suspend rollout for object %s", rolloutSuspendName))
		})
		// cleanup
		AfterEach(func() {
			// Cleanup rollout objects regardless of test outcome
			By(fmt.Sprintf("ts:%s Test completed cleaning up rollout objects if any", time.Now().String()))

			Eventually(func() error {
				oMeta := api.ObjectMeta{Tenant: "default"}
				return cleanupRolloutObjects(oMeta)
			}, 60, 5).Should(BeNil(), "Encountered errors while trying to cleanup rollout objects")

		})
	})
})
