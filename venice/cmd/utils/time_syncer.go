package utils

import (
	"context"
	"fmt"
	"os/exec"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

// NtpContainer is the name of the ntp container which is used to set the time in the venice cluster.
//	name is obtained from venice.json and updated here by cmd
var NtpContainer string
var ntpCommand string // overridden for tests

// SyncTimeOnce syncs time with ntp servers specified below. As soon as it succeeds with atleast one servers, it returns
//	This tries each server with a timeout of 5 seconds
func SyncTimeOnce(ntpServers []string) {
	if NtpContainer == "" && ntpCommand == "" {
		log.Errorf("NTP Container image not known to be able sync time once")
		return
	}

	for _, v := range ntpServers {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err := syncTime(ctx, v, NtpContainer); err == nil {
			return
		}
	}
}

// syncTime does a one-time sync of time from ntpServer.
func syncTime(ctx context.Context, ntpServer, ntpContainer string) error {
	log.Debugf("Syncing time from server %s using container %s", ntpServer, ntpContainer)

	cmd := &exec.Cmd{}
	if ntpCommand == "" {
		cmd = exec.Command("docker", "run", "--net", "host", "--privileged", ntpContainer, "chronyd", "-q", fmt.Sprintf("pool %s iburst", ntpServer))
	} else {
		cmd = exec.Command(ntpCommand)
	}

	doneCh := make(chan error)
	go func(ch chan error) {
		// docker run --net host --privileged  registry.test.pensando.io:5000/pens-ntp:v0.4  chronyd -q 'pool pool.ntp.org iburst'
		_, err := cmd.CombinedOutput()
		if err != nil {
			ch <- errors.Errorf("Error %v doing one-time sync of clock", err)
		}
		close(ch)
	}(doneCh)

	// Wait for either timeout or the command completion
	select {
	case <-ctx.Done():
		return errors.Errorf("Timedout doing one-time sync of time from server %s", ntpServer)
	case e := <-doneCh:
		return e
	}
}
