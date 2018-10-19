- tools directory for nic sw/p4/p4+ development


README for tclsh.py
===================
    1) memtun needs to be running on both the host and the NIC
    2) Script can only be run from a server where pexpect is installed (does NOT work on servers where pexpect is not installed)
    3) Only arg to the script is the hostname (--host cap-rdma)
    4) Logfile is created in the home dir of the user, and named tclsh.log.<pid>

    Known issues:
    ============
    - doesn't work if memtun is not running
    - script errors out if there is not enough memory to untar the tar file

    Fixed issues:
    ============
    Issue: doesn't work when scp of nic.tar.gz to the NIC does not immediately ask for password, and asks if connection can be trusted.
    Fix: disabled strict host checking

    Issue: When logging to stdout everything was getting printed multiple times.
    Fix: Disabled 2nd arg to script. Always log to file
