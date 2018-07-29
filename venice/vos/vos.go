package vos

import (
	"os"

	minio "github.com/minio/minio/cmd"

	"github.com/pensando/sw/venice/utils/log"
)

// New creaate an instance of obj store
func New(args []string) error {
	os.Setenv("MINIO_ACCESS_KEY", "miniokey")
	os.Setenv("MINIO_SECRET_KEY", "minio0523")
	log.Infof("minio env: %+v", os.Environ())
	log.Infof("minio args:  %+v", args)
	minio.Main(args)
	return nil
}
