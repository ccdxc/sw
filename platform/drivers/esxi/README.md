Pensando ESXi Native Ethernet Driver

Installation:

1. Copy the VIB (VMware Installation Bundle) or offline bundle to your server.

2. Run the following commands to install the VIB or offline bundle:
        a) For VIB installation:
        esxcli software vib install -v {file_full_path}

        b) For offline bundle installation:
        esxcli software vib install -d {file_full_path}

3. Reboot the ESXi server.
