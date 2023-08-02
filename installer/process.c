#include "process.h"

#include <tlhelp32.h>





// Some definitions from NTDDK and other sources
//
// callback function for window enumeration
static BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam)
{
    DWORD dwID;

    GetWindowThreadProcessId(hwnd, &dwID);

    if (dwID == (DWORD)lParam)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }

    return TRUE;
}


static void KillProcess_Init(struct KillProcess* pThis) {
    pThis->FCreateToolhelp32Snapshot = NULL;
    pThis->FProcess32First = NULL;
    pThis->FProcess32Next = NULL;
    pThis->hKernelLib = NULL;
    pThis->hNTLib = NULL;

    pThis->hKernelLib = LoadLibraryA("Kernel32");
    if (pThis->hKernelLib)
    {
        // Find ToolHelp functions
        pThis->FCreateToolhelp32Snapshot =
            (PFCreateToolhelp32Snapshot)
            GetProcAddress(pThis->hKernelLib,
                             "CreateToolhelp32Snapshot");
        pThis->FProcess32First = (PFProcess32First)
            GetProcAddress(pThis->hKernelLib,
                             "Process32FirstW");
        pThis->FProcess32Next = (PFProcess32Next)
            GetProcAddress(pThis->hKernelLib,
                             "Process32NextW");
    }
    if (!pThis->FCreateToolhelp32Snapshot ||
        !pThis->FProcess32First || !pThis->FProcess32Next)
    { // i.e. we couldn't find the ToolHelp functions, 
        //so we must be on NT4. Let's load the
        // undocumented one instead.
        if (!pThis->hKernelLib)
            return; // can't do anything at all without 
        //the kernel.

        pThis->hNTLib = LoadLibraryA("ntdll.dll");
        if (pThis->hNTLib)
        {
            pThis->FQuerySysInfo =
                (PFZwQuerySystemInformation)
                GetProcAddress(pThis->hNTLib,
                                 "ZwQuerySystemInformation");
            // load some support funcs from the kernel
            pThis->FGetProcessHeap = (PFGetProcessHeap)
                GetProcAddress(pThis->hKernelLib,
                                 "GetProcessHeap");
            pThis->FHeapAlloc = (PFHeapAlloc)
                GetProcAddress(pThis->hKernelLib,
                                 "HeapAlloc");
            pThis->FHeapFree = (PFHeapFree)
                GetProcAddress(pThis->hKernelLib,
                                 "HeapFree");
        }
    }
}
static void KillProcess_Destroy(struct KillProcess* pThis)
{
    if (pThis->hKernelLib)
        FreeLibrary(pThis->hKernelLib);
    if (pThis->hNTLib)
        FreeLibrary(pThis->hNTLib);
}
static BOOL KillProcess_KillProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName)
{
    DWORD dwId;
    HANDLE hProcess = KillProcess_FindProcess(pThis, pstrProcessName, &dwId);
    BOOL bResult;
    if (!hProcess)
        return FALSE;

    // TerminateAppEnum() posts WpThis->CLOSE to all windows whose PID
    // matches your process's.
    EnumWindows((WNDENUMPROC)
                  TerminateAppEnum,
                  (LPARAM)dwId);
    // Wait on the handle. If it signals, great. 
    //If it times out, then you kill it.
    if (WaitForSingleObject(hProcess, 5000)
        != WAIT_OBJECT_0)
        bResult = TerminateProcess(hProcess, 0);
    else
        bResult = TRUE;
    CloseHandle(hProcess);
    return bResult == TRUE;
}
//private:
static HANDLE KillProcess_FindProcess(struct KillProcess* pThis,
                               const WCHAR* pstrProcessName,
                               DWORD* dwId)
{
    if (!pThis->hKernelLib)
        return NULL;

    if (pThis->FCreateToolhelp32Snapshot && pThis->FProcess32First &&
        pThis->FProcess32Next) // use toolhelpapi
        return KillProcess_THFindProcess(pThis, pstrProcessName, dwId);
    if (pThis->FQuerySysInfo && pThis->FHeapAlloc &&
        pThis->FGetProcessHeap && pThis->FHeapFree) // use NT api
        return KillProcess_NTFindProcess(pThis, pstrProcessName, dwId);
    // neither one got loaded. Strange.
    return NULL;
}

static HANDLE KillProcess_THFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName, DWORD* dwId)
{
    HANDLE            hSnapShot = NULL;
    HANDLE            hResult = NULL;
    PROCESSENTRY32    processInfo;
    WCHAR* pstrExeName;

    BOOL bFirst = TRUE;
    ZeroMemory(&processInfo, sizeof(PROCESSENTRY32));
    processInfo.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = pThis->FCreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS, 0);
    if (hSnapShot == INVALID_HANDLE_VALUE)
        return NULL;

    // ok now let's iterate with Process32Next until we 
    // match up the name of our process
    while ((bFirst ? pThis->FProcess32First(hSnapShot,&processInfo) : pThis->FProcess32Next(hSnapShot, &processInfo)))
    {
        bFirst = FALSE;
        // we need to check for path... and extract 
        // just the exe name
        pstrExeName = wcsrchr(processInfo.szExeFile,
                              '\\');
        if (!pstrExeName)
            pstrExeName = processInfo.szExeFile;
        else
            pstrExeName++; // skip the \
            // ok now compare against our process name
        if (wcsicmp(pstrExeName, pstrProcessName) == 0)
            // wee weee we found it
        {
            // let's get a HANDLE on it
            hResult = OpenProcess(
                SYNCHRONIZE | PROCESS_TERMINATE, TRUE,
                processInfo.th32ProcessID);
            *dwId = processInfo.th32ProcessID;
            break;
        }
    } // while(Process32Next(hSnapShot, &processInfo){
    if (hSnapShot)
        CloseHandle(hSnapShot);
    return hResult;
}
static HANDLE KillProcess_NTFindProcess(struct KillProcess* pThis,  const WCHAR* pstrProcessName,
                                    DWORD* dwId)
{
    HANDLE hHeap = pThis->FGetProcessHeap();
    NTSTATUS Status;
    ULONG cbBuffer = 0x8000;
    PVOID pBuffer = NULL;
    HANDLE hResult = NULL;
    // it is difficult to say a priory which size of 
    // the buffer will be enough to retrieve all 
    // information, so we startwith 32K buffer and 
    // increase its size until we get the
    // information successfully
    do
    {
        pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
        if (pBuffer == NULL)
            return SetLastError(
            ERROR_NOT_ENOUGH_MEMORY), NULL;

        Status = pThis->FQuerySysInfo(
            SystemProcessesAndThreadsInformation,
            pBuffer, cbBuffer, NULL);

        if (Status == STATUS_INFO_LENGTH_MISMATCH)
        {
            HeapFree(hHeap, 0, pBuffer);
            cbBuffer *= 2;
        }
        else if (!NT_SUCCESS(Status))
        {
            HeapFree(hHeap, 0, pBuffer);
            return SetLastError(Status), NULL;
        }
    }
    while (Status == STATUS_INFO_LENGTH_MISMATCH);

    PSYSTEM_PROCESSES pProcesses =
        (PSYSTEM_PROCESSES)pBuffer;

    for (;;)
    {
        PCWSTR pszProcessName =
            pProcesses->ProcessName.Buffer;
        if (pszProcessName == NULL)
            pszProcessName = L"Idle";

        //CHAR szProcessName[MAX_PATH];
        //WideCharToMultiByte(CP_ACP, 0, pszProcessName,
          //                  -1, szProcessName, MAX_PATH, NULL, NULL);

        if (wcsicmp(pszProcessName, pstrProcessName)
            == 0) // found it
        {
            hResult = OpenProcess(
                SYNCHRONIZE | PROCESS_TERMINATE, TRUE,
                pProcesses->ProcessId);
            *dwId = pProcesses->ProcessId;
            break;
        }

        if (pProcesses->NextEntryDelta == 0)
            break;

        // find the address of the next process 
        // structure
        pProcesses = (PSYSTEM_PROCESSES)(
            ((LPBYTE)pProcesses)
            + pProcesses->NextEntryDelta);
    }

    HeapFree(hHeap, 0, pBuffer);
    return hResult;
}


void SystemCreateProcess(const WCHAR* moduleName, const WCHAR* cmdline)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    // Start the child process. 
    if (!CreateProcess(moduleName,   // No module name (use command line)
        cmdline,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        //printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    //CloseHandle(pi.hProcess);
    //CloseHandle(pi.hThread);
}
