package debug

// This package makes exporting of debug counters from internal components easy
//
// Usage:
//	Somewhere in the codepath, call this once
// debugStats := debug.New("instance")

// Starts a http server if you are not doing this already
//	go http.ListenAndServe(restURL, nil)

// if you are using a different http demux like gorilla/martini register a handler like below instead
//     martiniRouter.Get("/debug/vars", expvar.Handler())

// in your code you can increment any counter by giving a name like:
// debugStats.Increment("DeleteNetwork")

// From bash you can do GET to get the value of counters - Example output below
// 	curl http://10.100.0.11:9006/debug/vars

// {
// "cmdline": ["/npm","-resolver-urls","node1:9009,node2:9009,node3:9009"],
// "debugStats_instance": {"CreateNetwork": 123020, "DeleteNetwork": 123020}
// }
