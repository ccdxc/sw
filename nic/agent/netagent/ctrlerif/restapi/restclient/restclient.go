// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package restclient

// Response captures the HTTP Response sent by Agent REST Server
type Response struct {
	StatusCode int      `json:"status-code,omitempty"`
	Error      string   `json:"error,omitempty"`
	References []string `json:"references,omitempty"`
}

// NetagentClient is HTTP rest client for netagent
type NetagentClient struct {
	agentURL string // agent's URL
}

// NewNetagentClient returns a new netagent rest client
func NewNetagentClient(url string) *NetagentClient {
	return &NetagentClient{
		agentURL: url,
	}
}

/*
func (cl *NetagentClient) EndpointList() ([]netproto.Endpoint, error) {
	var endpointList []netproto.Endpoint

	err := netutils.HTTPGet("http://"+cl.agentURL+"/api/endpoints/", &endpointList)

	return endpointList, err
}
*/
