/*
The MIT License (MIT)

Copyright (c) 2015 Daniel Schick

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "clrutils.hpp"

#include <Windows.h>
#include <metahost.h>
#include <atlbase.h>

typedef HRESULT(__stdcall *CLRCreateInstancePtr)(REFCLSID clsid, REFIID riid, /*iid_is(riid)*/ LPVOID *ppInterface);
typedef CComPtr<ICLRRuntimeInfo> ClrRuntimeInfoPtr;
typedef CComPtr<ICLRRuntimeHost> ClrRuntimeHostPtr;

namespace ClrUtils {

static CComPtr<ICLRMetaHost> _clrMetaHost = nullptr;
static HMODULE _hMscoreeLib = nullptr;
static bool _isInit = false;
static std::vector<ClrRuntime> _runtimes;

struct AppDomainSpecificProcData {
    AppDomainSpecificProc _proc;
    void* _args;

    AppDomainSpecificProcData(AppDomainSpecificProc proc, void* args) :
        _proc(proc),
        _args(args){}
};

struct ClrRuntimeImpl {
    ClrRuntimeInfoPtr _runtimeInfo;
    ClrRuntimeHostPtr _runtimeHost;

    ClrRuntimeImpl(ClrRuntimeInfoPtr& runtimeInfo) :
        _runtimeInfo(runtimeInfo) {}

    ~ClrRuntimeImpl(void) {}
};

ClrRuntime::ClrRuntime(void** runtimeInfo) :
    _impl(new ClrRuntimeImpl(*reinterpret_cast<ClrRuntimeInfoPtr*>(runtimeInfo))) {
    (void)_impl->_runtimeInfo->GetInterface(
        CLSID_CLRRuntimeHost,
        IID_ICLRRuntimeHost,
        reinterpret_cast<void**>(&_impl->_runtimeHost));
}

ClrRuntime::ClrRuntime(ClrRuntime&& rhs) {
    (void)operator=(std::forward<ClrRuntime>(rhs));
}

ClrRuntime::~ClrRuntime(void) {
    delete _impl;
}

ClrRuntime& ClrRuntime::operator = (ClrRuntime&& rhs) {
    if(this != &rhs) {
        _impl = rhs._impl;
        rhs._impl = nullptr;
    }

    return *this;
}

static HRESULT __stdcall appDomainSpecificProcEntry(void* data) {
    auto realData = reinterpret_cast<AppDomainSpecificProcData*>(data);
    realData->_proc(realData->_args);
    return 0;
}

std::wstring ClrRuntime::getVersionString(void) const {
    DWORD verLen = MAX_PATH;
    wchar_t ver[MAX_PATH];
    std::memset(ver, 0, verLen * sizeof(wchar_t));
    _impl->_runtimeInfo->GetVersionString(ver, &verLen);
    return std::wstring(ver);
}

uint32_t ClrRuntime::getCurrentAppDomainId(void) const {
    DWORD appDomainId;
    (void)_impl->_runtimeHost->GetCurrentAppDomainId(&appDomainId);
    return appDomainId;
}

long ClrRuntime::executeInAppDomain(uint32_t appDomainId, AppDomainSpecificProc proc, void* args) const {
    AppDomainSpecificProcData data(proc, args);
    return _impl->_runtimeHost->ExecuteInAppDomain(appDomainId, appDomainSpecificProcEntry, &data);
}

bool init(void) {
    if(_isInit) {
        return false;
    }

    _hMscoreeLib = LoadLibraryA("mscoree.dll");
    if(nullptr == _hMscoreeLib) {
        uninit();
        return false;
    }

    CLRCreateInstancePtr CLRCreateInstanceFn = reinterpret_cast<CLRCreateInstancePtr>(GetProcAddress(_hMscoreeLib, "CLRCreateInstance"));
    if(nullptr == CLRCreateInstanceFn) {
        uninit();
        return false;
    }

    _runtimes.clear();
    
    HRESULT hr;
    hr = CLRCreateInstanceFn(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void**>(&_clrMetaHost));
    if(SUCCEEDED(hr)) {
        CComPtr<IEnumUnknown> pEnumUnk;
        hr = _clrMetaHost->EnumerateLoadedRuntimes(::GetCurrentProcess(), &pEnumUnk);

        if(SUCCEEDED(hr)) {
            CComPtr<IUnknown> pUnk;
            ICLRRuntimeInfo* pRnt;
            ULONG fetch = 0;
            while(true) {
                hr = pEnumUnk->Next(1, &pUnk, &fetch);
                if((fetch == 1) && SUCCEEDED(hr)) {
                    hr = pUnk->QueryInterface<ICLRRuntimeInfo>(&pRnt);
                    if(SUCCEEDED(hr)) {
                         _runtimes.push_back(std::move(ClrRuntime(reinterpret_cast<void**>(&pRnt))));
                        pRnt->Release();
                    }

                    pUnk.Release();
                } else {
                    break;
                }
            }
        }
    } else {
        _clrMetaHost = nullptr;
    }

    _isInit = true;
    return true;
}

void uninit(void) {
    _runtimes.clear();

    if(nullptr != _clrMetaHost) {
        _clrMetaHost.Release();
    }

    _clrMetaHost = nullptr;

    FreeLibrary(_hMscoreeLib);
    _hMscoreeLib = nullptr;

    _isInit = false;
}

bool isCLrLoaded(void) {
    return _runtimes.size() > 0;
}

const std::vector<ClrRuntime>& getRuntimes(void) {
    return _runtimes;
}

}
