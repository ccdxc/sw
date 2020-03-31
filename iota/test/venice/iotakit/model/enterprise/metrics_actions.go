// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"encoding/json"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

func checkIPAddrInFwlog(ips []string, res []*telemetry_query.FwlogsQueryResult) bool {
	for _, r := range res {
		for _, l := range r.Logs {
			if l.Dest == ips[0] && l.Src == ips[1] {
				return true
			}
		}
	}
	log.Infof("failed to find %+v in fwlog", ips)
	return false
}

// VerifyRuleStats verifies rule stats for policies
func (sm *SysModel) VerifyRuleStats(timestr string, spc *objects.NetworkSecurityPolicyCollection, minCounts []map[string]float64) error {
	stsc, err := spc.Status()
	if err != nil {
		return err
	}

	// get node collection and init telemetry client
	vnc := sm.VeniceNodes()
	err = vnc.InitTelemetryClient()
	if err != nil {
		return err
	}

	var notFound []string
	// for each policy
	for _, sts := range stsc {
		// verify user has supplied expected count for each rule
		if len(minCounts) < len(sts.RuleStatus) {
			return fmt.Errorf("Not enough entries in minCounts argument. Need %d, have %d", len(sts.RuleStatus), len(minCounts))
		}

		// walk each rule
		for idx, rule := range sts.RuleStatus {
			res, err := vnc.QueryMetrics("RuleMetrics", rule.RuleHash, timestr, int32(len(sm.NaplesNodes)))
			if err != nil {
				log.Errorf("Error during metrics query %v", err)
				continue
			}
			if res == nil {
				log.Errorf("query result is nil")
				return fmt.Errorf("Error get nil result")
			}

			expCount := minCounts[idx]
			matchFound := false
			// walk the result for each reporter-id
			for _, rslt := range res.Results {
				for _, series := range rslt.Series {
					seriesMatch := true
					// find the column
					for cidx, colName := range series.Columns {
						if mcount, ok := expCount[colName]; ok {
							colMatch := false
							// walk each result
							for _, vals := range series.Values {
								switch vals[cidx].(type) {
								case float64:
									if vals[cidx].(float64) >= mcount {
										colMatch = true
									}
								default:
									return fmt.Errorf("Invalid type for column %s", colName)
								}
							}
							if !colMatch {
								seriesMatch = false
							}
						}
					}
					if seriesMatch {
						matchFound = true
					}
				}
			}
			if !matchFound {
				b, _ := json.MarshalIndent(res, "", "    ")
				fmt.Printf("Got rule metrics resp: %s\n", string(b))
				merr := fmt.Sprintf("Match not found for %+v for rule %v", minCounts, rule)
				log.Infof("Rule stats did not match: %v", merr)
				notFound = append(notFound, merr)
			}
		}
	}
	if len(notFound) != 0 {
		sm.ForEachNaples(func(nc *objects.NaplesCollection) error {
			if out, err := sm.RunNaplesCommand(nc,
				"LD_LIBRARY_PATH=/platform/lib:/nic/lib /nic/bin/delphictl metrics list RuleMetrics"); err == nil {
				fmt.Printf("delphictl metrics for %v:\n %s\n", nc.Names(), strings.Join(out, ","))
			} else {
				log.Infof("failed to run delphictl, %v", err)
			}
			return nil
		})

		log.Errorf("Rule stats not found: %v", notFound)
		return fmt.Errorf("Rule stats not found: %v", notFound)
	}

	return nil
}
