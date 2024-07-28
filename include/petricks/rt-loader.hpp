#include "./rt-basics.hpp"
#include "./rt-reflect.hpp"
#include "./rt-winapi.hpp"

#if !defined(_WIN32) && !defined(_WIN64)
#error This file needs win32/win64 environment!
#endif

namespace pe::runtime::loader {

class memory_module {
    ebco_pair<winapi_default, void*> _impl;

public:
    memory_module(const winapi_default& api = {}) : _impl(api, nullptr) {}
    ~memory_module() { close(); }

    const winapi_default& api() const { return _impl.first(); }
    void* base_addr() const { return _impl.second(); }
    operator bool() { return bool(base_addr()); }

    enum class errc {
        ok = 0,
        not_pe_file, // file missing DOE/PE signature
        arch_mismatch, // file architecture does not match current program
        alloc_fail, // cannot allocate needed memory
        attach_fail, // entry returns FALSE
    }; // enum class errc

    TyDllMain* entry() {
        auto& base_addr = _impl.second();
        if (!base_addr) { return nullptr; }
        auto& loaded_opthdr = reinterpret_cast<image::dos_header*>(base_addr)->nthdr().OptionalHeader.local;
        return loaded_opthdr.AddressOfEntryPoint ? ptr_at<TyDllMain>(base_addr, loaded_opthdr.AddressOfEntryPoint) : nullptr;
    }

    errc open(void* image) {
        auto& winapi = _impl.first();
        auto& base_addr = _impl.second();

        // basic signature and machine check
        auto& doshdr = *reinterpret_cast<image::dos_header*>(image);
        if (doshdr.e_magic != image::dos_signature) { return errc::not_pe_file; }
        auto& nthdr = doshdr.nthdr();
        if (nthdr.Signature != image::nt_signature) { return errc::not_pe_file; }
        if (nthdr.machine() != image::file_machine::local) { return errc::arch_mismatch; }
        auto& opthdr = nthdr.OptionalHeader.local;

        // allocate module memory
        base_addr = winapi.VirtualAlloc(reinterpret_cast<void*>(opthdr.ImageBase), opthdr.SizeOfImage, mem::reserve, page::readwrite);
        if (!base_addr) { base_addr = winapi.VirtualAlloc(nullptr, opthdr.SizeOfImage, mem::reserve, page::readwrite); }
        if (!base_addr) { return errc::alloc_fail; }
        size_t reloc_offset = reinterpret_cast<size_t>(base_addr) - opthdr.ImageBase;

        // copy headers and set real module base
        memcpy(base_addr, image, opthdr.SizeOfHeaders);
        auto& loaded_nthdr = reinterpret_cast<image::dos_header*>(base_addr)->nthdr();
        auto& loaded_opthdr = loaded_nthdr.OptionalHeader.local;
        loaded_opthdr.ImageBase = reinterpret_cast<size_t>(base_addr);

        // copy sections
        for (auto& sechdr : nthdr.sechdrs()) {
            auto sec_addr = ptr_at<void>(base_addr, sechdr.VirtualAddress);
            if (sechdr.SizeOfRawData == 0) {
                if (opthdr.SectionAlignment != 0) {
                    auto empty_sec = winapi.VirtualAlloc(sec_addr, opthdr.SectionAlignment, mem::commit, page::readwrite);
                    memset(empty_sec, 0, opthdr.SectionAlignment);
                }
            } else {
                auto section = winapi.VirtualAlloc(sec_addr, sechdr.SizeOfRawData, mem::commit, page::readwrite);
                memcpy(section, ptr_at<void>(image, sechdr.PointerToRawData), sechdr.SizeOfRawData);
            }
        }

        // handle relocation
        if (reloc_offset != 0) {
            for (auto& reloc_base : image::basereloc_view(loaded_opthdr)) {
                bool is_highadj_param = false;
                for (auto& reloc : reloc_base.entries()) {
                    // skip this if last is highadj
                    if (is_highadj_param) { is_highadj_param = false; continue; }
                    // Note: for most modules, only highlow and dir64 is used.
                    switch (reloc.flag()) {
                        case image::rel_based::absolute: break;
                        auto patch_pos = ptr_at<void>(base_addr, reloc_base.VirtualAddress + reloc.offset());
                        case image::rel_based::high: {
                            ref_at<u16 __unaligned>(patch_pos) += u32(reloc_offset) >> 16;
                        } break;
                        case image::rel_based::low: {
                            ref_at<u16 __unaligned>(patch_pos) += u32(reloc_offset) & 0xFFFF;
                        } break;
                        case image::rel_based::highlow: {
                            ref_at<u32 __unaligned>(patch_pos) += u32(reloc_offset);
                        } break;
                        case image::rel_based::highadj: {
                            // I just cannot understand what microsoft said on this in documentation.
                            // You may hope sane compilers never use this.
                            // The following is based on: https://github.com/BHTY/EmuWoW/blob/main/pe.c#L121-L127
                            u32 wtf = u32(reloc_offset) + u32(reloc.highadj_param()) + 0x8000;
                            ref_at<u16 __unaligned>(patch_pos) += u16(wtf >> 16);
                            is_highadj_param = true;
                        } break;
                        case image::rel_based::dir64: {
                            ref_at<u64 __unaligned>(patch_pos) += reloc_offset;
                        } break;
                        default: ; // skips unknown reloc 
                    }
                }
            }
        }

        // import dependencies
        for (auto& import_desc : image::imports_view(loaded_opthdr)) {
            auto depmod = winapi.LoadLibraryA(ptr_at<char>(base_addr, import_desc.Name));
            if (!depmod) { continue; }
            auto lookup_table = ptr_at<image::thunk_data>(base_addr, import_desc.OriginalFirstThunk);
            auto address_table = ptr_at<image::thunk_data>(base_addr, import_desc.FirstThunk);
            for (size_t i = 0; !lookup_table[i].termination(); ++i) {
                char* name = lookup_table[i].flag()
                    ? reinterpret_cast<char*>(address_table[i].ordinal())
                    : ref_at<image::import_by_name>(base_addr, lookup_table[i].name_rva()).Name;
                address_table[i].value = reinterpret_cast<size_t>(winapi.GetProcAddress(depmod, name));
            }
        }

        // set section protection
        for (auto& sechdr : loaded_nthdr.sechdrs()) {
            if (sechdr.Characteristics & image::scn::mem_discardable) {
                winapi.VirtualFree(ptr_at<void>(base_addr, sechdr.VirtualAddress), sechdr.SizeOfRawData, mem::decommit);
            } else {
                u32 sec_prot, sec_old_prot;
                switch (sechdr.Characteristics & (image::scn::mem_read | image::scn::mem_write)) {
                    case 0: sec_prot = page::noaccess; break;
                    case image::scn::mem_read: sec_prot = page::readonly; break;
                    case image::scn::mem_write: sec_prot = page::writecopy; break;
                    case (image::scn::mem_read | image::scn::mem_write): sec_prot = page::readwrite; break;
                }
                if (sechdr.Characteristics & image::scn::mem_execute) { sec_prot <<= 4; }
                if (sechdr.Characteristics & image::scn::mem_not_cached) { sec_prot |= page::nocache; }
                u32 sec_size = 0;
                if (sechdr.SizeOfRawData != 0) { sec_size = sechdr.SizeOfRawData; }
                else if (sechdr.Characteristics & image::scn::cnt_initialized_data) { sec_size = loaded_opthdr.SizeOfInitializedData; }
                else if (sechdr.Characteristics & image::scn::cnt_uninitialized_data) { sec_size = loaded_opthdr.SizeOfUninitializedData; }
                if (sec_size != 0) {
                    winapi.VirtualProtect(ptr_at<void>(base_addr, sechdr.VirtualAddress), sechdr.SizeOfRawData, sec_prot, &sec_old_prot);
                }
            }
        }

        auto mod_entry = entry();
        if (mod_entry) {
            auto success = mod_entry(base_addr, dll::process_attach, 0);
            if (!success) { close(); return errc::attach_fail; }
        }

        return errc::ok;
    }

    void close() {
        auto& winapi = _impl.first();
        auto& base_addr = _impl.second();

        if (!base_addr) { return; }
        auto& loaded_nthdr = reinterpret_cast<image::dos_header*>(base_addr)->nthdr();
        auto& loaded_opthdr = loaded_nthdr.OptionalHeader.local;

        auto mod_entry = entry();
        if (mod_entry) { mod_entry(base_addr, dll::process_detach, 0); }

        // free dependencies
        for (auto& import_desc : image::imports_view(loaded_opthdr)) {
            auto depmod = winapi.GetModuleHandleA(ptr_at<char>(base_addr, import_desc.Name));
            if (depmod) { winapi.FreeLibrary(depmod); }
        }

        winapi.VirtualFree(base_addr, 0, mem::release);
    }
}; // class memory_module

} // namespace pe::runtime::loader
