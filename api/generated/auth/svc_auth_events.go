// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package auth is a auto generated package.
Input file: svc_auth.proto
*/
package auth

const (
	LoginFailed = "LoginFailed"
)

// GetEventTypes returns the list of event types; this will be set of event types supported,
// anything beyond this list will not be supported by the events recorder.
func GetEventTypes() []string {
	return []string{
		LoginFailed,
	}
}
