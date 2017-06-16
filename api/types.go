package api

// TODO(sanjayt): Move StatusResults to enum in .proto file
var (
	// StatusResultOk indicates that the operation succeeded (http code 200).
	StatusResultOk = StatusResult{"Ok"}

	// StatusResultExpired indicates that the data being requested has expired
	// (http code 410).
	StatusResultExpired = StatusResult{"Expired"}

	// StatusResultInternalError indicates that the server had an internal error
	// processing the request (http code 500).
	StatusResultInternalError = StatusResult{"Internal Error"}
)
