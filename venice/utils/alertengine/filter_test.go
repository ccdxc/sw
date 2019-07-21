package alertengine

import (
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFilter(t *testing.T) {
	nw := time.Now()
	ts, _ := types.TimestampProto(nw)
	evt := &evtsapi.Event{
		TypeMeta: api.TypeMeta{
			Kind: "Event",
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
			Severity: eventattrs.Severity_INFO.String(),
			Type:     eventtypes.SERVICE_RUNNING.String(),
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
				{Key: "count", Operator: "lt", Values: []string{"7"}},
				{Key: "severity", Operator: "equals", Values: []string{eventattrs.Severity_INFO.String()}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "Count", Operator: "lt", Values: []string{"7"}}, ObservedValue: "5"},
				{Requirement: &fields.Requirement{Key: "Severity", Operator: "equals", Values: []string{eventattrs.Severity_INFO.String()}}, ObservedValue: eventattrs.Severity_INFO.String()},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "type", Operator: "notIn", Values: []string{eventtypes.EventType_name[int32(eventtypes.SERVICE_STARTED)], eventtypes.EventType_name[int32(eventtypes.SERVICE_STOPPED)]}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "Type", Operator: "notIn", Values: []string{eventtypes.EventType_name[int32(eventtypes.SERVICE_STARTED)], eventtypes.EventType_name[int32(eventtypes.SERVICE_STOPPED)]}}, ObservedValue: ""},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "object-ref.kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectRef.Kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}}, ObservedValue: "Node"},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "object-ref.kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}},
				{Key: "object-ref.name", Operator: "equals", Values: []string{"node1"}},
				{Key: "type", Operator: "in", Values: []string{eventtypes.SERVICE_RUNNING.String()}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectRef.Kind", Operator: "notEquals", Values: []string{"Cluster", "Network"}}, ObservedValue: "Node"},
				{Requirement: &fields.Requirement{Key: "ObjectRef.Name", Operator: "equals", Values: []string{"node1"}}, ObservedValue: "node1"},
				{Requirement: &fields.Requirement{Key: "Type", Operator: "in", Values: []string{eventtypes.SERVICE_RUNNING.String()}}, ObservedValue: eventtypes.SERVICE_RUNNING.String()},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "kind", Operator: "equals", Values: []string{"Event"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "Kind", Operator: "equals", Values: []string{"Event"}}, ObservedValue: "Event"},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "count", Operator: "gte", Values: []string{"5"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "Count", Operator: "gte", Values: []string{"5"}}, ObservedValue: "5"},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "meta.name", Operator: "equals", Values: []string{"evt1"}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectMeta.Name", Operator: "equals", Values: []string{"evt1"}}, ObservedValue: "evt1"},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "meta.creation-time", Operator: "equals", Values: []string{ts.String()}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "equals", Values: []string{ts.String()}}, ObservedValue: ts.String()},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "meta.creation-time", Operator: "lte", Values: []string{nw.Format(time.RFC3339Nano)}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "lte", Values: []string{nw.Format(time.RFC3339Nano)}}, ObservedValue: ts.String()},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "meta.creation-time", Operator: "gte", Values: []string{ts.String()}},
				{Key: "meta.mod-time", Operator: "lt", Values: []string{nw.Add(10 * time.Second).Format(time.RFC3339Nano)}},
			},
			expSuccess: true,
			expResp: []*monitoring.MatchedRequirement{
				{Requirement: &fields.Requirement{Key: "ObjectMeta.CreationTime", Operator: "gte", Values: []string{ts.String()}}, ObservedValue: ts.String()},
				{Requirement: &fields.Requirement{Key: "ObjectMeta.ModTime", Operator: "lt", Values: []string{nw.Add(10 * time.Second).Format(time.RFC3339Nano)}}, ObservedValue: ts.String()},
			},
		},
		{
			reqs: []*fields.Requirement{
				{Key: "count", Operator: "gte", Values: []string{"15"}},
			},
			expSuccess: false,
		},
		{
			reqs: []*fields.Requirement{
				{Key: "type", Operator: "notIn", Values: []string{eventtypes.SERVICE_RUNNING.String()}},
			},
			expSuccess: false,
		},
		{
			reqs: []*fields.Requirement{
				{Key: "dummy-field", Operator: "notIn", Values: []string{eventtypes.SERVICE_RUNNING.String()}},
			},
			expSuccess: false,
		},
		{
			reqs:       []*fields.Requirement{},
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
