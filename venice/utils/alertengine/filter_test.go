package alertengine

import (
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFilter(t *testing.T) {
	nw := time.Now()
	ts, _ := types.TimestampProto(nw)
	evt := &evtsapi.Event{
		TypeMeta: api.TypeMeta{
			Kind: "events.Event",
		},
		ObjectMeta: api.ObjectMeta{
			CreationTime: api.Timestamp{
				Timestamp: *ts,
			},
			ModTime: api.Timestamp{
				Timestamp: *ts,
			},
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Name:      "evt1",
		},
		EventAttributes: evtsapi.EventAttributes{
			Count:    5,
			Severity: evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)],
			Type:     evtsapi.ServiceRunning,
			ObjectRef: &api.ObjectRef{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Name:      "node1",
				Kind:      "Node",
			},
		},
	}

	tests := []struct {
		reqs       []*fields.Requirement
		expSuccess bool
		expResp    []*monitoring.MatchedRequirement
	}{
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "count", Operator: "lt", Values: []string{"7"}},
				&fields.Requirement{Key: "severity", Operator: "equals", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)]}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Count", Operator: "lt", Values: []string{"7"}}, ObservedValue: "5"},
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Severity", Operator: "equals", Values: []string{evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)]}}, ObservedValue: "INFO"},
			},
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "type", Operator: "notIn", Values: []string{evtsapi.ServiceStarted, evtsapi.ServiceStopped}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Type", Operator: "notIn", Values: []string{evtsapi.ServiceStarted, evtsapi.ServiceStopped}}, ObservedValue: ""},
			},
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "object-ref.kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectRef.Kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}}, ObservedValue: "Node"},
			},
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "object-ref.kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}},
				&fields.Requirement{Key: "object-ref.name", Operator: "equals", Values: []string{"node1"}},
				&fields.Requirement{Key: "type", Operator: "in", Values: []string{evtsapi.ServiceRunning}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectRef.Kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}}, ObservedValue: "Node"},
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectRef.Name", Operator: "equals", Values: []string{"node1"}}, ObservedValue: "node1"},
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Type", Operator: "in", Values: []string{evtsapi.ServiceRunning}}, ObservedValue: evtsapi.ServiceRunning},
			},
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "kind", Operator: "equals", Values: []string{"events.Event"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Kind", Operator: "equals", Values: []string{"events.Event"}}, ObservedValue: "events.Event"},
			},
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "count", Operator: "gte", Values: []string{"5"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "Count", Operator: "gte", Values: []string{"5"}}, ObservedValue: "5"},
			},
		},
		// TODO: enable these tests once the schema is fixed
		// {
		// 	reqs: []*fields.Requirement{
		// 		&fields.Requirement{Key: "meta.name", Operator: "equals", Values: []string{"evt1"}},
		// 	},
		// 	expSuccess: true,
		// 	expResp: []*monitoring.MatchedRequirement{
		// 		&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectMeta.Name", Operator: "equals", Values: []string{"evt1"}}, ObservedValue: "evt1"},
		// 	},
		// },
		// {
		// 	reqs: []*fields.Requirement{
		// 		&fields.Requirement{Key: "meta.creation-time", Operator: "equals", Values: []string{ts.String()}},
		// 	},
		// 	expSuccess: true,
		// 	expResp: []*monitoring.MatchedRequirement{
		// 		&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "equals", Values: []string{ts.String()}}, ObservedValue: ts.String()},
		// 	},
		// },
		// {
		// 	reqs: []*fields.Requirement{
		// 		&fields.Requirement{Key: "meta.creation-time", Operator: "lte", Values: []string{nw.Format(time.RFC3339Nano)}},
		// 	},
		// 	expSuccess: true,
		// 	expResp: []*monitoring.MatchedRequirement{
		// 		&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "lte", Values: []string{nw.Format(time.RFC3339Nano)}}, ObservedValue: ts.String()},
		// 	},
		// },
		// {
		// 	reqs: []*fields.Requirement{
		// 		&fields.Requirement{Key: "meta.creation-time", Operator: "gte", Values: []string{ts.String()}},
		// 		&fields.Requirement{Key: "meta.mod-time", Operator: "lt", Values: []string{nw.Add(10 * time.Second).Format(time.RFC3339Nano)}},
		// 	},
		// 	expSuccess: true,
		// 	expResp: []*monitoring.MatchedRequirement{
		// 		&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "gte", Values: []string{ts.String()}}, ObservedValue: ts.String()},
		// 		&monitoring.MatchedRequirement{Requirement: &fields.Requirement{Key: "ObjectMeta.ModTime", Operator: "lt", Values: []string{nw.Add(10 * time.Second).Format(time.RFC3339Nano)}}, ObservedValue: ts.String()},
		// 	},
		// },
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "count", Operator: "gte", Values: []string{"15"}},
			},
			expSuccess: false,
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "type", Operator: "notIn", Values: []string{evtsapi.ServiceRunning}},
			},
			expSuccess: false,
		},
		{
			reqs: []*fields.Requirement{
				&fields.Requirement{Key: "dummy-field", Operator: "notIn", Values: []string{evtsapi.ServiceRunning}},
			},
			expSuccess: false,
		},
	}

	for i, test := range tests {
		match, matchedReqs := Match(test.reqs, evt)
		Assert(t, test.expSuccess == match, "test #%v failed, expected: %v, got: %v", i, test.expSuccess, match)
		if match {
			for j := range test.expResp {
				Assert(t, test.expResp[j].Key == matchedReqs[j].Key, "test #%v failed, expected: %v, got: %v", i, test.expResp[j].Key, matchedReqs[j].Key)
				Assert(t, test.expResp[j].Operator == matchedReqs[j].Operator, "test #%v failed, expected: %v, got: %v", i, test.expResp[j].Operator, matchedReqs[j].Operator)
				Assert(t, len(test.expResp[j].Values) == len(matchedReqs[j].Values), "test #%v failed, expected: %v, got: %v", i, len(test.expResp[j].Values), len(matchedReqs[j].Values))
				for k := range test.expResp[j].Values {
					Assert(t, test.expResp[j].Values[k] == matchedReqs[j].Values[k], "test #%v failed, expected: %v, got: %v", i, test.expResp[j].Values[k], matchedReqs[j].Values[k])
				}
				Assert(t, test.expResp[j].ObservedValue == matchedReqs[j].ObservedValue, "test #%v failed, expected: %v, got: %v", i, test.expResp[j].ObservedValue, matchedReqs[j].ObservedValue)
			}
		}
	}
}
