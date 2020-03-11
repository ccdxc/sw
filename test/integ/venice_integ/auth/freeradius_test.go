package auth

import (
	"testing"

	. "github.com/pensando/sw/venice/utils/authn/testutils"
)

func getFreeRadiusConfig() *RadiusConfig {
	return &RadiusConfig{
		URL:        "10.2.60.91:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "radiususer",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{"Admin"},
	}
}

func getFreeRadiusNoGroupUserConfig() *RadiusConfig {
	return &RadiusConfig{
		URL:        "10.2.60.91:1812",
		NasID:      "Venice",
		NasSecret:  "testing123",
		User:       "nogroup",
		Password:   "password",
		Tenant:     "default",
		UserGroups: []string{},
	}
}

func TestFreeRadiusAuthentication(t *testing.T) {
	config := getFreeRadiusConfig()
	testAuthenticator(t, config)
}

func TestFreeRadiusNoUserGroup(t *testing.T) {
	testNoGroupUser(t, getFreeRadiusNoGroupUserConfig())
}
