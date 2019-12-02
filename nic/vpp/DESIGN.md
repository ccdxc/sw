vpp pensando plugins are developed to support:
  * control plane features that has
      * vpp base support (Ex: dhcp relay)
      * features customized by pensando (Ex: arp proxy)
  * data path features which are tightly integrated with p4 (Ex: flow)

The vpp custom plugin consists of three distinct layers:

  * An infra layer comprising of common code which is required by all
    custom plugins. The common code can be further split into different
    directories based on the functionality that can be self contained.
    See nic/vpp/infra/

  * Each feature layer is implemented under the respective <feature>
    folder. This is purely pipeline independent and contains stuff
    that can be shared across all pipelines.
    See nic/vpp/{dhcp_relay, arp_proxy, etc}

  * An implmentation layer (impl/) that has api for dealing with hardware
    access. Under impl/, there will be subdirectories dealing with each
    specific pipline.
    See nic/vpp/impl/{apulu, apollo, etc}

Typical workflow:

+--------------+    +---------------+    +----------------+    +--------------+
|              |    |               |    |  +----------+  |    | +----------+ |
|  +--------+  |    |  +---------+  |    |  | Flow     |  |    | |          | |
|  |        |  |    |  | P4 Hdr  |  |    |  | Plugin   |  |    | |  Apulu   | |
|  | DPDK   +--------->+ Lookup  |  |    |  |          |  |    | |          | |
|  | PMD    |  |    |  | Node    |  |    |  +----------+  |    | +----------+ |
|  +--------+  |    |  +---------+  |    |                |    |              |
|              |    |               |    |  +----------+  |    | +----------+ |
| Packet path  |    |               |    |  | ARP Proxy|  |    | |          | |
+--------------+    |               |    |  | Plugin   |  |    |    Apollo  | |
                    |               +--->+  |          |  +<-->+ |          | |
+--------------+    |               |    |  +----------+  |    | +----------+ |
|              |    |               |    |                |    |              |
|  +--------+  |    |  +---------+  |    |  +----------+  |    |              |
|  |        |  |    |  | IPC     |  |    |  | DHCPRelay|  |    |              |
|  | HAL    |--------->+ Message |  |    |  | Plugin   |  |    |              |
|  | IPC    |  |    |  | Callback|  |    |  |          |  |    |              |
|  +--------+  |    |  +---------+  |    |  +----------+  |    |              |
|              |    |               |    |                |    |              |
| Control path |    |  Infra Layer  |    |  Plugin Layer  |    | Impl Layer   |
| (cfg, etc)   |    |               |    |                |    |              |
+--------------+    +---------------+    +----------------+    +--------------+

