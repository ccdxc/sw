package main

import (
	"crypto/rand"
	"encoding/base64"
	"flag"
	"strconv"

	"github.com/pensando/sw/venice/utils/log"
	pkg1 "github.com/pensando/sw/venice/utils/log/example/new_logger/pkg1"
	pkg2 "github.com/pensando/sw/venice/utils/log/example/new_logger/pkg2"
)

func getRandomString() string {
	rb := make([]byte, 32)
	rand.Read(rb)
	rs := base64.URLEncoding.EncodeToString(rb)
	return rs
}

func main() {
	var (
		logoutput = flag.String("logfile", "/tmp/logtester1.log", "redirect logs to file")
	)

	flag.Parse()

	// Instantiate logger once in main() and pass it to other components
	// or packages via dependency injection pattern. Also this approach
	// avoids package-globals loggers, which clearly will causes issues
	// when multiple packages share a dependent package.
	var logger log.Logger
	{
		logConfig := &log.Config{
			Module:      "LogTester1",
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
		logger = log.GetNewLogger(logConfig)
	}

	for i := 1; i <= 5; i++ {
		logger.InfoLog("obj", strconv.Itoa(i), "data", getRandomString())
	}

	// Instantiate service1 component, passing logger as dependency
	svc1 := pkg1.NewInstance(logger)
	svc1.Run()

	// Instantiate service2 component, passing logger as dependency
	svc2 := pkg2.NewInstance(logger)
	svc2.Run()

	svc1.Close()
	svc2.Close()

	logger.Info("Completed")
}
