# Time keeping on venice

This document examines various issues related to keeping the clock in sync in a
venice cluster.

## Need to keep the time synchronized in venice

When we talk about time synchronization, there are 3 aspects to be considered.

1. NTPSYNC : All venice and naples nodes have synchronized time within
   millisecond resolution to the rest of the world
1. LOCALNTPSYNC: All venice and naples nodes have synchronized time within
   millisecond resolution but not necessarily in sync with the rest of the
   world
1. MANUALSYNC: Venice and naples nodes are synchronized in seconds resolution
1. LOCALNTP_MANUALSYNC: All venice and naples nodes have synchronized time
   within millisecond resolution but sync with seconds resolution with outside
   world
1. UNSYNC: Venice and Naples nodes are un-synchronized

We explore the pros and cons of each of the above and various other issues

### NTPSYNC : All venice and naples nodes have synchronized time within millisecond

This implies that venice needs NTP server reachability for the cluster to come
up.  In many cases user can provide local NTP servers or some other publicly
available NTP servers.  However there might be some enterprise and labs
situations where there is no NTP server thats available and very likely the
reachability to public NTP servers is restricted (NTP uses port 123 and many
enterprises only allow 80 and 443 and select few ports to go out).

### LOCALNTPSYNC: All venice and naples nodes have synchronized time within millisecond resolution but not with rest of world

In this mode, venice and naples are synchronized with NTP. However these are
not synchronized with external world.  NTP is configured such that all the
venice nodes sync with each other and hence unaffected if one of the nodes has
a large drift of clock.

As expected, the cluster need not necessarily have a good starting time.  So
when some user-visible time elements are exported from cluster and examined
offline, if the times are off, then the user will get very confused.
User-visible time elements are typically:

* Audits/Events/Alerts
* Time inside objects : like creation time, last modified time etc

e.g: server claims that NIC went down at 'xxx' where as Naples shows unusual
activity at 'zzz' time!!

### LOCALNTP_MANUALSYNC: All venice and naples nodes have synchronized time within millisecond resolution but sync with seconds resolution with outside world

The nodes within the cluster are synchronised with milliseconds resolution with
NTP.  The time is synced with the outside world periodically - say by running
'date' in an ssh script or htpdate or some such mechanism.

While this solution can ensure that cluster can continue its operation even without
external NTP accesss, the periodic time fetch can indeed cause jumps in time.

### MANUALSYNC

Here all the nodes run periodic scripts to keep time in sync. This can only ensure
correctness to the order of seconds.

It can be very tough to debug low-latency issues.  E.g: a client sends a
message to server and gets response in few seconds when the expectation is to
get the response in milliseconds. If the clock are seconds apart then logs wont
help debug this.

### UNSYNC

In a completely unsynchronized system, its almost impossible to correlate
events happening in the cluster based on logs/timestamp events.

Wherever we use certificates, we must have reasonably accurate time:

1. Venice and NAPLES attestation: the burned-in certificates will have a
       large validity window (say 10 years) but if the clock is wildly off,
       these certificates will be considered invalid and cluster formation/join
       will not happen
1. User-provided certificates also have a validity window that we do not
       control, so Venice/NAPLES clock cannot be too off

## Issues with changing schemes at runtime

The main issue with changing schemes while the cluster is already running is
dealing with jumps in time - both forward and backward jumps soon after a
connection to an NTP server is established.  This can also happen anytime clock
gets changed manually.

By following the coding guidelines mentioned later in the document, we can
minimize errors and see that cluster operates without big issues.

However debuggability is definitely a challenge with changing time. Imagine
looking at logs at a central monitoring place where time goes backwards or time
being different on different nodes. Hence we should strive keep time
synchronized with smooth changes as much as possible.

Other common issues with time changes:

1. Timers may not fire or fire too early/late. If a timer does not fire,
       FSMs can get stuck/messed up.
1. Entities whose identity depends on time (stat readings, events, etc.)
       may become meaningless (e.g. have a timestamp in the future). Also, if
       timestamping is used in any way to achieve identity uniqueness, you can
       end up with duplicates.

## Third-party code

while we can ensure that pensando code will follow strict code review process so
that it can handle jumps in the time, we still use a fair amount of third-party
code. Its not guaranteed that all the third-party code can handle the jumps in
time. The only way to make sure is by testing.


In general - In a distributed system, there will always be some code or other
which relies on clocks being close.  Some bugs in Kubernetes when clocks are
too far off : https://github.com/kubernetes/kubernetes/issues/29229

## Proposal of timesync in venice

kubernetes can not work if NTP is not enabled. Since kubernetes is
critical part of the venice infra - we will always run NTP on venice.

An Initial time-sync of time is mandatory for a venice cluster to be up.

When a request for cluster create is received, the node (lets call it bootstrap node)
tries to sync time from an external source in the following order.

1. Sync NTP time with the NTP server specified in the cluster creation request
1. Sync NTP with any NTP server, if provided with DHCP option 42.
1. Public NTP server pool. e.g: pool.ntp.org
1. Time from http web service using 'Date' field in HTTP header similar to
   htpdate program (This is not planned in the first release).

If all these conditions fail, then cluster creation is failed.  In the worst
case, for POCs we can have a debug knob to continue with the cluster creation.
If the user does not specify an NTP server, pool.ntp.org
(or some such equivalent to be decided later) is used as default.

As part of creation of cluster (and any time afterwards), user specifies other
venice nodes. The bootstrap venice node, sends the NTP server information to
each of these nodes in the prejoin() step.  On each of these  nodes, NTP syncs
time immediately to the bootstrap server.  Only after a successful time sync
does the node reply to the prejoin request and the node proceeds with joining
the venice cluster.  Please note that all we need is one successful time set by
the NTP on these nodes.  Its not necessary that full fledged NTP daemon has to
be running on the node before joining the cluster. Infact NTPD is most likely
started as kubernetes daemon-set after the node joins the cluster.

After a node joins a venice, all the venice nodes act as peers and adjust each
others clock. i.e each node becomes a ntp client of all other venice nodes.
This way all the venice nodes  give the appearance of similar clock speed which
is average of clocks of all the nodes.

Continuous reachability to NTP server is not mandatory for venice cluster
operation. NTP daemons generally have some resiliency if one of the nodes
starts acting weird regarding the NTP clock and it does not mess up the clocks
on other nodes in such cases.

Every Naples node point to (a max of) 3 venice nodes as the NTP servers and
receive updates from all of them. Hence when a naples connects to venice for
the first time time change is expected. Since this approach does time sync over
the LAN, the cluster as a whole has better accuracy of time than each node
syncing time individually to an external NTP server.


## Implementation notes

There are many different implementations of NTP. Popular ones are chrony,
ntpd, [openntpd](http://www.openntpd.org),
[systemd-timesyncd](https://wiki.archlinux.org/index.php/systemd-timesyncd).
timesyncd is the client-only implementation on ubuntu. This is likely the
option to run on the naples.

On venice we need a server and hence need to pick among the other 3
implementations.  [Comparison](https://chrony.tuxfamily.org/comparison.html) of
these narrows down the choices to chrony or ntp. Both are good choices and we
can go with chrony.

While configuring ntpd, venice should be able to serve NTP to any server in the
network. However the updates should only be taken from trusted servers and peers.

There should be no assumption that the NTP server should be reachable on the
same interface as other venice nodes.  Its possible that ntp server is on a
different subnet than venice nodes.

On a cold boot of a venice node (after it joins a cluster), various interfaces
can come up in different order.  There should be no dependency in the code that
the user-specified NTP server must be reachable (after a cold reboot) for the
venice cluster to be up. Even if atleast one more venice node is up and is
serving NTP, then this node which is going through the cold reboot can sync the
from that available NTP server and proceed with the boot up sequence. NTP
syncing time is NOT a pre-requisite for the venice bootup.  i.e bootup should
not be delayed by more than few seconds while waiting to get NTP info from
other venice nodes.

NAPLES, after reboot should start the ntp client early on in the bootup
sequence (after the network is configured and IP address is set). However it
should not wait for this to complete for the bootup sequence to proceed. It can
start with the time restored from the battery backed clock, if available.

It should be noted that after cold reboot of node, NTP can start serving time
even if its not able to sync time with any other NTP server (after specific amount of timeout).

We need the ability to monitor the service health of various services and have
ability to restart the services on failure. This is assuming that restarting a
process can fix the issue with the time jump. While restarting a service can fix
in most of the cases, its  not 100% fool proof. Special care needs to be taken
care anywhere the time is written to persistent storage and/or network -
Restarting a service does not change the time on persistent storage.

Venice and Naples code should be tested for jumps in time.  The code should
assume that wall time can change. (and so can the timezone).

## Coding guidelines

C programmers should probably never use gettimeofday(). Main reason is that it
can be affected by the jumps in time. Beware of libraries which use
gettimeofday().

As a good practice, its better to use clock_gettime() with CLOCK_MONOTONIC for
anything thats related to time duration.

gettimeofday should only be used when showing time to user (and not for any
calculations in the code)

In Golang, time.Time has support for monotonic time and should be used in any
code which deals with duration. However time.Parse() or JSONMarshall() or any
such ways of constructing time from strings and network do not support
monotonic time. Monotonic time is something within this node and not across the
network.

Whenever storing time, store in UTC within the cluster. i.e when storing time in
objects in persistent store/kvstore or sending time in objects in GRPC messages
between Venice and Naples, use the UTC.

Deal with Timezone only at the peripheral where the time is converted to string
and presented to user. Timestamp in Logs is treated like the point of conversion
to string. Hence they are in local timezone of source of log.

When dealing with any creating certificates give a small margin of time.
i.e set the certificate the valid time to few seconds/minutes in past so that
we can communicate with peers whose time is few seconds in past..

Any time based  arithmetic must be used with caution.  If  not the results might
range from simple  errors (e.g.  computing average time to  run an operation can
use  wrong  values),  to  fatal  errors  (in  worst  case,  e.g.  waiting  while
currentTime is not < 'x')

## Requirements of Timekeeping service

* Ability to set the NTP server to a list of NTP servers.
* Blocking call to set the date/time to one the NTP servers and keep retrying
  for user-specified timeout (say 5 seconds).
* Get the time by issuing HTTP get call and extract time from Date header
* Cooperate with peers to make sure that all peers have an average clock thats
  similar. This is not mandatory but a very good to have feature.
* Be a server of time so that Naples can ask time from this service.

## References

* https://github.com/golang/proposal/blob/master/design/12914-monotonic.md
* https://lwn.net/Articles/629010/
* https://linux.die.net/man/8/htpdate
* https://github.com/coreos/etcd/pull/6888
* http://www.ntp.org/ntpfaq/