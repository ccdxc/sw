package ipif

import (
	"testing"

	"github.com/pensando/sw/nic/agent/nmd/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// Mock IP Client is testing in other packages. This dummy test is added to ensure ipif coverage doesn't report a spurious failure.
func TestNewMockIPClient(t *testing.T) {
	mockNMD := mock.CreateMockNMD(t.Name())
	mockClient, err := NewMockIPClient(mockNMD, "lo")
	AssertOk(t, err, "Mock IP Client instantiation must suceeed")

	mockClient.DoDHCPConfig()
	mockClient.DoNTPSync()
	mockClient.GetDHCPState()
	mockClient.StopDHCPConfig()
}
