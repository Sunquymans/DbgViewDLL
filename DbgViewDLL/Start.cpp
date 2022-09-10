#include "Start.h"

#include <algorithm>
#include <iterator>

#include "RaW.h"

bool isRun = false;
HANDLE DBGVIEW = nullptr;

void mytolower(char* s)
{
    int len = strlen(s);
    for (int i = 0; i < len; i++)
    {
        if (s[i] >= 'A' && s[i] <= 'Z')
        {
            s[i] = tolower(s[i]);
        }
    }
}

void mytoupper(char* s)
{
    int len = strlen(s);
    for (int i = 0; i < len; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            s[i] = toupper(s[i]);
        }
    }
}

void GlobalStart()
{
    if (isRun == false)
    {
        isRun = true;
        DWORD dwThreadId;
        HANDLE hThread = CreateThread(nullptr, 0, GlobalStartThread, nullptr, 0, &dwThreadId);
        if (hThread != nullptr)
            CloseHandle(hThread);
    }
}

DWORD WINAPI GlobalStartThread(LPVOID lpParam)
{
    char chPath[MAX_PATH + 1] = { 0 };
    ::SHGetSpecialFolderPathA(nullptr, chPath, CSIDL_APPDATA, 0);

    char buffer[1024] = { 0 };
    sprintf_s(buffer, sizeof(buffer), "%s\\DbgView.ini", chPath);
    ::WritePrivateProfileStringA("DebugView", "Filter", "图灵", buffer);

    do
    {
        Sleep(300);
        DBGVIEW = GetModuleHandle(nullptr);
    } while (DBGVIEW == nullptr);

    const DWORD filterEditAddress = reinterpret_cast<DWORD>(DBGVIEW) + 0x86720;
    const DWORD filterRealAddress = reinterpret_cast<DWORD>(DBGVIEW) + 0x88240;

    GetPrivateProfileStringA("DebugView", "Filter", "", buffer, sizeof(buffer), buffer);

    char filterValue[] = "TuRingLogger";
    Raw::wpm(filterEditAddress, filterValue, sizeof(filterValue));

    mytoupper(filterValue);
    Raw::wpm(filterRealAddress, filterValue, sizeof(filterValue));

    return 0;
}
