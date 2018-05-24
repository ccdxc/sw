// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"context"
	"time"

	. "gopkg.in/check.v1"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var timeFormat = "2006-01-02 15:04:05 MST"

var testMirrorSessions = []monitoring.MirrorSession{
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 1",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{"ALL_PKTS"},
			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
			},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: "VENICE",
				},
				{
					Type: "ERSPAN",
					ExportCfg: api.ExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"1234"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						Endpoints: []string{"Endpoint1"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"TCP/1234"},
					},
				},
			},
		},
	},
	{
		ObjectMeta: api.ObjectMeta{
			Name:   "Test Mirror Session 2",
			Tenant: "Tenant 1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "MirrorSession",
			APIVersion: "v1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{"ALL_DROPS"},
			StartConditions: monitoring.MirrorStartConditions{
				// schedule *after* 10 sec - Fix it based on current time when creating a session
				ScheduleTime: &api.Timestamp{
					Timestamp: types.Timestamp{
						Seconds: 10,
					},
				},
			},

			StopConditions: monitoring.MirrorStopConditions{
				MaxPacketCount: 1000,
				ExpiryDuration: "5m",
			},

			Collectors: []monitoring.MirrorCollector{
				{
					Type: "ERSPAN",
					ExportCfg: api.ExportConfig{
						Destination: "111.1.1.1",
					},
				},
			},

			MatchRules: []monitoring.MatchRule{
				{
					AppProtoSel: &monitoring.AppProtoSelector{
						Ports: []string{"UDP"},
					},
				},
			},
		},
	},
}

// TestMirrorSessions - creates a mirror session using REST api
func (it *veniceIntegSuite) TestMirrorSessions(c *C) {
	// This is a dummy test case and will be replaced with more realistic test cases
	// It is used only to iron-out framework and overall code structure of TS controller
	for i, ms := range testMirrorSessions {
		log.Infof("Test Create Mirror Session[%d] %v", i+1, ms.Name)
		if ms.Spec.StartConditions.ScheduleTime != nil {
			// fix the time
			schAfter := time.Duration(uint64(ms.Spec.StartConditions.ScheduleTime.Seconds) * uint64(time.Second))
			log.Infof("Duration=%v, Now=%v = %v\n", schAfter, time.Now(), time.Now().Format(timeFormat))
			schTime := time.Now().Add(schAfter)
			ts, _ := types.TimestampProto(schTime)
			ms.Spec.StartConditions.ScheduleTime.Timestamp = *ts
			log.Infof("Setting ScheduleTime to %v\n", ms.Spec.StartConditions.ScheduleTime)
		}

		ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
		AssertOk(c, err, "Error creating logged in context")
		_, err = it.restClient.MonitoringV1().MirrorSession().Create(ctx, &ms)
		if err != nil {
			log.Infof("%v", err)
			Assert(c, false, "Failed to create mirror session")
		}
	}
	var intervals []string
	intervals = []string{"1s", "30s"}
	AssertEventually(c, func() (bool, interface{}) {
		for _, ms := range testMirrorSessions {
			ctx, err := authntestutils.NewLoggedInContext(context.Background(), integTestAPIGWURL, it.userCred)
			AssertOk(c, err, "Error creating logged in context")
			tms, err := it.restClient.MonitoringV1().MirrorSession().Get(ctx, &ms.ObjectMeta)
			if err != nil {
				log.Infof("Get():MirrorSession %s:%s ... Not Found", ms.Tenant, ms.Name)
				return false, nil
			}
			if tms.Status.State != "RUNNING" && tms.Status.State != "STOPPED" {
				log.Infof("Get():MirrorSession %s:%s ... Not Running/Stopped - %v", ms.Tenant, ms.Name, tms.Status.State)
				return false, nil
			}
			log.Infof("Successful Get():MirrorSession %s:%s - %v", tms.Tenant, tms.Name, tms.Status.State)
		}
		return true, nil
	}, "All Mirror Sessions not found in TsCtrler", intervals...)

	return
}
