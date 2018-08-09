package libs

import (
	"os"
	"os/exec"
)

// RunCommand is a wrapper function to pipe command output to os.Stdout.
func RunCommand(command string, args ...string) error {
	cmd := exec.Command(command, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	return cmd.Run()
}

// CaptureCommandStdout runs a command and captures its stdout and stderr.
func CaptureCommandStdout(command string, args ...string) ([]byte, error) {
	return exec.Command(command, args...).CombinedOutput()
}

// RunCommandNonBlocking runs a command in the background, and returns the command handler
func RunCommandNonBlocking(command string, args ...string) (*exec.Cmd, error) {
	cmd := exec.Command(command, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	err := cmd.Start()
	return cmd, err
}
