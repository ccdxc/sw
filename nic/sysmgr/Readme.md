# System Manager

Sysmgr is responsible for bringing up the processes at boot up and monitoring them.

It does *not* support graceful shutdown, i.e. if a non-restartable process dies
then sysmgr will just kill the rest of the processes without any warning and
restart the system.

## Integration

### Mandatory

1. The process must be a Delphi client
2. Similarly to `nic/sysmgr/example/example.cc` the process must create a
   sysmgr::Client object with parameter the delphi:SdkPtr and the process name.
   This has to happen before calling `delphi::SdkPtr::RegisterService()`.
3. Once initialization is done the process must call sysmgr::Client::init_done()
   on the sysmgr::Client object it created in step (2)
4. The process must be added in `nic/sysmgr/spec.cpp` in the SPECS in order to
   be brought up. The arguments are:
   (NAME, FLAGS, EXECUTABLE, [DEPENDENCIES])
   NAME: must match the name in step(2)
   FLAGS: NO_WATCHDOG - disables the watchdog for the process
          RESTARTABLE - if the process is restartable
   EXEC: the full path of the processes' binary executable
   DEPENDENCIES: An array of the process names this process depends on

### Optional

The heartbeat system is integrated in the delphi client and add the following
extra virtual function to delphi::Service

1. `SkipHeartbeat` will be called before a heartbeat is sent. If this function
   returns true, then we will skip the heartbeat.
2. `Heartbeat` this function is called before a heartbeat is sent as well, and
   give the opportunity to the process to declare the process as stuck by returing
   delphi::error::NotOK() and an error message. When it does this, sysmgr will
   assume the process is stuck and will either reload it(if it is restartable)
   or reload the whole system.
