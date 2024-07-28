#if __cplusplus >= 202002L
#define PETRICKS_ENABLE_CONCEPTS
#endif

#ifdef PETRICKS_ENABLE_CONCEPTS
#include <concepts>
#endif
#include "./rt-basics.hpp"
#include "./rt-reflect.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#error This file needs win32/win64 environment!
#endif

/**
 *  Usage of Win32 API in this project is like allocator in STL -- you can use the default one or write one yourself!
 */

namespace pe {
namespace runtime {


#ifdef PETRICKS_ENABLE_CONCEPTS
template <typename T>
concept winapi_provider =
        requires(T self, handle hModule, const char* lpProcName) { { self.GetProcAddress(hModule, lpProcName) } -> std::convertible_to<winproc>; }
    &&  requires(T self, const char* lpModuleName) { { self.GetModuleHandleA(lpModuleName) } -> std::convertible_to<handle>; }
    &&  requires(T self, const wchar_t* lpModuleName) { { self.GetModuleHandleW(lpModuleName) } -> std::convertible_to<handle>; }
    &&  requires(T self, const char* lpLibFileName) { { self.LoadLibraryA(lpLibFileName) } -> std::convertible_to<handle>; }
    &&  requires(T self, const wchar_t* lpLibFileName) { { self.LoadLibraryW(lpLibFileName) } -> std::convertible_to<handle>; }
    &&  requires(T self, handle hLibModule) { { self.FreeLibrary(hLibModule) } -> std::convertible_to<winbool>; }
    &&  requires(T self, void* lpAddress, size_t dwSize, u32 flAllocationType, u32 flProtect) { { self.VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect) } -> std::convertible_to<void*>; }
    &&  requires(T self, void* lpAddress, size_t dwSize, u32 dwFreeType) { { self.VirtualFree(lpAddress, dwSize, dwFreeType) } -> std::convertible_to<winbool>; }
    &&  requires(T self, const void* lpAddress, memory_basic_information* lpBuffer, size_t dwLength) { { self.VirtualQuery(lpAddress, lpBuffer, dwLength) } -> std::convertible_to<size_t>; }
    &&  requires(T self, void* lpAddress, size_t dwSize, u32 flNewProtect, u32* lpflOldProtect) { { self.VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect) } -> std::convertible_to<winbool>; };
#endif

struct winapi_dynamic {
    handle hKernel32;
    TyGetProcAddress* GetProcAddress = nullptr;
    TyGetModuleHandleA* GetModuleHandleA = nullptr;
    TyGetModuleHandleW* GetModuleHandleW = nullptr;
    TyLoadLibraryA* LoadLibraryA = nullptr;
    TyLoadLibraryW* LoadLibraryW = nullptr;
    TyFreeLibrary* FreeLibrary = nullptr;
    TyVirtualAlloc* VirtualAlloc = nullptr;
    TyVirtualFree* VirtualFree = nullptr;
    TyVirtualQuery* VirtualQuery = nullptr;
    TyVirtualProtect* VirtualProtect = nullptr;

    void load() {
        hKernel32 = reflect::get_module_base<wchar_t>(L"kernel32.dll");
        if (!hKernel32) { return; }
        // load them all!
        this->GetProcAddress = reinterpret_cast<TyGetProcAddress*>(reflect::get_proc_addr(hKernel32, "GetProcAddress"));
        this->GetModuleHandleA = reinterpret_cast<TyGetModuleHandleA*>(this->GetProcAddress(hKernel32, "GetModuleHandleA"));
        this->GetModuleHandleW = reinterpret_cast<TyGetModuleHandleW*>(this->GetProcAddress(hKernel32, "GetModuleHandleW"));
        this->LoadLibraryA = reinterpret_cast<TyLoadLibraryA*>(this->GetProcAddress(hKernel32, "LoadLibraryA"));
        this->LoadLibraryW = reinterpret_cast<TyLoadLibraryW*>(this->GetProcAddress(hKernel32, "LoadLibraryW"));
        this->FreeLibrary = reinterpret_cast<TyFreeLibrary*>(this->GetProcAddress(hKernel32, "FreeLibrary"));
        this->VirtualAlloc = reinterpret_cast<TyVirtualAlloc*>(this->GetProcAddress(hKernel32, "VirtualAlloc"));
        this->VirtualFree = reinterpret_cast<TyVirtualFree*>(this->GetProcAddress(hKernel32, "VirtualFree"));
        this->VirtualQuery = reinterpret_cast<TyVirtualQuery*>(this->GetProcAddress(hKernel32, "VirtualQuery"));
        this->VirtualProtect = reinterpret_cast<TyVirtualProtect*>(this->GetProcAddress(hKernel32, "VirtualProtect"));
    }

    operator bool() const {
        // check them all!
        return hKernel32
            && GetProcAddress
            && GetModuleHandleA
            && GetModuleHandleW
            && LoadLibraryA
            && LoadLibraryW
            && FreeLibrary
            && VirtualAlloc
            && VirtualFree
            && VirtualQuery
            && VirtualProtect;
    }
}; // struct winapi_dynamic


// ApiPtrT can be any valid pointer, including unique_ptr and shared_ptr
template <typename ApiPtrT = winapi_dynamic*>
#ifdef PETRICKS_ENABLE_CONCEPTS
    requires winapi_provider<decltype(*std::declval<ApiPtrT>())>
#endif
struct winapi_dynamic_ref {
    ApiPtrT ptr;
    // boilerplate forwarding
    winproc GetProcAddress(handle hModule, const char* lpProcName) { return ptr->GetProcAddress(hModule, lpProcName); }
    handle GetModuleHandleA(const char* lpModuleName) { return ptr->GetModuleHandleA(lpModuleName); }
    handle GetModuleHandleW(const wchar_t* lpModuleName) { return ptr->GetModuleHandleW(lpModuleName); }
    handle LoadLibraryA(const char* lpLibFileName) { return ptr->LoadLibraryA(lpLibFileName); }
    handle LoadLibraryW(const wchar_t* lpLibFileName) { return ptr->LoadLibraryW(lpLibFileName); }
    winbool FreeLibrary(handle hLibModule) { return ptr->FreeLibrary(hLibModule); }
    void* VirtualAlloc(void* lpAddress, size_t dwSize, u32 flAllocationType, u32 flProtect) { return ptr->VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect); }
    winbool VirtualFree(void* lpAddress, size_t dwSize, u32 dwFreeType) { return ptr->VirtualFree(lpAddress, dwSize, dwFreeType); }
    size_t VirtualQuery(const void* lpAddress, memory_basic_information* lpBuffer, size_t dwLength) { return ptr->VirtualQuery(lpAddress, lpBuffer, dwLength); }
    winbool VirtualProtect(void* lpAddress, size_t dwSize, u32 flNewProtect, u32* lpflOldProtect) { return ptr->VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect); }
}; // struct winapi_dynamic_ref


namespace winapi {

#ifndef PETRICKS_NO_STATIC_IMPORT

extern "C" {

__declspec(dllimport) TyGetProcAddress GetProcAddress;
__declspec(dllimport) TyGetModuleHandleA GetModuleHandleA;
__declspec(dllimport) TyGetModuleHandleW GetModuleHandleW;
__declspec(dllimport) TyLoadLibraryA LoadLibraryA;
__declspec(dllimport) TyLoadLibraryW LoadLibraryW;
__declspec(dllimport) TyFreeLibrary FreeLibrary;
__declspec(dllimport) TyVirtualAlloc VirtualAlloc;
__declspec(dllimport) TyVirtualFree VirtualFree;
__declspec(dllimport) TyVirtualQuery VirtualQuery;
__declspec(dllimport) TyVirtualProtect VirtualProtect;

} // extern "C"

#else

// magic statics
inline winapi_dynamic& __api() {
    static struct Singleton {
        winapi_dynamic data;
        Singleton() { data.load(); }
        ~Singleton() {}
    } api;
    return api.data;
}

// boilerplate forwarding
static inline winproc GetProcAddress(handle hModule, const char* lpProcName) { return __api().GetProcAddress(hModule, lpProcName); }
static inline handle GetModuleHandleA(const char* lpModuleName) { return __api().GetModuleHandleA(lpModuleName); }
static inline handle GetModuleHandleW(const wchar_t* lpModuleName) { return __api().GetModuleHandleW(lpModuleName); }
static inline handle LoadLibraryA(const char* lpLibFileName) { return __api().LoadLibraryA(lpLibFileName); }
static inline handle LoadLibraryW(const wchar_t* lpLibFileName) { return __api().LoadLibraryW(lpLibFileName); }
static inline winbool FreeLibrary(handle hLibModule) { return __api().FreeLibrary(hLibModule); }
static inline void* VirtualAlloc(void* lpAddress, size_t dwSize, u32 flAllocationType, u32 flProtect) { return __api().VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect); }
static inline winbool VirtualFree(void* lpAddress, size_t dwSize, u32 dwFreeType) { return __api().VirtualFree(lpAddress, dwSize, dwFreeType); }
static inline size_t VirtualQuery(const void* lpAddress, memory_basic_information* lpBuffer, size_t dwLength) { return __api().VirtualQuery(lpAddress, lpBuffer, dwLength); }
static inline winbool VirtualProtect(void* lpAddress, size_t dwSize, u32 flNewProtect, u32* lpflOldProtect) { return __api().VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect); }

#endif // PETRICKS_NO_STATIC_IMPORT

} // namespace winapi


struct winapi_default {
    // boilerplate forwarding
    static winproc GetProcAddress(handle hModule, const char* lpProcName) { return winapi::GetProcAddress(hModule, lpProcName); }
    static handle GetModuleHandleA(const char* lpModuleName) { return winapi::GetModuleHandleA(lpModuleName); }
    static handle GetModuleHandleW(const wchar_t* lpModuleName) { return winapi::GetModuleHandleW(lpModuleName); }
    static handle LoadLibraryA(char* lpLibFileName) { return winapi::LoadLibraryA(lpLibFileName); }
    static handle LoadLibraryW(wchar_t* lpLibFileName) { return winapi::LoadLibraryW(lpLibFileName); }
    static winbool FreeLibrary(handle hLibModule) { return winapi::FreeLibrary(hLibModule); }
    static void* VirtualAlloc(void* lpAddress, size_t dwSize, u32 flAllocationType, u32 flProtect) { return winapi::VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect); }
    static winbool VirtualFree(void* lpAddress, size_t dwSize, u32 dwFreeType) { return winapi::VirtualFree(lpAddress, dwSize, dwFreeType); }
    static size_t VirtualQuery(const void* lpAddress, memory_basic_information* lpBuffer, size_t dwLength) { return winapi::VirtualQuery(lpAddress, lpBuffer, dwLength); }
    static winbool VirtualProtect(void* lpAddress, size_t dwSize, u32 flNewProtect, u32* lpflOldProtect) { return winapi::VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect); }
}; // struct winapi_static


} // namespace runtime
} // namespace pe
