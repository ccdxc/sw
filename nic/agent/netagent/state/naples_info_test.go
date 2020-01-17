// +build ignore

package state

import (
	"fmt"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

//--------------------- Happy Path Tests ---------------------//
func TestNaplesInfo(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	naplesInfo, err := ag.GetNaplesInfo()
	AssertOk(t, err, "could not get naples info")
	Assert(t, len(naplesInfo.Mode) != 0, "Naples mode not set")
	fmt.Println(naplesInfo)

}

func TestNaplesUptime(t *testing.T) {
	ag, _, _ := createNetAgent(t)
	Assert(t, ag != nil, "Failed to create agent %#v", ag)
	defer ag.Stop()

	uptime, err := ag.GetNetagentUptime()
	AssertOk(t, err, "could not get naples uptime")
	fmt.Println(uptime)
	Assert(t, len(uptime) != 0, "Uptime is nil")
}
