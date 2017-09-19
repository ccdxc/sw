package main

import (
	"crypto/rand"
	"encoding/base64"
	"flag"
	"github.com/pensando/sw/venice/utils/log"
	pkg1 "github.com/pensando/sw/venice/utils/log/example/default_logger/pkg1"
	pkg2 "github.com/pensando/sw/venice/utils/log/example/default_logger/pkg2"
	"strconv"
)

func getRandomString() string {
	rb := make([]byte, 32)
	rand.Read(rb)
	rs := base64.URLEncoding.EncodeToString(rb)
	return rs
}

func main() {
	var (
		logoutput = flag.String("logfile", "/tmp/logtester2.log", "redirect logs to file")
	)

	flag.Parse()

	logConfig := &log.Config{
		Module:      "LogTester2",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logoutput,
			MaxSize:    5, // 5 MB
			MaxBackups: 2,
			MaxAge:     7,
		},
	}

	log.SetConfig(logConfig)

	for i := 1; i <= 5; i++ {
		log.InfoLog("obj", strconv.Itoa(i), "data", getRandomString())
	}

	pkg1.Run()
	pkg2.Run()

	pkg1.Close()
	pkg2.Close()

	log.Info("Completed")
}
