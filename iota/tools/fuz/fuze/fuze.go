package fuze

// fuz exported metadata

import (
	"time"
)

// Input is connection lists
type Input struct {
	Connections []*Connection `json:"connections"`
}

// Connection is connection instance
type Connection struct {
	ServerIPPort string `json:"ServerIPPort"`
	Proto        string `json:"proto"`
}

// ConnectionData has info about conn
type ConnectionData struct {
	ServerIPPort string        `json:"ServerIPPort"`
	ClientIPPort string        `json:"ClientIPPort"`
	Proto        string        `json:"proto"`
	ErrorMsg     string        `json:"error"`
	DataSent     int           `json:"dataSent"`
	DataReceived int           `json:"dataReceived"`
	ConnDurtion  time.Duration `json:"duration"`
	IsServer     bool          `json:"isserver"`
	Success      int32         `json:"success"`
	Failed       int32         `json:"failed"`
}

// Output is output!
type Output struct {
	ErrorMsg           string            `json:"error"`
	SuccessConnections int32             `json:"successConnections"`
	FailedConnections  int32             `json:"failedConnections"`
	Connections        []*ConnectionData `json:"connections"`
}

// NewConnData returns a new connection data
func NewConnData(serverIP, proto string, timeDuration time.Duration, isserver bool) *ConnectionData {
	return &ConnectionData{ServerIPPort: serverIP, Proto: proto, ConnDurtion: timeDuration,
		IsServer: isserver}
}
