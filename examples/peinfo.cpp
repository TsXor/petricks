/**
 *  原代码来自：https://www.cnblogs.com/LyShark/articles/15019702.html
 *  此文件为基于petricks的现代C++重写
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include "petricks/basics.hpp"

extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);

const char* directory_name(size_t idx) {
    static const char* names[] = {
        /*  0 */ "输出表",
        /*  1 */ "输入表",
        /*  2 */ "资源",
        /*  3 */ "异常",
        /*  4 */ "安全",
        /*  5 */ "重定位",
        /*  6 */ "调试",
        /*  7 */ "版权",
        /*  8 */ "全局指针",
        /*  9 */ "TLS表",
        /* 10 */ "载入配置",
        /* 11 */ "输入范围",
        /* 12 */ "IAT",
        /* 13 */ "延迟导入",
        /* 14 */ "COM",
        /* 15 */ "保留",
    };
    return names[idx];
}

std::string lengthed_hex(size_t length, size_t val) {
    static char hextab[] = "0123456789ABCDEF";
    size_t rlen = 0;
    for (size_t v = val; v > 0; v >>= 4) { ++rlen; }
    length = std::max(rlen, length);
    std::string result("0x");
    for (size_t i = 0; i < length; ++i)
        { result.push_back(hextab[(val >> (length - 1 - i) * 4) & 0xF]); }
    return result;
}

template <typename T>
std::string pad_hex(T val) {
    return lengthed_hex(sizeof(T) * 2, val);
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(65001);

    if (argc < 2) {
        std::cout << "未提供文件名" << std::endl;
        return 1;
    }

    std::vector<char> file_buf;
    {
        std::ifstream file(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "打开文件失败" << std::endl;
            return 0;
        }
        size_t v = file.tellg();
        file_buf.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(file_buf.data(), file_buf.size());
    }

    auto& doshdr = *reinterpret_cast<pe::image::dos_header*>(file_buf.data());
    if (doshdr.e_magic != pe::image::dos_signature) {
        std::cout << "不是PE文件：DOS签名错误" << std::endl;
        return 0;
    }

    auto& nthdr = doshdr.nthdr();
    if (nthdr.Signature != pe::image::nt_signature) {
        std::cout << "不是PE文件：NT签名错误" << std::endl;
        return 0;
    }

    std::cout << std::left;

    auto& coffhdr = nthdr.FileHeader;
    pe::image::data_directory* datadirs;

    if (nthdr.machine() == pe::image::file_machine::i386) {
        auto& opthdr = nthdr.OptionalHeader.x32;
        std::cout
            << "================== 基 本 P E 头 信 息 ==================" << std::endl
            << "入口点:         " << pad_hex(opthdr.AddressOfEntryPoint)    << std::endl
            << "子系统:         " << pad_hex(opthdr.Subsystem)              << std::endl
            << "镜像基址:       " << pad_hex(opthdr.ImageBase)              << std::endl
            << "区段数目:       " << pad_hex(coffhdr.NumberOfSections)      << std::endl
            << "镜像大小:       " << pad_hex(opthdr.SizeOfImage)            << std::endl
            << "日期时间标志:   " << pad_hex(coffhdr.TimeDateStamp)         << std::endl
            << "代码基址:       " << pad_hex(opthdr.BaseOfCode)             << std::endl
            << "文件头大小:     " << pad_hex(opthdr.SizeOfHeaders)          << std::endl
            << "数据基址:       " << pad_hex(opthdr.BaseOfData)             << std::endl
            << "特征值:         " << pad_hex(coffhdr.Characteristics)       << std::endl
            << "块对齐:         " << pad_hex(opthdr.SectionAlignment)       << std::endl
            << "校验和:         " << pad_hex(opthdr.CheckSum)               << std::endl
            << "文件块对齐:     " << pad_hex(opthdr.FileAlignment)          << std::endl
            << "可选头部大小:   " << pad_hex(coffhdr.SizeOfOptionalHeader)  << std::endl
            << "标志字:         " << pad_hex(opthdr.Magic)                  << std::endl
            << "RVA数及大小:    " << pad_hex(opthdr.NumberOfRvaAndSizes)    << std::endl;
        datadirs = opthdr.DataDirectory;
    } else if (nthdr.machine() == pe::image::file_machine::amd64) {
        auto& opthdr = nthdr.OptionalHeader.x64;
        std::cout
            << "================== 基 本 P E 头 信 息 ==================" << std::endl
            << "入口点:         " << pad_hex(opthdr.AddressOfEntryPoint)    << std::endl
            << "子系统:         " << pad_hex(opthdr.Subsystem)              << std::endl
            << "镜像基址:       " << pad_hex(opthdr.ImageBase)              << std::endl
            << "区段数目:       " << pad_hex(coffhdr.NumberOfSections)      << std::endl
            << "镜像大小:       " << pad_hex(opthdr.SizeOfImage)            << std::endl
            << "日期时间标志:   " << pad_hex(coffhdr.TimeDateStamp)         << std::endl
            << "代码基址:       " << pad_hex(opthdr.BaseOfCode)             << std::endl
            << "文件头大小:     " << pad_hex(opthdr.SizeOfHeaders)          << std::endl
            << "数据基址:       " << "N/A in x64"                           << std::endl
            << "特征值:         " << pad_hex(coffhdr.Characteristics)       << std::endl
            << "块对齐:         " << pad_hex(opthdr.SectionAlignment)       << std::endl
            << "校验和:         " << pad_hex(opthdr.CheckSum)               << std::endl
            << "文件块对齐:     " << pad_hex(opthdr.FileAlignment)          << std::endl
            << "可选头部大小:   " << pad_hex(coffhdr.SizeOfOptionalHeader)  << std::endl
            << "标志字:         " << pad_hex(opthdr.Magic)                  << std::endl
            << "RVA数及大小:    " << pad_hex(opthdr.NumberOfRvaAndSizes)    << std::endl;
        datadirs = opthdr.DataDirectory;
    } else {
        std::cout << "暂不支持此架构的PE文件" << std::endl;
        return 0;
    }

    std::cout
        << "======================= 目 录 表 =======================" << std::endl
            << std::setw(15) << "类型"
            << std::setw(15) << "RVA"
            << std::setw(15) << "大小"
        << std::endl;
    for (size_t idx = 0; idx < 15; ++idx) {
        std::cout
            << std::setw(15) << directory_name(idx)
            << std::setw(15) << pad_hex(datadirs[idx].VirtualAddress)
            << std::setw(15) << pad_hex(datadirs[idx].Size)
        << std::endl;
    }

    std::cout
        << "======================= 区 段 表 =======================" << std::endl
            << std::setw(15) << "名称"
            << std::setw(15) << "VOffset"
            << std::setw(15) << "VSize"
            << std::setw(15) << "ROffset"
            << std::setw(15) << "RSize"
            << std::setw(15) << "标志"
        << std::endl;
    for (auto& section : nthdr.sechdrs()) {
        std::cout
            << std::setw(15) << reinterpret_cast<char*>(&section.Name)
            << std::setw(15) << pad_hex(section.VirtualAddress)
            << std::setw(15) << pad_hex(section.Misc.VirtualSize)
            << std::setw(15) << pad_hex(section.PointerToRawData)
            << std::setw(15) << pad_hex(section.SizeOfRawData)
            << std::setw(15) << pad_hex(section.Characteristics)
        << std::endl;
    }
    return 0;
}
