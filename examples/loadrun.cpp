#include <iostream>
#include <fstream>
#include <vector>
#include "petricks.hpp"

extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int wCodePageID);

using pe::runtime::loader::memory_module;

std::vector<char> read_file(const char* name) {
    std::vector<char> file_buf;
    std::ifstream file(name, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) { return file_buf; }
    size_t v = file.tellg();
    file_buf.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(file_buf.data(), file_buf.size());
    return file_buf;
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(65001);
    auto libdat = read_file("libhello.dll");
    memory_module libhello;
    libhello.open(libdat.data());
    auto say_hello = libhello.proc<void(const char*)>("say_hello");
    say_hello("world");
    libhello.close();
    return 0;
}

