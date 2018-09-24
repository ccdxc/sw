package main

import (
	"net/http"
)

func main() {
	fs := http.FileServer(http.Dir("/var/log/pensando/"))
	http.Handle("/logs/", http.StripPrefix("/logs", fs))

	http.ListenAndServe(":8080", nil)
}
