package debug

// This package is for easily collecting internal metrics locally and
// for keeping status data that is available for a socket.
//
// Usage:
// Somewhere in the codepath, call this once
//
// 		dbg := debug.New("path/to/debug.sock", SocketInfoFunction
//
// To start the socket server, call:
//
//		dbg.StartServer()
//
// You can then connect to the socket with
//
//  	curl --unix-socket /path/to/debug.sock http://localhost/debug
//
// Which will trigger socketInfoFunction to run and will return
// the value over the socket
//
// If you already have a server running, you can just use the
// DebugHandler function
//
// 		r.HandleFunc("/debug", dbg.DebugHandler).Methods("GET")
//
// For collecting local metrics, the module must have first initialized ntsdb:
//
// 		ntsdb.Init(ctx, opts)
//
// then build the metric obj
//
//    dbg.BuildMetricObj(tableName, keyTags)
//
// You can the access the table through
//
//		dbg.MetricObj
//
// Collected metrics are aviailable on the local server being run by ntsdb
