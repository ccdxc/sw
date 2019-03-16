package state

import "fmt"

func syncTimeOnce(servers []string) error {
	if len(servers) == 0 {
		return fmt.Errorf("Need to specify a server to sync time")
	}
	cmd := "/usr/sbin/ntpd -q "
	for _, s := range servers {
		cmd = cmd + "-p " + s
	}
	return runCmd(cmd)
}

func startNtpClient(servers []string) error {
	if len(servers) == 0 {
		return fmt.Errorf("Need to specify a server to sync time")
	}

	cmd := "/sbin/start-stop-daemon -b -S  -m -p /var/run/ntpd.pid --exec /usr/sbin/ntpd -- -n "
	for _, s := range servers {
		cmd = cmd + "-p " + s
	}
	return runCmd(cmd)
}

func stopNtpClient() error {
	cmd := "/sbin/start-stop-daemon -K -q -p /var/run/ntpd.pid"
	return runCmd(cmd)
}
