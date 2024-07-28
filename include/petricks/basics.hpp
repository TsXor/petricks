#pragma once
#ifndef __PETRICKS_BASICS__
#define __PETRICKS_BASICS__

#include <cstdint>
#include "./reimpl.hpp"

namespace pe {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using boolean = u8;

template <typename T>
static inline T* ptr_at(const void* ptr, size_t offset = 0) {
    auto base = reinterpret_cast<size_t>(ptr);
    return reinterpret_cast<T*>(base + offset);
}

template <typename T>
static inline T& ref_at(const void* ptr, size_t offset = 0) {
    return *ptr_at<T>(ptr, offset);
}

template <typename T, typename ItCtrl = typename T::iter>
class sentinel_view {
    T* _first;
public:
    sentinel_view(T* first) : _first(first) {}

    class iterator {
        T* _pos;
    public:
        iterator(T* pos) : _pos(pos) {}
        T& operator*() const { return *_pos; }
        bool operator==(iterator other) const { return _pos == other._pos; }
        bool operator!=(iterator other) const { return !(*this == other); }
        iterator& operator++() {
            _pos = ItCtrl::increment(_pos);
            if (ItCtrl::sentinel(_pos)) { _pos = nullptr; }
            return *this;
        }
    };
    iterator begin() { return {_first}; }
    iterator end() { return {nullptr}; }
}; // class sentinel_span

namespace image {

constexpr size_t numberof_directory_entries = 16;
constexpr u32 dos_signature = 0x5A4D;
constexpr u32 nt_signature = 0x00004550;
constexpr size_t sizeof_short_name = 8;

enum class directory_entry {
    export_ = 0,
    import_ = 1,
    resource = 2,
    exception = 3,
    security = 4,
    basereloc = 5,
    debug = 6,
    architecture = 7,
    globalptr = 8,
    tls = 9,
    load_config = 10,
    bound_import = 11,
    iat = 12,
    delay_import = 13,
    com_descriptor = 14,
    reserved = 15,
}; // enum class directory_entry

enum class file_machine {
    unknown = 0,
    i386 = 0x014c,
    r3000 = 0x0162,
    r4000 = 0x0166,
    r10000 = 0x0168,
    wcemipsv2 = 0x0169,
    alpha = 0x0184,
    sh3 = 0x01a2,
    sh3dsp = 0x01a3,
    sh3e = 0x01a4,
    sh4 = 0x01a6,
    sh5 = 0x01a8,
    arm = 0x01c0,
    armv7 = 0x01c4,
    armnt = 0x01c4,
    arm64 = 0xaa64,
    thumb = 0x01c2,
    am33 = 0x01d3,
    powerpc = 0x01F0,
    powerpcfp = 0x01f1,
    ia64 = 0x0200,
    mips16 = 0x0266,
    alpha64 = 0x0284,
    mipsfpu = 0x0366,
    mipsfpu16 = 0x0466,
    axp64 = alpha64,
    tricore = 0x0520,
    cef = 0x0CEF,
    ebc = 0x0EBC,
    amd64 = 0x8664,
    m32r = 0x9041,
    cee = 0xc0ee,
#if defined(_WIN64)
    local = amd64,
#elif defined(_WIN32)
    local = i386,
#endif
}; // enum class file_machine

enum class rel_based : u16 {
    absolute = 0,
    high = 1,
    low = 2,
    highlow = 3,
    highadj = 4,
    mips_jmpaddr = 5,
    arm_mov32 = 5,
    thumb_mov32 = 7,
    mips_jmpaddr16 = 9,
    ia64_imm64 = 9,
    dir64 = 10,
}; // enum class rel_based

namespace scn {
    constexpr u32 type_no_pad = 0x00000008;
    constexpr u32 cnt_code = 0x00000020;
    constexpr u32 cnt_initialized_data = 0x00000040;
    constexpr u32 cnt_uninitialized_data = 0x00000080;
    constexpr u32 lnk_other = 0x00000100;
    constexpr u32 lnk_info = 0x00000200;
    constexpr u32 lnk_remove = 0x00000800;
    constexpr u32 lnk_comdat = 0x00001000;
    constexpr u32 no_defer_spec_exc = 0x00004000;
    constexpr u32 gprel = 0x00008000;
    constexpr u32 mem_fardata = 0x00008000;
    constexpr u32 mem_purgeable = 0x00020000;
    constexpr u32 mem_16bit = 0x00020000;
    constexpr u32 mem_locked = 0x00040000;
    constexpr u32 mem_preload = 0x00080000;
    constexpr u32 align_1bytes = 0x00100000;
    constexpr u32 align_2bytes = 0x00200000;
    constexpr u32 align_4bytes = 0x00300000;
    constexpr u32 align_8bytes = 0x00400000;
    constexpr u32 align_16bytes = 0x00500000;
    constexpr u32 align_32bytes = 0x00600000;
    constexpr u32 align_64bytes = 0x00700000;
    constexpr u32 align_128bytes = 0x00800000;
    constexpr u32 align_256bytes = 0x00900000;
    constexpr u32 align_512bytes = 0x00A00000;
    constexpr u32 align_1024bytes = 0x00B00000;
    constexpr u32 align_2048bytes = 0x00C00000;
    constexpr u32 align_4096bytes = 0x00D00000;
    constexpr u32 align_8192bytes = 0x00E00000;
    constexpr u32 align_mask = 0x00F00000;
    constexpr u32 lnk_nreloc_ovfl = 0x01000000;
    constexpr u32 mem_discardable = 0x02000000;
    constexpr u32 mem_not_cached = 0x04000000;
    constexpr u32 mem_not_paged = 0x08000000;
    constexpr u32 mem_shared = 0x10000000;
    constexpr u32 mem_execute = 0x20000000;
    constexpr u32 mem_read = 0x40000000;
    constexpr u32 mem_write = 0x80000000;
    constexpr u32 scale_index = 0x00000001;
} // namespace scn

struct nt_headers;

#pragma pack(push,2)
struct dos_header {
    u16 e_magic;
    u16 e_cblp;
    u16 e_cp;
    u16 e_crlc;
    u16 e_cparhdr;
    u16 e_minalloc;
    u16 e_maxalloc;
    u16 e_ss;
    u16 e_sp;
    u16 e_csum;
    u16 e_ip;
    u16 e_cs;
    u16 e_lfarlc;
    u16 e_ovno;
    u16 e_res[4];
    u16 e_oemid;
    u16 e_oeminfo;
    u16 e_res2[10];
    u32 e_lfanew;

    nt_headers& nthdr() {
        return ref_at<nt_headers>(this, e_lfanew);
    }
}; // struct dos_header
#pragma pack(pop)

struct file_header {
    u16 Machine;
    u16 NumberOfSections;
    u32 TimeDateStamp;
    u32 PointerToSymbolTable;
    u32 NumberOfSymbols;
    u16 SizeOfOptionalHeader;
    u16 Characteristics;
}; // struct file_header

struct data_directory {
    u32 VirtualAddress;
    u32 Size;
}; // struct data_directory

struct optional_header32 {
    u16 Magic;
    u8 MajorLinkerVersion;
    u8 MinorLinkerVersion;
    u32 SizeOfCode;
    u32 SizeOfInitializedData;
    u32 SizeOfUninitializedData;
    u32 AddressOfEntryPoint;
    u32 BaseOfCode;
    u32 BaseOfData;
    u32 ImageBase;
    u32 SectionAlignment;
    u32 FileAlignment;
    u16 MajorOperatingSystemVersion;
    u16 MinorOperatingSystemVersion;
    u16 MajorImageVersion;
    u16 MinorImageVersion;
    u16 MajorSubsystemVersion;
    u16 MinorSubsystemVersion;
    u32 Win32VersionValue;
    u32 SizeOfImage;
    u32 SizeOfHeaders;
    u32 CheckSum;
    u16 Subsystem;
    u16 DllCharacteristics;
    u32 SizeOfStackReserve;
    u32 SizeOfStackCommit;
    u32 SizeOfHeapReserve;
    u32 SizeOfHeapCommit;
    u32 LoaderFlags;
    u32 NumberOfRvaAndSizes;
    data_directory DataDirectory[numberof_directory_entries];

    data_directory& datadir(directory_entry type) {
        return DataDirectory[size_t(type)];
    }
}; // struct optional_header32

struct optional_header64 {
    u16 Magic;
    u8 MajorLinkerVersion;
    u8 MinorLinkerVersion;
    u32 SizeOfCode;
    u32 SizeOfInitializedData;
    u32 SizeOfUninitializedData;
    u32 AddressOfEntryPoint;
    u32 BaseOfCode;
    u64 ImageBase;
    u32 SectionAlignment;
    u32 FileAlignment;
    u16 MajorOperatingSystemVersion;
    u16 MinorOperatingSystemVersion;
    u16 MajorImageVersion;
    u16 MinorImageVersion;
    u16 MajorSubsystemVersion;
    u16 MinorSubsystemVersion;
    u32 Win32VersionValue;
    u32 SizeOfImage;
    u32 SizeOfHeaders;
    u32 CheckSum;
    u16 Subsystem;
    u16 DllCharacteristics;
    u64 SizeOfStackReserve;
    u64 SizeOfStackCommit;
    u64 SizeOfHeapReserve;
    u64 SizeOfHeapCommit;
    u32 LoaderFlags;
    u32 NumberOfRvaAndSizes;
    data_directory DataDirectory[numberof_directory_entries];

    data_directory& datadir(directory_entry type) {
        return DataDirectory[size_t(type)];
    }
}; // struct optional_header64

#if defined(_WIN64)
using optional_header = optional_header64;
#elif defined(_WIN32)
using optional_header = optional_header32;
#endif

struct section_header;

struct nt_headers {
    u32 Signature;
    file_header FileHeader;
    union {
        optional_header64 x64;
        optional_header32 x32;
        optional_header local;
    } OptionalHeader;

    file_machine machine() { return file_machine(FileHeader.Machine); }
    section_header& first_section() {
        return ref_at<section_header>(&OptionalHeader, FileHeader.SizeOfOptionalHeader);
    }
    span<section_header> sechdrs() {
        return {&first_section(), FileHeader.NumberOfSections};
    }
}; // struct nt_headers

struct export_directory {
    u32 Characteristics;
    u32 TimeDateStamp;
    u16 MajorVersion;
    u16 MinorVersion;
    u32 Name;
    u32 Base;
    u32 NumberOfFunctions;
    u32 NumberOfNames;
    u32 AddressOfFunctions;
    u32 AddressOfNames;
    u32 AddressOfNameOrdinals;
}; // struct export_directory

struct section_header {
    u8 Name[sizeof_short_name];
    union {
        u32 PhysicalAddress;
        u32 VirtualSize;
    } Misc;
    u32 VirtualAddress;
    u32 SizeOfRawData;
    u32 PointerToRawData;
    u32 PointerToRelocations;
    u32 PointerToLinenumbers;
    u16 NumberOfRelocations;
    u16 NumberOfLinenumbers;
    u32 Characteristics;
}; // struct section_header

struct base_relocation {
    u32 VirtualAddress;
    u32 SizeOfBlock;
    
    struct entry {
        u16 value;
        rel_based flag() { return rel_based(value >> 12); }
        u16 highadj_param() & { return (this + 1)->value; }
        u16 offset() { return value & 0x0FFF; }
    }; // struct entry

    base_relocation& next() {
        return ref_at<base_relocation>(this, SizeOfBlock);
    }
    span<entry> entries() {
        return {reinterpret_cast<entry*>(this + 1), (SizeOfBlock - sizeof(*this)) / sizeof(entry)};
    }

    struct iter {
        static base_relocation* increment(base_relocation* pos) { return &pos->next(); }
        static bool sentinel(base_relocation* pos) { return pos->VirtualAddress == 0; }
    };
}; // struct base_relocation

template <typename OpthdrT>
static inline sentinel_view<base_relocation> basereloc_view(void* base, OpthdrT& opthdr) {
    data_directory& reloc_pos = opthdr.datadir(directory_entry::basereloc);
    if (!reloc_pos.Size) { return {nullptr}; }
    return {ptr_at<image::base_relocation>(base, reloc_pos.VirtualAddress)};
}
template <typename OpthdrT>
static inline sentinel_view<base_relocation> basereloc_view(OpthdrT& opthdr) {
    return imports_view(reinterpret_cast<void*>(opthdr.ImageBase), opthdr);
}

struct import_descriptor {
    u32 OriginalFirstThunk; // The name "Characteristics" is used in Winnt.h, but no longer describes this field.
    u32 TimeDateStamp;
    u32 ForwarderChain;
    u32 Name;
    u32 FirstThunk;

    bool termination() {
        return OriginalFirstThunk == 0 && TimeDateStamp == 0
            && ForwarderChain == 0 && Name == 0 && FirstThunk == 0;
    }

    struct iter {
        static import_descriptor* increment(import_descriptor* pos) { return pos + 1; }
        static bool sentinel(import_descriptor* pos) { return pos->termination(); }
    };
}; // struct import_descriptor

template <typename OpthdrT>
static inline sentinel_view<import_descriptor> imports_view(void* base, OpthdrT& opthdr) {
    data_directory& import_pos = opthdr.datadir(directory_entry::import_);
    if (!import_pos.Size) { return {nullptr}; }
    return {ptr_at<image::import_descriptor>(base, import_pos.VirtualAddress)};
}
template <typename OpthdrT>
static inline sentinel_view<import_descriptor> imports_view(OpthdrT& opthdr) {
    return imports_view(reinterpret_cast<void*>(opthdr.ImageBase), opthdr);
}

struct import_by_name {
    u16 Hint;
    char Name[1];
}; // struct import_by_name

struct thunk_data32 {
    u32 value;
    bool termination() { return value == 0; }
    bool flag() { return value >> 31; }
    u16 ordinal() { return value & 0xFFFF; }
    u32 name_rva() { return value & 0x7FFFFFFF; }
}; // struct thunk_data32

struct thunk_data64 {
    u64 value;
    bool termination() { return value == 0; }
    bool flag() { return value >> 63; }
    u16 ordinal() { return value & 0xFFFF; }
    u32 name_rva() { return value & 0x7FFFFFFF; } // Even in 64-bit, name RVA only uses 31 bits
}; // struct thunk_data64

#if defined(_WIN64)
using thunk_data = thunk_data64;
#elif defined(_WIN32)
using thunk_data = thunk_data32;
#endif

} // namespace image

} // namespace pe

#endif // __PETRICKS_BASICS__
