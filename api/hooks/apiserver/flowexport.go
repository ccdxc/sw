package impl

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	hooksutils "github.com/pensando/sw/api/hooks/apiserver/utils"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/ctrler/tpm"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type flowExpHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (r *flowExpHooks) validateFlowExportPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	policy, ok := i.(monitoring.FlowExportPolicy)
	if ok != true {
		return i, false, fmt.Errorf("invalid object %T instead of monitoring.FlowExportPolicy", i)
	}

	if err := flowexportPolicyValidator(&policy); err != nil {
		return i, false, err
	}

	switch oper {
	case apiintf.CreateOper:
		pl := monitoring.FlowExportPolicyList{}
		policyKey := strings.TrimSuffix(pl.MakeKey(string(apiclient.GroupMonitoring)), "/")
		err := kv.List(ctx, policyKey, &pl)
		if err != nil {
			return nil, false, fmt.Errorf("error retrieving FlowExportPolicy: %v", err)
		}

		if len(pl.Items) >= tpm.MaxNumExportPolicy {
			return nil, false, fmt.Errorf("can't configure more than %v FlowExportPolicy", tpm.MaxNumExportPolicy)
		}
	}

	return i, true, nil
}

func registerFlowExpPolicyHooks(svc apiserver.Service, logger log.Logger) {
	r := flowExpHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "flowexphooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("FlowExportPolicy", apiintf.CreateOper).WithPreCommitHook(r.validateFlowExportPolicy)
	svc.GetCrudService("FlowExportPolicy", apiintf.UpdateOper).WithPreCommitHook(r.validateFlowExportPolicy)
}

func flowexportPolicyValidator(p *monitoring.FlowExportPolicy) error {
	spec := p.Spec
	if _, err := validateFlowExportInterval(spec.Interval); err != nil {
		return err
	}

	if _, err := validateTemplateInterval(spec.TemplateInterval); err != nil {
		return err
	}

	if err := validateFlowExportFormat(spec.Format); err != nil {
		return err
	}

	if len(spec.MatchRules) != 0 {
		data, err := json.Marshal(spec.MatchRules)
		if err != nil {
			return fmt.Errorf("failed to marshal, %v", err)
		}

		matchrules := []netproto.MatchRule{}
		if err := json.Unmarshal(data, &matchrules); err != nil {
			return fmt.Errorf("failed to unmarshal, %v", err)
		}

		if err := hooksutils.ValidateMatchRules(p.ObjectMeta, matchrules,
			func(meta api.ObjectMeta) (*netproto.Endpoint, error) { return nil, nil }); err != nil {
			return fmt.Errorf("error in match-rule, %v", err)
		}
	}

	if len(spec.Exports) == 0 {
		return fmt.Errorf("no targets configured")
	}

	if len(spec.Exports) > tpm.MaxNumCollectorsPerPolicy {
		return fmt.Errorf("cannot configure more than %d targets", tpm.MaxNumCollectorsPerPolicy)
	}

	feTargets := map[string]bool{}
	for _, export := range spec.Exports {
		if key, err := json.Marshal(export); err == nil {
			ks := string(key)
			if _, ok := feTargets[ks]; ok {
				return fmt.Errorf("found duplicate target %v %v", export.Destination, export.Transport)
			}
			feTargets[ks] = true
		}

		dest := export.Destination
		if dest == "" {
			return fmt.Errorf("destination is empty")
		}

		netIP, _, err := net.ParseCIDR(dest)
		if err == nil {
			dest = netIP.String()
		} else {
			netIP = net.ParseIP(dest)
		}

		if netIP == nil {
			// treat it as hostname and resolve
			s, err := net.LookupHost(dest)
			if err != nil || len(s) == 0 {
				return fmt.Errorf("failed to resolve name {%s}, error: %s", dest, err)
			}
		}

		if _, err := parsePortProto(export.Transport); err != nil {
			return err
		}
	}

	return nil
}

func validateFlowExportInterval(s string) (time.Duration, error) {

	interval, err := time.ParseDuration(s)
	if err != nil {
		return interval, fmt.Errorf("invalid interval %s", s)
	}

	if interval < time.Second {
		return interval, fmt.Errorf("too small interval %s", s)
	}

	if interval > 24*time.Hour {
		return interval, fmt.Errorf("too large interval %s", s)
	}

	return interval, nil
}

func validateTemplateInterval(s string) (time.Duration, error) {

	interval, err := time.ParseDuration(s)
	if err != nil {
		return interval, fmt.Errorf("invalid template interval %s", s)
	}

	if interval < time.Minute {
		return interval, fmt.Errorf("too small template interval %s", s)
	}

	if interval > 30*time.Minute {
		return interval, fmt.Errorf("too large template interval %s", s)
	}

	return interval, nil
}

func validateFlowExportFormat(s string) error {
	if strings.ToUpper(s) != "IPFIX" {
		return fmt.Errorf("invalid format %s", s)
	}
	return nil
}

func parsePortProto(src string) (uint32, error) {
	s := strings.Split(src, "/")
	if len(s) != 2 {
		return 0, fmt.Errorf("transport should be in protocol/port format")
	}

	if strings.ToUpper(s[0]) != "UDP" {
		return 0, fmt.Errorf("invalid protocol in %s", src)
	}

	port, err := strconv.Atoi(s[1])
	if err != nil {
		return 0, fmt.Errorf("invalid port in %s", src)
	}
	if uint(port) > uint(^uint16(0)) {
		return 0, fmt.Errorf("invalid port in %s", src)
	}

	return uint32(port), nil
}
