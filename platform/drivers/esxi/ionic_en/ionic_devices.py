"""
/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_devices.py] in the main directory of this source tree.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
"""

DEV = { 
   "priority"                   : "default",
   "bus type"                   : "pci",
   "device info"                : { 
      "vendor list"                : [ 
         {   
            "vendor id"               : "1dd8",
            "vendor description"      : "Pensando Systems, Inc",
            "devices" : [ 
               {   
                  "device id"         : "1002",
                  "description"       : "Pensando Ethernet PF",
               }, 
               {   
                  "device id"         : "1003",
                  "description"       : "Pensando Ethernet VF",
               }, 
               {   
                  "device id"         : "1004",
                  "description"       : "Pensando Ethernet Management",
               }, 
            ],  
         },  
      ]   
   },  
}
