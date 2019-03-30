package testenv

// shouldLogToStdout determines whether logs should be sent to stdout by default
var shouldLogToStdout bool

// EnableLogToStdout switches logging to log to stdout by default
// It is invoked by utils/testutils init() function.
// This is set to true in development (not in production) environment
func EnableLogToStdout() {
	shouldLogToStdout = true
}

// GetLogToStdout returns true if rpckit is set to execute in test mode
func GetLogToStdout() bool {
	return shouldLogToStdout
}
