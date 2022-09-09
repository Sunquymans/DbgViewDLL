#include "framework.h"
#include "Start.h"

bool IsMutex()
{
    HANDLE hObject = ::CreateMutex(nullptr, FALSE, _T("DebugViewMutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (hObject != nullptr)
            ::CloseHandle(hObject);

        return FALSE;
    }
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, const DWORD ulReasonForCall, LPVOID lpReserved)
{
    switch (ulReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
        if (IsMutex())
        {
            GlobalStart();
        }
        else
        {
            ::TerminateProcess(::GetCurrentProcess(), 0);
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default: ;
    }
    return TRUE;
}

int WINAPI _SunquymanA()
{
    return 0;
}
