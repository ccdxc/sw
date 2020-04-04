
#include "windows.h"
#include "stdio.h"
#include <shlwapi.h>

#include <netcfgx.h>
#include <netcfgn.h>
#include <setupapi.h>
#include <devguid.h>
#include <objbase.h>
#include <strsafe.h>
#include <cfgmgr32.h>

#include <newdev.h> // for the API UpdateDriverForPlugAndPlayDevices().
#include <setupapi.h> // for SetupDiXxx functions.

#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h> // for SetupDiXxx functions.
#include <iostream>

#include "UserCommon.h"

#include "pci_ids.h"

int 
GetOEMFile( WCHAR *OEMFile);

BOOL 
IsOEMPackage(LPCTSTR InfName);

int 
AddDriverPkg( WCHAR *InfFile);

int 
RemoveDriverPkg( void);

int 
RestartDevice( WCHAR *HardwareId);

LPWSTR
GetDeviceHwId( HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo);

int 
ChangeDeviceState( HDEVINFO Devs, 
                   PSP_DEVINFO_DATA DevInfo, 
                   DWORD State);

int 
RemoveDevice( WCHAR *HardwareId);

int 
RemoveDeviceState(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo);

int 
UpdateDriver( WCHAR *InfFile, WCHAR *HardwareId, BOOL *reboot);

int
InstallMiniport(WCHAR *InfFile)
{

    int rc = ERROR_SUCCESS;

    printf("Installing INF %S\n", InfFile);

    rc = AddDriverPkg( InfFile);

    if (rc != ERROR_SUCCESS) {
        printf("Failed to install driver package for %S Error %d\n",
                            InfFile,
                            rc);
        goto exit;
    }

    rc = RestartDevice((WCHAR *)L"PCI\\VEN_1DD8&DEV_1002"); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to restart device PCI\\VEN_1DD8&DEV_1002 Error %d\n", rc);
    }

    rc = RestartDevice((WCHAR *)L"PCI\\VEN_1DD8&DEV_1004"); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to restart device PCI\\VEN_1DD8&DEV_1004 Error %d\n", rc);
    }

    printf("Successfully installed driver\n");

exit:

    return rc;
}

int
UninstallMiniport()
{

    int rc = ERROR_SUCCESS;

    printf("Removing Pensando driver package\n");

    rc = RemoveDevice((WCHAR *)L"PCI\\VEN_1DD8&DEV_1002"); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to disable device PCI\\VEN_1DD8&DEV_1002 Error %d\n", rc);
        goto exit;
    }

    rc = RemoveDevice((WCHAR *)L"PCI\\VEN_1DD8&DEV_1004"); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to disable device PCI\\VEN_1DD8&DEV_1004 Error %d\n", rc);
        goto exit;
    }

    rc = RemoveDriverPkg();

    if (rc != ERROR_SUCCESS) {
        printf("Failed to remove driver package Error %d\n",
                                        rc);
        goto exit;
    }

    printf("Successfully removed driver, reboot system\n");

exit:

    return rc;
}

int
UpdateMiniport(WCHAR *InfFile)
{

    int rc = ERROR_SUCCESS;
    BOOL reboot = TRUE;

    printf("Updating INF %S\n", InfFile);

    rc = UpdateDriver(InfFile, (WCHAR *)L"PCI\\VEN_1DD8&DEV_1002", &reboot); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to update device PCI\\VEN_1DD8&DEV_1002 Error %d\n", rc);
        goto exit;
    }

    rc = UpdateDriver(InfFile, (WCHAR *)L"PCI\\VEN_1DD8&DEV_1004", NULL); 
    if( rc != ERROR_SUCCESS) {
        printf("Failed to update device PCI\\VEN_1DD8&DEV_1004 Error %d\n", rc);
        goto exit;
    }

    printf("Successfully updated driver, reboot required %s\n", reboot?"Yes":"No");

exit:

    return rc;
}

int 
AddDriverPkg( WCHAR *InfFile)
{

    int        rc = ERROR_SUCCESS;
    DWORD    res;
    WCHAR    SourceInfFileName[MAX_PATH];
    WCHAR    DestinationInfFileName[MAX_PATH];
    LPWSTR    DestinationInfFileNameComponent = NULL;
    LPWSTR    FilePart = NULL;

    res = GetFullPathName(InfFile,
                          ARRAYSIZE(SourceInfFileName),
                          SourceInfFileName,
                          &FilePart);
    if ((!res) || (res >= ARRAYSIZE(SourceInfFileName))) {
        rc = GetLastError();
        printf("Failed to get full path to INF %S Error %d\n",
                    InfFile,
                    rc);
        goto exit;
    }

    if (!SetupCopyOEMInf(SourceInfFileName,
                         NULL,
                         SPOST_PATH,
                         0, //SP_COPY_NOOVERWRITE,
                         DestinationInfFileName,
                         ARRAYSIZE(DestinationInfFileName),
                         NULL,
                         &DestinationInfFileNameComponent)) {
        rc = GetLastError();
        printf("Failed SetupCopyOEMInf() Error %d\n",
                        rc);
        goto exit;
    }

    printf("Successfully installed driver package to %S\n",
                                DestinationInfFileName);

exit:

    return rc;
}

int 
RemoveDriverPkg()
{

    int rc = ERROR_SUCCESS;
    DWORD result;
    WCHAR InfFileName[MAX_PATH];
    WCHAR OEMFileName[MAX_PATH];
    PWSTR FilePart = NULL;
    HMODULE setupapiMod = NULL;

    while( TRUE) {

        rc = GetOEMFile( OEMFileName);

        if (rc != ERROR_SUCCESS) {
            if( rc != ERROR_FILE_NOT_FOUND) {
                printf("Failed to locate OEM file Error %d\n",
                                rc);
            }
            else {
                rc = ERROR_SUCCESS;
            }
            goto exit;
        }

        result = GetFullPathName(OEMFileName,
                                 ARRAYSIZE(InfFileName),
                                 InfFileName,
                                 &FilePart);
        if ((!result) || (!FilePart)) {
            rc = GetLastError();
            printf("Failed to get full path name to %S Error %d\n",
                                    OEMFileName,
                                    rc);
            goto exit;
        }

        if (!SetupUninstallOEMInf(FilePart,
                                  SUOI_FORCEDELETE,
                                  NULL)) {
            rc = GetLastError();
            if( rc != ERROR_FILE_NOT_FOUND) {
                printf("Failed to remove OEM File %S Error %d\n",
                                        OEMFileName,
                                        rc);
            }
            else {
                rc = ERROR_SUCCESS;
            }

            goto exit;
        }

        printf("Successfully removed driver package %S\n",
                            OEMFileName);
    }

exit:

    return rc;
}

int 
GetOEMFile( WCHAR *OEMFile)
{
    int rc = ERROR_SUCCESS;
    HRESULT result = S_OK;
    TCHAR FindName[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA wfd;

    if (!GetWindowsDirectory(FindName, ARRAYSIZE(FindName)) ||
        FAILED( result = StringCchCat(FindName, ARRAYSIZE(FindName), TEXT("\\INF\\OEM*.INF")))) {
        printf("Failed to loate system folder Error 0x%08lX\n",
                                result);
        goto exit;
    }

    hFind = FindFirstFile(FindName, &wfd);
    if (hFind == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        printf("No OEM files located Error %d\n",
                            rc);
        goto exit;
    }

    do {
        if (IsOEMPackage(wfd.cFileName)) {
            printf("Located OEM file %S\n", wfd.cFileName);
            wcscpy_s( OEMFile, MAX_PATH, wfd.cFileName);
            break;
        }
    } while (FindNextFile(hFind, &wfd));

    FindClose(hFind);

exit:

    return rc;
}

BOOL 
IsOEMPackage(LPCTSTR InfName)
{
    BOOL IsPackage = FALSE;
    DWORD rc = ERROR_SUCCESS;
    HINF hInf = INVALID_HANDLE_VALUE;
    WCHAR provider[ 2048];
    INFCONTEXT Context;

    hInf = SetupOpenInfFile(InfName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);
    if (hInf == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        printf("Failed to open OEM file %S Error %d\n",
                                InfName,
                                rc);
        goto exit;
    }

    //
    // Dump out the provider.
    //
    if (!SetupFindFirstLine(hInf,
                            L"Version",
                            L"Provider",
                            &Context)) {
        rc = GetLastError();
        printf("Failed to locate provider in OEM %S Error %d\n",
                                InfName,
                                rc);
        goto exit;
    }

    if( !SetupGetStringField(&Context,
                             1,
                             provider,
                             ARRAYSIZE(provider),
                             NULL)) {
        rc = GetLastError();
        printf("Failed to locate provider in OEM %S Error %d\n",
                                InfName,
                                rc);
        goto exit;
    }

    if (_wcsicmp(L"pensando systems",
        provider) == 0) {
        IsPackage = TRUE;
    }

exit:

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }

    return IsPackage;
}

int 
RestartDevice( WCHAR *HardwareId)
{
    int rc = ERROR_SUCCESS;
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    WCHAR devID[200];
    LPWSTR hwIds = NULL;
    BOOLEAN match = FALSE;

    devs = SetupDiGetClassDevsEx(NULL,
                                 NULL,
                                 NULL,
                                 DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                 NULL,
                                 NULL,
                                 NULL);
    if(devs == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto exit;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        match = FALSE;

        if(CM_Get_Device_ID_Ex(devInfo.DevInst, devID, 200, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS) {
            devID[0] = TEXT('\0');
        }

        hwIds = GetDeviceHwId(devs, &devInfo);

        if( hwIds != NULL) {

            if( _wcsnicmp( hwIds, HardwareId, wcslen( HardwareId)) == 0) {
                match = TRUE;
            }

            free(hwIds);
        }

        if( match) {

            rc = ChangeDeviceState( devs, 
                                    &devInfo,
                                    DICS_PROPCHANGE);

            if (rc != ERROR_SUCCESS) {
                printf("%s ChangeDeviceState() failed Error %d\n",
                                    __FUNCTION__,
                                    rc);
            }
        }
    }

exit:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }

    return rc;
}

int 
DisableDevice( WCHAR *HardwareId)
{
    int rc = ERROR_SUCCESS;
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    WCHAR devID[200];
    LPWSTR hwIds = NULL;
    BOOLEAN match = FALSE;

    devs = SetupDiGetClassDevsEx(NULL,
                                 NULL,
                                 NULL,
                                 DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                 NULL,
                                 NULL,
                                 NULL);
    if(devs == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto exit;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        match = FALSE;

        if(CM_Get_Device_ID_Ex(devInfo.DevInst, devID, 200, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS) {
            devID[0] = TEXT('\0');
        }

        hwIds = GetDeviceHwId(devs, &devInfo);

        if( hwIds != NULL) {

            if( _wcsnicmp( hwIds, HardwareId, wcslen( HardwareId)) == 0) {
                match = TRUE;
            }

            free(hwIds);
        }

        if( match) {

            rc = ChangeDeviceState( devs, 
                                    &devInfo,
                                    DICS_DISABLE);

            if (rc != ERROR_SUCCESS) {
                printf("%s ChangeDeviceState() failed Error %d\n",
                                    __FUNCTION__,
                                    rc);
            }
        }
    }

exit:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }

    return rc;
}

int 
EnableDevice( WCHAR *HardwareId)
{
    int rc = ERROR_SUCCESS;
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    WCHAR devID[200];
    LPWSTR hwIds = NULL;
    BOOLEAN match = FALSE;

    devs = SetupDiGetClassDevsEx(NULL,
                                 NULL,
                                 NULL,
                                 DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                 NULL,
                                 NULL,
                                 NULL);
    if(devs == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto exit;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        match = FALSE;

        if(CM_Get_Device_ID_Ex(devInfo.DevInst, devID, 200, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS) {
            devID[0] = TEXT('\0');
        }

        hwIds = GetDeviceHwId(devs, &devInfo);

        if( hwIds != NULL) {

            if( _wcsnicmp( hwIds, HardwareId, wcslen( HardwareId)) == 0) {
                match = TRUE;
            }

            free(hwIds);
        }

        if( match) {

            rc = ChangeDeviceState( devs, 
                                    &devInfo,
                                    DICS_ENABLE);

            if (rc != ERROR_SUCCESS) {
                printf("%s ChangeDeviceState() failed Error %d\n",
                                    __FUNCTION__,
                                    rc);
            }
        }
    }

exit:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }

    return rc;
}

int 
RemoveDevice( WCHAR *HardwareId)
{
    int rc = ERROR_SUCCESS;
    HDEVINFO devs = INVALID_HANDLE_VALUE;
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail;
    DWORD devIndex;
    SP_DEVINFO_DATA devInfo;
    WCHAR devID[200];
    LPWSTR hwIds = NULL;
    BOOLEAN match = FALSE;

    devs = SetupDiGetClassDevsEx(NULL,
                                 NULL,
                                 NULL,
                                 DIGCF_ALLCLASSES | DIGCF_PRESENT,
                                 NULL,
                                 NULL,
                                 NULL);
    if(devs == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail)) {
        goto exit;
    }

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0;SetupDiEnumDeviceInfo(devs,devIndex,&devInfo);devIndex++) {

        match = FALSE;

        if(CM_Get_Device_ID_Ex(devInfo.DevInst, devID, 200, 0, devInfoListDetail.RemoteMachineHandle) != CR_SUCCESS) {
            devID[0] = TEXT('\0');
        }

        hwIds = GetDeviceHwId(devs, &devInfo);

        if( hwIds != NULL) {

            if( _wcsnicmp( hwIds, HardwareId, wcslen( HardwareId)) == 0) {
                match = TRUE;
            }

            free(hwIds);
        }

        if( match) {

            rc = RemoveDeviceState( devs, 
                                    &devInfo);

            if (rc != ERROR_SUCCESS) {
                printf("%s RemoveDeviceState() failed Error %d\n",
                                    __FUNCTION__,
                                    rc);
            }
        }
    }

exit:
    if(devs != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(devs);
    }

    return rc;
}
int 
ChangeDeviceState( HDEVINFO Devs, 
                   PSP_DEVINFO_DATA DevInfo, 
                   DWORD State)
{
    int rc = ERROR_SUCCESS;
    SP_PROPCHANGE_PARAMS pcp;

    switch(State) 
    {
        case DICS_ENABLE:
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.StateChange = State;
            pcp.Scope = DICS_FLAG_GLOBAL;
            pcp.HwProfile = 0;
            if(SetupDiSetClassInstallParams(Devs,DevInfo,&pcp.ClassInstallHeader,sizeof(pcp))) {
               SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,Devs,DevInfo);
            }

        default:
            //
            // operate on config-specific profile
            //
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            pcp.StateChange = State;
            pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
            pcp.HwProfile = 0;
            break;
    }

    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&pcp.ClassInstallHeader,sizeof(pcp))) { 
        rc = GetLastError();
        printf("%s SetupDiSetClassInstallParams failed Error %d\n",
                            __FUNCTION__,
                            rc);
        goto exit;  
    }

    if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,Devs,DevInfo)) {
        rc = GetLastError();
        printf("%s SetupDiSetClassInstallParams failed Error %d\n",
                            __FUNCTION__,
                            rc);
        goto exit; 
    }

exit:

    return rc;
}

LPWSTR
GetDeviceHwId( HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo)
{

    int rc = ERROR_SUCCESS;
    LPWSTR buffer;
    DWORD size;
    DWORD reqSize;
    DWORD dataType;

    size = 8192; // initial guess, nothing magic about this
    buffer = (LPWSTR)malloc( size);
    if(!buffer) {
        return NULL;
    }
    while(!SetupDiGetDeviceRegistryProperty( Devs, DevInfo, SPDRP_HARDWAREID, &dataType, (LPBYTE)buffer, size, &reqSize)) {
        rc = GetLastError();
        if(rc != ERROR_INSUFFICIENT_BUFFER) {
            free(buffer);
            buffer = NULL;
            goto failed;
        }
        if(dataType != REG_MULTI_SZ) {
            goto failed;
        }
        size = reqSize;
        free( buffer);
        buffer = (LPWSTR)malloc( size);
        if(!buffer) {
            goto failed;
        }
    }

    return buffer;

failed:

    if(buffer) {
        free( buffer);
    }
    return NULL;
}

int 
RemoveDeviceState(HDEVINFO Devs, PSP_DEVINFO_DATA DevInfo)
{
    int rc = ERROR_SUCCESS;
    SP_REMOVEDEVICE_PARAMS rmdParams;

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&rmdParams.ClassInstallHeader,sizeof(rmdParams))) {
        rc = GetLastError();
        printf("%s SetupDiSetClassInstallParams failed Error %d\n",
                            __FUNCTION__,
                            rc);
        goto exit; 
    }

    if(!SetupDiCallClassInstaller(DIF_REMOVE,Devs,DevInfo)) {
        rc = GetLastError();
        printf("%s SetupDiCallClassInstaller failed Error %d\n",
                            __FUNCTION__,
                            rc);
        goto exit; 

    }

exit:

    return rc;
}

int 
UpdateDriver( WCHAR *InfFile, WCHAR *HardwareId, BOOL *reboot)
{
    int rc = ERROR_SUCCESS;
    DWORD flags = 0;
    DWORD res;
    TCHAR InfPath[MAX_PATH];
    BOOL reboot_request = FALSE;

    //
    // Inf must be a full pathname
    //
    res = GetFullPathName( InfFile, MAX_PATH, InfPath, NULL);
    if((res >= MAX_PATH) || (res == 0)) {
        rc = GetLastError();
        printf("Failed to get full path for %S Error %d\n",
                            InfFile,
                            rc);
        goto exit;
    }
    // Check if the file exists
    if(GetFileAttributes( InfPath)==(DWORD)(-1)) {
        rc = GetLastError();
        printf("Invalid Inf file %S Error %d\n",
                            InfFile,
                            rc);
        goto exit;
    }

    //flags |= INSTALLFLAG_FORCE;

    if(!UpdateDriverForPlugAndPlayDevices( NULL, HardwareId, InfPath, flags, &reboot_request)) {
        rc = GetLastError();
        printf("Failed UpdateDriverForPlugAndPlayDevices() HwId %S Inf %S Error %d\n",
                                        HardwareId,
                                        InfPath,
                                        rc);
        goto exit;
    }

    if (reboot != NULL) {
        *reboot = reboot_request;
    }

exit:

    return rc;
}