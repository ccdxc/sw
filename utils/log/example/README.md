
### Logger library usage guidelines

The recommendation is to configure and instantiate logger once in main 
package and reuse the logger in other associated packages by passing them as
dependency as illustrated in log_example.go. The logger can be decorated with
more specific context further in other packages, as needed.

Please follow these rules and conventions when logging.

- Logging in main and directly associated packages should be consistent per service
- Minimize logging in shared packages
- Eliminate logging in most commonly shared or utility packages

#### Sample log generated from log_example.go

```
ts=2017-07-24T18:04:59.095508888Z module=LogTester pid=99535 caller=log_example.go:50 obj=1 data="6-WWnmt_T68jkMuMb1MVy1qsnLw031tQEiH6vSsR5Ns=" level=info
ts=2017-07-24T18:04:59.095750488Z module=LogTester pid=99535 caller=log_example.go:50 obj=2 data="a6-4YFwyRH0HadkYMD91K8cPRIDQPJ2X4Zply0lPum8=" level=info
...
...
ts=2017-07-24T18:04:59.095807623Z module=LogTester pid=99535 caller=service1.go:27 svc=svc1 level=info msg="started service"
ts=2017-07-24T18:04:59.095818857Z module=LogTester pid=99535 caller=service1.go:28 svc=svc1 level=debug msg="data bar"
ts=2017-07-24T18:04:59.095836148Z module=LogTester pid=99535 caller=service2.go:28 svc=svc2 level=info msg="started service"
ts=2017-07-24T18:04:59.095846078Z module=LogTester pid=99535 caller=service2.go:29 svc=svc2 level=debug msg="data foo"
ts=2017-07-24T18:04:59.095854247Z module=LogTester pid=99535 caller=service1.go:35 svc=svc1 level=info msg="terminated service"
ts=2017-07-24T18:04:59.095871878Z module=LogTester pid=99535 caller=service2.go:35 svc=svc2 level=error msg="failed to terminate service: invalid argument"
ts=2017-07-24T18:04:59.095880989Z module=LogTester pid=99535 caller=log_example.go:64 level=info msg=Completed

```