package impl

import (
	"context"
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type mirrorSessionHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

const (
	// Finalize these parameters once we decide how to store the packets captured by Venice
	veniceMaxPacketSize  = 256
	veniceMaxPacketCount = 1000
)

func (r *mirrorSessionHooks) validateMirrorSession(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	ms, ok := i.(monitoring.MirrorSession)
	if !ok {
		return i, false, errors.New("Invalid input type")
	}
	// checks:
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

	if len(ms.Spec.Collectors) == 0 {
		return i, false, errors.New("Need atleast one mirror collector")
	}
	numVeniceCollectors := 0
	for _, c := range ms.Spec.Collectors {
		if c.Type == monitoring.PacketCollectorType_VENICE.String() {
			if numVeniceCollectors > 0 {
				return i, false, errors.New("Only one Venice collector is supported per mirror session")
			}
			numVeniceCollectors++
		} else if c.Type == monitoring.PacketCollectorType_ERSPAN.String() {
			if c.ExportCfg.Destination == "" {
				return i, false, errors.New("Provide valid destination for ERSPAN collector")
			}
			// Checking for Destition and other parameters inside ExportCfg XXX
		} else {
			// this is already checked by venice.check
			return i, false, errors.New("Unsupported collector type")
		}
	}
	if numVeniceCollectors > 0 && ms.Spec.PacketSize > veniceMaxPacketSize {
		errStr := fmt.Errorf("Max packet size allowed by Venice collector is %v", veniceMaxPacketSize)
		return i, false, errStr
	}
	if ms.Spec.StartConditions.ScheduleTime != nil {
		schTime, err := ms.Spec.StartConditions.ScheduleTime.Time()
		if err != nil {
			return i, false, errors.New("Unsupported format used for schedule-time")
		}
		if ms.Status.State == "" || ms.Status.State == monitoring.MirrorSessionState_SCHEDULED.String() {
			// Validator is called even on status update... should be avoided, until then
			// avoid time check if not in SCHEDULED state XXX this is not fool-proof
			if !schTime.After(time.Now()) {
				return i, false, errors.New("Schedule time must be in future")
			}
		}
	}
	if numVeniceCollectors != 0 {
		if ms.Spec.StopConditions.MaxPacketCount == 0 || ms.Spec.StopConditions.MaxPacketCount > veniceMaxPacketCount {
			errStr := fmt.Errorf("Invalid max-packets specified for Venice collector must be 0 < max-packets < %d", veniceMaxPacketCount)
			return i, false, errStr
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
		if dropReasonFilter == true && dropAllFilter == true {
			return i, false, errors.New("DROP_ALL cannot be specified with any other drop reason")
		}
	}
	dropOnlyFilter := (dropAllFilter || dropReasonFilter) && !allPktsFilter
	matchAll := false
	if len(ms.Spec.MatchRules) == 0 {
		matchAll = true
	}
	for _, mr := range ms.Spec.MatchRules {
		allSrc := false
		allDst := false
		if mr.Src == nil && mr.Dst == nil {
			matchAll = true
			if !dropOnlyFilter {
				return i, false, errors.New("Match-all type rule can be used only for mirror-on-drop")
			}
			continue
		}
		if mr.Src != nil {
			if len(mr.Src.Endpoints) == 0 && len(mr.Src.IPAddresses) == 0 && len(mr.Src.MACAddresses) == 0 {
				allSrc = true
			}
			// TBD - Ensure only one of the three is specified? not all.
		}
		if mr.Dst != nil {
			if len(mr.Dst.Endpoints) == 0 && len(mr.Dst.IPAddresses) == 0 && len(mr.Dst.MACAddresses) == 0 {
				allDst = true
			}
		}
		if allSrc && allDst {
			matchAll = true
			if !dropOnlyFilter {
				return i, false, errors.New("Match-all type rule can be used only for mirror-on-drop")
			}
			continue
		}
		if matchAll {
			return i, false, errors.New("Cannot use multiple match-rules when match-all is used")
		}
	}
	if matchAll && !dropOnlyFilter {
		return i, false, errors.New("Match-all type rule can be used only for mirror-on-drop")
	}

	return i, true, nil
}

func registerMirrorSessionHooks(svc apiserver.Service, logger log.Logger) {
	r := mirrorSessionHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "MirrorSession")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("MirrorSession", apiserver.CreateOper).WithPreCommitHook(r.validateMirrorSession)
	svc.GetCrudService("MirrorSession", apiserver.UpdateOper).WithPreCommitHook(r.validateMirrorSession)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("monitoring.MonitoringV1", registerMirrorSessionHooks)
}
