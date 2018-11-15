package vcli

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"os/user"
	"path"

	log "github.com/sirupsen/logrus"
)

const (
	tokenFile = "token"
)

type tokenInfo struct {
	Token  string `json:"token"`
	Server string `json:"server"`
}

func getCfgDir() string {
	usr, err := user.Current()
	if err == nil {
		return path.Join(usr.HomeDir, ".pensando")
	}
	return ".pensando"
}

func saveToken(token, server string) {
	cfgDir := getCfgDir()
	if _, err := os.Stat(cfgDir); os.IsNotExist(err) {
		err = os.Mkdir(cfgDir, 0700)
		if err != nil {
			log.Fatalf("Failed to create config directory: %v", err)
		}
	}

	ti := &tokenInfo{Token: token, Server: server}
	jData, err := json.MarshalIndent(ti, "", "  ")
	if err != nil {
		log.Fatalf("unable to marshal structure %+v, error '%s'", ti, err)
	}

	if err := ioutil.WriteFile(path.Join(cfgDir, tokenFile), jData, 0600); err != nil {
		log.Fatalf("Failed to save token: %v", err)
	}
}

func getTokenInfo() *tokenInfo {
	ti := &tokenInfo{}
	cfgFile := path.Join(getCfgDir(), tokenFile)
	if _, err := os.Stat(cfgFile); os.IsNotExist(err) {
		return ti
	}

	jData, err := ioutil.ReadFile(cfgFile)
	if err != nil {
		log.Fatalf("Failed to read token: %v", err)
	}
	if err := json.Unmarshal(jData, ti); err != nil {
		log.Fatalf("Unmarshal error '%s' data '%s'", err, jData)
	}
	return ti
}

func getLoginToken() string {
	ti := getTokenInfo()
	return ti.Token
}

func getLoginServer() string {
	ti := getTokenInfo()
	return ti.Server
}

func clearToken() {
	os.RemoveAll(path.Join(getCfgDir(), tokenFile))
}
