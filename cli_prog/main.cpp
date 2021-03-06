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

#include <iostream>
#include <clrutils.hpp>

int main(int argc, char** argv) {
    ClrUtils::init();

    bool clrLoaded = ClrUtils::isCLrLoaded();
    if(clrLoaded) {
        auto& rt = ClrUtils::getRuntimes().front();
        auto id = rt.getCurrentAppDomainId();

        std::wcout << L"-- First .NET Runtime is version " << rt.getVersionString().c_str() << L" --" << std::endl;
        std::cout << "-- Executing on AppDomain " << id << " --" << std::endl;
        rt.executeInAppDomain(id, [](void*) { std::cout << "Hello World from a .NET AppDomain!" << std::endl; }, nullptr);
    }
    else {
        std::cout << "-- No .NET runtime loaded --" << std::endl;
        std::cout << "Hello world from native domain!" << std::endl;
    }

    ClrUtils::uninit();

    return 0;
}
