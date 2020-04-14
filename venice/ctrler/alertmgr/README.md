Object Based Alerts Manager
---------------------------
Spec: https://docs.google.com/document/d/12LK50KkePYjc73S9zSdURIHVuDD0uRL7jtDQN9nziyw

Files
-----
<pre>
.
├── README.md
├── alertengine              - create, delete, clear, reopen, resolve, garbage-collect alerts
│   ├── alertengine.go
│   └── alertengine_test.go
├── alertmgr.go              - run watcher, policy engine, alert engine, exporter
├── cmd - run alertmgr
│   └── alertmgr
│       └── main.go
├── exporter                 - export alerts to destinations
│   └── exporter.go
├── objdb                    - local cache of KV Store
│   └── objdb.go
├── policyengine             - run alert policies on managed objects
│   ├── policyengine.go
│   └── policyengine_test.go
└── watcher                  - run API group watchers
    ├── watcher.go
    └── watcher_test.go
</pre>


Concurrency Model
-----------------
1. Alert Manager is implemented as a pipeline using channels.

   [API server] -> watcher -> policyengine -> alertengine -> exporter

2. Watcher, policyengine, alertengine, exporter (pipeline stages) run in
   separate goroutines.


Error Handling
--------------
1. Each pipeline stage has its own context. Alertmgr has the main
   context.
2. Each pipeline stage has a buffered error channel with room for one
   error; on encountering an error, the error is sent on the error
   channel.
3. The run goroutine (in alertmgr.go) monitors the error channels. On
   receiving an error, it restarts the pipeline stages (by calling their
   cancel() function) and the API client.
4. Each pipeline stage handles ctx.Done() and cleans up resources like
   the error channel, output channel. Internal maps/caches are retained.
5. Each pipeline stage can have multiple goroutines which may
   independently attempt to send an error on the error channel; only one
   of them will succeed. Each such goroutine should also handle
   ctx.Done() and terminate.

Initialization / Restart
------------------------
1. On start up, alertmgr fetches alerts that are already in the KV
   store, along with the referenced objects and alert polices, and
   caches them in the object DB. This is done before the pipeline stages
   are started. The watcher will watch only subsequent updates to KV
   store using the resource version.

The object DB is also used when a new alert policy is created; the
policy is run on all existing objects of the referenced kind.


TODO
----
1. Error handling in function dispatchAlert() in alertengine.go.
1. Exporter.
2. Packaging.
3. Generate events for all alert state changes.
4. More unit tests.
5. Integration tests.
6. E2E tests.
7. Output channels of watcher, policyengine, alertengine may have to be
   buffered channels - will updated after E2E tests.
8. Since alert policies are tenanted, tenant needs to be taken into
   account when applying policies on objects.
9. Remove support for updates on alert policy in API server.
10. Alert policy status counters.
11. User updation of alerts.
