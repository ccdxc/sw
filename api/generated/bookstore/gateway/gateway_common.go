// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package bookstoreGwService is a auto generated package.
Input file: protos/example.proto
*/
package bookstoreGwService

import (
	"net/http"

	"github.com/GeertJohan/go.rice"
	"github.com/pkg/errors"

	"github.com/pensando/grpc-gateway/runtime"
	"github.com/pensando/sw/utils/log"
)

var mux *runtime.ServeMux
var fileCount int

const codecSize = 1024 * 1024

func registerSwaggerDef(m *http.ServeMux, logger log.Logger) error {
	box, err := rice.FindBox("../../../../../sw/api/generated/bookstore/swagger")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.Box")
		return err
	}
	content, err := box.Bytes("example.swagger.json")
	if err != nil {
		err = errors.Wrap(err, "error opening rice.File")
		return err
	}
	m.HandleFunc("/swagger/bookstore/", func(w http.ResponseWriter, r *http.Request) {
		w.Write(content)
	})
	return nil
}
