package radius

import (
	"errors"

	"github.com/pensando/sw/api/generated/auth"
)

// ValidateRadiusConfig validates radius configuration
func ValidateRadiusConfig(config *auth.Radius) []error {
	var errs []error
	if config == nil {
		errs = append(errs, errors.New("radius authenticator config not defined"))
		return errs
	}
	if config.Enabled {
		if config.NasID == "" || len(config.NasID) > 253 {
			errs = append(errs, errors.New("NAS ID cannot be empty or longer than 253 bytes"))
		}
		if len(config.Servers) == 0 {
			errs = append(errs, errors.New("radius server not defined"))
		}
		for _, srv := range config.Servers {
			if srv.Url == "" {
				errs = append(errs, errors.New("radius <address:port> not defined"))
			}
		}
	}
	return errs
}
