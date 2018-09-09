Overview:
1. Implemented common APIs that intend to be shared by ionic Ethernet driver and
future RDMA driver for ESXi.
    a) Implement our ionic Makefile, corresponding SCONS file(ionic.sc) and device
         identification file (ionic_devices.py) which will be used together to generate
         vSphere Installation Bundle.
    b) Implement common ionic work queue. ESXi doesn't have generic Linux work queue
        implementation. We need to define our own work and work queue structures and
        APIs to enable this support.
    c) Implement common ionic memory management. In ESXi, each driver should create
        a heap for its use. When developing drivers, keep in mind that the network packet heap
        is shared across all network drivers. Because of this, it is important that no individual driver
        consume the entire heap, as other network devices may be present on the system.
        We create a memory pool upfront and use it to back our heap.
    d) Implement common ionic completion mechanism. Unlike Linux which has generic
        completion implementation(struct completion), we need to implement our own
        completion struct and corresponding APIs to interact with it for ESXi.
    e) Implement common ionic DMA related APIs. In ESXi, before doing any internal IO to
        an adapter itself, it must first register a vmk_DmaEngine and perform the right calls to
        do any mapping before issuing the IO. Then it must unmap it after it completes.
    f) Implement common ionic locks. In ESXi, in order to prevent lock conflicts between the
        multiple drivers, and between driver and other modules that may exist on an ESXi system,
        we must create a “domain” for spinlocks associated with your driver. The ESXi kernel is now
        preemptible. We need to take preemption into account when working with locks I have
        implemented spin-lock, mutex-lock and semaphore related APIs that can be consumed
        by all both Ethernet and RDMA(future).
    g) Implement common ionic interrupt handle. 
    h) Implement common function to register logical device(Uplink device).
    i)  Implemented common APIs to maintain device list. Our driver is holding a device list which
        contains all the devices that claim ionic_en as driver. 
    j) Implemented common ionic PCIe related APIs.
    k) Implemented common utilities/time/types APIs, macros, data structures.

2. Implemented initial version of ESXi native driver which registers the driver, claim and bring
    up the physical device and create/register logical uplink device to ESXi networking subsystem.

3. Implemented all uplink vlan id filter callbacks and plug them to ESXi uplink layer.

4. Re-Implemented/Modified many ionic Linux APIs/data structures. For reviewing purpose, instead
    of deleting the ionic Linux code, I commented out and add new/modified version of the code above/below
    the original code. Example:
    // original Linux ionic code;
    New ESXi native code;


Test:
1. Build source code and generate vSphere Installation Bundle. Install/Uninstall it on ESXi machine, everything
    is OK. 
=============================================================================================================================

[root@localhost:~] esxcli software vib install -v file:///tmp/ionic-en-0.1-1vmw.670.0.0.8169922.x86_64.vib
Installation Result
   Message: The update completed successfully, but the system needs to be rebooted for the changes to be effective.
   Reboot Required: true
   VIBs Installed: VMW_bootbank_ionic-en_0.1-1vmw.670.0.0.8169922
   VIBs Removed: 
   VIBs Skipped: 

[root@localhost:~] esxcli software vib remove -n ionic-en
Removal Result
   Message: The update completed successfully, but the system needs to be rebooted for the changes to be effective.
   Reboot Required: true
   VIBs Installed: 
   VIBs Removed: VMW_bootbank_ionic-en_0.1-1vmw.670.0.0.8169922
   VIBs Skipped: 
=============================================================================================================================


2. After installing the VIB file, reboot the system, our ionic_en kernel module can be loaded automatically. All the
    Resources can be claimed properly. We are able to complete ionic_identify and get all information.
=============================================================================================================================
2018-08-28T21:42:14.336Z cpu3:1001392389)Loading module ionic_en ...
2018-08-28T21:42:14.336Z cpu3:1001392389)Elf: SetLicenseInfo:2101: module ionic_en has license BSD
2018-08-28T21:42:14.337Z cpu3:1001392389)<IONIC_INFO> ionic: init module...
2018-08-28T21:42:14.337Z cpu3:1001392389)Device: Driver_Register:192: Registered driver 'ionic' from 97
2018-08-28T21:42:14.337Z cpu3:1001392389)Mod: LoadDone:4962: Initialization of ionic_en succeeded with module ID 97.
2018-08-28T21:42:14.337Z cpu3:1001392389)ionic_en loaded successfully.
2018-08-28T21:42:25.215Z cpu2:1001390390)<IONIC_INFO> ionic_en_attach() called
2018-08-28T21:42:25.215Z cpu2:1001390390)VMK_PCI: VMKAPIPCIMapWorker:914: device 0000:03:00.0 pciBar 0 bus_addr 0xfe600000 size 0x8000
2018-08-28T21:42:25.215Z cpu2:1001390390)VMK_PCI: VMKAPIPCIMapWorker:914: device 0000:03:00.0 pciBar 2 bus_addr 0xfe608000 size 0x4000
2018-08-28T21:42:25.216Z cpu2:1001390390)<IONIC_INFO> ionic_setup() called
2018-08-28T21:42:25.216Z cpu2:1001390390)<IONIC_INFO> ionic_reset() called
2018-08-28T21:42:25.217Z cpu2:1001390390)<IONIC_INFO> ionic_identify() called
2018-08-28T21:42:25.217Z cpu2:1001390390)DMA: EngineCreate:646: No mapper exists for configured policy, reverting to default.
2018-08-28T21:42:25.217Z cpu2:1001390390)DMA: EngineCreate:679: DMA Engine 'dma_engine_streaming' created using mapper 'DMANull'.
2018-08-28T21:42:25.218Z cpu2:1001390390)<IONIC_INFO> ionic_identify() completed successfully!
2018-08-28T21:42:25.218Z cpu2:1001390390)<IONIC_INFO> ASIC: Capri rev: 0xA0 serial num: serial_num0001 fw version: fwvers0002
2018-08-28T21:42:25.218Z cpu2:1001390390)<IONIC_INFO> ionic_en_attach() completed successfully!
2018-08-28T21:42:25.218Z cpu2:1001390390)Device: Driver_AnnounceDevice:327: Found driver ionic for device 0x1300430516b87d88
2018-08-28T21:42:25.218Z cpu2:1001390390)<IONIC_INFO> ionic_en_scan() called
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:565: 0000:03:00.0: MSIX interrupts numDesired 3, numRequired 3, intrVirt unsupported
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:578: MSI-X found at location: 0xa0
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:589: control: 0x3, table size: 4, fmask: 0, enable: 0
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIXGetAddr:359: MSI-X table at capIdx 160 + birOff 2: bar: 0xfe600000 MA: 0xfe600000 bir: 24576
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:624: MSI-X vector table located at: 0xfe606000
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIXGetAddr:359: MSI-X table at capIdx 160 + birOff 6: bar: 0xfe600000 MA: 0xfe600000 bir: 28672
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:631: MSI-X pba located at: 0xfe607000
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:637: vectorTable startMPN = 0xfe606, size = 0x40, numMPNs = 0x1
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:652: pbaTable startMPN = 0xfe607, size = 0x20, numMPNs = 0x1
2018-08-28T21:42:25.218Z cpu2:1001390390)MSI: MSIX_AllocateVectors:698: 0000:03:00.0: Allocated vector 0x70 on CPU 3 for index 0
2018-08-28T21:42:25.221Z cpu2:1001390390)MSI: MSIXIntrLog:99: vecCtrl = 0x1, addr_lo = 0xfee03000, addr_hi = 0x0, data = 0x70
2018-08-28T21:42:25.221Z cpu2:1001390390)MSI: MSIX_AllocateVectors:698: 0000:03:00.0: Allocated vector 0x70 on CPU 0 for index 1
2018-08-28T21:42:25.224Z cpu2:1001390390)MSI: MSIXIntrLog:99: vecCtrl = 0x1, addr_lo = 0xfee00000, addr_hi = 0x0, data = 0x70
2018-08-28T21:42:25.224Z cpu2:1001390390)MSI: MSIX_AllocateVectors:698: 0000:03:00.0: Allocated vector 0x70 on CPU 1 for index 2
2018-08-28T21:42:25.227Z cpu2:1001390390)MSI: MSIXIntrLog:99: vecCtrl = 0x1, addr_lo = 0xfee01000, addr_hi = 0x0, data = 0x70
2018-08-28T21:42:25.227Z cpu2:1001390390)MSI: MSIX_AllocateVectors:747: MSI-X control = 0x8003
2018-08-28T21:42:25.227Z cpu2:1001390390)MSI: MSIX_AllocateVectors:752: 0000:03:00.0: vector 0x70 index 0x0
2018-08-28T21:42:25.227Z cpu2:1001390390)MSI: MSIX_AllocateVectors:752: 0000:03:00.0: vector 0x70 index 0x1
2018-08-28T21:42:25.227Z cpu2:1001390390)MSI: MSIX_AllocateVectors:752: 0000:03:00.0: vector 0x70 index 0x2
2018-08-28T21:42:25.230Z cpu2:1001390390)VMK_PCI: vmk_PCIAllocIntrCookie:764: device 0000:03:00.0 allocated 3 MSIX interrupts
2018-08-28T21:43:09.780Z cpu3:1001392394)Mod: Unload:5059: Unloading module <ionic_en> ...
2018-08-28T21:43:09.780Z cpu3:1001392394)<IONIC_INFO> ionic_en_quiesce() called
2018-08-28T21:43:09.780Z cpu3:1001392394)IntrCookie: FreeMSIX:1669: Freeing 3 cookie(s) for device 0000:03:00.0
2018-08-28T21:43:09.780Z cpu3:1001392394)IntrCookie: ReleaseInterrupt:961: Releasing interrupt 0x14
2018-08-28T21:43:09.780Z cpu3:1001392394)IntrCookie: ReleaseInterrupt:961: Releasing interrupt 0x15
2018-08-28T21:43:09.780Z cpu3:1001392394)IntrCookie: ReleaseInterrupt:961: Releasing interrupt 0x16
2018-08-28T21:43:09.780Z cpu3:1001392394)MSI: MSIXDoReleaseVectors:281: Disabling vector 0x70 index 0x0 on dev 0000:03:00.0
2018-08-28T21:43:09.782Z cpu3:1001392394)MSI: MSIXDoReleaseVectors:281: Disabling vector 0x70 index 0x1 on dev 0000:03:00.0
2018-08-28T21:43:09.783Z cpu3:1001392394)MSI: MSIXDoReleaseVectors:281: Disabling vector 0x70 index 0x2 on dev 0000:03:00.0
2018-08-28T21:43:09.784Z cpu3:1001392394)<IONIC_INFO> ionic_en_detach() called
2018-08-28T21:43:09.784Z cpu3:1001392394)DMA: EngineDestroy:724: DMA Engine 'dma_engine_streaming' destroyed.
2018-08-28T21:43:09.786Z cpu3:1001392394)Mod: Unload:5144: Calling cleanup
2018-08-28T21:43:09.786Z cpu3:1001392394)Device: Driver_Unregister:250: Unregistered driver 'ionic' from 97
2018-08-28T21:43:09.786Z cpu3:1001392394)Mod: Unload:5158: !Calling lateCleanup
2018-08-28T21:43:10.340Z cpu3:1001392394)Mod: Unload:5265: Unload complete
=============================================================================================================================

3. After booting the system(with ionic-en via installed), we tried to manually load/unload the module for multiple times,
    no memory leaking detected.
4. After loading the module, we are able to see ionic uplink device displayed in ESXi:
[root@localhost:~] esxcfg-nics -l
Name    PCI          Driver      Link Speed      Duplex MAC Address       MTU    Description
vmnic0  0000:00:02.0 e1000       Up   1000Mbps   Full   52:54:00:12:34:56 1500   Intel Corporation 82540EM Gigabit Ethernet Controller
vmnic1  0000:03:00.0             Down 0Mbps      Half   5a:5b:03:1b:3a:5b 1500   Pensando Systems, Inc Pensando Ethernet PF

0000:03:00.0
   Address: 0000:03:00.0
   Segment: 0x0000
   Bus: 0x03
   Slot: 0x00
   Function: 0x0
   VMkernel Name: vmnic1
   Vendor Name: Pensando Systems, Inc
   Device Name: Pensando Ethernet PF
   Configured Owner: VMkernel
   Current Owner: VMkernel
   Vendor ID: 0x1dd8
   Device ID: 0x1002
   SubVendor ID: 0x1dd8
   SubDevice ID: 0x4001
   Device Class: 0x0200
   Device Class Name: Ethernet controller
   Programming Interface: 0x00
   Revision ID: 0x00
   Interrupt Line: 0x0b
   IRQ: 255
   Interrupt Vector: 0x00
   PCI Pin: 0x00
   Spawned Bus: 0x00
   Flags: 0x3201
   Module ID: 97
   Module Name: ionic_en
   Chassis: 0
   Physical Slot: 4294967295
   Slot Description: Chassis slot 2; function 0; relative bdf 02:00.0
   Passthru Capable: false
   Parent Device: PCI 0:2:0:0
   Dependent Device: PCI 0:3:0:0
   Reset Method: Function reset
   FPT Sharable: false



