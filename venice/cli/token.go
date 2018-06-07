package vcli

import (
	"io/ioutil"
	"os"
	"os/user"
	"path"

	log "github.com/sirupsen/logrus"
)

const (
	tokenFile = "token"
)

func getCfgDir() string {
	usr, err := user.Current()
	if err != nil {
		log.Fatalf("Unable to determine current user: %v", err)
	}

	return path.Join(usr.HomeDir, ".pensando")
}

func saveToken(token string) {
	cfgDir := getCfgDir()
	if _, err := os.Stat(cfgDir); os.IsNotExist(err) {
		err = os.Mkdir(cfgDir, 0700)
		if err != nil {
			log.Fatalf("Failed to create config directory: %v", err)
		}
	}

	if err := ioutil.WriteFile(path.Join(cfgDir, tokenFile), []byte(token), 0700); err != nil {
		log.Fatalf("Failed to save token: %v", err)
	}
}

func getToken() string {
	cfgFile := path.Join(getCfgDir(), tokenFile)
	if _, err := os.Stat(cfgFile); os.IsNotExist(err) {
		return ""
	}

	token, err := ioutil.ReadFile(cfgFile)
	if err != nil {
		log.Fatalf("Failed to read token: %v", err)
	}
	return string(token)
}

func clearToken() {
	os.RemoveAll(path.Join(getCfgDir(), tokenFile))
}
