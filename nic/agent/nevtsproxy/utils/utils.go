package utils

import (
	"crypto/rand"
	"encoding/json"
	"fmt"
	"os/exec"

	"github.com/pkg/errors"
)

const (
	naplesHwUUIDFile = "/tmp/fru.json"
)

// GetNaplesDefaultUUID gets the Naples card UUID
func GetNaplesDefaultUUID() (string, error) {
	hwUUID, err := getHwUUID()
	if err == nil {
		return hwUUID, nil
	}

	b := make([]byte, 8)
	_, err = rand.Read(b)
	if err != nil {
		return "", err
	}

	// This will ensure that we have unicast MAC
	b[0] = (b[0] | 2) & 0xfe
	fakeMAC := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])

	return fakeMAC, nil

}

func getHwUUID() (uuid string, err error) {
	out, err := exec.Command("/bin/bash", "-c", "cat "+naplesHwUUIDFile).Output()
	if err != nil {
		return "", err
	}

	var deviceJSON map[string]interface{}
	if err := json.Unmarshal([]byte(out), &deviceJSON); err != nil {
		return "", errors.Errorf("Error reading %s file", naplesHwUUIDFile)
	}

	if val, ok := deviceJSON["mac-address"]; ok {
		return val.(string), nil
	}

	return "", errors.Errorf("Mac address not present in %s file", naplesHwUUIDFile)
}
