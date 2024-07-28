# petricks
`petricks` is a *header-only* C++ library for manipulating PE files. Basically C++11 compliant.

## Contents
- Headers that helps interpret PE structure & some windows internal buffers with some handy inline functions and operator overloading, which **does not pollute your global namespace with macros and capitalized typedefs**.
- Implementation for:
    - getting base address of a loaded module, i.e. `GetModuleHandle`
    - finding address of exported functions in a loaded module (forwarders supported), i.e. `GetProcAddress`
    - loading a module from memory

## Features
- Zero dependency on `windows.h`!
- A "no static import" mode, where this library produces no import table entries.

## TODO
- This is not tested, written for learning purpose.
- Module name must be all ASCII chars.
- `pe::runtime::reflect::get_module_base` can only find **already loaded** modules from its **base name**.
- `pe::runtime::loader::memory_module::open` skips ISA-specific relocations. (which is fine on x86, for they have none)
- `pe::runtime::loader::memory_module::open` requires all imports to be findable through `LoadLibraryA`, i.e. the in-memory module cannot depend on other in-memory modules.
- `pe::runtime::loader::memory_module::open` does not utilize bound imports.

## See Also
- ["PE Format" on Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format)
- 0xRick's "A dive into the PE file format": [1](https://0xrick.github.io/win-internals/pe2/), [2](https://0xrick.github.io/win-internals/pe3/), [3](https://0xrick.github.io/win-internals/pe4/), [4](https://0xrick.github.io/win-internals/pe5/), [5](https://0xrick.github.io/win-internals/pe6/), [6](https://0xrick.github.io/win-internals/pe7/)
- [Daax's Custom GetProcAddress and GetModuleHandle Implementation](https://revers.engineering/custom-getprocaddress-and-getmodulehandle-implementation-x64/)
- [bokernb's "PEB and LDR chain"](https://www.cnblogs.com/bokernb/p/6404795.html)
- [`ntpebteb.h` in phnt](https://github.com/winsiderss/phnt/blob/master/ntpebteb.h)
- "An In-Depth Look into the Win32 Portable Executable File Format" on MSDN Magazine: [1](https://learn.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail), [2](https://learn.microsoft.com/en-us/archive/msdn-magazine/2002/march/inside-windows-an-in-depth-look-into-the-win32-portable-executable-file-format-part-2)
- [Hughes's "PE loading process"](https://hughlhz.github.io/2020/05/12/pe_load/)
