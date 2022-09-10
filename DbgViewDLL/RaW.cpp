#include "RaW.h"

namespace Raw
{
    bool rpm(const u64 address, void* buffer, const size_t size)
    {
        __try
        {
            memcpy(buffer, reinterpret_cast<const void*>(address), size);
            return true;
        }
        __except (1)
        {
            return false;
        }
    }

    bool rpm(const u64 address, char* buffer, const size_t size)
    {
        bool result = rpm(address, static_cast<void*>(buffer), size);
        if (result)
        {
            strncpy_s(buffer, size, buffer, size);
        }
        return result;
    }

    bool rpm(const u64 address, wchar_t* buffer, const size_t size)
    {
        bool result = rpm(address, static_cast<void*>(buffer), size);
        if (result)
        {
            wcsncpy_s(buffer, size, buffer, size);
        }
        return result;
    }

    bool rpm(const u64 address, const size_t size, std::vector<byte>& bytes)
    {
        bytes.resize(size);

        if (rpm(address, bytes.data(), size))
        {
            return true;
        }
        return false;
    }

    bool wpm(const u64 address, const void* buffer, const size_t size)
    {
        __try
        {
            DWORD oldProtect = 0;
            bool result = static_cast<bool>(VirtualProtect(reinterpret_cast<LPVOID>(address), size,
                PAGE_EXECUTE_READWRITE, &oldProtect));
            if (result)
            {
                memcpy(reinterpret_cast<void*>(address), buffer, size);
                result = static_cast<bool>(VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect,
                    &oldProtect));
            }
            return result;
        }
        __except (1)
        {
            return false;
        }
    }

    bool wpm(const u64 address, const std::string& value)
    {
        return wpm(address, value.data(), value.size());
    }

    bool wpm(const u64 address, const std::wstring& value)
    {
        return wpm(address, value.data(), value.size());
    }

    bool wpm(const u64 address, const std::vector<u8>& bytes)
    {
        bool result = false;
        if (!bytes.empty())
        {
            result = wpm(address, bytes.data(), bytes.size());
        }
        return result;
    }

    u64 apm(const size_t size)
    {
        return reinterpret_cast<u64>(VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
    }

    bool fpm(const u64 address)
    {
        return static_cast<bool>(VirtualFree(reinterpret_cast<LPVOID>(address), 0, MEM_COMMIT));
    }

    u64 gmb(const std::string& module_name)
    {
        return reinterpret_cast<u64>(GetModuleHandleA(module_name.c_str()));
    }

    size_t gms(const std::string& module_name)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
        if (!hSnapshot)
            return 0;

        auto hModule = reinterpret_cast<HANDLE>(gmb(module_name));
        if (!hModule)
            return 0;

        MODULEENTRY32W pme = {};
        BOOL R = Module32FirstW(hSnapshot, &pme);
        while (R)
        {
            if (pme.modBaseAddr == hModule)
            {
                return pme.modBaseSize;
            }
            R = Module32FirstW(hSnapshot, &pme);
        }
        return 0;
    }

    u64 gmp(const std::string& module_name, const std::string& proc_name)
    {
        return reinterpret_cast<u64>(GetProcAddress(reinterpret_cast<HMODULE>(gmb(module_name)), proc_name.c_str()));
    }

    HANDLE crt(const u64 start_address, const u64 start_context)
    {
        uint64_t lit_address = gmp("ntdll.dll", "LdrInitializeThunk");
        uint64_t lit_jmp_address = lit_address + rpm<int32_t>(lit_address + 1) + 5;
        uint8_t lit_jmp_address_old_opcode[11] = { 0 };
        uint8_t lit_jmp_address_new_opcode[11] = { 0 };
        rpm(lit_jmp_address, lit_jmp_address_old_opcode, sizeof(lit_jmp_address_old_opcode));
        memcpy(lit_jmp_address_new_opcode, reinterpret_cast<void*>(lit_address), sizeof(lit_jmp_address_new_opcode));

        uint64_t nct_address = gmp("ntdll.dll", "NtCreateThreadEx");
        uint64_t nct_jmp_address = nct_address + rpm<int32_t>(nct_address + 1) + 5;
        uint8_t nct_jmp_address_old_opcode[13] = { 0 };
        uint8_t nct_jmp_address_new_opcode[13] = { 0 };
        rpm(nct_jmp_address, nct_jmp_address_old_opcode, sizeof(nct_jmp_address_old_opcode));
        memcpy(nct_jmp_address_new_opcode, reinterpret_cast<void*>(nct_address), sizeof(nct_jmp_address_new_opcode));

        // LdrInitializeThunk 处理
        {
            *(lit_jmp_address_new_opcode + 6) = 0xe9;
            *reinterpret_cast<uint32_t*>(lit_jmp_address_new_opcode + 7) = static_cast<int32_t>((lit_address + 6) - (
                lit_jmp_address + 6) - 5);
            wpm(lit_jmp_address, lit_jmp_address_new_opcode, sizeof(lit_jmp_address_new_opcode));
        }
        // NtCreateThreadEx 处理
        {
            *(nct_jmp_address_new_opcode + 8) = 0xe9;
            *reinterpret_cast<uint32_t*>(nct_jmp_address_new_opcode + 9) = static_cast<int32_t>((nct_address + 8) - (
                nct_jmp_address + 8) - 5);
            wpm(nct_jmp_address, nct_jmp_address_new_opcode, sizeof(nct_jmp_address_new_opcode));
        }

        return CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(start_address),
            reinterpret_cast<LPVOID>(start_context), 0, nullptr);
    }
}
