package types

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"strings"
)

// Client is a client to the testtracker service.
type Client struct {
	url    *url.URL
	client *http.Client
}

// NewClient returns a new testtracker service client
func NewClient(baseurl string) (*Client, error) {
	b := baseurl
	if !strings.HasPrefix(b, "http://") {
		b = fmt.Sprintf("http://%s", b)
	}
	_, _, err := net.SplitHostPort(baseurl)
	if err != nil {
		b = fmt.Sprintf("%s:%d", b, TrackerPort)
	}

	u, err := url.Parse(b)
	if err != nil {
		return nil, err
	}

	return &Client{url: u, client: &http.Client{}}, nil
}

// Report sents report to remote testtracker service
func (c *Client) Report(r *Reports) error {
	content, err := json.Marshal(r)
	if err != nil {
		return err
	}

	u, err := c.url.Parse("/report")
	if err != nil {
		return err
	}

	req, err := http.NewRequest("POST", u.String(), bytes.NewBuffer(content))
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		return err
	}

	if resp.StatusCode != 200 {
		content, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			return err
		}
		return errors.New(strings.TrimSpace(string(content)))
	}

	return nil
}
