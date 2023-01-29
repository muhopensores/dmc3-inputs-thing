#include <shlwapi.h>

#include "String.hpp"
#include "Module.hpp"

using namespace std;

namespace utility {
    optional<size_t> get_module_size(const string& module) {
        return get_module_size(GetModuleHandle(module.c_str()));
    }

    optional<size_t> get_module_size(HMODULE module) {
        if (module == nullptr) {
            return {};
        }

        // Get the dos header and verify that it seems valid.
        auto dosHeader = (PIMAGE_DOS_HEADER)module;

        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return {};
        }

        // Get the nt headers and verify that they seem valid.
        auto ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)dosHeader + dosHeader->e_lfanew);

        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            return {};
        }

        // OptionalHeader is not actually optional.
        return ntHeaders->OptionalHeader.SizeOfImage;
    }

    optional<std::string> get_module_directory(HMODULE module) {
        wchar_t fileName[MAX_PATH]{ 0 };
        if (GetModuleFileNameW(module, fileName, MAX_PATH) >= MAX_PATH) {
            return {};
        }

        PathRemoveFileSpecW(fileName);

        return utility::narrow(fileName);
    }

    optional<HMODULE> get_module_within(Address address) {
        HMODULE module = nullptr;
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, address.as<LPCSTR>(), &module)) {
            return module;
        }

        return {};
    }

    std::optional<std::string> get_module_path(HMODULE module) {
        wchar_t filename[MAX_PATH]{ 0 };
        if (GetModuleFileNameW(module, filename, MAX_PATH) >= MAX_PATH) {
            return {};
        }

        return utility::narrow(filename);
    }

    optional<uintptr_t> ptr_from_rva(uint8_t* dll, uintptr_t rva) {
        // Get the first section.
        auto dosHeader = (PIMAGE_DOS_HEADER)&dll[0];
        auto ntHeaders = (PIMAGE_NT_HEADERS)&dll[dosHeader->e_lfanew];
        auto section = IMAGE_FIRST_SECTION(ntHeaders);

        // Go through each section searching for where the rva lands.
        for (uint16_t i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section) {
            auto size = section->Misc.VirtualSize;

            if (size == 0) {
                size = section->SizeOfRawData;
            }

            if (rva >= section->VirtualAddress && rva < (section->VirtualAddress + size)) {
                auto delta = section->VirtualAddress - section->PointerToRawData;

                return (uintptr_t)(dll + (rva - delta));
            }
        }

        return {};
    }
}
