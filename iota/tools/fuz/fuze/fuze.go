package fuze

// fuz exported metadata

import (
	"time"
)

type Input struct {
	Connections []*Connection `json:"connections"`
}

type Connection struct {
	ServerIPPort string `json:"ServerIPPort"`
	Proto        string `json:"proto"`
}

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

type Output struct {
	ErrorMsg           string            `json:"error"`
	SuccessConnections int32             `json:"successConnections"`
	FailedConnections  int32             `json:"failedConnections"`
	Connections        []*ConnectionData `json:"connections"`
}

func NewConnData(serverIP, proto string, timeDuration time.Duration, isserver bool) *ConnectionData {
	return &ConnectionData{ServerIPPort: serverIP, Proto: proto, ConnDurtion: timeDuration,
		IsServer: isserver}
}
