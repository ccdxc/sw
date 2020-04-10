package radius

import (
	"errors"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
)

const defaultRadiusPort int = 1812

// AddDefaultPort checks if a port is present in URL of Radius servers, else it adds default port
func AddDefaultPort(url string) (string, error) {
	_, _, err := net.SplitHostPort(url)
	if err != nil && strings.Contains(err.Error(), "missing port in address") {
		url = url + ":" + strconv.Itoa(defaultRadiusPort)
		return url, nil
	}
	return url, err
}

// ValidateRadiusConfig validates radius configuration
func ValidateRadiusConfig(config *auth.Radius) []error {
	var errs []error
	if config == nil {
		errs = append(errs, errors.New("radius authenticator config not defined"))
		return errs
	}
	if len(config.Domains) > 1 {
		errs = append(errs, errors.New("only one radius domain is supported"))
		return errs
	}
	if len(config.Domains) == 0 {
		errs = append(errs, errors.New("no radius domain defined"))
		return errs
	}
	domain := config.Domains[0]
	if domain.NasID == "" || len(domain.NasID) > 253 {
		errs = append(errs, errors.New("NAS ID cannot be empty or longer than 253 bytes"))
	}
	if len(domain.Servers) == 0 {
		errs = append(errs, errors.New("radius server not defined"))
	}
	for _, srv := range domain.Servers {
		if srv.Url == "" {
			errs = append(errs, errors.New("radius <address:port> not defined"))
		} else {
			if url, portErr := AddDefaultPort(srv.Url); portErr == nil {
				srv.Url = url
			} else {
				errs = append(errs, portErr)
			}
		}
	}
	return errs
}
