This directory contains asic/pipeline shim layer (impl) used by all vpp plugins.

Directory Layout:
    1. include/ - contains the shim API headers required by vpp plugins
    2. <pipeline-name>/ - contains pipeline specific code to implement the API

The idea is to share the code across all vpp plugins and hence impl/ is kept out
of respective vpp plugin code.

Typically, the p4/hw touch points are:
    1. The p4 rx meta (hw assists) received from the hardware for every packet
    2. Any extra information (outside #1), that needs to be queried from p4
       tables with the assist information provided.
    3. Question: Is there any p4 tx meta that needs to be passed while sending
       the packet out? If same structure is used both ways, will rename
       p4_rx_meta to p4_meta

The p4 rx meta is passed as opaque data (void *) to the respective API call
from the respective vpp plugin code.
