package services

import (
	"testing"

	"github.com/pensando/sw/venice/utils/log"
)

func TestNonExistentIP(t *testing.T) {
	ipService := NewIPService()
	found, err := ipService.HasIP("4.2.2.2")
	if err != nil {
		log.Errorf("Could not check for IP: %v", err)
	}
	if found {
		t.Errorf("Found a non-existent IP")
	}
}
