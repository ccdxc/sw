#### Test application for core upgrade logic
- This application can be use to test upgrade orchestration in absence real svcs.
- Many services can be started from the same binary with configurable thread-id and service name.
- Service specific thread id and svc name comes from start-up JSON.  For example :-
    > svc_1_init.json
    > svc_2_init.json
    > svc_3_init.json
- The behavior of those services is also configurable which comes from test-config json file, named as
    > test.json

#### How to run ?
> Terminal 1:

```sh
$ cd /usr/src/github.com/pensando/sw/nic/apollo/upgrade/test/workflow
$ ./ start_services.sh
```

>Terminal 2:

```sh
$ cd /usr/src/github.com/pensando/sw/nic/apollo/upgrade/test
$ ./start_upgmgr.sh
```
