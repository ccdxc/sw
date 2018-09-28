package server

import (
	"io"
	"log"
	"os"

	agent "github.com/pensando/sw/iota/svcs/agent"
)

type server struct {
	name   string
	logger *log.Logger // Logger for each entity.
}

func (s *server) log(msg interface{}) {

	if s.logger == nil {
		file, err := os.OpenFile(agent.LogDir+"/"+s.name+".log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
		if err != nil {
			log.Fatalln("Failed to open log file", "file.txt", ":", err)
		}
		multi := io.MultiWriter(file, os.Stdout)
		s.logger = log.New(multi, s.name, log.Ldate|log.Ltime)
		s.logger.Printf("Logger %p", s.logger)
	}
	s.logger.Println(msg)
}
