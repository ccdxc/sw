// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"time"

	objstore "github.com/pensando/sw/venice/utils/objstore/client"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/gogo/protobuf/types"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"

	"github.com/pensando/sw/api/generated/rollout"
)

const (
	rolloutName        = "e2e_rollout"
	rolloutSuspendName = "e2e_rollout_suspend"
)

var version string

type ImageConfig struct {
	ImageMap                map[string]string   `json:"imageMap,omitempty"`
	UpgradeOrder            []string            `json:"upgradeOrder,omitempty"`
	SupportedNaplesVersions map[string][]string `json:"supportedNaplesVersions,omitempty"`
	GitVersion              map[string]string   `json:"cmdVersionMap,omitempty"`
}

func readImageConfigFile(imageConfig *ImageConfig) error {
	confFile := "/import/src/github.com/pensando/sw/bin/venice.json"
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

func uploadBundleImg(ctx context.Context, client objstore.Client, rootDir string, version string, imageName string, objStoreFile string) error {

	imageFile := rootDir + "/" + imageName
	objectStoreFileName := "Bundle/" + version + "_img/" + objStoreFile
	return UploadImage(ctx, client, version, imageFile, objectStoreFileName)

}

func uploadVeniceImg(ctx context.Context, client objstore.Client, rootDir string, version string, imageName string, objStoreFile string) error {

	imageFile := rootDir + "/" + imageName
	objectStoreFileName := "Venice/" + version + "_img/" + objStoreFile
	return UploadImage(ctx, client, version, imageFile, objectStoreFileName)

}

//UploadImage uploads a given image to objectStore /rootDir/version/imageName should give the path to the file
func UploadImage(ctx context.Context, client objstore.Client, version string, imageName string, objStoreFileName string) error {

	meta := make(map[string]string)
	imageFile := imageName

	_, err := client.StatObject(objStoreFileName)
	if err == nil {
		By(fmt.Sprintf("Image (%s) exists in objectStore", objStoreFileName))
		log.Errorf("Image (%s) exists in the object store", objStoreFileName)
		return nil
	}

	meta["Version"] = version
	meta["Environment"] = "production"
	meta["Description"] = objStoreFileName
	meta["ReleaseDate"] = "May2018"

	buf, err := ioutil.ReadFile(imageFile)
	if err != nil {
		By(fmt.Sprintf("Error (%+v) reading file %s", err, imageFile))
		return err
	}
	_, err = client.PutObject(context.Background(), objStoreFileName, bytes.NewBuffer(buf), meta)
	if err != nil {
		By(fmt.Sprintf("UploadImage: Could not put object (%s) to datastore", err))
		return err
	}
	By(fmt.Sprintf("Uploaded file %s to objectStore", imageFile))

	return nil
}

// run the tests
var _ = Describe("rollout object tests", func() {

	Context("Rollout tests", func() {

		// setup
		It("Rollout setup: upload image should succeed", func() {
			Skip(fmt.Sprintf("Skipping upload venice image test"))
			node := ts.tu.QuorumNodes[rand.Intn(len(ts.tu.QuorumNodes))]
			nodeIP := ts.tu.NameToIPMap[node]
			url := fmt.Sprintf("http://pxe.pensando.io/kickstart/veniceImageForRollout/venice.tgz --output /import/src/github.com/pensando/sw/bin/venice.upg.tgz")
			res := ts.tu.CommandOutput(nodeIP, fmt.Sprintf(`curl %s`, url))
			By(fmt.Sprintf("ts:%s CURL image download [%s]", time.Now().String(), res))

			url = fmt.Sprintf("http://pxe.pensando.io/kickstart/veniceImageForRollout/venice.json --output /import/src/github.com/pensando/sw/bin/venice.json")
			res = ts.tu.CommandOutput(nodeIP, fmt.Sprintf(`curl %s`, url))
			By(fmt.Sprintf("ts:%s CURL venice.json download [%s]", time.Now().String(), res))

			cmdVersion := GetGitVersion()
			for version = range cmdVersion {
				break
			}
			if version == "" {
				By(fmt.Sprintf("ts:%s Build Failure. Couldnt get version.json", time.Now().String()))
				err := errors.New("Build Failure. Couldnt get version information")
				Expect(err).Should(BeNil(), "Build Failure. Couldnt get version from version.json")

			}
			// location of the objstore.
			err := ts.tu.SetupObjstoreClient()
			filename := "venice.upg.tgz"

			By(fmt.Sprintf("\nts:%s uploading image to object store..", time.Now().String()))
			ctx := ts.tu.MustGetLoggedInContext(context.Background())
			err = uploadVeniceImg(ctx, ts.tu.VOSClient, "/import/src/github.com/pensando/sw/bin/", version, filename, "venice.tgz")
			Expect(err).Should(BeNil(), "Failed to upload file")
			meta := map[string]map[string]string{
				"Bundle": {"Version": "2.0",
					"Description": "Meta File",
					"ReleaseDate": "May2019",
					"Name":        "metadata.json"},

				"Venice": {"Version": version,
					"Description": "Venice Image",
					"ReleaseDate": "May2019",
					"Name":        "venice.tgz"},

				"Naples": {"Version": "5.1",
					"Description": "Naples Image",
					"ReleaseDate": "May2019",
					"Name":        "naples_fw.tar"}}

			b, err := json.Marshal(meta)
			if err != nil {
				By(fmt.Sprintf("ts:%s Failed to marshal meta info", time.Now().String()))
				err := errors.New("Failed to marshal meta info")
				Expect(err).Should(BeNil(), "Failed to marshal meta info")
			}

			err = ioutil.WriteFile("/tmp/metadata.json", b, 0644)
			if err != nil {
				By(fmt.Sprintf("ts:%s Failed to marshal meta info", time.Now().String()))
				err := errors.New("Failed to marshal meta info")
				Expect(err).Should(BeNil(), "Failed to marshal meta info")
			}
			err = uploadBundleImg(ctx, ts.tu.VOSClient, "/tmp/", "2.0", "metadata.json", "metadata.json")
			Expect(err).Should(BeNil(), "Failed to upload file")

		})

		// run tests
		It("Rollout operations should succeed", func() {
			Skip(fmt.Sprintf("Skipping rollout tests"))
			seconds := time.Now().Unix()
			scheduledStartTime := &api.Timestamp{
				Timestamp: types.Timestamp{
					Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
				},
			}

			rollout := rollout.Rollout{
				TypeMeta: api.TypeMeta{
					Kind: "Rollout",
				},
				ObjectMeta: api.ObjectMeta{
					Name: rolloutName,
				},
				Spec: rollout.RolloutSpec{
					Version:                   "2.0",
					ScheduledStartTime:        scheduledStartTime,
					Strategy:                  "LINEAR",
					MaxParallel:               0,
					MaxNICFailuresBeforeAbort: 0,
					OrderConstraints:          nil,
					Suspend:                   false,
					DSCsOnly:                  false,
					DSCMustMatchConstraint:    true, // hence venice upgrade only
					UpgradeType:               "Graceful",
				},
			}
			// Verify creation for rollout object
			Eventually(func() bool {
				oMeta := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				_, _ = ts.tu.APIClient.RolloutV1().RolloutAction().Delete(ts.loggedInCtx, &oMeta)
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", rolloutName))

			// Verify creation for rollout object
			Eventually(func() bool {
				r1, err := ts.restSvc.RolloutV1().Rollout().CreateRollout(ts.loggedInCtx, &rollout)
				if err != nil || r1.Name != rolloutName {
					By(fmt.Sprintf("ts:%s Rollout CREATE failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout created at [%s] and scheduled at [%+v] for [%s]", time.Now().String(), time.Unix(r1.CreationTime.Seconds, 0), time.Unix(r1.Spec.ScheduledStartTime.Seconds, 0), rolloutName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", rolloutName))

			// Verify GET for rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout GET validated for [%s]", time.Now().String(), rolloutName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", rolloutName))

			// Verify pre-install rollout Node status
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return false
				}
				status := rollout.Status.GetControllerNodesStatus()
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Pre-install in progress", time.Now().String()))
					return false
				}
				By(fmt.Sprintf("ts:%s Pre-install completed for : %d nodes", time.Now().String(), len(status)))
				return true
			}, 300, 5).Should(BeTrue(), "Failed to finish pre-install for controller node")

			// Verify rollout Node status
			Eventually(func() error {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return errors.New("No rollout object found")
				}
				var numNodes int
				status := rollout.Status.GetControllerNodesStatus()
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Pre-install in progress", time.Now().String()))
					return errors.New("No controller node status found")
				}

				for i := 0; i < len(status); i++ {
					if status[i].Phase != "WAITING_FOR_TURN" {
						By(fmt.Sprintf("ts:%s Controller node Pre-install Failed", time.Now().String()))
						return errors.New("Controller Pre-install Failed")
					}
					numNodes++
				}

				if numNodes != ts.tu.NumQuorumNodes {
					By(fmt.Sprintf("ts:%s Pre-install completed for : %d nodes", time.Now().String(), numNodes))
					return errors.New("Controller Pre-install continues")
				}
				By(fmt.Sprintf("ts:%s Pre-install Status: Complete", time.Now().String()))
				return nil
			}, 300, 5).Should(BeNil(), "Failed to complete Pre-install")

			// Verify Start Time of rollout object
			Eventually(func() bool {
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
			}, 100, 5).Should(BeTrue(), fmt.Sprintf("Failed to validate scheduled rollout for object %s", rolloutName))

			// Verify rollout Node status
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout GET failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return false
				}

				status := rollout.Status.GetControllerNodesStatus()
				var numNodes int
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Rollout controller node status: not found", time.Now().String()))
					return false
				}

				for i := 0; i < len(status); i++ {
					if status[i].Phase == "COMPLETE" {
						numNodes++
					}
				}
				if numNodes != len(status) {
					By(fmt.Sprintf("ts:%s Rollout completed for : %d nodes", time.Now().String(), numNodes))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout Node Status: Complete", time.Now().String()))

				return true
			}, 500, 5).Should(BeTrue(), "Failed to finish rollout controller node")

			// Verify rollout service status
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				rollout, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil {
					By(fmt.Sprintf("ts:%s Rollout LIST failed for status check, err: %+v rollouts: %+v", time.Now().String(), err, rollout))
					return false
				}
				status := rollout.Status.GetControllerServicesStatus()
				if len(status) == 0 {
					By(fmt.Sprintf("ts:%s Rollout controller services status: Not Found", time.Now().String()))
					return false
				}
				if status[0].Phase != "COMPLETE" {
					By(fmt.Sprintf("ts:%s Rollout controller services status: : %+v", time.Now().String(), status[0].Phase))
					return false
				}

				By(fmt.Sprintf("ts:%s Rollout Controller Services Status: Complete", time.Now().String()))

				return true
			}, 300, 5).Should(BeTrue(), "Failed to finish rollout for controller services")

			// Verify delete on rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Delete(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutName {
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout DELETE validated for [%s]", time.Now().String(), rolloutName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to delete %s object", rolloutName))

			// Verify GET for all rollout objects (LIST) returns empty
			Eventually(func() bool {

				ometa := api.ObjectMeta{Tenant: "default"}
				rollouts, err := ts.restSvc.RolloutV1().Rollout().List(ts.loggedInCtx, &api.ListWatchOptions{ObjectMeta: ometa})
				if err != nil || len(rollouts) != 0 {
					By(fmt.Sprintf("ts:%s Rollout LIST has unexpected objects, err: %+v rollouts: %+v", time.Now().String(), err, rollouts))
					return false
				}
				return true
			}, 30, 1).Should(BeTrue(), "Unexpected rollout objects found")

		})

		It("Rollout suspend operations should succeed", func() {
			Skip(fmt.Sprintf("Skipping suspend rollout tests"))
			if ts.tu.NumQuorumNodes < 2 {
				Skip(fmt.Sprintf("Skipping suspend rollout tests :%d quorum nodes found, need >= 2", ts.tu.NumQuorumNodes))
			}

			rollout := rollout.Rollout{
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
					UpgradeType:               "Graceful",
				},
			}

			// Verify creation for rollout object
			Eventually(func() bool {
				r1, err := ts.restSvc.RolloutV1().Rollout().CreateRollout(ts.loggedInCtx, &rollout)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout CREATE failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout created for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}).Should(BeTrue(), fmt.Sprintf("Failed to create %s object", rolloutSuspendName))

			// Verify GET for rollout object
			Eventually(func() bool {
				obj := api.ObjectMeta{Name: rolloutSuspendName, Tenant: "default"}
				r1, err := ts.restSvc.RolloutV1().Rollout().Get(ts.loggedInCtx, &obj)
				if err != nil || r1.Name != rolloutSuspendName {
					By(fmt.Sprintf("ts:%s Rollout GET failed for [%s] err: %+v r1: %+v", time.Now().String(), rolloutSuspendName, err, r1))
					return false
				}
				By(fmt.Sprintf("ts:%s Rollout GET validated for [%s]", time.Now().String(), rolloutSuspendName))
				return true
			}, 30, 1).Should(BeTrue(), fmt.Sprintf("Failed to get %s object", rolloutSuspendName))

			// Verify suspend for rollout object
			Eventually(func() bool {
				//Sleep 20 seconds
				time.Sleep(20 * time.Second)
				//Now Suspend rollout
				rollout.Spec.Suspend = true
				r1, err := ts.restSvc.RolloutV1().Rollout().StopRollout(ts.loggedInCtx, &rollout)
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
			obj1 := api.ObjectMeta{Name: rolloutName, Tenant: "default"}
			ts.restSvc.RolloutV1().Rollout().Delete(ts.loggedInCtx, &obj1)
		})
	})
})
