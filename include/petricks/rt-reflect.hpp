#pragma once
#ifndef __PETRICKS_RT_REFLECT__
#define __PETRICKS_RT_REFLECT__

#include <intrin.h>
#include <tuple>
#include <algorithm>
#include "./rt-pebteb.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#error This file needs win32/win64 environment!
#endif

namespace pe {
namespace runtime {
namespace reflect {

template <typename CharT1, typename CharT2>
bool dll_name_cmp(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2) {
    string_view suffix(".dll");
    if (windows_style_cmp<CharT1, char>(s1.substr(s1.size() - suffix.size()), suffix)) { s1 = s1.substr(0, s1.size() - suffix.size()); }
    if (windows_style_cmp<CharT2, char>(s2.substr(s2.size() - suffix.size()), suffix)) { s2 = s2.substr(0, s2.size() - suffix.size()); }
    return windows_style_cmp<CharT1, CharT2>(s1, s2);
}

static inline teb* get_current_teb() {
    return reinterpret_cast<teb*>(
#ifdef _WIN64
        __readgsqword
#else
        __readfsdword
#endif
            (offsetof(nt_tib, Self))
    );
}

template <typename PredT>
static inline ldr_data_table_entry* find_module(PredT pred) {
    auto ldr = get_current_teb()->ProcessEnvironmentBlock->Ldr;
    for (auto& mod : ldr->modules(ldr_order::load)) {
        if (!mod.DllBase) { break; }
        if (pred(mod)) { return &mod; }
    }
    return nullptr;
}

template <typename CharT>
static inline void* get_module_base(basic_string_view<CharT> name) {
    auto mod = find_module([&](ldr_data_table_entry& mod) { return dll_name_cmp<wchar_t, CharT>(mod.BaseDllName, name); });
    return mod == nullptr ? nullptr : mod->DllBase;
}

static inline std::pair<bool, u32> find_module_export(void* mod_base, const char* name) {
    auto& opthdr = reinterpret_cast<image::dos_header*>(mod_base)->nthdr().OptionalHeader.local;
    auto& export_pos = opthdr.datadir(image::directory_entry::export_);
    if (export_pos.Size == 0) { return {false, 0}; }
    auto& export_dir = ref_at<image::export_directory>(mod_base, export_pos.VirtualAddress);

    u16 proc_idx;
    if (reinterpret_cast<size_t>(name) >> 16) { // by name
        auto export_name_addrs = ptr_at<u32>(mod_base, export_dir.AddressOfNames);
        auto export_name_ordinals = ptr_at<u16>(mod_base, export_dir.AddressOfNameOrdinals);
        // https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-name-pointer-table
        // Export name table is lexically ordered to allow binary searches.
        auto name_pos = std::lower_bound(export_name_addrs, export_name_addrs + export_dir.NumberOfNames, name,
            [&](const u32& export_name_addr, const char* name) {
                // strcmp here is from intrin.h
                return strcmp(ptr_at<char>(mod_base, export_name_addr), name) < 0;
            }
        );
        if (strcmp(ptr_at<char>(mod_base, *name_pos), name) != 0) { return {false, 0}; }
        proc_idx = export_name_ordinals[name_pos - export_name_addrs];
    } else { // by ordinal
        u16 ordinal = reinterpret_cast<size_t>(name) & 0xFFFF;
        proc_idx = ordinal - export_dir.Base;
    }
    
    auto export_proc_addrs = ptr_at<u32>(mod_base, export_dir.AddressOfFunctions);
    auto export_rva = export_proc_addrs[proc_idx];
    auto is_forward = export_rva >= export_pos.VirtualAddress && export_rva < export_pos.VirtualAddress + export_pos.Size;
    return {is_forward, export_rva};
}

static inline void* get_proc_addr(void* mod_base, const char* name) {
    auto export_pos = find_module_export(mod_base, name);
    if (!export_pos.first) { return export_pos.second == 0 ? nullptr : ptr_at<void>(mod_base, export_pos.second); }
    // this is a forwarder, find recursively
    auto forwarder_string = ptr_at<char>(mod_base, export_pos.second);
    for (size_t dot_pos = 0; forwarder_string[dot_pos] != 0; ++dot_pos) {
        if (forwarder_string[dot_pos] == '.') {
            auto forward_mod_base = get_module_base(string_view(forwarder_string, dot_pos));
            if (forward_mod_base == nullptr) { return nullptr; }
            auto forward_name = forwarder_string + dot_pos + 1;
            if (forward_name[0] == '#') { forward_name = reinterpret_cast<char*>(number_from_string(forward_name + 1)); }
            return get_proc_addr(forward_mod_base, forward_name);
        }
    }
    return nullptr;
}

} // namespace reflect
} // namespace runtime
} // namespace pe

#endif // __PETRICKS_RT_REFLECT__
