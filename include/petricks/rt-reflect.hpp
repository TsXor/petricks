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

namespace pe::runtime::reflect {

// compares two strings, ignoring case and assuming only ascii chars
template <typename CharT1, typename CharT2>
bool windows_style_cmp(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2) {
    if (s1.size() != s2.size()) { return false; }
    for (size_t i = 0; i < s1.size(); ++i) {
        auto ch1 = s1[i]; auto ch2 = s2[i];
        if (ch1 >= 'A' && ch1 <= 'Z') { ch1 += 'a' - 'A'; }
        if (ch2 >= 'A' && ch2 <= 'Z') { ch2 += 'a' - 'A'; }
        if (ch1 != ch2) { return false; }
    }
    return true;
}

template <typename CharT1, typename CharT2>
bool dll_name_cmp(basic_string_view<CharT1> s1, basic_string_view<CharT2> s2) {
    if (windows_style_cmp<CharT1, char>(s1.substr(s1.size() - 4), string_view(".dll"))) { s1 = s1.substr(0, s1.size() - 4); }
    if (windows_style_cmp<CharT2, char>(s2.substr(s2.size() - 4), string_view(".dll"))) { s2 = s2.substr(0, s2.size() - 4); }
    return windows_style_cmp<CharT1, CharT2>(s1, s2);
}

static inline size_t convert_decimal(const char* str) {
    size_t result = 0;
    for (auto pos = str; *pos != 0; ++pos) {
        result *= 10;
        result += (*pos  - '0');
    }
    return result;
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
            if (forward_name[0] == '#') { forward_name = reinterpret_cast<char*>(convert_decimal(forward_name + 1)); }
            return get_proc_addr(forward_mod_base, forward_name);
        }
    }
    return nullptr;
}

} // namespace pe::runtime::reflect

#endif // __PETRICKS_RT_REFLECT__
