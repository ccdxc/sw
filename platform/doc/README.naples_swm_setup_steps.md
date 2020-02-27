These are the steps that user needs to follow to prepare the Naples25 SWM cards before deploying the testbed in jobd infra.

Note: To speed up the process of deployment user can connect 1G cable to Naples25 SWM's R45 port so Naples can be accessible through netwrok. if 1G cable is connected to Naples' RJ45 port then user can just run "dhclient oob_mnic0" command on Naples console to get DHCP IP so user can access the Naples over network through oob_mnic0 interface.

1. Update the goldfw on Naples25 SWM card to the latest goldfw. Latest goldfw can be found under pensando/sw/platform/goldfw/naples directory.
2. Update the mainfwa and mainfwb on the Naples25 SWM card. This can be any of the latest hourly build.
3. Boot the naples in one of the mainfwa or mainfwb where we expect NCSI is working.
4. Login to iLO and configure it to use Shared Network Port (NCSI) and reset iLO.
5. iLO should get DHCP IP over NCSI.
6. User should disconnect the 1G cable from Naples' RJ45 port before running the next step.
7. if iLO gets NCSI IP as expected then user should APC power cycle the server (while iLO is configured in NCSI) and check whether iLO is able to get NCSI IP after power cycle or not.
8. At this point iLO and Naples25 SWM has been configured properly for NCSI workflow.

If iLO doesn't get DHCP IP over NCSI then user may take following steps to debug the first level of issues.

Debug Notes:
1. Check if DHCP server and TORs are configured correctly to provide DHCP IP to iLO. To verify this user can run dhclient command on Naples' uplink interface from Naples console.

    halctl show port status (User should check the uplink status to make sure that links for 25G ports are up. if links are down for 25G ports then there is no way iLO will get DHCP IP from DHCP server)

    ifconfig inb_mnic0
    ifconfig inb_mnic1
    dhclient bond0

    If bond0 gets the IP address from DHCP server then TORs and DHCP server is configured properly.

2. bond0 gets IP address through DHCP but iLO doesn't get DHCP IP: In such situation, mostly the NCSI protocol stack in Naples firmware might be broken or not functioning correctly. To verify NCSI protocol stack sanity user can check the /var/log/pensando/ncsid.log
    tail -f /var/log/pensando/ncsid.log (In working condition we expoect this log to spit out something at every second since iLO sends the GetLinkStatus NCSI commands every second to Naples)
