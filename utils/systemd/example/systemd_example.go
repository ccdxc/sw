package main

import (
	"flag"
	"os"
	"time"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/utils/systemd"
)

var (
	waitTime    = flag.Int("wait", 10, "Duration to wait.")
	startFlag   = flag.Bool("start", false, "Start a target")
	stopFlag    = flag.Bool("stop", false, "Stop a target")
	restartFlag = flag.Bool("restart", false, "Restart a target")
	target      = flag.String("target", "pensando.target", "Target to watch/control")
)

func main() {
	flag.Parse()

	s, evChan, errChan := systemd.NewWatcher()

	s.Subscribe(*target)

	// keep observing events till timeout fires
	timeout := make(chan bool, 1)
	go func() {
		time.Sleep(time.Duration(*waitTime) * time.Second)
		close(timeout)
	}()

	if *startFlag {
		err := systemd.StartTarget(*target)
		log.Printf("startTarget %v err: %v\n", *target, err)
	}
	if *restartFlag {
		err := systemd.RestartTarget(*target)
		log.Printf("RestartTarget %v err: %v\n", *target, err)
	}
	if *stopFlag {
		err := systemd.StopTarget(*target)
		log.Printf("StopTarget %v err: %v\n", *target, err)
	}
	for {
		select {
		case v, ok := <-evChan:
			if !ok {
				log.Printf("event channel closed. Exiting.")
				os.Exit(1)
			}
			log.Printf("update event %+v \n", v)
		case err := <-errChan:
			log.Printf("from error channel %+v \n", err)
		case <-timeout:
			log.Printf("Reached timeout. Exiting.")
			s.Close()
			// to check for any race conditions in exit path
			time.Sleep(time.Second)
			os.Exit(0)
		}
	}

}
