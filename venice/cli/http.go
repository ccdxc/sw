package vcli

import (
	"bytes"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"

	log "github.com/sirupsen/logrus"
)

var client = &http.Client{}

// httpGetRaw gets the raw data from an http request, i.e. it basically execute curl
func httpGetRaw(url string) ([]byte, error) {
	r, err := http.Get(url)
	if err != nil {
		return []byte{}, err
	}
	defer r.Body.Close()

	log.Debugf("HTTP Get: %s", url)
	if r.StatusCode != 200 {
		return []byte{}, errors.New(r.Status)
	}

	response, err := ioutil.ReadAll(r.Body)

	return response, err
}

// httpGet fetches json object from an http get request
func httpGet(url string, jdata interface{}) error {
	response, err := httpGetRaw(url)
	if err != nil {
		return err
	}

	return json.Unmarshal(response, jdata)
}

// httpDelete sssues http delete request at specified url
func httpDelete(url string) error {

	req, err := http.NewRequest("DELETE", url, nil)

	log.Debugf("URL Delete: %s\n", url)
	r, err := http.DefaultClient.Do(req)
	if err != nil {
		panic(err)
	}
	defer r.Body.Close()

	if r.StatusCode != 200 {
		return errors.New(r.Status)
	}

	return nil
}

// httpPost posts a json object on the specified url
func httpPost(url string, jdata interface{}) error {
	buf, err := json.Marshal(jdata)
	if err != nil {
		return err
	}

	log.Debugf("HTTP Post: %s \n\tdata %+v", url, jdata)

	body := bytes.NewBuffer(buf)
	r, err := http.Post(url, "application/json", body)
	if err != nil {
		return err
	}
	defer r.Body.Close()

	if r.StatusCode != 200 {
		return errors.New(r.Status)
	}

	response, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return err
	}
	log.Debugf(string(response))

	return nil
}
