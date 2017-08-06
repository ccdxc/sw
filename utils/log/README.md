
# Logger library usage guidelines - for creating new logger per process/service

The recommendation is to configure and instantiate logger once in main 
package and reuse the logger in other associated packages by passing them as
dependency as illustrated in example/new_logger/log_example.go. The logger can be decorated with
more specific context further in other packages, as needed.

#####This is the recommended option for anyone starting to write new service from scratch.

Please follow these rules and conventions when logging.

- Logging in main and directly associated packages should be consistent per service
- Minimize logging in shared packages
- Eliminate logging in most commonly shared or utility packages

```
func main() {
    
    // Create logConfig  per application/service
    logConfig := &log.Config{
        Module:      "App1",
        Format:      log.LogFmt,
        Filter:      log.AllowAllFilter,
        Debug:       false,
        Context:     true,
        LogToStdout: true,
        LogToFile:   true,
        FileCfg: log.FileConfig{
            Filename:   *logoutput,
            MaxSize:    5, // 5 MB
            MaxBackups: 2,
            MaxAge:     7,
            },
    }
    
    // Create new logger instance
    logger = log.GetNewLogger(logConfig)
	
    // Instantiate pkg1 component, passing logger as dependency
    p1 := pkg1.NewInstance(logger)
    p1.Run()
}

```
## Sample log generated from example/new_logger/log_example.go
```
ts=2017-08-04T17:28:15.377687102Z module=LogTester1 pid=39957 caller=log_example.go:52 level=info obj=1 data="HP3r60UM_7rM0dxpBlm-jPsZntKPDnnkTdMruvJwEmk="
ts=2017-08-04T17:28:15.377949628Z module=LogTester1 pid=39957 caller=log_example.go:52 level=info obj=2 data="imDwNFuOzouJuDYGsoOFlIh5p29_WQkqTHSizYd_Nas="
...
ts=2017-08-04T17:28:15.378049598Z module=LogTester1 pid=39957 caller=service1.go:27 level=info pkg=pkg1 msg="started service"
ts=2017-08-04T17:28:15.378075249Z module=LogTester1 pid=39957 caller=service1.go:28 level=debug pkg=pkg1 msg="data bar"
ts=2017-08-04T17:28:15.378096375Z module=LogTester1 pid=39957 caller=service2.go:28 level=info pkg=pkg2 msg="started service"
ts=2017-08-04T17:28:15.378105071Z module=LogTester1 pid=39957 caller=service2.go:29 level=debug pkg=pkg2 msg="data foo"
ts=2017-08-04T17:28:15.37811974Z module=LogTester1 pid=39957 caller=service1.go:34 level=info pkg=pkg1 msg="terminated service"
ts=2017-08-04T17:28:15.378132661Z module=LogTester1 pid=39957 caller=service2.go:35 level=error pkg=pkg2 msg="failed to terminate service: invalid argument"
ts=2017-08-04T17:28:15.378141973Z module=LogTester1 pid=39957 caller=log_example.go:66 level=info msg=Completed
```

# Logger library usage guidelines - for using default_logger

The default logger can be used by process/service as is, by calling log.Info()/log.Debug() functions which uses default logger.

Applications can also override the default logger, with customized configs for logging to File with specific settings as shown in this example by calling log.SetConfig(config) function which will override the default logger with new logger for this process. This is
illustrated in detail in example/default_logger/log_example.go

```
func main() {
    
    // Create logConfig per Application/service in main
    logConfig := &log.Config{
        Module:      "App1",
        Format:      log.LogFmt,
        Filter:      log.AllowAllFilter,
        Debug:       false,
        Context:     true,
        LogToStdout: true,
        LogToFile:   true,
        FileCfg: log.FileConfig{
            Filename:   *logoutput,
            MaxSize:    5,
            MaxBackups: 2,
            MaxAge:     7,
        },
    }
    
    // Initialize config, this will override the default
    // logger with the configs passed
    log.SetConfig(logConfig)
    
    log.Debugln("App1 started")
}

```
## Sample log generated from example/new_logger/log_example.go
```
ts=2017-08-04T17:27:09.264470623Z module=LogTester2 pid=39942 caller=log_example.go:46 level=info obj=1 data="MFLlk9OUbJES4EH-X351lC7MfmFBDNfAYqexCF3eImU="
ts=2017-08-04T17:27:09.264845106Z module=LogTester2 pid=39942 caller=log_example.go:46 level=info obj=2 data="GdmeHSEgawxXWm4dz2EkCwQqS-Zd7tuL6HpwsGt5zTE="
...
ts=2017-08-04T17:27:09.264928772Z module=LogTester2 pid=39942 caller=service1.go:15 level=info pkg=pkg1 svc=svc1 msg="started svc1"
ts=2017-08-04T17:27:09.264942663Z module=LogTester2 pid=39942 caller=service1.go:16 level=debug pkg=pkg1 svc=svc1 msg="svc1 data bar"
ts=2017-08-04T17:27:09.264953059Z module=LogTester2 pid=39942 caller=service2.go:10 level=info msg="started service2"
ts=2017-08-04T17:27:09.264961484Z module=LogTester2 pid=39942 caller=service2.go:11 level=debug msg="svc2 data foo"
ts=2017-08-04T17:27:09.264978476Z module=LogTester2 pid=39942 caller=service1.go:22 level=info pkg=pkg1 svc=svc1 msg="terminated service1"
ts=2017-08-04T17:27:09.264992902Z module=LogTester2 pid=39942 caller=service2.go:17 level=error msg="failed to terminate service2: invalid argument"
ts=2017-08-04T17:27:09.265001932Z module=LogTester2 pid=39942 caller=log_example.go:55 level=info msg=Completed

```

