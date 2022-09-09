#include "RaW.h"

namespace Raw
{
    bool rpm(u64 address, void* buffer, size_t size)
    {
        __try
        {
            memcpy(buffer, (const void*)address, size);
            return true;
        }
        __except (1)
        {
            return false;
        }
    }

    bool rpm(u64 address, char* buffer, size_t size)
    {
        bool result = rpm(address, (void*)buffer, size);
        if (result)
        {
            strncpy_s(buffer, size, buffer, size);
        }
        return result;
    }

    bool rpm(u64 address, wchar_t* buffer, size_t size)
    {
        bool result = rpm(address, (void*)buffer, size);
        if (result)
        {
            wcsncpy_s(buffer, size, buffer, size);
        }
        return result;
    }

    bool rpm(u64 address, size_t size, std::vector<byte>& bytes)
    {
        bytes.resize(size);

        if (rpm(address, bytes.data(), size))
        {
            return true;
        }
        return false;
    }

    bool wpm(u64 address, void* buffer, size_t size)
    {
        __try
        {
            DWORD oldProtect = 0;
            bool result = (bool)::VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
            if (result)
            {
                memcpy((void*)address, buffer, size);
                result = (bool)::VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
            }
            return result;
        }
        __except (1)
        {
            return false;
        }
    }

    bool wpm(u64 address, std::string value)
    {
        return wpm(address, (void*)value.data(), value.size());
    }

    bool wpm(u64 address, std::wstring value)
    {
        return wpm(address, (void*)value.data(), value.size());
    }

    bool wpm(u64 address, std::vector<u8> bytes)
    {
        bool result = false;
        if (bytes.size() > 0)
        {
            result = wpm(address, bytes.data(), bytes.size());
        }
        return result;
    }

    u64 apm(size_t size)
    {
        return (u64)::VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    }

    bool fpm(u64 address)
    {
        return (bool)::VirtualFree((LPVOID)address, 0, MEM_COMMIT);
    }

    u64 gmb(std::string module_name)
    {
        return (u64)::GetModuleHandleA(module_name.c_str());
    }

    size_t gms(std::string module_name)
    {
        HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ::GetCurrentProcessId());
        if (!hSnapshot)
            return 0;

        HANDLE hModule = (HANDLE)gmb(module_name);
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

    u64 gmp(std::string module_name, std::string proc_name)
    {
        return (u64)::GetProcAddress((HMODULE)gmb(module_name), proc_name.c_str());
    }

    HANDLE crt(u64 start_address, u64 start_context)
    {
        uint64_t lit_address = gmp("ntdll.dll", "LdrInitializeThunk");
        uint64_t lit_jmp_address = lit_address + rpm<int32_t>(lit_address + 1) + 5;
        uint8_t lit_jmp_address_old_opcode[11] = { 0 };
        uint8_t lit_jmp_address_new_opcode[11] = { 0 };
        rpm(lit_jmp_address, lit_jmp_address_old_opcode, sizeof(lit_jmp_address_old_opcode));
        memcpy(lit_jmp_address_new_opcode, (void*)lit_address, sizeof(lit_jmp_address_new_opcode));

        uint64_t nct_address = gmp("ntdll.dll", "NtCreateThreadEx");
        uint64_t nct_jmp_address = nct_address + rpm<int32_t>(nct_address + 1) + 5;
        uint8_t nct_jmp_address_old_opcode[13] = { 0 };
        uint8_t nct_jmp_address_new_opcode[13] = { 0 };
        rpm(nct_jmp_address, nct_jmp_address_old_opcode, sizeof(nct_jmp_address_old_opcode));
        memcpy(nct_jmp_address_new_opcode, (void*)nct_address, sizeof(nct_jmp_address_new_opcode));

        // 干掉LdrInitializeThunk 钩子
        {
            *(uint8_t*)(lit_jmp_address_new_opcode + 6) = 0xe9;
            *(uint32_t*)(lit_jmp_address_new_opcode + 7) = int32_t((lit_address + 6) - (lit_jmp_address + 6) - 5);
            wpm(lit_jmp_address, lit_jmp_address_new_opcode, sizeof(lit_jmp_address_new_opcode));
        }
        // 干掉NtCreateThreadEx钩子
        {
            *(uint8_t*)(nct_jmp_address_new_opcode + 8) = 0xe9;
            *(uint32_t*)(nct_jmp_address_new_opcode + 9) = int32_t((nct_address + 8) - (nct_jmp_address + 8) - 5);
            wpm(nct_jmp_address, nct_jmp_address_new_opcode, sizeof(nct_jmp_address_new_opcode));
        }

        return ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)start_address, (LPVOID)start_context, 0, nullptr);
    }
}
