"""
/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */
"""


driverName          = "ionic_en"
driverVersion       = "1.4"
driverSummary       = "Pensando Systems Native Ethernet Driver"
driverDescription   = "Pensando Systems Native Ethernet Driver"
vendorName          = "Pensando Systems"
vendorMailingList   = "jingyiz@pensando.io"
vendorCode          = "PEN"
moreCCFlags         = ""


# Driver definition for ionic_en driver.
#
# When developing a driver for release through the async program:
#  * set "vendor" to the name of your company
#  * set "license" to one of the VMK_MODULE_LICENSE_* strings if applicable;
#    otherwise, set it to a 1-word identification of your module's license
#  * set "vendor_code" to your company's VMware-assigned Vendor Code
#  * set "vendor_email" to the contact e-mail provided by your company
#  * increment the version number if the source has come from VMware
#  * remove "version_bump" if present
#
# When bringing an async driver inbox at VMware:
#  * leave "version" as is from the async release
#  * set "version_bump" to 1
#  * set "vendor" to 'VMware, Inc.'
#  * set "vendor_code" to "VMW"
#  * set "vendor_email" to the VMware contact e-mail address
#
# If updating the driver at VMware:
#  * increment "version bump" or contact the IHV for a new version number
#
# If updating the driver at an async vendor:
#  * increment the version number (do not touch version bump)

#
# identification section
#
ionic_en_identification = {
   "name"            : driverName,
   "module type"     : "device driver",
   "binary compat"   : "yes",
   "summary"         : driverSummary,
   "description"     : driverDescription,
   "version"         : driverVersion,
#   "version_bump"    : 1, 
   "license"         : "Pensando_Proprietary",
   "vendor"          : vendorName,
   "vendor_code"     : vendorCode,
   "vendor_email"    : vendorMailingList,
}

#
# Build the Driver Module
#
module_def = {
   "identification"  : ionic_en_identification,
   "source files"    : ["ionic_main.c",
                        "ionic_dev.c",
                        "ionic_bus_pci.c",
                        "ionic_lif.c",
			"ionic_api.c",
                        "ionic_en_uplink.c",
                        "ionic_rx_filter.c",
                        "ionic_txrx.c",
                        "ionic_en_mq.c",
                        "../common/ionic_memory.c",
                        "../common/ionic_locks.c",
                        "../common/ionic_pci.c",
                        "../common/ionic_utilities.c",
                        "../common/ionic_hash.c",
                        "../common/ionic_completion.c",
                        "../common/ionic_dma.c",
                        "../common/ionic_work_queue.c",
                        "../common/ionic_interrupt.c",
                        "../common/ionic_logical_dev_register.c",
                        "../common/ionic_device_list.c",
                     ],
    "includes"       : [
                         "../common/include",
                         "../../common",
                     ],
    "cc flags"       : [
                         "-Wall -Werror",
                         moreCCFlags,
                     ],
    "cc defs"        : { 'DRV_NAME=\\"%s\\"' % (driverName) : None,
                         'DRV_VERSION=\\"%s\\"' % (driverVersion) : None,
                     },
}
ionic_en_module = defineKernelModule(module_def)

#
# Build the Driver's Device Definition
# - Beta and obj builds will have their map file priority set to
#   default so that native driver ionic_en takes priority for these
#   builds.
# - Release and opt build, the map priority will remain as fallback
#   so the vmklinux driver takes precedence on these builds.
#
import vmware
buildtype = vmware.BuildType()

deviceSpec = "ionic_devices.py"
device_def = {
   "identification"  : ionic_en_identification,
   "device spec"     : deviceSpec,
}
ionic_en_device_def = defineDeviceSpec(device_def)

#
# Build the VIB
#
ionic_en_vib_def = {
   "identification"  : ionic_en_identification,
   "payload"         : [ ionic_en_module,
                         ionic_en_device_def,
                       ],
   "vib properties"  : {
      "provides"                : [
                                        {'name': '1OEM'}
      
                                  ],
      "depends"                 : [ ],
      "conflicts"               : [ ],
      "replaces"                : [ ],
      "maintenance-mode"        : True,
      "live-install-allowed"    : False,
      "live-remove-allowed"     : False,
      "stateless-ready"         : True,
      "acceptance-level"        : "certified",
   }
}
ionic_en_vib = defineModuleVib(ionic_en_vib_def)

#
# Build the Offline Bundle
#
from devkitUtilities import GenerateFullVibVersionNumber

fullVersion = GenerateFullVibVersionNumber(ionic_en_identification['version'],
                                           ionic_en_identification['vendor_code'],
                                           ionic_en_identification['binary compat'],
					   0)
shortVersion = '%s' % (ionic_en_identification['version'])

ionic_en_bulletin_def = {
   "identification" : ionic_en_identification,
   "vib"            : ionic_en_vib,
   "bulletin" : {
      # These elements show the default values for the corresponding items in bulletin.xml file
      # Uncomment a line if you need to use a different value
      #'severity'    : 'general',
      #'category'    : 'Enhancement',
      #'releaseType' : 'extension',
      #'urgency'     : 'Important',

      'kbUrl'       : 'http://kb.vmware.com/kb/example.html',

      'componentNameSpec' : { 'name' : 'ionic-en',
                              'uistring' : driverSummary
                            },
      'componentVersionSpec' : { 'version' : fullVersion, 'uistring' : shortVersion },


      # 1. At least one target platform needs to be specified with 'productLineID'
      # 2. The product version number may be specified explicitly, like 7.8.9,
      # or, when it's None or skipped, be a default one for the devkit
      # 3. 'locale' element is optional
      'platforms'   : [ {'productLineID':'ESXi'},
      #                 {'productLineID':'ESXi', 'version':"7.8.9", 'locale':''}
                      ]
   }
}
ionic_en_bundle =  defineOfflineBundle(ionic_en_bulletin_def)

