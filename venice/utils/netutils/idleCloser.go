// +build go1.12

package netutils

// CloseIdleConnections closes any idle connections
func (hc *HTTPClient) CloseIdleConnections() {
	hc.c.CloseIdleConnections()
}
