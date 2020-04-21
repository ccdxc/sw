package genfields

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetFieldNamesFromKind(t *testing.T) {
	// Simulated test case
	kindToFieldNameMap["metric_zero"] = []string{}
	v := GetFieldNamesFromKind("metric_zero")
	Assert(t, reflect.DeepEqual(v, kindToFieldNameMap["metric_zero"]), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap"))

	kindToFieldNameMap["metric_one"] = []string{"field_one"}
	v = GetFieldNamesFromKind("metric_one")
	Assert(t, reflect.DeepEqual(v, kindToFieldNameMap["metric_one"]), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap"))

	kindToFieldNameMap["metric_two"] = []string{"field_one", "field_two"}
	v = GetFieldNamesFromKind("metric_two")
	Assert(t, reflect.DeepEqual(v, kindToFieldNameMap["metric_two"]), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap"))

	// Real test case for three proto files
	// If the original field config changed, the answer here must also be changed
	v = GetFieldNamesFromKind("IPv4FlowBehavioralMetrics")
	flowstatsAnswer := []string{
		"Instances",
		"PpsThreshold",
		"PpsThresholdExceedEvents",
		"PpsThresholdExceedEventFirstTimestamp",
		"PpsThresholdExceedEventLastTimestamp",
		"BwThreshold",
		"BwThresholdExceedEvents",
		"BwThresholdExceedEventFirstTimestamp",
		"BwThresholdExceedEventLastTimestamp",
	}
	sort.Strings(v)
	sort.Strings(flowstatsAnswer)
	Assert(t, reflect.DeepEqual(v, flowstatsAnswer), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap for flowstats IPv4FlowBehavioralMetrics"))

	v = GetFieldNamesFromKind("RuleMetrics")
	rulestatsAnswer := []string{
		"TcpHits",
		"UdpHits",
		"IcmpHits",
		"EspHits",
		"OtherHits",
		"TotalHits",
	}
	sort.Strings(v)
	sort.Strings(rulestatsAnswer)
	Assert(t, reflect.DeepEqual(v, rulestatsAnswer), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap for rulestats RuleMetrics"))

	v = GetFieldNamesFromKind("FteLifQMetrics")
	ftestatsAnswer := []string{
		"FlowMissPackets",
		"FlowRetransmitPackets",
		"L4RedirectPackets",
		"AlgControlFlowPackets",
		"TcpClosePackets",
		"TlsProxyPackets",
		"FteSpanPackets",
		"SoftwareQueuePackets",
		"QueuedTxPackets",
		"FreedTxPackets",
		"MaxSessionThresholdDrops",
		"SessionCreatesIgnored",
	}
	sort.Strings(v)
	sort.Strings(ftestatsAnswer)
	Assert(t, reflect.DeepEqual(v, ftestatsAnswer), fmt.Sprintf("failed to get correct field list from kindToFieldNameMap for ftestats FteLifQMetrics"))
}

func TestGetAllFieldNames(t *testing.T) {
	// This test is conduct adter adding three simulated key value pairs
	kindToFieldNameMap["metric_zero"] = []string{}
	kindToFieldNameMap["metric_one"] = []string{"field_one"}
	kindToFieldNameMap["metric_two"] = []string{"field_one", "field_two"}
	keys := []string{}
	for k := range kindToFieldNameMap {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	allKeysResult := GetAllFieldNames()
	sort.Strings(allKeysResult)
	Assert(t, reflect.DeepEqual(keys, allKeysResult),
		fmt.Sprintf("failed to get correct all keys from kindToFieldNameMap %v %v", keys, sort.StringSlice(GetAllFieldNames())))
}

func TestGlobalMetricsMap(t *testing.T) {
	Assert(t, IsGroupValid("ftestats"), "expected: true, got: false")
	Assert(t, !IsGroupValid("invalid"), "expected: false, got: true")

	Assert(t, IsKindValid("flowstats", "IPv4FlowDropMetrics"), "expected: true, got: false")
	Assert(t, !IsKindValid("flowstats", "IPv4FlowDropMetricsInvalid"), "expected: false, got: true")

	Assert(t, IsFieldNameValid("ftestats", "FteCPSMetrics", "ConnectionsPerSecond"), "expected: true, got: false")
	Assert(t, !IsFieldNameValid("ftestats", "FteCPSMetrics", "CConnectionsPerSecond"), "expected: false, got: true")
}
