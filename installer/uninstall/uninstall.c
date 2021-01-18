// uninstall.cpp : Defines the entry point for the application.
//

#include <stdio.h>

#include "..\installer\script.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>
#include <Windows.h>
#include <tlhelp32.h>

typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID {
    DWORD        UniqueProcess;
    DWORD        UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR        Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG        PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

// Note that the size of the SYSTEM_PROCESSES structure is 
// different on NT 4 and Win2K, but we don't care about it, 
// since we don't access neither IoCounters member nor 
//Threads array

typedef struct _SYSTEM_PROCESSES {
    ULONG            NextEntryDelta;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY        BasePriority;
    ULONG            ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS        VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS        IoCounters;
#endif
    SYSTEM_THREADS  Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


typedef HANDLE(WINAPI* PFCreateToolhelp32Snapshot)(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

typedef BOOL(WINAPI* PFProcess32First)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

typedef BOOL(WINAPI* PFProcess32Next)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

// Native NT API Definitions
typedef NTSTATUS(WINAPI* PFZwQuerySystemInformation)
(UINT, PVOID, ULONG, PULONG);
typedef HANDLE(WINAPI* PFGetProcessHeap)(VOID);
typedef LPVOID(WINAPI* PFHeapAlloc)
(HANDLE, DWORD, SIZE_T);
typedef BOOL(WINAPI* PFHeapFree)(HANDLE, DWORD, LPVOID);

struct KillProcess
{
    HMODULE            hNTLib;
    HMODULE            hKernelLib;
    // ToolHelp related functions
    PFCreateToolhelp32Snapshot    FCreateToolhelp32Snapshot;
    PFProcess32First            FProcess32First;
    PFProcess32Next                FProcess32Next;
    // native NT api functions
    PFZwQuerySystemInformation    FQuerySysInfo;
    PFGetProcessHeap            FGetProcessHeap;
    PFHeapAlloc                    FHeapAlloc;
    PFHeapFree                    FHeapFree;
};

void KillProcess_Init(struct KillProcess* pThis);
void KillProcess_Destroy(struct KillProcess* pThis);

HANDLE KillProcess_FindProcess(struct KillProcess* pThis,
                               const WCHAR* pstrProcessName,
                               DWORD* dwId);
HANDLE KillProcess_FindProcess(struct KillProcess* pThis,
                               const WCHAR* pstrProcessName,
                               DWORD* dwId);

HANDLE KillProcess_NTFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName,
                                 DWORD* dwId);

HANDLE KillProcess_THFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName, DWORD* dwId);

BOOL KillProcess_KillProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName);

void SystemCreateProcess(const WCHAR* moduleName, const WCHAR* cmdline);

#include <assert.h>
#include <Windows.h>

static HKEY OpenRegKey(HKEY hKeyParent,
                 LPCTSTR lpszKeyName,
                 REGSAM samDesired/* = KEY_READ | KEY_WRITE*/)
{
    assert(hKeyParent != NULL);
    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyExW(hKeyParent, lpszKeyName, 0, samDesired, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        assert(lRes == ERROR_SUCCESS);
    }

    return hKey;
}

static LONG RegKey_QueryStringValue(HKEY hKey,
                             LPCTSTR pszValueName,
                             LPTSTR pszValue,
                             ULONG* pnChars)
{
    LONG lRes;
    DWORD dwType;
    ULONG nBytes;

    assert(hKey != NULL);
    assert(pnChars != NULL);

    nBytes = (*pnChars) * sizeof(TCHAR);
    *pnChars = 0;
    lRes = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)(pszValue),
                           &nBytes);

    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        return ERROR_INVALID_DATA;
    }

    if (pszValue != NULL)
    {
        if (nBytes != 0)
        {

            if ((nBytes % sizeof(TCHAR) != 0) || (pszValue[nBytes / sizeof(TCHAR) - 1] != 0))
            {
                return ERROR_INVALID_DATA;
            }

        }
        else
        {
            pszValue[0] = L'\0';
        }
    }

    *pnChars = nBytes / sizeof(TCHAR);

    return ERROR_SUCCESS;
}


static LONG RegKey_QueryDWORDValue(HKEY hKey, LPCTSTR pszValueName, DWORD* dwValue)
{
    LONG lRes;
    ULONG nBytes;
    DWORD dwType;

    assert(hKey != NULL);

    nBytes = sizeof(DWORD);
    lRes = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)(&dwValue),
                           &nBytes);

    if (lRes != ERROR_SUCCESS)
        return lRes;

    if (dwType != REG_DWORD)
        return ERROR_INVALID_DATA;

    return ERROR_SUCCESS;
}

static LONG RegKey_SetStringValue(
    HKEY hKey,
    LPCTSTR pszValueName,
    LPCTSTR pszValue,
    DWORD dwType)
{
    assert(hKey != NULL);
    assert((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ));

    if (pszValue == NULL)
    {
        return ERROR_INVALID_DATA;
    }

    return RegSetValueEx(hKey,
                         pszValueName,
                         0,
                         dwType,
                         (const BYTE*)(pszValue),
                         ((DWORD)(wcslen(pszValue)) + 1) * sizeof(TCHAR));
}

static BOOL DeleteRegValue(HKEY hKeyParent,
                    LPCTSTR pszSubkey,
                    LPCTSTR pszValueName)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);

    if (hKey)
    {
        LSTATUS e = RegDeleteKeyValue(hKey, pszSubkey, pszValueName);
        RegCloseKey(hKey);
    }
    return bResult;
}


static BOOL DeleteRegKey(HKEY hKeyParent,
                  LPCTSTR pszSubkey)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);

    if (hKey)
    {
        LSTATUS e = RegDeleteKey(hKey, pszSubkey);
        RegCloseKey(hKey);
    }
    return bResult;
}


static BOOL WriteRegStr(HKEY hKeyParent,
                 LPCTSTR lpszKeyName,
                 LPCTSTR pszKeyName,
                 LPCTSTR pszValue)
{
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    //Computador\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    //                              Software\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    BOOL bResult = FALSE;
    HKEY hKey = 0;
    LONG lRes = RegCreateKeyExW(hKeyParent, lpszKeyName, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);

    if (lRes == ERROR_SUCCESS)
    {
        RegKey_SetStringValue(hKey, pszKeyName, pszValue, REG_SZ);
        RegCloseKey(hKey);
    }
    return bResult;
}

static BOOL ReadRegStr(HKEY hKeyParent,
                LPCTSTR pszSubkey,
                LPCTSTR pszKeyName,
                LPTSTR pszValue,
                ULONG* pnChars)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);


    if (hKey)
    {
        RegKey_QueryStringValue(hKey,
                                pszKeyName,
                                pszValue,
                                pnChars);
        RegCloseKey(hKey);
    }
    return bResult;
}

//ReadRegStr $R1 ${ PRODUCT_UNINST_ROOT_KEY } "Software\Microsoft\Windows\CurrentVersion\Uninstall\MobileServer" "UninstallString"

static void AddSystemVariablesPath(const wchar_t* pathToAdd)
{
    //https://nsis.sourceforge.io/Docs/Chapter4.html
    //root_key subkey key_name value
    //WriteRegStr HKLM "Software\My Company\My Software" "String Value" "dead beef"
    //

    //ver classe class CRegKey da ATL
    //exemplo de como adicionar uma sistem variable no path
    //HKEY_CURRENT_USER\Software\Elipse Software\E3\Studio\RecentDomains
#define PATH_REGISTRY_RECENT L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"


    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, PATH_REGISTRY_RECENT, KEY_READ | KEY_WRITE);


    if (hKey)
    {

        wchar_t buffer[2000];
        ULONG chars = 2000;
        RegKey_QueryStringValue(hKey,
                                L"Path",
                                buffer,
                                &chars);
        wcscat(buffer, L";");
        wcscat(buffer, pathToAdd);

        RegKey_SetStringValue(hKey, L"Path", buffer, REG_SZ);

        RegCloseKey(hKey);
    }
}


//*************************************************************
//  RegDelnodeRecurse()
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************

static BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    WCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having to recurse
    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
    {
        //wprintf(L"LRegDeleteKey() - Key and subkey successfully deleted!\n");

        return TRUE;

    }

    else

    {

        //wprintf(L"LRegDeleteKey() - Failed to delete key and subkey!Error % d.\n, GetLastError()");

        //wprintf(L"LTrying again..\n");

    }



    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);



    if (lResult != ERROR_SUCCESS)

    {

        if (lResult == ERROR_FILE_NOT_FOUND)

        {

            //wprintf(L"RegOpenKeyEx() - Key not found!\n");

            return TRUE;

        }

        else

        {

            //wprintf(L"LRegOpenKeyEx() - Error opening key, error % d\n, GetLastError()");

            return FALSE;

        }

    }

    else

    {
        //wprintf(L"RegOpenKeyEx() - Key opened successfully!\n");
    }


    // Check for an ending slash and add one if it is missing

    lpEnd = lpSubKey + lstrlen(lpSubKey);



    if (*(lpEnd - 1) != L'\\')

    {

        *lpEnd = L'\\';

        lpEnd++;

        *lpEnd = L'\0';

    }



    // Enumerate the keys

    dwSize = MAX_PATH;

    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);



    if (lResult == ERROR_SUCCESS)

    {

        //wprintf(L"RegEnumKeyEx() is pretty fine!\n");

        do {

            wcscpy_s(lpEnd, MAX_PATH * 2, szName);



            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey))

            {

                break;

            }



            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);



        }
        while (lResult == ERROR_SUCCESS);

    }

    else
    {
        //wprintf(LRegEnumKeyEx() failed lol!\n);
    }



    lpEnd--;

    *lpEnd = L'\0';



    if (RegCloseKey(hKey) == ERROR_SUCCESS)
    {
        //wprintf(L"hKey key was closed successfully!\n");
    }
    else
    {
        //wprintf(L"Failed to close hKey key!\n");
    }

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
    {
        //  wprintf(L"RegDeleteKey() is OK!\n");
        return TRUE;
    }

    else
    {
        //   wprintf(L"RegDeleteKey() failed!\n");
    }


    return FALSE;

}



//*************************************************************

//  RegDelnode()
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
static BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    WCHAR szDelKey[MAX_PATH * 2];
    wcscpy(szDelKey, lpSubKey);
    // Recurse starts from root key, HKEY_CLASSES_ROOT
    return RegDelnodeRecurse(hKeyRoot, szDelKey);
}




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
    while ((bFirst ? pThis->FProcess32First(hSnapShot, &processInfo) : pThis->FProcess32Next(hSnapShot, &processInfo)))
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

static HANDLE KillProcess_NTFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName,
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


static void SystemCreateProcess(const WCHAR* moduleName, const WCHAR* cmdline)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    // Start the child process. 
    if (!CreateProcessW(moduleName,   // No module name (use command line)
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


#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE

static DWORD GetModuleDir(HMODULE hModule, LPTSTR   lpFilename, DWORD   nSize)
{
    if (GetModuleFileNameW(NULL, lpFilename, nSize) > 0)
    {
        //return 0;
    }

    //wchar_t path_buffer[_MAX_PATH];
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    _wsplitpath(lpFilename, drive, dir, fname, ext);

    lpFilename[0] = 0;
    wcscat(lpFilename, drive);
    wcscat(lpFilename, dir);

    return 1;
}

static void Finish()
{
    //A ultima fase do instalador eh copiar-se 
    //para uma pasta temp e depois 
    //apagar-se do lugar original
    //junto com a pasta

    //Esta funcao chama o desintalador da temp passando -d

    WCHAR value[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;
    if (GetModuleFileNameW(NULL, value, MAX_PATH) > 0)
    {
        //return 0;
    }

    WCHAR tempPath[MAX_PATH] = { 0 };
    DWORD dw = GetTempPathW(MAX_PATH, tempPath);    
    wcscat(tempPath, L"uninstall.exe");
    CopyFile(value, tempPath, FALSE);

    WCHAR commandLine[MAX_PATH] = { 0 };
    wcscat(commandLine, L"-d ");
    if (GetModuleFileNameW(NULL, &commandLine[3], MAX_PATH - 4) > 0)
    {
    }

    SystemCreateProcess(tempPath, commandLine);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    

    char* cmd = GetCommandLineA();
    
    //Ao passar -d significa que é a última fase e ele se auto deleta.
    BOOL bRemoveUninstall = (cmd[0] == '-' && cmd[1] == 'd');
    if (bRemoveUninstall)
    {
        for (int i = 0; i < 100; i++)
        {
            if (remove(&cmd[3]) == 0)
            {
                char* dir = &cmd[3];
                int dirlen = strlen(dir);
                dir[dirlen - sizeof("uninstall.exe")] = 0;                
                _rmdir(dir);
                break;
            }
            else
            {
                //aguarda um pouco
                Sleep(500);
            }
        }
        return 0;
    }



    WCHAR exePath[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;

    if (GetModuleDir(NULL, exePath, MAX_PATH) > 0)
    {
        //return 0;
    }

    //Poe o current dir como sendo o path do exe
    //
    if (_wchdir(exePath) == -1)
    {
        //nao eh para acontecer isso nunca
        MessageBoxA(NULL, "Error setting current dir", "Uninstall", MB_ICONERROR | MB_OK);
        return 1;
    }



    //Vamos remover todas as chaves do registro
    if (!RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY))
    {
        MessageBoxA(NULL, "Error removing registry keys", "Uninstall", MB_ICONERROR | MB_OK);
    }

    //Depois remover todos os arquivos salvos
    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };


    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        if (remove(files[i].dest) != 0)
        {
            int e = errno;
            char errorMessage[MAX_PATH + 200];
            snprintf(errorMessage, sizeof(errorMessage), "Error deleting file %s. (%d)", files[i].dest, e);            
            MessageBoxA(NULL, errorMessage, "Uninstall",  MB_ICONERROR | MB_OK);            
        }
    }

    //Remover pastas
    _rmdir("default/.vscode");
    _rmdir("default");


    MessageBox(NULL, DISPLAY_NAME L" was successfully removed from  your computer.", DISPLAY_NAME L" Unistall", MB_ICONINFORMATION |MB_OK);

    //auto deletar-se
    Finish();

    return 0;
}

