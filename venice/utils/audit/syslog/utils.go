package syslog

import (
	"encoding/json"
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/syslog"
)

// ValidateSyslogAuditorConfig validates syslog auditor config
func ValidateSyslogAuditorConfig(config *monitoring.SyslogAuditor) []error {
	var errs []error
	if config != nil && config.Enabled {
		if len(config.Targets) == 0 {
			errs = append(errs, fmt.Errorf("no syslog servers specified"))
		}
		evTargets := map[string]bool{}
		for _, target := range config.Targets {
			if key, err := json.Marshal(target); err == nil {
				ks := string(key)
				if _, ok := evTargets[ks]; ok {
					errs = append(errs, fmt.Errorf("found duplicate target %v %v", target.Destination, target.Transport))
				}
				evTargets[ks] = true
			}
			if err := syslog.ValidateDestination(target.Destination); err != nil {
				errs = append(errs, err)
			}
			if err := syslog.ValidateTransport(target.Transport); err != nil {
				errs = append(errs, err)
			}
		}
	}
	return errs
}
