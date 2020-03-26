package impl

import (
	"context"
	"fmt"
	"strings"

	govldtr "github.com/asaskevich/govalidator"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
)

type mirrorSessionHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

const (
	// Finalize these parameters once we decide how to store the packets captured by Venice
	veniceMaxPacketSize           = 256
	veniceMaxCollectorsPerSession = 4
)

func (r *mirrorSessionHooks) validateMirrorSession(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	ms, ok := i.(monitoring.MirrorSession)
	if !ok {
		return i, false, fmt.Errorf("Invalid input type")
	}
	// Perform validation only if Spec has changed
	// No change to spec indicates status update, for which no need to validate the spec
	oldms := monitoring.MirrorSession{}
	err := kv.Get(ctx, key, &oldms)
	if err == nil {
		// found old object, compare spec
		if oper == apiintf.CreateOper {
			// Create on already existing mirror session
			// return success and let api server take care of this error
			return i, true, nil
		}
		if _, diff := ref.ObjDiff(ms.Spec, oldms.Spec); !diff {
			return i, true, nil
		}
	} else if oper == apiintf.UpdateOper {
		// update on non-existing mirror session
		// return success and let api server take care of this error
		return i, true, nil
	}
	// checks:
	// Either Match Rule could be present or Interface selector, not both
	// If Match rule we treat it as flow based rule else it is treated as catch all for interfaces
	// PacketSize <= 256 if collector is Venice
	// StartCondition: if specified, must not be in the past
	// StopCondition: MUST be specified, MaxPacketCount <= 1000, expiryDuration <= 2h
	// Collectors: atleast 1 must be specified, max 2 collectors, max 1 can be venice
	// For erspan collectors, valid export config must be specified
	// ExportCfg Validator: Destination - must be valid IP address (vrf?), Transport="GRE/ERSPANv3"
	//  no credentials
	// MatchRule:
	//  Src/Dst: Atleast 1 EP name OR Atleast 1 valid IP address OR atleast 1 valid MACAddr
	//  Valid Src OR Valid Dst if ALL_PKTS are chosen
	//  AppProtoSel: If specified, known L4+ proto (TCP/UDP/..)
	// Atleast 1 valid match rule
	// Filter validation, ALL_DROPS cannot be used with specific DROP conditions
	// ALL_PKTS implies all good packets, it can be specified with DROP condition(s)

	if len(ms.Spec.MatchRules) != 0 && ms.Spec.Interfaces != nil {
		return i, false, fmt.Errorf("Either Match rules or Interfaces can be set, not both")
	}

	if len(ms.Spec.PacketFilters) != 0 && ms.Spec.Interfaces != nil {
		return i, false, fmt.Errorf("Interfaces could only be set with no packet filters")
	}

	if ms.Spec.Interfaces != nil && ms.Spec.Interfaces.Selector == nil {
		return i, false, fmt.Errorf("Interface selector must be specified with Interfaces option")
	}

	if len(ms.Spec.Collectors) == 0 || len(ms.Spec.Collectors) > veniceMaxCollectorsPerSession {
		return i, false, fmt.Errorf("Need atleast one mirror collector, upto %d max", veniceMaxCollectorsPerSession)
	}
	numVeniceCollectors := 0

	var mirrors monitoring.MirrorSessionList
	mirror := monitoring.MirrorSession{}
	mirrorKey := strings.TrimSuffix(mirror.MakeKey(string(apiclient.GroupMonitoring)), "/")
	if err := kv.List(ctx, mirrorKey, &mirrors); err != nil {
		return nil, true, fmt.Errorf("failed to list mirrors. Err: %v", err)
	}
	expConfig := make(map[string]*monitoring.MirrorExportConfig)
	for _, mir := range mirrors.Items {
		for _, col := range mir.Spec.Collectors {
			if col.ExportCfg != nil {
				expConfig[col.ExportCfg.Destination] = col.ExportCfg
			}
		}
	}
	for _, c := range ms.Spec.Collectors {
		if c.Type == monitoring.PacketCollectorType_ERSPAN.String() {
			if c.ExportCfg == nil || c.ExportCfg.Destination == "" {
				return i, false, fmt.Errorf("Provide valid destination for ERSPAN collector")
			}

			if c.ExportCfg.Gateway != "" && !govldtr.IsIPv4(c.ExportCfg.Gateway) {
				return i, false, fmt.Errorf("Gateway can be empty or must be a valid IPv4 address")
			}
			// Checking for Destition and other parameters inside ExportCfg XXX
		} else {
			// this is already checked by venice.check
			return i, false, fmt.Errorf("Unsupported collector type")
		}

		if c.ExportCfg != nil {
			existingCfg, ok := expConfig[c.ExportCfg.Destination]
			if ok && existingCfg.Gateway != c.ExportCfg.Gateway {
				return i, false, fmt.Errorf("Collector %v already added with different gateway %v, current %v",
					c.ExportCfg.Destination, existingCfg.Gateway, c.ExportCfg.Gateway)
			}
		}
	}
	if numVeniceCollectors > 0 && ms.Spec.PacketSize > veniceMaxPacketSize {
		errStr := fmt.Errorf("Max packet size allowed by Venice collector is %v", veniceMaxPacketSize)
		return i, false, errStr
	}
	if ms.Spec.StartConditions.ScheduleTime != nil {
		_, err := ms.Spec.StartConditions.ScheduleTime.Time()
		if err != nil {
			return i, false, fmt.Errorf("Unsupported format used for schedule-time")
		}
	}

	dropAllFilter := false
	dropReasonFilter := false
	allPktsFilter := false
	for _, pf := range ms.Spec.PacketFilters {
		if pf == monitoring.MirrorSessionSpec_ALL_DROPS.String() {
			dropAllFilter = true
		} else if pf != monitoring.MirrorSessionSpec_ALL_PKTS.String() {
			dropReasonFilter = true
		} else {
			allPktsFilter = true
		}
		if dropReasonFilter && dropAllFilter {
			return i, false, fmt.Errorf("DROP_ALL cannot be specified with any other drop reason")
		}
	}
	dropOnlyFilter := (dropAllFilter || dropReasonFilter) && !allPktsFilter
	matchAll := false
	if len(ms.Spec.MatchRules) == 0 {
		matchAll = true
	}

	for _, mr := range ms.Spec.MatchRules {

		if mr.AppProtoSel != nil {
			for _, appName := range mr.AppProtoSel.Apps {
				// XXX: Check the App database once it is available
				return i, false, fmt.Errorf("Application Name %v is not identified, provide L4proto/port", appName)
			}
		}

		allSrc := false
		allDst := false
		if mr.Src == nil && mr.Dst == nil {
			matchAll = true
			if !dropOnlyFilter {
				return i, false, fmt.Errorf("Match-all type rule can be used only for mirror-on-drop")
			}
			continue
		}
		if mr.Src != nil {
			if len(mr.Src.IPAddresses) == 0 && len(mr.Src.MACAddresses) == 0 {
				allSrc = true
			}
			// TBD - Ensure only one of the three is specified? not all.
		}
		if mr.Dst != nil {
			if len(mr.Dst.IPAddresses) == 0 && len(mr.Dst.MACAddresses) == 0 {
				allDst = true
			}
		}
		if allSrc && allDst {
			matchAll = true
			if !dropOnlyFilter {
				return i, false, fmt.Errorf("Match-all type rule can be used only for mirror-on-drop")
			}
			continue
		}
		if matchAll {
			return i, false, fmt.Errorf("Cannot use multiple match-rules when match-all is used")
		}
	}
	if ms.Spec.Interfaces == nil && matchAll && !dropOnlyFilter {
		return i, false, fmt.Errorf("Match-all type rule can be used only for mirror-on-drop")
	}

	switch oper {
	case apiintf.CreateOper:
		pl := monitoring.MirrorSessionList{}
		policyKey := strings.TrimSuffix(pl.MakeKey(string(apiclient.GroupMonitoring)), "/")
		err := kv.List(ctx, policyKey, &pl)
		if err != nil {
			return nil, false, fmt.Errorf("error retrieving mirror policy: %v", err)
		}

		if len(pl.Items) >= statemgr.MaxMirrorSessions {
			return nil, false, fmt.Errorf("can't configure more than %v mirror policy", statemgr.MaxMirrorSessions)
		}
	}

	return i, true, nil
}

func registerMirrorSessionHooks(svc apiserver.Service, logger log.Logger) {
	r := mirrorSessionHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "MirrorSession")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("MirrorSession", apiintf.CreateOper).WithPreCommitHook(r.validateMirrorSession)
	svc.GetCrudService("MirrorSession", apiintf.UpdateOper).WithPreCommitHook(r.validateMirrorSession)
}
