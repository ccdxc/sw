package vcli

import (
	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/venice/utils/netutils"
)

// httpGet fetches json object from an http get request
func httpGet(url string, jdata interface{}) error {
	return netutils.HTTPGet(url, jdata)
}

// httpDelete sssues http delete request at specified url
func httpDelete(url string) error {
	return netutils.HTTPDelete(url, nil, nil)
}

// httpPost posts a json object on the specified url
func httpPost(url string, jdata interface{}) error {
	var response []byte
	netutils.HTTPPost(url, jdata, response)
	log.Debugf(string(response))

	return nil
}

// httpPut updates a json object on the specified url
func httpPut(url string, jdata interface{}) error {
	var response []byte
	netutils.HTTPPut(url, jdata, response)
	log.Debugf(string(response))

	return nil
}
