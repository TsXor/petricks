#pragma once
#ifndef __PETRICKS_RT_BASICS__
#define __PETRICKS_RT_BASICS__

#include "./basics.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#error This file needs win32/win64 environment!
#endif

namespace pe::runtime {

using handle = void *;
using winbool = i32;
using winproc = size_t (__stdcall *)();

using TyGetProcAddress = winproc __stdcall (handle hModule, const char* lpProcName);
using TyGetModuleHandleA = handle __stdcall (const char* lpModuleName);
using TyGetModuleHandleW = handle __stdcall (const wchar_t* lpModuleName);
using TyLoadLibraryA = handle __stdcall (char* lpLibFileName);
using TyLoadLibraryW = handle __stdcall (wchar_t* lpLibFileName);
using TyFreeLibrary = winbool __stdcall (handle hLibModule);

namespace dll {
    constexpr u32 process_attach = 1;
    constexpr u32 thread_attach = 2;
    constexpr u32 thread_detach = 3;
    constexpr u32 process_detach = 0;
    constexpr u32 process_verifier = 4;
} // namespace dll

using TyDllMain = winbool __stdcall (handle hinstDLL, u32 fdwReason, void* lpvReserved);

struct memory_basic_information {
    void* BaseAddress;
    void* AllocationBase;
    u32 AllocationProtect;
#if defined (_WIN64)
    u16 PartitionId;
#endif
    size_t RegionSize;
    u32 State;
    u32 Protect;
    u32 Type;
}; // struct memory_basic_information

namespace mem {
    constexpr u32 commit = 0x1000;
    constexpr u32 reserve = 0x2000;
    constexpr u32 replace_placeholder = 0x4000;
    constexpr u32 decommit = 0x4000;
    constexpr u32 release = 0x8000;
    constexpr u32 free = 0x10000;
    constexpr u32 private_ = 0x20000;
    constexpr u32 reserve_placeholder = 0x40000;
    constexpr u32 mapped = 0x40000;
    constexpr u32 reset = 0x80000;
    constexpr u32 top_down = 0x100000;
    constexpr u32 write_watch = 0x200000;
    constexpr u32 physical = 0x400000;
    constexpr u32 rotate = 0x800000;
    constexpr u32 different_image_base_ok = 0x800000;
    constexpr u32 reset_undo = 0x1000000;
    constexpr u32 large_pages = 0x20000000;
    constexpr u32 pages_4mb = 0x80000000;
    constexpr u32 pages_64k = large_pages | physical;
    constexpr u32 unmap_with_transient_boost = 0x00000001;
    constexpr u32 coalesce_placeholders = 0x00000001;
    constexpr u32 preserve_placeholder = 0x00000002;
} // namespace mem

namespace page {
    constexpr u32 noaccess = 0x01;
    constexpr u32 readonly = 0x02;
    constexpr u32 readwrite = 0x04;
    constexpr u32 writecopy = 0x08;
    constexpr u32 execute = 0x10;
    constexpr u32 execute_read = 0x20;
    constexpr u32 execute_readwrite = 0x40;
    constexpr u32 execute_writecopy = 0x80;
    constexpr u32 guard = 0x100;
    constexpr u32 nocache = 0x200;
    constexpr u32 writecombine = 0x400;
    constexpr u32 graphics_noaccess = 0x0800;
    constexpr u32 graphics_readonly = 0x1000;
    constexpr u32 graphics_readwrite = 0x2000;
    constexpr u32 graphics_execute = 0x4000;
    constexpr u32 graphics_execute_read = 0x8000;
    constexpr u32 graphics_execute_readwrite = 0x10000;
    constexpr u32 graphics_coherent = 0x20000;
    constexpr u32 graphics_nocache = 0x40000;
    constexpr u32 enclave_thread_control = 0x80000000;
    constexpr u32 revert_to_file_map = 0x80000000;
    constexpr u32 targets_no_update = 0x40000000;
    constexpr u32 targets_invalid = 0x40000000;
    constexpr u32 enclave_unvalidated = 0x20000000;
    constexpr u32 enclave_mask = 0x10000000;
    constexpr u32 enclave_decommit = enclave_mask | 0;
    constexpr u32 enclave_ss_first = enclave_mask | 1;
    constexpr u32 enclave_ss_rest = enclave_mask | 2;
} // namespace page

using TyVirtualAlloc = void* __stdcall (void* lpAddress, size_t dwSize, u32 flAllocationType, u32 flProtect);
using TyVirtualFree = winbool __stdcall (void* lpAddress, size_t dwSize, u32 dwFreeType);
using TyVirtualQuery = size_t __stdcall (const void* lpAddress, memory_basic_information* lpBuffer, size_t dwLength);
using TyVirtualProtect = winbool __stdcall (void* lpAddress, size_t dwSize, u32 flNewProtect, u32* lpflOldProtect);

struct list_entry {
    list_entry *Flink;
    list_entry *Blink;
}; // struct list_entry

template <typename T, list_entry T::*node_rel>
class list_view {
    list_entry *_head;
public:
    using value_type = T;
    
    list_view(list_entry *head) : _head(head) {}

    class iterator {
        list_entry *_pos;
        value_type *_real() const { return reinterpret_cast<value_type *>(_pos); }
    public:
        iterator(list_entry *pos) : _pos(pos) {}
        value_type &operator*() const { return *_real(); }
        bool operator==(iterator other) const { return _pos == other._pos; }
        bool operator!=(iterator other) const { return !(*this == other); }
        iterator &operator++() {
            _pos = _real()->*node_rel.Flink;
            return *this;
        }
        iterator &operator--() {
            _pos = _real()->*node_rel.Blink;
            return *this;
        }
    };
    
    iterator begin() { return {_head->Flink}; }
    iterator end() { return {_head}; }
};

struct unicode_string {
    u16 Length;
    u16 MaximumLength;
    wchar_t *Buffer;

    wchar_t& operator[](size_t idx) { return Buffer[idx]; }
    const wchar_t& operator[](size_t idx) const { return Buffer[idx]; }
    operator wstring_view() const { return {Buffer, Length}; }
    
    bool operator==(const unicode_string& other) const {
        return static_cast<wstring_view>(*this) == static_cast<wstring_view>(other);
    }
    bool operator==(const wstring_view& other) const {
        return static_cast<wstring_view>(*this) == other;
    }
    bool operator==(const wchar_t* other) const {
        return static_cast<wstring_view>(*this) == other;
    }
}; // struct unicode_string

} // namespace pe::runtime

#endif // __PETRICKS_RT_BASICS__
