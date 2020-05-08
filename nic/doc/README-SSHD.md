SSHD behavior on boot up and customzation of it:
================================================

sshd being brought up or not on Naples Distributed Services Card (DSC) boot up and the sshd configuration to be used ('sshd_config' file content) is controlled by sshd behavior customization file '/nic/conf/sshd_boot_config'. This file gets processed at the boot up time of DSC.  

There are two knobs provided through this file:
    1.    BOOT_BEHAVIOR={ON|OFF}
    2.    PERSIST={ON|OFF}

    BOOT_BEHAVIOR
    --------------
    Setting of ON indicates that sshd be started on system boot up, with default
    sshd_config file content.

    This setting solely controls start of sshd in any of the below three cases:
    1. If the system is being booted up for the first of its life.
    2. If 'PERSIST' setting is not turned ON.
    3. If 'PERSIST' is turned ON but there is no prior boot up time 
       cached sshd state on the system.

    Refer to description of 'PERSIST' setting for explanation of sshd 
    configuration cache.

    PERSIST
    -------
    Setting this to 'ON' means sshd should retain its running configuration
    ('sshd_config' file content, and the state of sshd being UP or not) across
    reloads of the system.

    Default setting is OFF.
    
    Enabling this setting gets sshd start with default configuration content,
    when system comes up for the first time(assuming BOOT_BEHAVIOR is set to ON
    as well), caches it as the file '/sysconfig/config0/ssh/sshd_config' and
    uses it across reloads. Seperately sshd run state (i.e sshd is UP or
    stopped) is cached as well.
    On subsequent reloads whole of this cached sshd state is restored.

    With persistence enabled, any customization that system administrator
    needs to make to sshd configuration itself should be done by editing
    this cached file and should re-start sshd or reload the system.

    sshd can be stopped and started or just re-started, on a running system, by:
        /etc/init.d/S50sshd {stop|start|restart}
