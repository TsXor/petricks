#pragma once
#ifndef __PETRICKS_RT_PEBTEB__
#define __PETRICKS_RT_PEBTEB__

#include "./rt-basics.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#error This file needs win32/win64 environment!
#endif

namespace pe {
namespace runtime {

/* A mini rip of winternl.h and phnt ... */

struct client_id {
    handle UniqueProcess;
    handle UniqueThread;
}; // struct client_id

struct rtl_user_process_parameters {
    u8 Reserved1[16];
    void *Reserved2[10];
    unicode_string ImagePathName;
    unicode_string CommandLine;
}; // struct rtl_user_process_parameters

struct ldr_data_table_entry {
    list_entry InLoadOrderLinks;
    list_entry InMemoryOrderLinks;
    list_entry InInitializationOrderLinks;
    void *DllBase;
    void *EntryPoint;
    u32 SizeOfImage;
    unicode_string FullDllName;
    unicode_string BaseDllName;
    u32 Flags;
    u16 ObsoleteLoadCount;
    u16 TlsIndex;
    list_entry HashLinks;
    u32 TimeDateStamp;
}; // struct ldr_data_table_entry

template <list_entry ldr_data_table_entry::*node_rel>
using ldr_list_view = list_view<ldr_data_table_entry, node_rel>;

namespace ldr_order {

struct load_t {} load;
struct memory_t {} memory;
struct initialization_t {} initialization;

} // namespace ldr_order

struct peb {
    u8 Reserved1[2];
    u8 BeingDebugged;
    u8 Reserved2[1];
    void *Reserved3[2];
    struct ldr_data {
        u32 Length;
        boolean Initialized;
        void *SsHandle;
        list_entry InLoadOrderModuleList;
        list_entry InMemoryOrderModuleList;
        list_entry InInitializationOrderModuleList;
        ldr_list_view<&ldr_data_table_entry::InLoadOrderLinks> modules(ldr_order::load_t) { return {&InLoadOrderModuleList}; }
        ldr_list_view<&ldr_data_table_entry::InMemoryOrderLinks> modules(ldr_order::memory_t) { return {&InMemoryOrderModuleList}; }
        ldr_list_view<&ldr_data_table_entry::InInitializationOrderLinks> modules(ldr_order::initialization_t) { return {&InInitializationOrderModuleList}; }
    } *Ldr;
    rtl_user_process_parameters *ProcessParameters;
    void *Reserved4[3];
    void *AtlThunkSListPtr;
    void *Reserved5;
    u32 Reserved6;
    void *Reserved7;
    u32 Reserved8;
    u32 AtlThunkSListPtr32;
    void *Reserved9[45];
    u8 Reserved10[96];
    void *PostProcessInitRoutine;
    u8 Reserved11[128];
    void *Reserved12[1];
    u32 SessionId;
}; // struct peb

struct nt_tib {
    void /* exception_registration_record */ *ExceptionList;
    void *StackBase;
    void *StackLimit;
    void *SubSystemTib;
    union {
        void *FiberData;
        u32 Version;
    };
    void *ArbitraryUserPointer;
    nt_tib *Self;
}; // struct nt_tib

struct teb {
    nt_tib NtTib;
    void *EnvironmentPointer;
    client_id ClientId;
    void *ActiveRpcHandle;
    void *ThreadLocalStoragePointer;
    peb *ProcessEnvironmentBlock;
    void *Reserved2[399];
    u8 Reserved3[1952];
    void *TlsSlots[64];
    u8 Reserved4[8];
    void *Reserved5[26];
    void *ReservedForOle;
    void *Reserved6[4];
    void *TlsExpansionSlots;
}; // struct teb

} // namespace runtime
} // namespace pe

#endif // __PETRICKS_RT_PEBTEB__
