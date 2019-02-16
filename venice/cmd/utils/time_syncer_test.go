package utils

import "testing"

func TestSyncTimeOnce(t *testing.T) {
	ntpCommand = "ls" // dummy Command
	SyncTimeOnce([]string{"first.server.name", "second.server.name"})
	SyncTimeOnce([]string{"one.server.name"})
}
