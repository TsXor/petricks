#include <cstdint>
#include <iostream>
#include "petricks/rt-basics.hpp"

namespace dll = pe::runtime::dll;
using pe::runtime::winbool;
using pe::runtime::handle;

extern "C" __declspec(dllexport) void say_hello(const char* x) {
    std::cout << "Hello, " << x << "!" << '\n';
}

winbool __stdcall DllMain(handle hinstDLL, uint32_t fdwReason, void* lpvReserved) {
    switch (fdwReason) {
        case dll::process_attach:
            std::cout << "process_attach" << '\n';
            break;
        case dll::process_detach:
            std::cout << "process_detach" << '\n';
            break;
        case dll::thread_attach:
            std::cout << "thread_attach" << '\n';
            break;
        case dll::thread_detach:
            std::cout << "thread_detach" << '\n';
            break;
	}
	return true;
}
