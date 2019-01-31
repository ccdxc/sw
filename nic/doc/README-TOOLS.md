- tools directory for nic sw/p4/p4+ development


README for tclsh.py
===================
    1) Eth driver needs to be loaded and interfaces must be present, since the script uses the MNIC interface to scp files (e.g. enp98s0 on Linux and ionic2 on FreeBSD)
    2) Script can only be run from a server where pexpect is installed (does NOT work on servers where pexpect is not installed)
    3) Only arg to the script is the hostname (--host cap-rdma)
    4) Logfile is created in the home dir of the user, and named tclsh.log.<pid>
    5) MNIC interface information is picked up from nic/conf/dev.json. This file needs to be updated with setup info before the script is used

    Known issues:
    ============
    - doesn't work if eth driver is not already loaded
    - script errors out if there is not enough memory to untar the tar file

    Fixed issues:
    ============
    Issue: doesn't work when scp of nic.tar.gz to the NIC does not immediately ask for password, and asks if connection can be trusted.
    Fix: disabled strict host checking

    Issue: When logging to stdout everything was getting printed multiple times.
    Fix: Disabled 2nd arg to script. Always log to file

    Issue: Does not work on FreeBSD
    Fix: Added support


README for reload.py
====================
    - The reload script can be used to:
        1) reset naples with existing image OR
        2) update the image and then reset naples

        Update image + reset
        ./reload.py --host cap-srv11 --update 1

        Update image + reset
        ./reload.py --host cap-srv11

    - After resetting naples, the host is rebooted. Once it is back up, the eth and rdma drivers are loaded. The drivers package is taken from the workspace
    - Interfaces are brought up using the information in nic/conf/dev.json
    - dev.json is a static file where the information related to the host and its interfaces are populated ahead of time
    - Information specific to each naples which is rebooted using the script should be added to dev.json
    - The image to update to must be already built - the script looks for nic/naples_fw.tar in the workspace from where the script is run

    TODO:
    ======
    - Clear the console once done using
    - Add option to load with golden image (to recover)
    - Error out or get to the prompt if capview is running or if a file is opened on the console
    - Bring up interfaces optionally, based on json file input

    Fixed:
    =====
    - If memtun is not running on host, try to start it. If naples is running a good image, it should work. If not, throw an error


README for captrace_collect.py
==============================
    - This script can be used to configure, collect, decode and filter captrace
    - The script first configures captrace. It then asks for user input before dumping captrace. This is when the test (for which you intend to collect captrace), should be run before providing input to the script
    - The symbol file is generated in the workspace from where the script is run
    - captrace.decode can be found in nic/ directory after the script is run
    - The script asks for filters to be applied on the decode file after generating captrace.decode

    ./captrace_collect.py --host cap-srv11 --rxdma 1
