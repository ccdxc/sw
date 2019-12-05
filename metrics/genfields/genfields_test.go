package genfields

import (
	"fmt"
	"reflect"
	"sort"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetFields(t *testing.T) {
	// Simulated test case
	msgFieldMaps["metric_zero"] = []string{}
	v, ok := GetFields("metric_zero")
	Assert(t, ok, fmt.Sprintf("failed to get field list"))
	Assert(t, reflect.DeepEqual(v, msgFieldMaps["metric_zero"]), fmt.Sprintf("failed to get correct field list from msgFieldMaps"))

	msgFieldMaps["metric_one"] = []string{"field_one"}
	v, ok = GetFields("metric_one")
	Assert(t, ok, fmt.Sprintf("failed to get field list"))
	Assert(t, reflect.DeepEqual(v, msgFieldMaps["metric_one"]), fmt.Sprintf("failed to get correct field list from msgFieldMaps"))

	msgFieldMaps["metric_two"] = []string{"field_one", "field_two"}
	v, ok = GetFields("metric_two")
	Assert(t, ok, fmt.Sprintf("failed to get field list"))
	Assert(t, reflect.DeepEqual(v, msgFieldMaps["metric_two"]), fmt.Sprintf("failed to get correct field list from msgFieldMaps"))

	// Real test case for three proto files
	// If the original field config changed, the answer here must also be changed
	v, ok = GetFields("IPv4FlowBehavioralMetrics")
	Assert(t, ok, fmt.Sprintf("failed to get field list for flowstats IPv4FlowBehavioralMetrics"))
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
	Assert(t, reflect.DeepEqual(v, flowstatsAnswer), fmt.Sprintf("failed to get correct field list from msgFieldMaps for flowstats IPv4FlowBehavioralMetrics"))

	v, ok = GetFields("RuleMetrics")
	Assert(t, ok, fmt.Sprintf("failed to get field list for rulestats RuleMetrics"))
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
	Assert(t, reflect.DeepEqual(v, rulestatsAnswer), fmt.Sprintf("failed to get correct field list from msgFieldMaps for rulestats RuleMetrics"))

	v, ok = GetFields("FteLifQMetrics")
	Assert(t, ok, fmt.Sprintf("failed to get field list for ftestats FteLifQMetrics"))
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
	}
	sort.Strings(v)
	sort.Strings(ftestatsAnswer)
	Assert(t, reflect.DeepEqual(v, ftestatsAnswer), fmt.Sprintf("failed to get correct field list from msgFieldMaps for ftestats FteLifQMetrics"))
}

func TestGetAllKeys(t *testing.T) {
	// This test is conduct adter adding three simulated key value pairs
	msgFieldMaps["metric_zero"] = []string{}
	msgFieldMaps["metric_one"] = []string{"field_one"}
	msgFieldMaps["metric_two"] = []string{"field_one", "field_two"}
	keys := []string{}
	for k := range msgFieldMaps {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	allKeysResult := GetAllKeys()
	sort.Strings(allKeysResult)
	Assert(t, reflect.DeepEqual(keys, allKeysResult), fmt.Sprintf("failed to get correct all keys from msgFieldMaps %v %v", keys, sort.StringSlice(GetAllKeys())))
}
