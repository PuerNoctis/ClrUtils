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

#ifndef CLRUTILS_HPP
#define CLRUTILS_HPP

/* Symbol handling */
#ifdef CLRUTILS_SYMEXPORT
#   define CLRUTILS_SYMPUBLIC __declspec(dllexport)
#   define CLRUTILS_SYMPRIVATE
#else
#   define CLRUTILS_SYMPUBLIC __declspec(dllimport)
#   define CLRUTILS_SYMPRIVATEö
#endif

/* Includes */
#include <stdint.h>
#include <vector>

/* ClrUtils */
namespace ClrUtils {

typedef void(*AppDomainSpecificProc)(void* args);

class CLRUTILS_SYMPUBLIC ClrRuntime final {
    private:
        struct ClrRuntimeImpl* _impl;

    public:
        ClrRuntime& operator = (const ClrRuntime& rhs) = delete;
        ClrRuntime& operator = (ClrRuntime&& rhs);

        uint32_t getCurrentAppDomainId(void) const;
        long executeInAppDomain(uint32_t appDomainId, AppDomainSpecificProc proc, void* args) const;

        ClrRuntime(void) = delete;
        ClrRuntime(void** runtimeInfo);
        ClrRuntime(const ClrRuntime& rhs) = delete;
        ClrRuntime(ClrRuntime&& rhs);
        ~ClrRuntime(void);
};

CLRUTILS_SYMPUBLIC extern bool init(void);
CLRUTILS_SYMPUBLIC extern void uninit(void);
CLRUTILS_SYMPUBLIC extern bool isCLrLoaded(void);
CLRUTILS_SYMPUBLIC extern const std::vector<ClrRuntime>& getRuntimes(void);

}

#endif
