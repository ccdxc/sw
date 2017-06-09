package main

import (
	"flag"
	"fmt"
	"io"
	"os"

	"google.golang.org/grpc/grpclog"

	_ "github.com/pensando/sw/api/generated/exports/apigw"
	"github.com/pensando/sw/apigw"
	apigwpkg "github.com/pensando/sw/apigw/pkg"
	"github.com/pensando/sw/utils/log"
)

func main() {
	var (
		httpaddr  = flag.String("port", ":8080", "HTTP port to listen on")
		debugflag = flag.Bool("debug", false, "enable debug mode")
		host      = flag.String("host", "localhost", "host identity")
		logoutput = flag.String("logfile", "", "redirect logs to file")
	)

	flag.Parse()

	var pl log.Logger
	{
		var w io.Writer
		var err error
		if *logoutput == "" {
			w = os.Stdout
			fmt.Printf("setting log output to Stdout\n")
		} else {
			w, err = os.OpenFile(*logoutput, os.O_TRUNC|os.O_CREATE|os.O_WRONLY, 0777)
			fmt.Printf("Output redirected to to [%v]\n", *logoutput)
			if err != nil {
				fmt.Printf("Unable to open file [%v]\n", *logoutput)
				return
			}
			w.Write([]byte("Logging start for apigateway\n"))

		}
		pl = log.GetNewLogger(*debugflag).SetOutput(w).WithContext("module", "ApiGateway", "host", *host+*httpaddr)

	}
	var config apigw.Config
	{
		config.HttpAddr = *httpaddr
		config.DebugMode = *debugflag
		config.Logger = pl
	}
	pl.Log("msg", "Starting Run")
	gw := apigwpkg.MustGetApiGateway()
	grpclog.SetLogger(pl)
	gw.Run(config)
}
