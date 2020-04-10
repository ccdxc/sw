#include "Config.h"

#include "UserCommon.h"
#include "ionic_types.h"

DWORD
get_interface_name(WCHAR *name,
				   DWORD name_sz,
                   WCHAR *ifname,
				   DWORD ifname_sz,
				   WCHAR *descrname,
				   DWORD descrname_sz) // in WCHARS, not bytes
{

	DWORD rc = ERROR_SUCCESS;
	char buffer[ 256];
	char *target = NULL;
    bp::ipstream output;
    std::vector<std::string> data;
    std::string line;
    std::vector<std::string> fields;
    std::string key;
    std::string value;

    if (wcsnlen_s( name, name_sz) == 0 &&
		wcsnlen_s( ifname, ifname_sz) == 0) {
        sprintf_s(buffer, 256, "powershell.exe \"Get-NetAdapter -InterfaceDescription \\\"%S\\\" | Format-List -Property \\\"InterfaceDescription\\\"\", \\\"ifIndex\\\"\", \\\"Name\\\"\" ", descrname);
    }
    else if( wcsnlen_s( name, name_sz) == 0) {
        sprintf_s(buffer, 256, "powershell.exe \"Get-NetAdapter -ifIndex \\\"%S\\\" | Format-List -Property \\\"InterfaceDescription\\\"\", \\\"ifIndex\\\"\", \\\"Name\\\"\" ", ifname);
    }
    else if( wcsnlen_s( ifname, ifname_sz) == 0) {
        sprintf_s(buffer, 256, "powershell.exe \"Get-NetAdapter -Name \\\"%S\\\" | Format-List -Property \\\"InterfaceDescription\\\"\", \\\"ifIndex\\\"\", \\\"Name\\\"\" ", name);
    }
	else {
		return ERROR_INVALID_PARAMETER;
	}

    bp::child c((const char *)buffer, 
        bp::std_out > output);

    while (output && std::getline(output, line)) {
        boost::split(fields, line, boost::is_any_of(":"));
        if (fields.size() == 2) {
            boost::algorithm::trim(fields[0]);
            boost::algorithm::trim(fields[1]);

            if (descrname != NULL && fields[0] == "InterfaceDescription")
            {
                const char *pName = fields[1].c_str();
                size_t convertedChars = 0;
                rc = mbstowcs_s(&convertedChars, descrname, (size_t)descrname_sz, pName, _TRUNCATE);

                if (rc != 0)
                {
                    goto out;
                }
            }
            else if (ifname != NULL && fields[0] == "ifIndex")
            {
                const char *pName = fields[1].c_str();
                size_t convertedChars = 0;
                rc = mbstowcs_s(&convertedChars, ifname, (size_t)ifname_sz, pName, _TRUNCATE);

                if (rc != 0)
                {
                    goto out;
                }
            }
            else if (name != NULL && fields[0] == "Name")
            {
                const char *pName = fields[1].c_str();
                size_t convertedChars = 0;
                rc = mbstowcs_s(&convertedChars, name, (size_t)name_sz, pName, _TRUNCATE);

                if (rc != 0)
                {
                    goto out;
                }
            }
        }
    }

    c.wait();

out:

	return rc;
}

DWORD
OptGetAlternateNames(command_info& info,
				  WCHAR *name,
				  DWORD name_sz,
				  WCHAR *ifname,
				  DWORD ifname_sz,
				  WCHAR *descrname,
				  DWORD descrname_sz)
{

	DWORD rc = ERROR_SUCCESS;

	if (info.vm.count("Name")) {
	    opval_wstrncpy(name, name_sz, info.vm, "Name");
	}
	else if (info.vm.count("ifIndex")) {
	    opval_wstrncpy(ifname, ifname_sz, info.vm, "ifIndex");
	}
	else if (info.vm.count("InterfaceDescription")) {
		opval_wstrncpy(descrname, descrname_sz, info.vm, "InterfaceDescription");
	}
	else {
		rc = ERROR_NOT_FOUND;
		goto out;
	}

	if (get_interface_name(name, name_sz, ifname, ifname_sz, descrname, descrname_sz) != ERROR_SUCCESS) {
		rc = ERROR_INVALID_PARAMETER;
		goto out;
	}

out:

	return rc;
}

void
OptGetDevName(command_info& info,
	WCHAR *devname,
	DWORD devname_sz,
	BOOL required)
{
	DWORD	rc = ERROR_SUCCESS;
	WCHAR	cap_name[ ADAPTER_NAME_MAX_SZ] = {};
	WCHAR  *name_ptr = NULL;
	WCHAR  *ifname_ptr = NULL;

	if (info.vm.count("Name")) {
	    opval_wstrncpy(cap_name, ADAPTER_NAME_MAX_SZ, info.vm, "Name");
		name_ptr = cap_name;
	}
	else if (info.vm.count("ifIndex")) {
		opval_wstrncpy(cap_name, ADAPTER_NAME_MAX_SZ, info.vm, "ifIndex");
		ifname_ptr = cap_name;
	}
	else if (info.vm.count("InterfaceDescription")) {
		opval_wstrncpy(devname, devname_sz, info.vm, "InterfaceDescription");
		return;
	}
	else if(required) {
		throw po::required_option("Name");
	}
	else {
		// not required param, and no param was given
		return;
	}

	rc = get_interface_name(name_ptr,
		name_ptr == NULL ? 0 : ADAPTER_NAME_MAX_SZ,
		ifname_ptr,
		ifname_ptr == NULL ? 0 : ADAPTER_NAME_MAX_SZ,
		devname,
		devname_sz);

	if (rc != ERROR_SUCCESS || devname[0] == 0) {
		std::string param;
		if (name_ptr != NULL) {
			param = "Name";
		}
		else {
			param = "ifIndex";
		}
		std::string val = to_bytes(opval_wstring(info.vm, param));
		invalid_option(param, val);
	}
}

void
OptAddDevName(po::options_description& opts)
{
	opts.add_options()
		("Name,n", optype_wstring(), "Interface name")
		("ifIndex,i", optype_wstring(), "Interface index")
		("InterfaceDescription,DevName,d", optype_wstring(), "Device friendly name");
}