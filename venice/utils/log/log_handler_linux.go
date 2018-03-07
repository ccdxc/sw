package log

import (
	"fmt"
	"os"
	"syscall"
)

// Redirect stderr to file (preferred) or stdout based on config
// TODO: Need to investigate a way to redirect stderr to both file & stdout
// Syscall Dup2 only allows redirecting to one Fd.

// LogtoFileHandler writes the log to the specified file on linux based platforms using Dup3 syscall
func (c *Config) LogtoFileHandler() (err error) {
	logFile, err := os.OpenFile(c.FileCfg.Filename, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		panic(fmt.Sprintf("Failed to open logfile: %s err: %v", c.FileCfg.Filename, err))
	}
	syscall.Dup3(int(logFile.Fd()), int(os.Stderr.Fd()), 0)
	return
}

// LogtoStdoutHandler writes the log to stdout on linux based platforms using Dup3 syscall
func (c *Config) LogtoStdoutHandler() error {
	return syscall.Dup3(int(os.Stdout.Fd()), int(os.Stderr.Fd()), 0)
}
