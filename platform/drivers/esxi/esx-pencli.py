import errno
import traceback
import http.client
import subprocess
import sys
import argparse
import os
import ipaddress
import uuid
import textwrap
import ssl
import shutil

TOOL_VERSION = '1.3'

CLI_UPLINKS = 'esxcfg-nics '
CLI_VS = 'esxcfg-vswitch '
CLI_VMK = 'esxcfg-vmknic '
CLI_GET_UPLINK_INFO = 'esxcli network nic get -n '
PEN_MGMT_VS = 'PenMgmtVS'
PEN_MGMT_PG = 'PenMgmtPG'

OLD_DRV_VER = '0.15'
OLD_FW_VER = '1.1.1'

HOST_MGMT_UPLINK_DESC = 'Pensando Ethernet Management'

class MgmtInterface:
    def __init__(self):
        self.name = ''
        self.bus_id = ''
        self.fw_ver = ''
        self.drv_ver = ''
    def __str__(self):
        return self.name + ' ' + self.bus_id + ' ' + self.fw_ver + ' ' + self.drv_ver

class Pencli:
    def CheckEnv(self):
        if self.__multi_dsc:
            if not self.__uplink:
                print('We are in dual DSCs environment, please specify --uplink option')
                return 1
            else:
                for mgmt_interface in self.__pen_mgmt_interface_list:
                    if self.__uplink not in mgmt_interface.name:
                        self.__pen_mgmt_interface_list.remove(mgmt_interface)
        else:
            if self.__uplink:
                print('We are in single DSC environment, --uplink option is not needed')
                return 1
            else:
                self.__uplink = self.__pen_mgmt_interface_list[0].name

        if OLD_FW_VER in self.__pen_mgmt_interface_list[0].fw_ver:
            self.__compat = True
            self.__dsc_int_ip = '169.254.0.1'
        else:
            self.__compat = False
            self.__dsc_int_ip = '169.254.' + self.__pen_mgmt_interface_list[0].bus_id + '.1'

        if OLD_DRV_VER in self.__pen_mgmt_interface_list[0].drv_ver:
            print('Current ionic_en driver version: ' + self.__pen_mgmt_interface_list[0].drv_ver + ' is too old, please update driver first')
            return 1

        return 0


    def __get_fw_drv_ver(self, uplink):
        out = subprocess.Popen(CLI_GET_UPLINK_INFO + uplink, shell=True, stdout=subprocess.PIPE).stdout
        props=out.read().split(b'\n')
        fw_ver = ''
        drv_ver = ''
        for prop in props:
            if 'Firmware Version' in str(prop):
                fw_ver = prop.split(b': ')[1].decode("utf-8")
                continue
            if '   Version:' in str(prop):
                drv_ver = prop.split(b': ')[1].decode("utf-8")
                continue

        return fw_ver, drv_ver

    def __detect_pen_mgmt_interface(self):
        pen_mgmt_interface_list = []
        out = subprocess.Popen(CLI_UPLINKS + ' -l ', shell=True, stdout=subprocess.PIPE).stdout
        uplinks = list(filter(None, out.read().split(b'\n')))[1:]

        for uplink in uplinks:
            if HOST_MGMT_UPLINK_DESC in str(uplink):
                uplink_name = uplink.split(b' ')[0].decode("utf-8")
                bus_id = int(uplink.split()[1].split(b':')[1].decode("utf-8"), 16)
                fw_ver, drv_ver = self.__get_fw_drv_ver(str(uplink_name))
                mgmt_interface = MgmtInterface()
                mgmt_interface.name = str(uplink_name)
                mgmt_interface.bus_id = str(bus_id)
                mgmt_interface.fw_ver = fw_ver
                mgmt_interface.drv_ver = drv_ver
                pen_mgmt_interface_list.append(mgmt_interface)

        return pen_mgmt_interface_list

    def __init__(self, params):
        self.__is_mgmt_nw_configured = False
        self.__dsc_int_ip = ''
        if hasattr(params, 'uplink'):
            self.__uplink = params.uplink
        else:
            self.__uplink = ''

        if hasattr(params, 'verbose'):
            self.__verbose = params.verbose
        else:
            self.__verbose = False

        self.__pen_mgmt_interface_list = self.__detect_pen_mgmt_interface()
        if len(self.__pen_mgmt_interface_list) > 1:
            self.__multi_dsc = True
        else:
            self.__multi_dsc = False


    def __del__(self):
        if self.__is_mgmt_nw_configured:
            self.CleanupPenMgmtNetwork()

    # Check if the given uplink is exist and act for management purpose
    def __find_uplink(self):
        out = subprocess.Popen(CLI_UPLINKS + ' -l ', shell=True, stdout=subprocess.PIPE).stdout
        uplinks = list(filter(None, out.read().split(b'\n')))[1:]

        for uplink in uplinks:
            if self.__uplink in str(uplink) and HOST_MGMT_UPLINK_DESC in str(uplink):
                self.__pcie_id = int(uplink.split()[1].split(b':')[1].decode("utf-8"), 16)
                return 0

        return 1

    # Check if the given uplink is used or not
    def __check_uplink_availability(self):
        out = subprocess.Popen(CLI_VS + ' -l ', shell=True, stdout=subprocess.PIPE).stdout.read()

        if self.__uplink in str(out):
            return 1

        return 0
        
    def __create_pen_mgmt_vs(self):
        return os.system(CLI_VS + ' -a ' + PEN_MGMT_VS)

    def __add_uplink_to_pen_mgmt_vs(self):
        return os.system(CLI_VS + ' -L ' + self.__uplink + ' ' + PEN_MGMT_VS)

    def __add_pg_to_pen_mgmt_vs(self):
        return os.system(CLI_VS + ' -A ' + PEN_MGMT_PG + ' ' + PEN_MGMT_VS)

    def __determine_ip_to_assign(self):
        starting_ip = ipaddress.ip_address(self.__dsc_int_ip) + 1
        while True:
            if os.system('vmkping -c 1 ' + str(starting_ip) + ' > /dev/null'):
                break
            else:
                starting_ip += 1

        return str(starting_ip)

    def __add_vmk_on_pen_mgmt_pg(self):
        # We need to check if 169.254.0.2 has been used?
        vmk_ip = self.__determine_ip_to_assign()
        netmask = '255.255.255.0'

        return os.system(CLI_VMK + ' -a -i ' + vmk_ip + ' -n ' + netmask + ' ' + PEN_MGMT_PG)

    def __configure_mgmt_network(self):
        ret = self.__check_uplink_availability()
        if ret:
            print(self.__uplink + ' is not available!' + ' Please delete the vSwitch and Portgroup that using this interface. (esxcfg-vmknic -d PortgroupName and esxcfg-vswitch -d vSwitchName)')
            return 1

        ret = self.__create_pen_mgmt_vs()
        if ret:
            print('Failed at creating vSwitch: ' + PEN_MGMT_VS)
            return 1

        ret = self.__add_uplink_to_pen_mgmt_vs()
        if ret:
            print('Failed at adding uplink: ' + self.__uplink + ' to vSwitch: ' + PEN_MGMT_VS)
            return 1

        ret = self.__add_pg_to_pen_mgmt_vs()
        if ret:
            print('Failed at adding port group: ' + PEN_MGMT_PG + ' to vSwitch: ' + PEN_MGMT_VS)
            return 1

        ret = self.__add_vmk_on_pen_mgmt_pg()
        if ret:
            print('Failed at adding vmk interface to port group: ' + PEN_MGMT_PG)
            return 1

        self.__is_mgmt_nw_configured = True
        return 0

    def __remove_vmk_from_pen_mgmt_pg(self):
        return os.system(CLI_VMK + ' -d ' + PEN_MGMT_PG)

    def __delete_pen_mgmt_vs(self):
        return os.system(CLI_VS + ' -d ' + PEN_MGMT_VS)

    def CleanupPenMgmtNetwork(self):
        ret = self.__remove_vmk_from_pen_mgmt_pg()
        if ret:
            print('Failed at removing vmk interface from port group: ' + PEN_MGMT_PG)
            return 1

        ret = self.__delete_pen_mgmt_vs()
        if ret:
            print('Failed at deleting Pensando DSC management vSwitch: ' + PEN_MGMT_VS)
            return 1

        self.__is_mgmt_nw_configured = False
        return 0

    def __check_vmk(self):
        out = subprocess.Popen(CLI_VMK + ' -l', shell=True, stdout=subprocess.PIPE).stdout.read()
        ip = self.__dsc_int_ip[:self.__dsc_int_ip.rfind('.')]
        if ip in out.decode('utf-8'):
            vmks = out.split(b'\n')[1:]
            for vmk in vmks:
                if ip in vmk.decode('utf-8'):
                    target_vmk_str = vmk.split(b' ')[0].decode('utf-8').replace(' ', '')
            print('Please check the output of "esxcfg-vmknic -l", unexpected vmk interface ' + target_vmk_str + ' is sitting in the same network as your DSC manangement interface, please remove this interface and rerun this tool.')
            return 1
        return 0

    def ValidateDscConnectivity(self):
        ret = self.__find_uplink()
        if ret:
            print('Cannot find uplink: ' + self.__uplink)
            return 1

        ret = self.__check_vmk()
        if ret:
            print('vmk interface needs to be removed, it is conflict with DSC manangement network')
            return 1

        ret = self.__configure_mgmt_network()
        if ret:
            print('Failed at configuring Pensando DSC management network')
            return 1

        return 0

    def __make_ssh_dir_on_dsc(self, token):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                context = ssl._create_unverified_context()
                context.load_cert_chain(certfile=token, keyfile=token)
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=context)
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"mksshdir\"}')
            response = connection.getresponse()

        except Exception as err:
            if err.errno == errno.ENOENT:
                print(token + ' does not exist, please double check')
                pass
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))
        
        return 0

    def __touch_ssh_auth_keys_on_dsc(self, token):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                context = ssl._create_unverified_context()
                context.load_cert_chain(certfile=token, keyfile=token)
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=context)
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"touchsshauthkeys\"}')
            response = connection.getresponse()

        except Exception as err:
            if err.errno == errno.ENOENT:
                print(token + ' does not exist, please double check')
                pass
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1
        
        return 0

    def __form_req_for_uploading_ssh_key(self, ssh_key):
        try:
            f = open(ssh_key, 'r').read()
        except Exception as err:
            print('Failed at opening file: ' + ssh_key + ', pleast specify the key file or run "/usr/lib/vmware/openssh/bin/ssh-keygen -t rsa -b 4096" first')
            return None, None

        boundary = 'pendsc' + uuid.uuid4().hex
        dataList = []
        dataList.append('--' + boundary)
        dataList.append(('Content-Disposition: form-data; name="uploadFile"; filename="{}"').format(ssh_key))
        dataList.append('Content-Type: application/octet-stream')
        dataList.append('')
        dataList.append(f)
        dataList.append('--'+boundary)
        dataList.append('Content-Disposition: form-data; name="uploadPath"')
        dataList.append('')
        dataList.append("/update/")
        dataList.append('--'+boundary+'--')
        contentType = 'multipart/form-data; boundary="' + boundary + '"'
        b='\r\n'.join(dataList)
        headers = {'Content-type': contentType}
        return b, headers

    def __upload_file(self, body, headers, file_name):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=2000)
            else:
                context = ssl._create_unverified_context()
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=2000, context=context)

            print('Uploading file: ' + file_name)
            connection.request("POST", "/update/", body, headers)
            response = connection.getresponse()

        except Exception as err:
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {} response : {}".format(response.status, response.reason, response))

        if response.reason != 'OK':
            return 1
        
        return 0

    def __set_ssh_key(self, token):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                context = ssl._create_unverified_context()
                context.load_cert_chain(certfile=token, keyfile=token)
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=context)
            print("Setting ssh auth keys on DSC")
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"setsshauthkey\", \"opts\":\"id_rsa.pub\"}')
            response = connection.getresponse()

        except Exception as err:
            if err.errno == errno.ENOENT:
                print(token + ' does not exist, please double check')
                pass
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0

    def __enable_ssh_key(self, token):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                context = ssl._create_unverified_context()
                context.load_cert_chain(certfile=token, keyfile=token)
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=context)
            print("Enabling ssh access on DSC")
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"enablesshd\"}')
            response = connection.getresponse()

        except Exception as err:
            if err.errno == errno.ENOENT:
                print(token + ' does not exist, please double check')
                pass
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0

    def __start_sshd(self, token):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                context = ssl._create_unverified_context()
                context.load_cert_chain(certfile=token, keyfile=token)
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=context)
            print("Starting sshd on DSC")
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"startsshd\"}')
            response = connection.getresponse()

        except Exception as err:
            if err.errno == errno.ENOENT:
                print(token + ' does not exist, please double check')
                pass
            if self.__verbose:
                traceback.print_exc() 
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0


    def __run_shell_cmd_on_dsc(self, cmd):
        final_cmd = 'ssh ' '-o StrictHostKeyChecking=no ' + 'root@{}'.format(self.__dsc_int_ip) + ' ' + cmd
        output = subprocess.getoutput(final_cmd)
        return output

    def __install_fw_on_dsc(self):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=1200)
            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=1200, context=ssl._create_unverified_context())

            print('Installing firmware on DSC')
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"installFirmware\", \"opts\":\"naples_fw.tar all\"}')
            response = connection.getresponse()

        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0

    def __remove_fw_on_dsc(self):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=100)
            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=100, context=ssl._create_unverified_context())

            print('Removing old firmware image on DSC')
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"penrmfirmware\", \"opts\":\"naples_fw.tar\"}')
            response = connection.getresponse()

        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0

    def __set_alt_fw_on_dsc(self):
        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=10)
            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=100, context=ssl._create_unverified_context())

            print("Setting alternative firmware")
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"setStartupToAltfw\", \"opts\":\"\"}')
            response = connection.getresponse()

        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))
 
        if response.reason != 'OK':
            return 1

        return 0

    def UpgradeDscFw(self):
        ret = self.__install_fw_on_dsc()
        if ret:
            print('Failed at installing firmware image on DSC')
            return 1

        ret = self.__remove_fw_on_dsc()
        if ret:
            print('Failed at removing old firmware image on DSC')
            return 1

        ret = self.__set_alt_fw_on_dsc()
        if ret:
            print('Failed at setting alternative firmware image on DSC')
            return 1

        return 0

    def ChangeDscMode(self, dsc_id, config_opt, management_network, mgmt_ip, gw, controllers):
        cmd = ''
        if config_opt == 'static':
            controllers_str = ''
            controllers = controllers.split(',')
            for controller in controllers:
                controllers_str += '"{}"'.format(controller)+','
            controllers_str = controllers_str[:-1]

            cmd = '{"kind":"","meta":{"name":"","generation-id":"","creation-time":"1970-01-01T00:00:00Z","mod-time":"1970-01-01T00:00:00Z"},"spec":{"ID":"' + dsc_id + '","ip-config":{"ip-address":"' + mgmt_ip + '","default-gw":"' + gw +'"},"mode":"NETWORK","network-mode":"' + management_network.upper() +'","controllers":[' + controllers_str + '],"naples-profile":"default"},"status":{"mode":""}}'
        else:
            cmd = '{"kind":"","meta":{"name":"","generation-id":"","creation-time":"1970-01-01T00:00:00Z","mod-time":"1970-01-01T00:00:00Z"},"spec":{"ID":"' + dsc_id + '","ip-config":{},"mode":"NETWORK","network-mode":"' + management_network.upper() +'","naples-profile":"default"},"status":{"mode":""}}'

        try:
            if self.__compat:
                connection = http.client.HTTPConnection(self.__dsc_int_ip, 8888, timeout=50)
            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=50, context=ssl._create_unverified_context())

            print("Changing DSC mode")
            connection.request("POST", "/api/v1/naples/", cmd)
            response = connection.getresponse()

        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return 1

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return 1

        return 0

    def EnableSshAccess(self, ssh_key, token):
        if self.__compat == False:
            if not token:
                print('The current firmware requires a token file for enabling ssh, please use --token option to specify a token file')
                return 1

        ret = self.__make_ssh_dir_on_dsc(token)
        if ret:
            print('Failed at making ssh directory on Pensando DSC')
            return 1

        ret = self.__touch_ssh_auth_keys_on_dsc(token)
        if ret:
            print('Failed at touching ssh auth key on Pensando DSC')
            return 1

        body, headers = self.__form_req_for_uploading_ssh_key(ssh_key)
        if body is None or headers is None:
            print('Failed at forming request for uploading ssh key')
            return 1

        ret = self.__upload_file(body, headers, ssh_key)
        if ret:
            print('Failed at uploading ssh key file: ' + ssh_key)
            return 1
        print('Upload completed for ssh key file: ' + ssh_key)

        ret = self.__set_ssh_key(token)
        if ret:
            print('Failed at setting ssh key')
            return 1

        ret = self.__enable_ssh_key(token)
        if ret:
            print('Failed at enabling ssh access')
            return 1

        ret = self.__start_sshd(token)
        if ret:
            print('Failed at starting sshd')
            return 1

        print('Setting up ssh access on DSC has completed')

        return 0
    def __read_in_chunks(self, file_object, chunk_size=4096):
        while True:
            data = file_object.read(chunk_size)
            if not data:
                break
            yield data

    def __form_req_for_uploading_fw_img(self, fw_img, path):
        try:
            shutil.rmtree(path)
        except Exception as err:
            pass

        try:
            os.mkdir(path)
        except Exception as err:
            if err.errno == errno.ENOSPC:
                print('No space left under this path: ' + path + '. Please cleanup.')
            else:
                pass
        try:
            fin = open(fw_img, 'rb')
        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            print('Failed at opening file: ' + fw_img)
            return None

        try:
            fout = open(path + 'naples_fw.tar', 'wb')
        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            print('Failed at creating tmp file: ' + path + 'naples_fw.tar')
            return None

        boundary = 'pendsc' + uuid.uuid4().hex
        try:
            fout.write(('--' + boundary + '\r\n').encode())
            fout.write((('Content-Disposition: form-data; name="uploadFile"; filename="{}"\r\n').format('/tmp/.pencli/naples_fw.tar')).encode())
            fout.write('Content-Type: application/octet-stream\r\n'.encode())
            fout.write('\r\n'.encode())
            for piece in self.__read_in_chunks(fin):
                fout.write(piece)
        
            fout.write('\r\n'.encode())
            fout.write(('--'+boundary+'\r\n').encode())
            fout.write(('Content-Disposition: form-data; name="uploadPath"\r\n').encode())
            fout.write('\r\n'.encode())
            fout.write("/update/\r\n".encode())
            fout.write(('--'+boundary+'--\r\n').encode())
        except Exception as err:
            if err.errno == errno.ENOSPC:
                print('No space left under this path: ' + path.replace('/.esx-pencli/', '') + '. Please cleanup.')
                shutil.rmtree(path)

            if self.__verbose:
                traceback.print_exc()
            return None

        fout.close()
        fin.close()

        contentType = 'multipart/form-data; boundary="' + boundary + '"'
        headers = {'Content-type': contentType}
        return headers

    def UploadFwImgToDsc(self, fw_img):
        path = os.getcwd() + '/.esx-pencli/'
        headers = self.__form_req_for_uploading_fw_img(fw_img, path)
        if headers is None:
            print('Failed at forming request for uploading firmware image')
            return 1

        fout = open(path + 'naples_fw.tar', 'rb')

        ret = self.__upload_file(fout, headers, fw_img)
        if ret:
            print('Failed at uploading firmware image file: ' + fw_img)
            return 1

        fout.close()
        shutil.rmtree(path)
        return 0

    def GetDscAllMgmtInterfaces(self):
        try:
            if self.__compat:
                print('Current DSC firmware does not support getting mac address of an interface on DSC, please move to a new version(at least 1.3)')
                return None

            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=ssl._create_unverified_context())
               
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"showinterfacemanagement\"}')
            response = connection.getresponse()
        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return None

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return None

        return response.read()


    def GetInfMacAddress(self, inf_name):
        try:
            if self.__compat:
                print('Current DSC firmware does not support getting mac address of an interface on DSC, please move to a new version(at least 1.3)')
                return None

            else:
                connection = http.client.HTTPSConnection(self.__dsc_int_ip, 8888, timeout=10, context=ssl._create_unverified_context())
               
            connection.request("GET", "/cmd/v1/naples/", '{\"executable\":\"showinterfacemanagement\"}')
            response = connection.getresponse()
        except Exception as err:
            if self.__verbose:
                traceback.print_exc()
            return None

        if self.__verbose:
            print("Status: {} and reason: {}".format(response.status, response.reason))

        if response.reason != 'OK':
            return None
        else:
            results = str(response.read()).split('\\n\\n')
            for result in results:
                if inf_name in result:
                    return result.split('HWaddr ',1)[1][:17]

        print('Interface: ' + inf_name + ' does not exist on DSC')
        return None

def ConfigureSSH(args):
    pencli = Pencli(args)

    ret = pencli.CheckEnv()
    if ret:
        print('Failed at checking environment')
        return 1

    ret = pencli.ValidateDscConnectivity()
    if ret:
        print('Failed at validating connectivity to Pensando DSC')
        return 1

    ret = pencli.EnableSshAccess(args.ssh_key, args.token)

    pencli.CleanupPenMgmtNetwork()

    return ret, pencli

def GetDscAllMgmtInterfaces(args):
    pencli = Pencli(args)

    ret = pencli.CheckEnv()
    if ret:
        print('Failed at checking environment')
        return 1

    ret = pencli.ValidateDscConnectivity()
    if ret:
        print('Failed at validating connectivity to Pensando DSC')
        return 1

    out = pencli.GetDscAllMgmtInterfaces()
    pencli.CleanupPenMgmtNetwork()
    if out is None:
        print('Failed at getting interfaces information on DSC')
        return 1

    print((out).decode('utf-8'))
    return ret


def GetInfMacAddress(args):
    pencli = Pencli(args)

    ret = pencli.CheckEnv()
    if ret:
        print('Failed at checking environment')
        return 1

    ret = pencli.ValidateDscConnectivity()
    if ret:
        print('Failed at validating connectivity to Pensando DSC')
        return 1

    out = pencli.GetInfMacAddress(args.inf_name)
    pencli.CleanupPenMgmtNetwork()
    if out is None:
        print('Failed at getting mac address of interface: ' + args.inf_name)
        return 1

    print(args.inf_name + ': ' + out)

    return ret

def UpgradeDsc(args):
    pencli = Pencli(args)

    ret = pencli.CheckEnv()
    if ret:
        print('Failed at checking environment')
        return 1

    ret = pencli.ValidateDscConnectivity()
    if ret:
        print('Failed at validating connectivity to Pensando DSC')
        return 1

    ret = pencli.UploadFwImgToDsc(args.fw_img)
    if ret:
        print('Failed at uploading file :' + args.fw_img + ' to Pensando DSC')
        return 1

    ret = pencli.UpgradeDscFw()
    pencli.CleanupPenMgmtNetwork()
    if ret:
        print('Failed at doing firmware upgrade on DSC')
        return 1

    print('Pensando DSC has been upgraded successfully, please reboot the host')

    return ret

def ValidateArgsForDscModeChange(args):
    if args.config_opt != 'static' and args.config_opt != 'dhcp':
        print('Please validate config_opt.(static or dhcp)')
        return 1

    if args.management_network != 'inband' and args.management_network != 'oob':
        print('Please provide manangement network information.(inband or oob)')
        return 1

    if args.config_opt == 'static':
        if args.mgmt_ip is None:
            print('Please provide management IP address')
            return 1
        if '/' not in args.mgmt_ip:
            print('Please provide manangement IP address in CIDR format(For example: 10.10.10.1/24')
            return 1
        if args.gw is None:
            print('Please provide default gateway')
            return 1
        if args.controllers is None:
            print('Please provide information of controller(s)')
            return 1
        if len(args.controllers.split(',')) % 2 == 0:
            print('Number of controllers must be an odd number, current number: ' + str(len(args.controllers.split(','))))
            return 1
    else:
        if args.mgmt_ip is not None:
            print('You do not need to provide management IP address for dhcp based configurations.')
            return 1
        if args.gw is not None:
            print('You do not need to provide default gateway for dhcp based configurations.')
            return 1
        if args.controllers is not None:
            print('You do not need to provide information of controller(s) for dhcp based configurations.')
            return 1
    return 0


def ChangeDscMode(args):
    ret = ValidateArgsForDscModeChange(args)
    if ret:
        print('Failed at validating parameters for DSC mode change')
        return 1

    pencli = Pencli(args)

    ret = pencli.CheckEnv()
    if ret:
        print('Failed at checking environment')
        return 1

    ret = pencli.ValidateDscConnectivity()
    if ret:
        print('Failed at validating connectivity to Pensando DSC')
        return 1

    ret = pencli.ChangeDscMode(args.dsc_id, args.config_opt, args.management_network, args.mgmt_ip, args.gw, args.controllers)
    pencli.CleanupPenMgmtNetwork()

    if ret:
        print('Failed at changing Pensando DSC mode')
        return 1

    print('Pensando DSC mode has been changed to network managed successfully, please reboot the host')

    return ret

def GetVersion(args):
    print('The current version of this tool is: ' + TOOL_VERSION)
    return 0

#------------------------------------------------------------------------------
# Main processing from here
#------------------------------------------------------------------------------
if __name__ == '__main__':

    # Parse command line args
    parser = argparse.ArgumentParser(prog='esx-pencli', formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent('''\
            Additional Tips:
                1. You can run "/usr/lib/vmware/openssh/bin/ssh-keygen -t rsa -b 4096" to generate ssh key on ESXi host
                2. You will need to disable your firewall by running "esxcli network firewall set -e 0"
                3. You can re-enable your firewall once you have completed all your tasks by running "esxcli network firewall set -e 1"

            Examples:
                1. Query mac address on a specific interface on DSC
                   python esx-pencli.pyc get_dsc_inf_mac_address --inf_name inb_mnic1
                2. Query detailed information of all interfaces on DSC
                   python esx-pencli.pyc get_dsc_all_mgmt_interfaces                   
                3. Perform FW upgrade(Require to reboot the ESXi host after this)
                   python esx-pencli.pyc upgrade_dsc  --fw_img /vmfs/volumes/datastore1/naples_fw.tar
                4. Change Pensando DSC mode from host managed to network managed(Require to reboot the ESXi host after this)
                   python esx-pencli.pyc change_dsc_mode --config_opt static --management_network oob --dsc_id pen_dsc1 --mgmt_ip 10.10.10.10/24 --gw 10.10.10.1 --controllers 10.10.10.11,10.10.10.12,10.10.10.13
        '''))          
    subparsers = parser.add_subparsers()

#    subparser = subparsers.add_parser('config_ssh', help = 'Configure ssh access to Pensando DSC')
#    subparser.set_defaults(callback=ConfigureSSH) 
#    subparser.add_argument('--ssh_key', default = '/.ssh/id_rsa.pub', help = 'Public ssh key of the host')
#    subparser.add_argument('--token', default = '', help = 'Token used for setting up ssh auth for firmware image equal or later than 1.3')
#    subparser.add_argument('--uplink', default = '', help = 'Which management uplink to be used, required only in dual DSCs environment')
#    subparser.add_argument('--verbose', action='store_true', help = 'increase output verbosity')

    subparser = subparsers.add_parser('get_dsc_inf_mac_address', help = 'Get the mac address of the given interface of the DSC')
    subparser.set_defaults(callback=GetInfMacAddress) 
    subparser.add_argument('--uplink', default = '', help = 'Which management uplink to be used, required only in dual DSCs environment')
    subparser.add_argument('--inf_name', required=True, help = 'Interface on DSC that we get mac from')
    subparser.add_argument('--verbose', action='store_true', help = 'increase output verbosity')

    subparser = subparsers.add_parser('get_dsc_all_mgmt_interfaces', help = 'Get detailed information of all interfaces of the DSC')
    subparser.set_defaults(callback=GetDscAllMgmtInterfaces) 
    subparser.add_argument('--uplink', default = '', help = 'Which management uplink to be used, required only in dual DSCs environment')
    subparser.add_argument('--verbose', action='store_true', help = 'increase output verbosity')

    subparser = subparsers.add_parser('upgrade_dsc', help = 'Perform FW upgrade(Require to reboot the ESXi host after this)')
    subparser.set_defaults(callback=UpgradeDsc) 
    subparser.add_argument('--fw_img', required=True, help = 'Which firmware image on the host file system that need to be used')
    subparser.add_argument('--uplink', default = '', help = 'Which management uplink to be used, required only in dual DSCs environment')
    subparser.add_argument('--verbose', action='store_true', help = 'increase output verbosity')

    subparser = subparsers.add_parser('change_dsc_mode', help = 'Change Pensando DSC mode from host managed to network managed(Require to reboot the ESXi host after this)')
    subparser.set_defaults(callback=ChangeDscMode) 
    subparser.add_argument('--config_opt', required=True, help = 'Use static/dhcp based configurations for DSC mode change(static/dhcp)')
    subparser.add_argument('--management_network', required=True, help = 'Management Network(inband or oob)')
    subparser.add_argument('--mgmt_ip', help = 'Management IP in CIDR format(only required for static configuration)')
    subparser.add_argument('--gw', help = 'Default GW for mgmt')
    subparser.add_argument('--controllers', help = 'List of controller IP addresses or ids, separated by commas(for example: 10.10.10.11,10.10.10.12,10.10.10.13')
    subparser.add_argument('--uplink', default = '', help = 'Which management uplink to be used, required only in dual DSCs environment')
    subparser.add_argument('--dsc_id', required=True, help = 'User defined DSC ID')
    subparser.add_argument('--verbose', action='store_true', help = 'increase output verbosity')

    subparser = subparsers.add_parser('version', help = 'Return the current version of this tool')
    subparser.set_defaults(callback=GetVersion) 

    # Parse the args
    args = parser.parse_args()
    
    try:
        args.callback(args)

    except AttributeError:
        parser.print_help()
        exit()






