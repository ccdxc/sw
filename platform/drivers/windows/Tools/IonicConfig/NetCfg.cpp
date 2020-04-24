#include "NetCfg.h"

#define LOCK_TIME_OUT     5000
// Function:  ReleaseRef
// Purpose:   Release reference.
// Arguments:
//    punk     [in]  IUnknown reference to release.
// Returns:   Reference count.
// Notes:
VOID ReleaseRef(IN IUnknown* punk) {
    if (punk) {
        punk->Release();
    }
    return;
}

// Function:  HrGetINetCfg
// Purpose:   Get a reference to INetCfg.
// Arguments:
//    fGetWriteLock  [in]  If TRUE, Write lock.requested.
//    lpszAppName    [in]  Application name requesting the reference.
//    ppnc           [out] Reference to INetCfg.
//    lpszLockedBy   [in]  Optional. Application who holds the write lock.
// Returns:   S_OK on success, otherwise an error code.
HRESULT HrGetINetCfg(IN  BOOL fGetWriteLock,
                     IN  LPCWSTR lpszAppName,
                     OUT INetCfg** ppnc,
                     OUT LPWSTR* lpszLockedBy) {
    INetCfg* pnc = NULL;
    INetCfgLock* pncLock = NULL;
    HRESULT      hr = S_OK;

    // Initialize the output parameters.
    *ppnc = NULL;
    if (lpszLockedBy) {
        *lpszLockedBy = NULL;
    }
    // Initialize COM
    hr = CoInitialize(NULL);
    if (hr == S_OK) {
        // Create the object implementing INetCfg.
        hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (void**)&pnc);
        if (hr == S_OK) {
            if (fGetWriteLock) {
                // Get the locking reference
                hr = pnc->QueryInterface(IID_INetCfgLock, (LPVOID*)&pncLock);
                if (hr == S_OK) {
                    // Attempt to lock the INetCfg for read/write
                    hr = pncLock->AcquireWriteLock(LOCK_TIME_OUT, lpszAppName, lpszLockedBy);
                    if (hr == S_FALSE) {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }
            if (hr == S_OK) {
                // Initialize the INetCfg object.
                hr = pnc->Initialize(NULL);

                if (hr == S_OK) {
                    *ppnc = pnc;
                    pnc->AddRef();
                }
                else {
                    // Initialize failed, if obtained lock, release it
                    if (pncLock) {
                        pncLock->ReleaseWriteLock();
                    }
                }
            }
            ReleaseRef(pncLock);
            ReleaseRef(pnc);
        }
        // In case of error, uninitialize COM.
        if (hr != S_OK) {
            CoUninitialize();
        }
    }
    return hr;
}

// Function:  HrReleaseINetCfg
// Purpose:   Get a reference to INetCfg.
// Arguments:
//    pnc           [in] Reference to INetCfg to release.
//    fHasWriteLock [in] If TRUE, reference was held with write lock.
// Returns:   S_OK on success, otherwise an error code.
HRESULT HrReleaseINetCfg(IN INetCfg* pnc,
                         IN BOOL fHasWriteLock) {
                         INetCfgLock* pncLock = NULL;
                         HRESULT        hr = S_OK;
    // Uninitialize INetCfg
    hr = pnc->Uninitialize();
    // If write lock is present, unlock it
    if (hr == S_OK && fHasWriteLock) {
        // Get the locking reference
        hr = pnc->QueryInterface(IID_INetCfgLock, (LPVOID*)&pncLock);
        if (hr == S_OK) {
            hr = pncLock->ReleaseWriteLock();
            ReleaseRef(pncLock);
        }
    }
    ReleaseRef(pnc);
    // Uninitialize COM.
    CoUninitialize();

    return hr;
}

// Function:  HrGetComponentEnum
// Purpose:   Get network component enumerator reference.
// Arguments:
//    pnc         [in]  Reference to INetCfg.
//    pguidClass  [in]  Class GUID of the network component.
//    ppencc      [out] Enumerator reference.
// Returns:   S_OK on success, otherwise an error code.
HRESULT HrGetComponentEnum(INetCfg* pnc,
                           IN const GUID* pguidClass,
                           OUT IEnumNetCfgComponent** ppencc) {
                           INetCfgClass* pncclass;
                           HRESULT       hr;
    *ppencc = NULL;
    // Get the class reference.
    hr = pnc->QueryNetCfgClass(pguidClass, IID_INetCfgClass, (PVOID*)&pncclass);
    if (hr == S_OK) {
        // Get the enumerator reference.
        hr = pncclass->EnumComponents(ppencc);
        // We don't need the class reference any more.
        ReleaseRef(pncclass);
    }

    return hr;
}

// Function:  HrGetFirstComponent
// Purpose:   Enumerates the first network component.
// Arguments:
//    pencc      [in]  Component enumerator reference.
//    ppncc      [out] Network component reference.
// Returns:   S_OK on success, otherwise an error code.
HRESULT HrGetFirstComponent(IN  IEnumNetCfgComponent* pencc,
                            OUT INetCfgComponent** ppncc) {
    HRESULT  hr;
    ULONG    ulCount;

    *ppncc = NULL;

    pencc->Reset();

    hr = pencc->Next(1, ppncc, &ulCount);

    return hr;
}

// Function:  HrGetNextComponent
// Purpose:   Enumerate the next network component.
// Arguments:
//    pencc      [in]  Component enumerator reference.
//    ppncc      [out] Network component reference.
// Returns:   S_OK on success, otherwise an error code.
// Notes:     The function behaves just like HrGetFirstComponent if
//            it is called right after HrGetComponentEnum.
HRESULT HrGetNextComponent(IN  IEnumNetCfgComponent* pencc,
                           OUT INetCfgComponent** ppncc) {
    HRESULT  hr;
    ULONG    ulCount;

    *ppncc = NULL;

    hr = pencc->Next(1, ppncc, &ulCount);
    
    return hr;
}

#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, REFGUID guid) {

    os << std::uppercase;
    os.width(8);
    os << std::hex << std::setfill('0') << guid.Data1 << '-';

    os.width(4);
    os << std::hex << std::setfill('0') << guid.Data2 << '-';

    os.width(4);
    os << std::hex << std::setfill('0') << guid.Data3 << '-';

    os.width(2);
    os << std::hex << std::setfill('0')
        << static_cast<short>(guid.Data4[0])
        << static_cast<short>(guid.Data4[1])
        << '-'
        << static_cast<short>(guid.Data4[2])
        << static_cast<short>(guid.Data4[3])
        << static_cast<short>(guid.Data4[4])
        << static_cast<short>(guid.Data4[5])
        << static_cast<short>(guid.Data4[6])
        << static_cast<short>(guid.Data4[7]);
    os << std::nouppercase;
    return os;
}
void DisplayInterface(PCWSTR strName) {
    INetCfg* pnetcfg = NULL;
    INetCfgComponent* pncfgcomp = NULL;
    IEnumNetCfgComponent* pencc;
    HRESULT hr = S_OK;
    LPWSTR strDevDesc = NULL;
    GUID InstanceGuid;
    DWORD Status = 0;
    bool bIonicComponent = false;
    
    std::ios  state(NULL);
    state.copyfmt(std::cout);

    hr = HrGetINetCfg(FALSE, L"IonicConfig", &pnetcfg, NULL);

    if (hr == S_OK) {

        hr = HrGetComponentEnum(pnetcfg, &GUID_DEVCLASS_NET, &pencc);
        if (hr == S_OK) {
            hr = HrGetFirstComponent(pencc, &pncfgcomp);
            while (hr == S_OK) {
                bIonicComponent = false;
                //std::cout << std::endl;
                hr = pncfgcomp->GetDisplayName(&strDevDesc);
                if (S_OK == hr) {
                    if (wcscmp(strDevDesc, strName) == 0) {
                        //std::wcout << L"Device desc: " << strDevDesc << std::endl;
                        bIonicComponent = true;
                    }
                    CoTaskMemFree(strDevDesc);
                    strDevDesc = NULL;
                }
                if (bIonicComponent) {
                    hr = pncfgcomp->GetInstanceGuid(&InstanceGuid);
                    if (S_OK == hr) {
                        std::cout << "\tNetCfgID:\t" << InstanceGuid << std::endl;
                    }
                    hr = pncfgcomp->GetPnpDevNodeId(&strDevDesc);
                    if (hr == S_OK) {
                        std::wcout << L"\tDev Node Id:\t" << strDevDesc << std::endl;
                        CoTaskMemFree(strDevDesc);
                        strDevDesc = NULL;
                    }
                    hr = pncfgcomp->GetBindName(&strDevDesc);
                    if (hr == S_OK) {
                        std::wcout << L"\tBind Name:\t" << strDevDesc << std::endl;
                        CoTaskMemFree(strDevDesc);
                        strDevDesc = NULL;
                    }
                    hr = pncfgcomp->GetDeviceStatus(&Status);
                    if (hr == S_OK) {
                        std::cout << "\tDevice Status:\t0x" << std::hex << std::setfill('0') << std::setw(8) << std::right << Status << std::endl;
                        CoTaskMemFree(strDevDesc);
                        strDevDesc = NULL;
                    }
                }
                
                ReleaseRef(pncfgcomp);
                hr = HrGetNextComponent(pencc, &pncfgcomp);
            }

            ReleaseRef(pencc);
        }
        else {
            std::cout << "IEnumNetCfgComponent not found.  Err:" <<  hr;
        }

        hr = pnetcfg->Apply();

        HrReleaseINetCfg(pnetcfg, TRUE);
    }

    std::cout.copyfmt(state);
}

