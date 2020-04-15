package vchub

import (
	"context"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/vmware/govmomi"
	"github.com/vmware/govmomi/vapi/rest"
	"github.com/vmware/govmomi/vapi/tags"
	"github.com/vmware/govmomi/vim25/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/sim"
	smmock "github.com/pensando/sw/venice/ctrler/orchhub/statemgr"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTagSync(t *testing.T) {
	logger := setupLogger("tagSync_test")

	tagSyncEventRecorder := mockevtsrecorder.NewRecorder("vchub_test",
		log.GetNewLogger(log.GetDefaultConfig("vchub_test")))
	_ = recorder.Override(tagSyncEventRecorder)

	var vchub *VCHub
	var s *sim.VcSim
	var err error

	defer func() {
		logger.Infof("Tearing Down")
		if vchub != nil {
			vchub.Destroy(false)
		}
		if s != nil {
			s.Destroy()
		}
	}()

	u := createURL(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)

	s, err = sim.NewVcSim(sim.Config{Addr: u.String()})
	AssertOk(t, err, "Failed to create vcsim")
	dc1, err := s.AddDC(defaultTestParams.TestDCName)
	AssertOk(t, err, "failed dc create")
	dc2 := "DC2"
	_, err = s.AddDC(dc2)
	AssertOk(t, err, "failed dc create")

	sm, _, err := smmock.NewMockStateManager()
	if err != nil {
		s.Destroy()
		t.Fatalf("Failed to create state manager. Err : %v", err)
		return
	}

	clusterConfig := &cluster.Cluster{
		ObjectMeta: api.ObjectMeta{
			Name: "testCluster",
		},
		TypeMeta: api.TypeMeta{
			Kind: "Cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
		},
	}

	err = sm.Controller().Cluster().Create(clusterConfig)
	AssertOk(t, err, "failed to create cluster config")
	clusterItems, err := sm.Controller().Cluster().List(context.Background(), &api.ListWatchOptions{})
	AssertOk(t, err, "failed to get cluster config")

	clusterID := defs.CreateClusterID(clusterItems[0].Cluster)

	orchConfig := smmock.GetOrchestratorConfig(defaultTestParams.TestHostName, defaultTestParams.TestUser, defaultTestParams.TestPassword)
	orchConfig.Spec.ManageNamespaces = []string{utils.ManageAllDcs}

	err = sm.Controller().Orchestrator().Create(orchConfig)
	AssertOk(t, err, "failed to create orch config")

	vchub = LaunchVCHub(sm, orchConfig, logger, WithTagSyncDelay(2*time.Second))

	// Wait for it to come up
	AssertEventually(t, func() (bool, interface{}) {
		return vchub.IsSyncDone(), nil
	}, "VCHub sync never finished")

	defer func() {
		vchub.Destroy(false)
		defer s.Destroy()
	}()

	orchInfo1 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
	}
	_, err = smmock.CreateNetwork(sm, "default", "n1", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo1)
	AssertOk(t, err, "Failed to create network")
	orchInfo2 := []*network.OrchestratorInfo{
		{
			Name:      orchConfig.Name,
			Namespace: defaultTestParams.TestDCName,
		},
		{
			Name:      orchConfig.Name,
			Namespace: dc2,
		},
	}
	_, err = smmock.CreateNetwork(sm, "default", "n2", "10.1.1.0/24", "10.1.1.1", 100, nil, orchInfo2)
	AssertOk(t, err, "Failed to create network")

	c, err := govmomi.NewClient(context.Background(), u, true)
	AssertOk(t, err, "Failed to create govmomi client")
	restCl := rest.NewClient(c.Client)
	tagClient := tags.NewManager(restCl)
	err = tagClient.Login(context.Background(), u.User)
	AssertOk(t, err, "Failed to create tags client")

	var pgRef types.ManagedObjectReference

	verifyPg := func(dcPgMap map[string][]string) {
		AssertEventually(t, func() (bool, interface{}) {
			for name, pgNames := range dcPgMap {
				dc := vchub.GetDC(name)
				if dc == nil {
					err := fmt.Errorf("Failed to find DC %s", name)
					logger.Errorf("%s", err)
					return false, err
				}

				attachedTags, err := tagClient.GetAttachedTags(context.Background(), dc.dcRef)
				AssertOk(t, err, "failed to get tags")
				if len(attachedTags) != 1 {
					return false, fmt.Errorf("DC didn't have expected tags, had %v", attachedTags)
				}
				AssertEquals(t, defs.CreateVCTagManagedTag(clusterID), attachedTags[0].Name, "DC didn't have managed tag")
				AssertEquals(t, defs.VCTagManagedDescription, attachedTags[0].Description, "DC didn't have managed tag")

				dvs := dc.GetPenDVS(CreateDVSName(name))
				if dvs == nil {
					err := fmt.Errorf("Failed to find dvs in DC %s", name)
					logger.Errorf("%s", err)
					return false, err
				}

				attachedTags, err = tagClient.GetAttachedTags(context.Background(), dvs.DvsRef)
				AssertOk(t, err, "failed to get tags")
				if len(attachedTags) != 1 {
					return false, fmt.Errorf("DVS didn't have expected tags, had %v", attachedTags)
				}
				AssertEquals(t, defs.CreateVCTagManagedTag(clusterID), attachedTags[0].Name, "DVS didn't have managed tag")

				for _, pgName := range pgNames {
					pgObj := dvs.GetPenPG(pgName)
					if pgObj == nil {
						err := fmt.Errorf("Failed to find %s in DC %s", pgName, name)
						logger.Errorf("%s", err)
						return false, err
					}
					attachedTags, err := tagClient.GetAttachedTags(context.Background(), pgObj.PgRef)
					AssertOk(t, err, "failed to get tags")
					if len(attachedTags) != 2 {
						return false, fmt.Errorf("PG didn't have expected tags, had %v", attachedTags)
					}
					expTags := []string{
						fmt.Sprintf("%s", defs.CreateVCTagManagedTag(clusterID)),
						fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 100),
					}
					for _, tag := range attachedTags {
						AssertOneOf(t, tag.Name, expTags)
					}
					pgRef = pgObj.PgRef
				}
				dvs.Lock()
				if len(dvs.Pgs) != len(pgNames) {
					err := fmt.Errorf("PG length didn't match: exp %v, actual %v", pgNames, dvs.Pgs)
					dvs.Unlock()
					logger.Errorf("%s", err)
					return false, err
				}
				dvs.Unlock()
			}
			return true, nil
		}, "Failed to find PGs")
	}

	// n1 should only be in defaultDC
	// n2 should be in both
	pg1 := CreatePGName("n1")
	pg2 := CreatePGName("n2")

	dcPgMap := map[string][]string{
		defaultTestParams.TestDCName: []string{pg1, pg2},
		dc2:                          []string{pg2},
	}

	verifyPg(dcPgMap)

	// Removed tags should be re-added
	tagClient.DetachTag(context.Background(), defs.CreateVCTagManagedTag(clusterID), dc1.Obj.Reference())
	tagClient.DetachTag(context.Background(), fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 100), pgRef)

	verifyPg(dcPgMap)

	// Delete tags, they should be re-written
	tag, err := tagClient.GetTag(context.Background(), defs.CreateVCTagManagedTag(clusterID))
	AssertOk(t, err, "Failed to get tag")
	err = tagClient.DeleteTag(context.Background(), tag)
	AssertOk(t, err, "Failed to delete tag")

	tag, err = tagClient.GetTag(context.Background(), fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 100))
	AssertOk(t, err, "Failed to get tag")
	err = tagClient.DeleteTag(context.Background(), tag)
	AssertOk(t, err, "Failed to delete tag")

	verifyPg(dcPgMap)

	// Delete category, should be re-written
	cat, err := tagClient.GetCategory(context.Background(), defs.VCTagCategory)
	err = tagClient.DeleteCategory(context.Background(), cat)
	AssertOk(t, err, "Failed to delete cat")

	verifyPg(dcPgMap)

	// Add vlan tag to non PG object, should be removed
	tag, err = tagClient.GetTag(context.Background(), fmt.Sprintf("%s%d", defs.VCTagVlanPrefix, 100))
	AssertOk(t, err, "Failed to get tag")
	err = tagClient.AttachTag(context.Background(), tag.ID, dc1.Obj.Reference())
	AssertOk(t, err, "Failed to attach tag")

	verifyPg(dcPgMap)

	// Verify duplicate pensando managed triggers event
	tagSyncEventRecorder.ClearEvents()
	cat, err = tagClient.GetCategory(context.Background(), defs.VCTagCategory)
	AssertOk(t, err, "Failed to get cat")

	tag = &tags.Tag{
		Name:        defs.CreateVCTagManagedTag("randomCluster"),
		Description: defs.VCTagManagedDescription,
		CategoryID:  cat.ID,
	}
	tagClient.CreateTag(context.Background(), tag)
	err = tagClient.AttachTag(context.Background(), tag.Name, dc1.Obj.Reference())
	AssertOk(t, err, "Failed to attach tag")

	AssertEventually(t, func() (bool, interface{}) {
		evts := tagSyncEventRecorder.GetEvents()
		for _, evt := range evts {
			logger.Errorf("ERROR %s", evt)
			if evt.EventType == eventtypes.ORCH_ALREADY_MANAGED.String() {
				if strings.Contains(evt.Message, tag.Name) && strings.Contains(evt.Message, defaultTestParams.TestDCName) {
					return true, nil
				}
			}
		}
		return false, nil
	}, "Already managed event was not generated")

}
