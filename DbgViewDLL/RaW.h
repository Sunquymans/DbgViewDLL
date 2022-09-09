#pragma once
#ifndef __RAW_H_
#define __RAW_H_

#include "framework.h"

namespace Raw
{
    bool rpm(u64 address, void* buffer, size_t size);
    bool rpm(u64 address, char* buffer, size_t size);
    bool rpm(u64 address, wchar_t* buffer, size_t size);
    bool rpm(u64 address, size_t size, std::vector<byte>& bytes);

    template <typename T>
    inline T rpm(u64 address)
    {
        T value = T();
        rpm(address, (void*)&value, sizeof(T));
        return value;
    }

    template <typename T>
    inline T rpm(u64 address, std::vector<u32> offsets)
    {
        T value = T();
        address = rpm<u64>(address);
        for (size_t i = 0; i < offsets.size(); i++)
        {
            if (address == 0)
                break;

            if (i == offsets.size() - 1)
            {
                value = rpm<T>(address + offsets[i]);
            }
            else
            {
                address = rpm<u64>(address + offsets[i]);
            }
        }
        return value;
    }

    bool wpm(u64 address, void* buffer, size_t size);
    bool wpm(u64 address, std::string value);
    bool wpm(u64 address, std::wstring value);
    bool wpm(u64 address, std::vector<u8> bytes);

    template <typename T>
    inline bool wpm(u64 address, T value)
    {
        return wpm(address, (void*)&value, sizeof(value));
    }

    template <typename T>
    inline bool wpm(u64 address, T value, std::vector<u32> offsets)
    {
        return wpm(rpm<u64>(address, offsets), value);
    }

    u64 apm(size_t size);
    u64 apm(std::string name, size_t size);
    bool fpm(u64 address);
    u64 gmb(std::string module_name);
    size_t gms(std::string module_name);
    u64 gmp(std::string module_name, std::string proc_name);
    HANDLE crt(u64 start_address, u64 start_context = 0);
};

#endif
