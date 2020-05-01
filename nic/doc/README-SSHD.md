SSHD behavior on boot up and customzation of it:
================================================

sshd start at boot up time of Naples Distributed Services Card (DSC), and sshd configuration persistence ('sshd_config' file content) across reloads of system is controlled through sshd behavior customization file that is packaged with the Naples system image.

That sshd behvaior customization file is kept as '/nic/conf/sshd_boot_config', which is processed at the boot up time of the system.

There are two knobs provided through this file:
    1.    BOOT_BEHAVIOR={ON|OFF|LAST}
    2.    PERSIST={yes}

    BOOT_BEHAVIOR
    --------------
    Setting of ON brings up sshd on system start while the setting OFF doesn't.
    Setting of 'LAST' means the previous run state of sshd on the system
    should be maintained.

    Note: If this knob is omitted in the boot config file, it is treated as
          'LAST'.
          With 'LAST' being the setting, if the system is coming up
          for the first time of its life, sshd will be kept down.

    PERSIST
    -------
    Setting this to 'yes' means sshd should retain its running configuration
    ('sshd_config' file content) across reloads of the system.
    Default setting is OFF, i.e. system comes up with default content in
    'sshd_config' file on every reload.

    Enabling this setting gets system start with default configuration content,
    when system comes up for the first time, caches it as the file
    '/sysconfig/config0/ssh/sshd_config' and uses it across reloads.

    With persistence enabled, any customization that system administrator
    needs to make to sshd configuration itself should be done by editing
    this cached file and should re-start sshd or reload the system.

    sshd can be stopped and started or just re-started, on a running system, by:
        /etc/init.d/S50sshd {stop|start|restart}

