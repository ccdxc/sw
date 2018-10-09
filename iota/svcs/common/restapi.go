package common

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

// HTTPPost does an HTTP Post to the URL
func HTTPPost(url string, jsonStr string) (string, error) {

	url = fmt.Sprintf("http://%s", url)
	// Perform HTTP POST operation
	res, err := http.Post(url, "application/json", strings.NewReader(jsonStr))
	if err != nil {
		log.Errorf("Error during http POST. Err: %v", err)
		return "", err
	}

	if res.StatusCode != http.StatusOK {
		log.Errorf("Server returned non OK HTTP Code: %v", res.StatusCode)
		return "", fmt.Errorf("server returned non OK HTTP Code: %v", res.StatusCode)
	}

	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Errorf("Could not read the server response. Err: %v", err)
		return "", fmt.Errorf("could not read the server response. Err: %v", err)
	}
	defer res.Body.Close()

	return string(body), nil
}
