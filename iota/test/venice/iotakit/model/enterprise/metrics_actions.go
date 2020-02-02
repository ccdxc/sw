// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package enterprise

import (
	"encoding/json"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/fields"
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

// FindFwlogForWorkloadPairs finds workload ip addresses in firewall log
func (sm *SysModel) FindFwlogForWorkloadPairs(protocol, fwaction, timestr string, port uint32, wpc *objects.WorkloadPairCollection) error {
	res, err := sm.QueryFwlog(protocol, fwaction, timestr, port)
	if err != nil {
		return err
	}

	for _, ips := range wpc.ListIPAddr() {
		if checkIPAddrInFwlog(ips, res.Results) != true {
			err := fmt.Errorf("did not find %v in fwlog", ips)
			log.Errorf("fwlog query failed, %v", err)
			for _, r := range res.Results {
				for i, l := range r.Logs {
					log.Infof("[%-3d] %v ", i+1, l.String())
				}
			}
			sm.ForEachNaples(func(nc *objects.NaplesCollection) error {
				if out, err := sm.RunNaplesCommand(nc,
					"/nic/bin/shmdump -file=/dev/shm/fwlog_ipc_shm -type=fwlog"); err == nil {
					log.Infof(strings.Join(out, ","))
				} else {
					log.Infof("failed to run shmdump, %v", err)
				}
				return nil
			})
			return err
		}
	}

	return nil
}

// VerifyRuleStats verifies rule stats for policies
func (sm *SysModel) VerifyRuleStats(timestr string, spc *objects.NetworkSecurityPolicyCollection, minCounts []map[string]float64) error {
	stsc, err := spc.Status()
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
			res, err := sm.QueryMetrics("RuleMetrics", rule.RuleHash, timestr, int32(len(sm.NaplesNodes)))
			if err != nil {
				log.Errorf("Error during metrics query %v", err)
				continue
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

func (sm *SysModel) QueryDropMetricsForWorkloadPairs(wpc *objects.WorkloadPairCollection, timestr string) error {
	for _, pair := range wpc.Pairs {
		dstIpAddr := pair.Second.GetIP()
		srcIpAddr := pair.First.GetIP()
		sel := fields.Selector{
			Requirements: []*fields.Requirement{
				{
					Key:    "source",
					Values: []string{srcIpAddr},
				},
				{
					Key:    "destination",
					Values: []string{dstIpAddr},
				},
			},
		}
		res, err := sm.QueryMetricsSelector("IPv4FlowDropMetrics", timestr, sel)
		if err != nil {
			return err
		}
		log.Infof("Done with query selection")
		testFields := map[string]string{
			"DropPackets": "1",
			"DropReason":  "1",
		}

		for _, rslt := range res.Results {
			log.Infof("Results %d", len(rslt.Series))
			for _, series := range rslt.Series {
				// find the column
				log.Infof("series")
				cIndex := map[string]int{}
				for i, c := range series.Columns {
					if _, ok := testFields[c]; ok {
						cIndex[c] = i

					}
				}
				for _, t := range series.Values {
					for k, v := range cIndex {
						temp := fmt.Sprintf("%d", int(t[v].(float64)))
						if temp != testFields[k] {
							return fmt.Errorf("received %v : %v expected: %v", k, temp, testFields[k])
						}
					}
				}
			}
		}
	}
	return nil
}
